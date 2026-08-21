#pragma once

#include "ngl_math.h"
#include "variable.h"

inline constexpr auto NGL_MAX_LIGHTS = 8;

inline constexpr auto NGL_LIGHTCAT_SHIFT = 24u;

enum nglLightType {};

struct nglDirLightInfo {
    math::VecClass<3, 0> Dir;
    math::VecClass<3, 0> Color;
};

struct nglPointLightInfo {
    math::VecClass<3, 1> ViewPos;
    math::VecClass<3, 1> Pos;
    float Near;
    float Far;
};

struct nglLightNode {
    nglLightNode *Next[NGL_MAX_LIGHTS]{};
    int field_20;
    uint32_t LightCat;
    nglLightType Type;
    void *Data;
};

struct nglLightContext {
    nglLightNode Head;
    nglLightNode ProjectorHead;
    vector4d Ambient;
};


extern Var<nglLightContext *> nglDefaultLightContext;

extern Var<nglLightContext *> nglCurLightContext;

extern void nglListAddDirLight(uint32_t a2, math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> a3,
                               math::VecClass<4, -1, void, void, math::Rep_Std<false>> a4);

extern void ngl_lighting_patch();
