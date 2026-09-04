#include "us_decal.h"

#include "trace.h"
#include "utility.h"
#include "ngl.h"
#include "us_exterior.h"

void US_Decal3DShader::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("US_Decal3DShader::BindMaterial");

    struct {
        char field_0[0x60];
        tlFixedString *field_60;
        nglTexture *field_64;
    } *v1 = CAST(v1, a1);

#ifdef TARGET_XBOX
    v1->field_64 = nglLoadTexture(*bit_cast<tlHashString *>(&v1->field_60));
#else
    v1->field_64 = nglLoadTexture(*bit_cast<tlFixedString *>(v1->field_60));
#endif
}

void US_Decal3DShader::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("US_Decal3DShader::RebaseMaterial");

    USExteriorMaterial *v1 = CAST(v1, a1);

#ifndef TARGET_XBOX
    auto v2 = v1->field_60;
    if (v2 != nullptr) {
        v1->field_60 = CAST(v1->field_60, int(v2) + a2);
    }
#endif
}

void us_decal_patch()
{
    {
        FUNC_ADDRESS(address, &US_Decal3DShader::_BindMaterial);
        set_vfunc(0x00870CA0, address);
    }

    {
        FUNC_ADDRESS(address, &US_Decal3DShader::_RebaseMaterial);
        set_vfunc(0x00870CA8, address);
    }
}
