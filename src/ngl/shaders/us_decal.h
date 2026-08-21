#pragma once

#include "nglshader.h"

struct nglMaterialBase;

struct US_Decal3DShader : nglShader {
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    //virtual
    void _RebaseMaterial(nglMaterialBase *a1, unsigned int a2);
};

extern void us_decal_patch();
