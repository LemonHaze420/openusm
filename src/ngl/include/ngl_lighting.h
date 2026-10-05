#pragma once

#include "ngl_math.h"
#include "variable.h"

inline constexpr auto NGL_MAX_LIGHTS = 8;

inline constexpr auto NGL_LIGHTCAT_SHIFT = 24u;

enum nglLightType {
    NGL_LIGHT_POINT = 0,
    NGL_LIGHT_DIRECTIONAL = 1,
    NGL_LIGHT_DIR_PROJECTOR = 2,
};

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
    nglLightNode *SelectedNext;
    uint32_t LightCat;
    nglLightType Type;
    void *Data;
};

struct nglLightContext {
    nglLightNode Head;
    nglLightNode ProjectorHead;
    vector4d Ambient;
};

struct nglTexture;
struct nglMeshNode;



struct nglDirProjectorLightInfo {
    matrix4x4 WorldToUV;
    matrix4x4 UVToWorld;
    int BlendMode;
    uint32_t Color;
    nglTexture *Texture;
    vector4d Extent;
    vector4d Position;
    vector4d XAxis;
    vector4d YAxis;
    vector4d ZAxis;
    vector4d Planes[6];
};

struct nglLightContextParam {
    nglLightContext *field_0;
    static inline Var<int> ID{0x00971EE4};
};

void nglListAddDirProjectorLight(uint32_t lightCat, const matrix4x4 &localToWorld,
                               float width, float height, float depth, float unusedW,
                               int blendMode, uint32_t color, nglTexture *texture);
void nglDetermineProjLights(nglMeshNode *node);
bool nglProjectorSphereVisible(const nglDirProjectorLightInfo &light,
                               const vector4d &center, float radius);

extern Var<nglLightContext *> nglDefaultLightContext;

extern Var<nglLightContext *> nglCurLightContext;

extern void nglListAddDirLight(uint32_t a2, math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> a3,
                               math::VecClass<4, -1, void, void, math::Rep_Std<false>> a4);

extern void ngl_lighting_patch();
