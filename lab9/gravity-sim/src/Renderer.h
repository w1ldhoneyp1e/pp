#pragma once

#include "Camera.h"
#include "Types.h"

#include <vector>

class Renderer {
public:
    void Render(const std::vector<Body> &bodies, const Camera &camera, int width, int height);

private:
    void SetPerspective(float fovYDegrees, float aspect, float zNear, float zFar);
};
