#include "us_simpleshader.h"

#include "ngl.h"
#include "trace.h"
#include "utility.h"
#include "us_interior.h"
#include "us_exterior.h"

namespace USSimpleShaderSpace {
template <>
void USSimpleShader<USInteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USSimpleShader<USInteriorMaterial>::BindMaterial");

#ifdef TARGET_XBOX
    a1->field_20 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_1C));
#else
    auto *Material = bit_cast<USInteriorMaterial *>(a1);
    Material->field_20 = nglLoadTexture(*Material->field_1C);
#endif
}

template <>
void USSimpleShader<USInteriorMaterial>::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("USSimpleShader<USInteriorMaterial>::RebaseMaterial");

    auto *v2 = a1->field_1C;
    if (v2 != nullptr) {
        a1->field_1C = (nglTexture *)((char *)v2 + a2);
    }
}

template <>
void USSimpleShader<USExteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USSimpleShader<USExteriorMaterial>::BindMaterial");

#ifdef TARGET_XBOX
    a1->field_20 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_1C));
#else
    auto *Material = bit_cast<USExteriorMaterial *>(a1);
    Material->field_64 = nglLoadTexture(*Material->field_60);
#endif
}

template <>
void USSimpleShader<USExteriorMaterial>::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("USSimpleShader<USExteriorMaterial>::RebaseMaterial");

    auto *Material = bit_cast<USExteriorMaterial *>(a1);
    auto *v2 = Material->field_60;
    if (v2 != nullptr) {
        Material->field_60 = CAST(Material->field_60, bit_cast<char *>(v2) + a2);
    }
}
}  // namespace USSimpleShaderSpace

void us_simpleshader_patch()
{
    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USInteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871658, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USExteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871600, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USInteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871660, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USExteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871608, address);
    }
}
