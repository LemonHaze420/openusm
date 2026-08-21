#include "motion_effect_struct.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(motion_effect_struct, 0x30u);

motion_effect_struct::motion_effect_struct(entity_base_vhandle a2, const mString &a3)
{
    void(__fastcall * func)(motion_effect_struct *, void *edx, entity_base_vhandle a2, const mString *a3) =
        CAST(func, 0x004DC560);
    func(this, nullptr, a2, &a3);
}

motion_effect_struct::~motion_effect_struct()
{
    THISCALL(0x004E56D0, this);
}

void motion_effect_struct::render_all_motion_fx(camera &a2, hull &a3)
{
    CDECL_CALL(0x004EFB20, &a2, &a3);
}

void motion_effect_struct::render_trail(vector3d a1, vector3d a4, vector3d a7, vector2d a10, vector2d a12, vector2d a14,
                                        color32 a16, color32 a17, color32 a18, bool a19, vector3d a20, vector3d a23)
{
    TRACE("motion_effect_struct::render_trail");
    THISCALL(0x004DC820, this, a1, a4, a7, a10, a12, a14, a16, a17, a18, a19, a20, a23);
}

void motion_effect_struct::render_distorted_trail(const vector3d &a1, const vector3d &a2, const vector3d &a3,
                                                  const nglVector &a4, const nglVector &a5, const nglVector &a6,
                                                  color32 a7, color32 a8, color32 a9, bool a10, vector3d &a11,
                                                  vector3d &a12)
{
    TRACE("motion_effect_struct::render_distorted_trail");

    THISCALL(0x004DCA10, this, &a1, &a2, &a3, &a4, &a5, &a6, a7, a8, a9, a10, &a11, &a12);
}

void motion_effect_struct::record_all_motion_fx(Float a1)
{
    TRACE("motion_effect_struct::record_all_motion_fx");

    CDECL_CALL(0x004EFA50, a1);
}

void motion_effect_struct_patch()
{
    REDIRECT(0x005584EE, motion_effect_struct::record_all_motion_fx);

    {
        FUNC_ADDRESS(address, &motion_effect_struct::render_trail);
        REDIRECT(0x004E7865, address);
        REDIRECT(0x004E792A, address);
        REDIRECT(0x004E7BD1, address);
        REDIRECT(0x004E7CE3, address);
    }

    {
        FUNC_ADDRESS(address, &motion_effect_struct::render_distorted_trail);
        REDIRECT(0x004E89C1, address);
        REDIRECT(0x004E8A6E, address);
        REDIRECT(0x004E8B1D, address);
        REDIRECT(0x004E8BD1, address);
        REDIRECT(0x004E8C7C, address);
        REDIRECT(0x004E8D29, address);
    }
}
