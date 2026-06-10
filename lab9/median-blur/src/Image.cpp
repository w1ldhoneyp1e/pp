#include "Image.h"

#include <jpeglib.h>
#include <png.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <csetjmp>
#include <cmath>
#include <fstream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>

namespace {
struct FileCloser
{
    void operator()(FILE *file) const
    {
        if (file) {
            std::fclose(file);
        }
    }
};

using FilePtr = std::unique_ptr<FILE, FileCloser>;

struct JpegErrorManager
{
    jpeg_error_mgr Base;
    jmp_buf Jump;
    char Message[JMSG_LENGTH_MAX] = {};
};

void OnJpegError(j_common_ptr cinfo)
{
    auto *manager = reinterpret_cast<JpegErrorManager *>(cinfo->err);
    (*cinfo->err->format_message)(cinfo, manager->Message);
    longjmp(manager->Jump, 1);
}

bool HasSignature(const std::string &path, const unsigned char *signature, std::size_t size)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open image: " + path);
    }

    unsigned char buffer[16] = {};
    input.read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(size));

    return input.gcount() == static_cast<std::streamsize>(size)
        && std::memcmp(buffer, signature, size) == 0;
}

Image LoadPngImage(const std::string &path)
{
    FilePtr file(std::fopen(path.c_str(), "rb"));
    if (!file) {
        throw std::runtime_error("Cannot open image: " + path);
    }

    unsigned char header[8] = {};
    if (std::fread(header, 1, sizeof(header), file.get()) != sizeof(header)
        || png_sig_cmp(header, 0, sizeof(header)) != 0) {
        throw std::runtime_error("Invalid PNG image: " + path);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        throw std::runtime_error("Cannot initialize PNG reader");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        throw std::runtime_error("Cannot initialize PNG info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        throw std::runtime_error("Cannot decode PNG image: " + path);
    }

    png_init_io(png, file.get());
    png_set_sig_bytes(png, sizeof(header));
    png_read_info(png, info);

    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bitDepth = 0;
    int colorType = 0;
    png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, nullptr, nullptr, nullptr);

    if (bitDepth == 16) {
        png_set_strip_16(png);
    }
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }
    if ((colorType & PNG_COLOR_MASK_ALPHA) == 0) {
        png_set_add_alpha(png, 255, PNG_FILLER_AFTER);
    }

    png_read_update_info(png, info);

    Image image;
    image.Width = static_cast<int>(width);
    image.Height = static_cast<int>(height);
    image.Pixels.resize(static_cast<std::size_t>(image.Width) * image.Height * 4);

    std::vector<png_bytep> rows(static_cast<std::size_t>(image.Height));
    const std::size_t stride = static_cast<std::size_t>(image.Width) * 4;
    for (int y = 0; y < image.Height; ++y) {
        rows[static_cast<std::size_t>(y)] = image.Pixels.data() + static_cast<std::size_t>(y) * stride;
    }

    png_read_image(png, rows.data());
    png_read_end(png, nullptr);
    png_destroy_read_struct(&png, &info, nullptr);

    return image;
}

Image LoadJpegImage(const std::string &path)
{
    FilePtr file(std::fopen(path.c_str(), "rb"));
    if (!file) {
        throw std::runtime_error("Cannot open image: " + path);
    }

    jpeg_decompress_struct cinfo = {};
    JpegErrorManager errorManager = {};
    cinfo.err = jpeg_std_error(&errorManager.Base);
    errorManager.Base.error_exit = OnJpegError;

    if (setjmp(errorManager.Jump)) {
        jpeg_destroy_decompress(&cinfo);
        throw std::runtime_error(std::string("Cannot decode JPEG image: ")
                                 + path
                                 + (errorManager.Message[0] ? ": " : "")
                                 + errorManager.Message);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file.get());
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    Image image;
    image.Width = static_cast<int>(cinfo.output_width);
    image.Height = static_cast<int>(cinfo.output_height);
    image.Pixels.resize(static_cast<std::size_t>(image.Width) * image.Height * 4);

    std::vector<unsigned char> row(static_cast<std::size_t>(image.Width) * 3);
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *rowPointer = row.data();
        const std::size_t y = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &rowPointer, 1);

        for (int x = 0; x < image.Width; ++x) {
            const std::size_t source = static_cast<std::size_t>(x) * 3;
            const std::size_t target = (y * image.Width + static_cast<std::size_t>(x)) * 4;
            image.Pixels[target + 0] = row[source + 0];
            image.Pixels[target + 1] = row[source + 1];
            image.Pixels[target + 2] = row[source + 2];
            image.Pixels[target + 3] = 255;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return image;
}
}

Image LoadImage(const std::string &path)
{
    static constexpr unsigned char PngSignature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    static constexpr unsigned char JpegSignature[] = {0xff, 0xd8, 0xff};

    if (HasSignature(path, PngSignature, sizeof(PngSignature))) {
        return LoadPngImage(path);
    }
    if (HasSignature(path, JpegSignature, sizeof(JpegSignature))) {
        return LoadJpegImage(path);
    }

    throw std::runtime_error("Unsupported image format. Use PNG or JPEG.");
}
