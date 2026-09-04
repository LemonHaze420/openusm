#pragma once

#include "vector3d.h"

struct sphere {
    vector3d center;
    float radius;

    sphere() = default;

    sphere(const vector3d &a2, float a3);
};
