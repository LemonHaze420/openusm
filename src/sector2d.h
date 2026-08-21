#pragma once

#include "float.hpp"

struct vector3d;

struct sector2d {
    float field_0;
    float field_4;
    float field_8;
    float field_C;
    float field_10;
    float field_14;
    float field_18;
    float field_1C;
    float field_20;
    float field_24;
    float field_28;
    float field_2C;
    float field_30;
    float field_34;
    float fov;

    sector2d(const vector3d &a2, const vector3d &a3, Float a4);

    auto get_fov()
    {
        return this->fov;
    }
};
