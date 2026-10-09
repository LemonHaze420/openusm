#pragma once

#include "variable.h"

struct nglMesh;
struct vector3d;

struct USOcean2Shader {
    //0x00403210
    static void Init();

    static void Release();

    static void Draw(const vector3d &a1);

    static Var<char *> OceanMeshFileName;

    static Var<nglMesh *> OceanMesh;
};

#if STANDALONE_SYSTEM
void initialize_ocean_material_shader();
#endif
