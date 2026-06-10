#include "Renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr const char *FontPath = "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf";

float FitScale(int imageWidth, int imageHeight, float boxWidth, float boxHeight)
{
    return std::min(boxWidth / static_cast<float>(std::max(imageWidth, 1)),
                    boxHeight / static_cast<float>(std::max(imageHeight, 1)));
}

std::uint8_t ColorByte(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}
}

struct Renderer::FontState
{
    FT_Library Library = nullptr;
    FT_Face Face = nullptr;

    FontState()
    {
        if (FT_Init_FreeType(&Library) != 0) {
            throw std::runtime_error("Cannot initialize FreeType");
        }
        if (FT_New_Face(Library, FontPath, 0, &Face) != 0) {
            throw std::runtime_error(std::string("Cannot load font: ") + FontPath);
        }
    }

    ~FontState()
    {
        if (Face) {
            FT_Done_Face(Face);
        }
        if (Library) {
            FT_Done_FreeType(Library);
        }
    }
};

Renderer::Renderer()
    : m_font(std::make_unique<FontState>())
{
    glGenTextures(1, &m_sourceTexture);
    glGenTextures(1, &m_filteredTexture);
}

Renderer::~Renderer()
{
    if (m_sourceTexture != 0) {
        glDeleteTextures(1, &m_sourceTexture);
    }
    if (m_filteredTexture != 0) {
        glDeleteTextures(1, &m_filteredTexture);
    }
}

void Renderer::SetSourceImage(const Image &image)
{
    m_imageWidth = image.Width;
    m_imageHeight = image.Height;
    UploadTexture(m_sourceTexture, image);
}

void Renderer::SetFilteredImage(const Image &image)
{
    UploadTexture(m_filteredTexture, image);
}

void Renderer::Render(int windowWidth, int windowHeight, int radius, int minRadius, int maxRadius)
{
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const float margin = 24.0f;
    const float gap = 18.0f;
    const float canvasHeight = static_cast<float>(std::max(windowHeight - ControlsHeight, 1)) - margin * 2.0f;
    const float boxWidth = (static_cast<float>(windowWidth) - margin * 2.0f - gap) * 0.5f;
    const float scale = FitScale(m_imageWidth, m_imageHeight, boxWidth, canvasHeight);
    const float drawWidth = std::floor(m_imageWidth * scale);
    const float drawHeight = std::floor(m_imageHeight * scale);
    const float top = margin + std::max(0.0f, (canvasHeight - drawHeight) * 0.5f);
    const float leftA = margin + std::max(0.0f, (boxWidth - drawWidth) * 0.5f);
    const float leftB = margin + boxWidth + gap + std::max(0.0f, (boxWidth - drawWidth) * 0.5f);

    DrawImage(m_sourceTexture, leftA, top, drawWidth, drawHeight);
    DrawImage(m_filteredTexture, leftB, top, drawWidth, drawHeight);
    DrawControls(windowWidth, windowHeight, radius, minRadius, maxRadius);
}

Renderer::ControlAction Renderer::HitTestControls(int x, int y, int windowHeight) const
{
    if (Contains(MinusButtonRect(windowHeight), x, y)) {
        return ControlAction::DecreaseRadius;
    }
    if (Contains(PlusButtonRect(windowHeight), x, y)) {
        return ControlAction::IncreaseRadius;
    }

    return ControlAction::None;
}

void Renderer::UploadTexture(GLuint texture, const Image &image)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 image.Width,
                 image.Height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image.Pixels.data());
}

void Renderer::DrawImage(GLuint texture, float left, float top, float width, float height)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(left, top);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(left + width, top);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(left + width, top + height);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(left, top + height);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void Renderer::DrawControls(int windowWidth, int windowHeight, int radius, int minRadius, int maxRadius)
{
    const Rect minus = MinusButtonRect(windowHeight);
    const Rect plus = PlusButtonRect(windowHeight);
    const int matrixSize = radius * 2 + 1;
    std::ostringstream info;
    info << "Radius: " << radius << "    Matrix: " << matrixSize << "x" << matrixSize;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.12f, 0.13f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, static_cast<float>(windowHeight - ControlsHeight));
    glVertex2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight - ControlsHeight));
    glVertex2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glVertex2f(0.0f, static_cast<float>(windowHeight));
    glEnd();

    DrawButton(minus, '-', radius > minRadius);
    DrawButton(plus, '+', radius < maxRadius);
    DrawText(plus.Left + plus.Width + 32.0f, plus.Top + 8.0f, info.str(), 24, 0.94f, 0.96f, 0.98f);
}

