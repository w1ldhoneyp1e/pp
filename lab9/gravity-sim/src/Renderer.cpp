#include "Renderer.h"

#include <SDL_opengl.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr float Pi = 3.1415926535f;
}

void Renderer::Render(const std::vector<Body> &bodies, const Camera &camera, int width, int height)
{
    glViewport(0, 0, width, height);
    SetPerspective(55.0f,
                   static_cast<float>(width) / static_cast<float>(std::max(height, 1)),
                   0.1f,
                   4000.0f);
    camera.Apply();

    glClearColor(0.015f, 0.017f, 0.024f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPointSize(2.0f);

    glBegin(GL_POINTS);
    for (const Body &body : bodies) {
        const float massGlow = std::min(1.0f, body.mass / 2.2f);
        const float red = 0.50f + 0.50f * massGlow;
        const float green = 1.00f - 0.35f * massGlow;
        const float blue = 0.55f - 0.20f * massGlow;
        glColor4f(red, green, blue, 0.72f);
        glVertex3f(body.x, body.y, body.z);
    }
    glEnd();

    glDisable(GL_BLEND);
}

void Renderer::SetPerspective(float fovYDegrees, float aspect, float zNear, float zFar)
{
    const float f = 1.0f / std::tan(fovYDegrees * 0.5f * Pi / 180.0f);
    const std::array<float, 16> matrix = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
        0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f,
    };

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrix.data());
}
