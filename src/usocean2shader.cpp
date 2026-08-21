#include "usocean2shader.h"

#include "func_wrapper.h"
#include "ngl.h"

Var<char *> USOcean2Shader::OceanMeshFileName{0x0091E1F4};

Var<nglMesh *> USOcean2Shader::OceanMesh{0x009562EC};

void USOcean2Shader::Init()
{
    tlFixedString a1{USOcean2Shader::OceanMeshFileName()};
    USOcean2Shader::OceanMesh() = nglGetFirstMeshInFile(a1);
}

void USOcean2Shader::Release()
{
    CDECL_CALL(0x00403240);
}

void USOcean2Shader::Draw(const vector3d &a1)
{
    CDECL_CALL(0x00408A00, &a1);
}
