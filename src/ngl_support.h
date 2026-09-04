#pragma once

#include "ngl_mesh.h"

//0x00507690
extern void FastListAddMesh(nglMesh *Mesh, const math::MatClass<4, 3> &LocalToWorld, nglMeshParams *FastListAddMesh,
                            nglParamSet<nglShaderParamSet_Pool> *ShaderParams);
