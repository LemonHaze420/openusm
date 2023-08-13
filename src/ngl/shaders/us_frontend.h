#pragma once

#include "nglshader.h"

#include <cstdint>

struct nglTexture;
struct nglMaterialBase;

struct FrontEnd_Shader : nglShader {
    FrontEnd_Shader();

    //0x00403AD0
    //virtual
    void Register();

    //0x004100C0
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    //0x00410970
    //virtual
    void ReleaseMaterial(nglMaterialBase *a1);

    //0x00410550
    //virtual
    void _RebaseMaterial(nglMaterialBase *, uint32_t);
};

struct FrontEnd_ShaderNode : nglShaderNode {
    int *field_14;

    //0x00406E50
    //virtual
    void Render();
};

extern void FrontEnd_Shader_patch();
