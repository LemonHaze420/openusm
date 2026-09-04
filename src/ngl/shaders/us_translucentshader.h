#pragma once

#include "nglshader.h"

struct nglMaterialBase;

namespace USTranslucentShaderSpace {

template <typename T>
struct USTranslucentShader : nglShader {
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    //virtual
    void _RebaseMaterial(nglMaterialBase *a1, uint32_t a2);
};
}  // namespace USTranslucentShaderSpace

extern void us_translucentshader_patch();
