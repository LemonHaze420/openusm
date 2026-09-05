#pragma once

#include "float.hpp"
#include "ngl.h"

#include <cstdint>

struct from_mash_in_place_constructor;
struct vector2d;
struct color32;
struct matrix4x4;

struct PanelQuadSection {
    int16_t field_0[4];
    int16_t field_8[4];
    char field_10[4];

    mNglQuad field_14;
    bool field_78;

    PanelQuadSection();
    PanelQuadSection(from_mash_in_place_constructor *a2);

    void Init(vector2d *pos, vector2d *uv, color32 *a4, Float a5);

    //0x00608CB0
    color32 GetColor(int a3) const;

    void sub_608EF0(float *a2, float *a3);
    void Animate(const matrix4x4 &transform, float z, bool relative);
};

extern void PanelQuadSection_patch();
