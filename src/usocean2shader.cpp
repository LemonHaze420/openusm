#include "usocean2shader.h"

#include "func_wrapper.h"
#include "ngl.h"

Var<char *> USOcean2Shader::OceanMeshFileName{0x0091E1F4};

Var<nglMesh *> USOcean2Shader::OceanMesh{0x009562EC};

void USOcean2Shader::Init()
{
#if STANDALONE_SYSTEM
    const tlFixedString mesh_file_name{"oceanmesh"};
#else
    const tlFixedString mesh_file_name{USOcean2Shader::OceanMeshFileName()};
#endif
    USOcean2Shader::OceanMesh() = nglGetFirstMeshInFile(mesh_file_name);
}

void USOcean2Shader::Release()
{
    CDECL_CALL(0x00403240);
}

void USOcean2Shader::Draw(const vector3d &a1)
{
    CDECL_CALL(0x00408A00, &a1);
}