void Renderer::DrawButton(const Rect &rect, char label, bool enabled)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(enabled ? 0.18f : 0.22f, enabled ? 0.55f : 0.24f, enabled ? 0.82f : 0.26f);
    glBegin(GL_QUADS);
    glVertex2f(rect.Left, rect.Top);
    glVertex2f(rect.Left + rect.Width, rect.Top);
    glVertex2f(rect.Left + rect.Width, rect.Top + rect.Height);
    glVertex2f(rect.Left, rect.Top + rect.Height);
    glEnd();

    DrawText(rect.Left + 17.0f,
             rect.Top + 5.0f,
             std::string(1, label),
             32,
             enabled ? 0.96f : 0.62f,
             enabled ? 0.98f : 0.66f,
             enabled ? 1.00f : 0.70f);
}

void Renderer::DrawText(float left,
                        float top,
                        const std::string &text,
                        unsigned int pixelSize,
                        float red,
                        float green,
                        float blue)
{
    if (FT_Set_Pixel_Sizes(m_font->Face, 0, pixelSize) != 0) {
        return;
    }

    const float baseline = top + static_cast<float>(pixelSize);
    for (char ch : text) {
        DrawCharacter(left, baseline, ch, red, green, blue);

        if (FT_Load_Char(m_font->Face, static_cast<unsigned char>(ch), FT_LOAD_DEFAULT) == 0) {
            left += static_cast<float>(m_font->Face->glyph->advance.x >> 6);
        } else {
            left += static_cast<float>(pixelSize) * 0.5f;
        }
    }
}

void Renderer::DrawCharacter(float left,
                             float baseline,
                             char ch,
                             float red,
                             float green,
                             float blue)
{
    if (FT_Load_Char(m_font->Face, static_cast<unsigned char>(ch), FT_LOAD_RENDER) != 0) {
        return;
    }

    const FT_GlyphSlot glyph = m_font->Face->glyph;
    const int width = static_cast<int>(glyph->bitmap.width);
    const int height = static_cast<int>(glyph->bitmap.rows);
    if (width <= 0 || height <= 0) {
        return;
    }

    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
    const std::uint8_t r = ColorByte(red);
    const std::uint8_t g = ColorByte(green);
    const std::uint8_t b = ColorByte(blue);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t source = static_cast<std::size_t>(y) * glyph->bitmap.pitch + x;
            const std::size_t target = (static_cast<std::size_t>(y) * width + x) * 4;
            pixels[target + 0] = r;
            pixels[target + 1] = g;
            pixels[target + 2] = b;
            pixels[target + 3] = glyph->bitmap.buffer[source];
        }
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels.data());

    const float x0 = left + static_cast<float>(glyph->bitmap_left);
    const float y0 = baseline - static_cast<float>(glyph->bitmap_top);
    const float x1 = x0 + static_cast<float>(width);
    const float y1 = y0 + static_cast<float>(height);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x0, y0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x1, y0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x1, y1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x0, y1);
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glDeleteTextures(1, &texture);
}

Renderer::Rect Renderer::MinusButtonRect(int windowHeight) const
{
    return Rect{80.0f, static_cast<float>(windowHeight - ControlsHeight + 16), 52.0f, 40.0f};
}

Renderer::Rect Renderer::PlusButtonRect(int windowHeight) const
{
    return Rect{148.0f, static_cast<float>(windowHeight - ControlsHeight + 16), 52.0f, 40.0f};
}

bool Renderer::Contains(const Rect &rect, int x, int y) const
{
    return static_cast<float>(x) >= rect.Left
        && static_cast<float>(x) <= rect.Left + rect.Width
        && static_cast<float>(y) >= rect.Top
        && static_cast<float>(y) <= rect.Top + rect.Height;
}
