#pragma once

#include "ngl.h"

#include "vector3d.h"

struct phys_vector3d;

struct rigid_body {
    matrix4x4 field_0;
    matrix4x4 field_40;
    int field_80;
    int field_84;
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    int field_A8;
    int field_AC;
    int field_B0;
    int field_B4;
    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    int field_CC;
    vector4d field_D0;
    vector4d field_E0;
    float field_F0[4];
    float field_100[4];
    float field_110[4];
    int field_120;
    int field_124;
    int field_128;
    int field_12C;
    float field_130;
    int field_134;
    int field_138;
    int field_13C;
    int field_140;
    uint32_t field_144;
    int field_148;
    int field_14C;
    int field_150;
    int field_154;
    int field_158;
    int field_15C;
    int field_160;
    int field_164;
    float field_168;

    rigid_body();

    void sub_5B2D50(const phys_vector3d &a2);

    vector3d sub_503B80();

    void sub_502600(const vector3d &a2);

    void sub_502640(const vector3d &a2);
};

struct user_rigid_body : rigid_body {
    int field_16C;
    int field_170;
    int field_174;
    int field_178;
    int field_17C;
    int field_180[11];
    const math::MatClass<4, 3> *m_dictator;
    int field_1B0;

    void set(const math::MatClass<4, 3> *dictator);
};

extern void rigid_body_patch();
