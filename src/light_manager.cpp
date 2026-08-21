#include "light_manager.h"

#include "func_wrapper.h"
#include "common.h"

VALIDATE_SIZE(light_manager, 0x34);

light_manager::light_manager() {}

light_manager::light_manager(int)
{
    auto *v72 = this;
    v72->field_0 = 0;
    v72->field_C = 0;

    v72->field_10.r = 0.0;
    v72->field_10.g = 0.0;
    v72->field_10.b = 0.0;
    v72->field_10.a = 0.0;

    v72->field_20.r = 0.0;
    v72->field_20.g = 0.0;
    v72->field_20.b = 0.0;
    v72->field_20.a = 0.0;

    v72->field_30 = 0.0;
    v72->field_8 = nullptr;
    auto *v73 = light_manager::active_light_managers();
    v72->field_4 = active_light_managers();
    if (active_light_managers() != nullptr) {
        v73->field_8 = v72;
    }

    active_light_managers() = v72;
}

void light_manager::frame_advance_all_light_managers(Float a1)
{
    CDECL_CALL(0x0053B040, a1);
}

void light_manager::frame_advance(region *a2, Float a3, bool a4)
{
    THISCALL(0x00534980, this, a2, a3, a4);
}

void light_manager::remove_from_list()
{
    THISCALL(0x00515E70, this);
}
