#pragma once

#include "float.hpp"
#include "matrix4x4.h"

struct nglTexture;

struct shadow_t {
    matrix4x4 field_0;
    float field_40;
    nglTexture *field_44;
    bool field_48;
};

inline Var<shadow_t[2]> g_shadow{0x00966EB0};

//0x0059B220
extern void send_shadow_projectors();

struct conglomerate;
struct vector3d;

//0x0059B090
extern bool render_projected_shadow(conglomerate &a1, Float a2, const vector3d &a3, const vector3d &a4, Float radius,
                                    Float fade);
