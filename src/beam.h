#pragma once

#include "entity.h"

#include "float.hpp"
#include "color32.h"
#include "vector2d.h"

struct vector3d;
struct PCUV_ShaderMaterial;

struct beam : entity {
    int *field_68;
    int *field_6C;
    float field_70;
    float field_74;
    float field_78;
    color32 field_7C;
    color32 field_80;
    int field_84;
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    vector3d field_98;
    float field_A4;
    float field_A8;
    vector3d field_AC;
    vector3d field_B8;
    float field_C4;
    float field_C8;
    vector2d field_CC[2];
    int field_DC;
    int16_t field_E0;
    bool field_E2;
    bool field_E3;
    PCUV_ShaderMaterial *my_material;

    beam();

    //0x005406D0
    void _render(Float a1);

    //0x0051CED0
    static void frame_advance_all_beams(Float a3);
};

extern void beam_patch();
