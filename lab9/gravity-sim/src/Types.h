#pragma once

struct Body {
    float x;
    float y;
    float z;
    float mass;
};

struct Velocity {
    float x;
    float y;
    float z;
    float pad;
};

struct SimulationParameters {
    float timeScale = 1.0f;
    float gravity = 14.0f;
    bool paused = false;
};

// контракст для openGL в float4
static_assert(sizeof(Body) == 16);
static_assert(sizeof(Velocity) == 16);
