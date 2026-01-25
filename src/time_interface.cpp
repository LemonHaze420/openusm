#include "time_interface.h"

#include "common.h"
#include "entity.h"
#include "func_wrapper.h"
#include "variables.h"
#include "wds.h"

VALIDATE_SIZE(time_interface, 0x34);

time_interface::time_interface(entity *a2)
{
    this->m_vtbl = 0x00883850;
    this->field_8 = false;
    this->field_4 = a2;
    this->field_8 = true;
    this->field_18 = 0;
    this->field_1C = 0;
    this->field_20 = 0;
    if (!g_generating_vtables)
    {
        this->field_C = 1.0;
        this->field_10 = 1.0;
        this->field_14 = 1.0;
        this->field_2C = 2;
        this->field_18 = 0;
        this->field_1C = 0;
        this->field_20 = 0;
        this->field_24 = 0;
        this->field_28 = 0;
        this->field_30 = 0;
        this->add_to_time_ifc_list();
    }
}

void time_interface::frame_advance_all_time_interfaces(Float a1) {
    CDECL_CALL(0x004D18D0, a1);
}

void time_interface::add_to_time_ifc_list() {
    THISCALL(0x004D9870, this);
}

bool time_interface::is_combat_dilated() const
{
    return this->field_30 != 0 && this->field_30 != 2;
}

double time_interface::sub_4ADE50()
{
    auto v1 = this->field_2C;
    const auto v8 = this->is_combat_dilated() ? this->field_10 : 1.0f;

    switch (v1) {
        case 0: {
            return v8 * g_world_ptr->field_158.field_0;
        }
        case 1: {
            return v8 * this->field_C;
        }
        case 2: {
            return v8 * (g_world_ptr->field_158.field_0 * this->field_C);
        }
        default: {
            return v8 * 1.0;
        }
    }
}
