#pragma once

#include "nglshader.h"

struct nglMaterialBase;

namespace USSimpleShaderSpace {
template <typename T>
struct USSimpleShader : nglShader {
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    //virtual
    void _RebaseMaterial(nglMaterialBase *a1, unsigned int a2);
};
}  // namespace USSimpleShaderSpace

extern void us_simpleshader_patch();
