#pragma once

#include "vector2d.h"
#include "vector3d.h"

#include <cassert>

extern float wrap_angle(float ang);

template <typename T>
T lerp(T a, T b, float t)
{
    assert(t >= 0.0f && t <= 1.0f);

    return a + (b - a) * t;
}
