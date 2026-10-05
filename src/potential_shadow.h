#pragma once

#include "variable.h"
#include "vector3d.h"

struct conglomerate;

struct potential_shadow {
    vector3d field_0;
    vector3d field_C;
    conglomerate *field_18;
    float field_1C;
    float m_radius;
    float m_fade;
    float field_28;

    potential_shadow();

    void sub_5932C0();

    void sub_593280();


    void commit();
};

extern Var<potential_shadow[2]> shadow_candidates;
