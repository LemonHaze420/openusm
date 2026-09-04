#pragma once

#include "nglshader.h"

extern void us_outline_patch();

struct nglMaterialBase;
struct nglTexture;

namespace USOutlineShaderSpace {

template <typename T>
struct Outline_ShaderNode : nglShaderNode {
    nglMaterialBase *field_14;
    nglTexture *field_18;

    void Render();
};

}  // namespace USOutlineShaderSpace
