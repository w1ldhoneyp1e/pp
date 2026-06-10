#pragma once

#include "Image.h"

#include <SDL_opengl.h>

#include <memory>
#include <string>

class Renderer
{
public:
    enum class ControlAction
    {
        None,
        DecreaseRadius,
        IncreaseRadius,
    };

    Renderer();
    ~Renderer();

    void SetSourceImage(const Image &image);
    void SetFilteredImage(const Image &image);
    void Render(int windowWidth, int windowHeight, int radius, int minRadius, int maxRadius);
    ControlAction HitTestControls(int x, int y, int windowHeight) const;

private:
    static constexpr int ControlsHeight = 72;

    struct FontState;

    struct Rect
    {
        float Left = 0.0f;
        float Top = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
    };

    void UploadTexture(GLuint texture, const Image &image);
    void DrawImage(GLuint texture, float left, float top, float width, float height);
    void DrawControls(int windowWidth, int windowHeight, int radius, int minRadius, int maxRadius);
    void DrawButton(const Rect &rect, char label, bool enabled);
    void DrawText(float left,
                  float top,
                  const std::string &text,
                  unsigned int pixelSize,
                  float red,
                  float green,
                  float blue);
    void DrawCharacter(float left,
                       float baseline,
                       char ch,
                       float red,
                       float green,
                       float blue);
    Rect MinusButtonRect(int windowHeight) const;
    Rect PlusButtonRect(int windowHeight) const;
    bool Contains(const Rect &rect, int x, int y) const;

    std::unique_ptr<FontState> m_font;
    GLuint m_sourceTexture = 0;
    GLuint m_filteredTexture = 0;
    int m_imageWidth = 1;
    int m_imageHeight = 1;
};
