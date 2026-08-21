#include "us_lod.h"

#include "ngl.h"
#include "trace.h"
#include "utility.h"

void USLODShader::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USLODShader::BindMaterial");

    for (auto i = 0; i < 4; ++i) {
        *((float *)&a1->field_1C + 4 * i) = *((float *)&a1->field_1C + 4 * i) * 0.5;
        *((float *)&a1->field_20 + 4 * i) = *((float *)&a1->field_20 + 4 * i) * 0.5;
        *((float *)&a1->field_24 + 4 * i) = *((float *)&a1->field_24 + 4 * i) * 0.5;
    }
}

void USLODShader::_RebaseMaterial(nglMaterialBase *, uint32_t)
{
    TRACE("USLODShader::RebaseMaterial");
    ;
}

void us_lod_patch()
{
    {
        FUNC_ADDRESS(address, &USLODShader::_BindMaterial);
        set_vfunc(0x00870D1C, address);
    }

    {
        FUNC_ADDRESS(address, &USLODShader::_RebaseMaterial);
        set_vfunc(0x00870D24, address);
    }
}
