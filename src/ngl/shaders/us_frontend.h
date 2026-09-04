#pragma once

#include "us_shaderbase.h"

#include "fixedstring.h"

#include <cstdint>

struct nglTexture;
struct nglMaterialBase;

struct FrontEnd_Shader : USShaderBase {
    FrontEnd_Shader();

    //0x00403AD0
    //virtual
    void _Register();

    //0x00410950
    //virtual
    tlFixedString _GetName() const;

    //0x00415EE0
    //virtual
    void _AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    //0x004100C0
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    //0x00410970
    //virtual
    void _ReleaseMaterial(nglMaterialBase *a1);

    //0x00410550
    //virtual
    void _RebaseMaterial(nglMaterialBase *, uint32_t);
};

extern FrontEnd_Shader &gFrontEnd_Shader;

struct FrontEnd_ShaderNode : nglShaderNode {
    nglMaterialBase *field_14;

    FrontEnd_ShaderNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4);

    //0x00406E50
    //virtual
    void Render();
};

extern void FrontEnd_Shader_patch();
