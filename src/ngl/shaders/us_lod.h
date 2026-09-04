#pragma once

#include "nglshader.h"

struct nglMaterialBase;

struct USLODShader : nglShader {
    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    void _RebaseMaterial(nglMaterialBase *, uint32_t);
};

extern void us_lod_patch();
