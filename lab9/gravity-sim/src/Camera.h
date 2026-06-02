#pragma once

class Camera {
public:
    void Rotate(float dx, float dy);
    void Zoom(float wheelY);
    void Apply() const;

private:
    float m_yaw = 0.45f;
    float m_pitch = 0.35f;
    float m_distance = 260.0f;
};
