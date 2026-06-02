#include "Camera.h"

#include <SDL_opengl.h>

#include <algorithm>

namespace {
constexpr float Pi = 3.1415926535f;
}

void Camera::Rotate(float dx, float dy)
{
    m_yaw += dx * 0.006f;
    m_pitch += dy * 0.006f;
    m_pitch = std::clamp(m_pitch, -1.45f, 1.45f);
}

void Camera::Zoom(float wheelY)
{
    m_distance *= wheelY > 0.0f ? 0.88f : 1.14f;
    m_distance = std::clamp(m_distance, 30.0f, 1200.0f);
}

void Camera::Apply() const
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -m_distance);
    glRotatef(m_pitch * 180.0f / Pi, 1.0f, 0.0f, 0.0f);
    glRotatef(m_yaw * 180.0f / Pi, 0.0f, 1.0f, 0.0f);
}
