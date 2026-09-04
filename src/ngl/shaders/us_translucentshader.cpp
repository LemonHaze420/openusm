#include "us_translucentshader.h"

#include "ngl.h"
#include "trace.h"
#include "utility.h"
#include "func_wrapper.h"
#include "us_interior.h"
#include "us_exterior.h"

namespace USTranslucentShaderSpace {

template <>
void USTranslucentShader<USExteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USTranslucentShader<USExteriorMaterial>::BindMaterial");

    auto *v1 = bit_cast<USExteriorMaterial *>(a1);

#ifdef TARGET_XBOX
    v1->field_64 = nglLoadTexture(*bit_cast<tlHashString *>(&v1->field_60));
    if (v1->field_84 > 0.0) {
        v1->field_84 = 0.0;
    }
#else
    v1->field_64 = nglLoadTexture(*bit_cast<tlFixedString *>(v1->field_60));
    auto v2 = v1->field_84;
    if (v2 > 0.0) {
        v1->field_84 = 0.0;
    }
#endif
}

template <>
void USTranslucentShader<USInteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USTranslucentShader<USInteriorMaterial>::BindMaterial");

#ifdef TARGET_XBOX
    a1->field_20 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_1C));
    if (a1->field_40 > 0.0) {
        a1->field_40 = 0.0;
    }
#else
    a1->field_20 = nglLoadTexture(*bit_cast<tlFixedString *>(a1->field_1C));
    auto v2 = a1->field_40;
    if (v2 > 0.0) {
        a1->field_40 = 0.0;
    }
#endif
}

template <>
void USTranslucentShader<USExteriorMaterial>::_RebaseMaterial([[maybe_unused]] nglMaterialBase *a1,
                                                              [[maybe_unused]] uint32_t a2)
{
    TRACE("USTranslucentShader<USExteriorMaterial>::RebaseMaterial");

    auto *v1 = bit_cast<USExteriorMaterial *>(a1);

#ifndef TARGET_XBOX
    if constexpr (0) {
        auto *v2 = v1->field_60;
        if (v2 != nullptr) {
            v1->field_60 = CAST(v1->field_60, int(v2) + a2);
        }
    } else {
        THISCALL(0x00410BF0, this, a1, a2);
    }
#endif
}

template <>
void USTranslucentShader<USInteriorMaterial>::_RebaseMaterial([[maybe_unused]] nglMaterialBase *a1,
                                                              [[maybe_unused]] uint32_t a2)
{
    TRACE("USTranslucentShader<USInteriorMaterial>::RebaseMaterial");

#ifndef TARGET_XBOX
    if constexpr (0) {
        auto *v2 = a1->field_1C;
        if (v2 != nullptr) {
            a1->field_1C = (nglTexture *)((char *)v2 + a2);
        }
    } else {
        THISCALL(0x00410C10, this, a1, a2);
    }
#endif
}
}  // namespace USTranslucentShaderSpace

void us_translucentshader_patch()
{
    {
        auto func = &USTranslucentShaderSpace::USTranslucentShader<USExteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871760, address);
    }

    {
        auto func = &USTranslucentShaderSpace::USTranslucentShader<USExteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871768, address);
    }

    {
        auto func = &USTranslucentShaderSpace::USTranslucentShader<USInteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008717BC, address);
    }

    {
        auto func = &USTranslucentShaderSpace::USTranslucentShader<USInteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x008717C4, address);
    }
}
