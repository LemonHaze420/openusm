#pragma once

#include "nglshader.h"

struct USShaderBase : nglShader {
    //virtual
    bool _IsSwitchable() const;
};

inline Var<float[4][16]> nglLightColors{0x00956448};
inline Var<float[4][16]> nglAltLightColors{0x009565A8};
inline Var<float[4]> nglHeightLightingConstants{0x0091E3C0};

extern void nglInitShaderLighting();
extern void nglSetupMaterialLighting(nglMaterialBase *material, nglMeshNode *mesh, uint32_t matrix_register,
                                   uint32_t color_register, uint32_t height_register,
                                   uint32_t palette_register, uint32_t palette_count);
extern void nglUpdateLODVertexColors(nglMeshNode *mesh, nglMeshSection *section);
