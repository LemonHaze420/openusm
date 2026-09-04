#pragma once

#include "nglshader.h"

#include "ngl.h"

struct PCUV_Shader : nglShader {
    PCUV_Shader();

    //0x00402BC0
    //virtual
    void Register();
};

inline Var<PCUV_Shader> gPCUV_Shader{0x0091E46C};

struct nglMeshNode;
struct nglMeshSection;

struct PCUV_ShaderMaterial {
    std::intptr_t m_vtbl{0x00870AD4};
    nglMeshSection *field_4;
    PCUV_Shader *field_8;
    int empty[4];
    tlFixedString *field_1C;
    nglTexture *m_texture;
    nglBlendModeType m_blend_mode;
    uint32_t field_28;
    uint32_t field_2C;

    //0x00402B00
    PCUV_ShaderMaterial();

    //0x00402B30
    PCUV_ShaderMaterial(nglTexture *a2, nglBlendModeType a3, int a4, int a5);
};

struct PCUV_ShaderNode {
    std::intptr_t m_vtbl;
    int field_4;
    int field_8;
    nglMeshNode *field_C;
    nglMeshSection *field_10;
    PCUV_ShaderMaterial *field_14;

    //0x00406350
    //virtual
    void Render();
};

extern void us_pcuv_patch();
