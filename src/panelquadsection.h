#pragma once

#include "float.hpp"
#include "ngl.h"

#include <cstdint>

struct from_mash_in_place_constructor;
struct vector2d;
struct color32;

struct PanelQuadSection {
    uint16_t field_0[4];
    uint16_t field_8[4];
    char field_10[4];

    mNglQuad field_14;
    bool field_78;

    PanelQuadSection(from_mash_in_place_constructor *a2);

    void Init(vector2d *pos, vector2d *uv, color32 *a4, Float a5);

    //0x00608CB0
    color32 GetColor(int a3) const;
};

extern void PanelQuadSection_patch();
