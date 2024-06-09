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
    if (!g_generating_vtables())
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

bool time_interface::is_combat_dilated() {
    auto *v1 = (int16_t *) this->field_30;
    return v1 && (v1[6] & 0x800) != 0;
}

double time_interface::sub_4ADE50()
{
    auto v1 = this->field_2C;
    if ( v1 != 0 )
    {
        auto v2 = v1 - 1;
        if ( v2 != 0 )
        {
            bool v3 = (v2 == 1);
            auto v4 = this->field_30;
            if ( v3 )
            {
                auto v6 = ( v4 == 0 || v4 == 2 ) ? 1.0f : this->field_10;
                return v6 * (g_world_ptr->field_158.field_0 * this->field_C);
            }
            else if ( !v4 || v4 == 2 )
            {
                return 1.0f;
            }
            else
            {
                return this->field_10;
            }
        }
        else
        {
            auto v7 = this->field_30;
            if ( v7 == 0 || v7 == 2 ) {
                return 1.0f * this->field_C;
            } else {
                return this->field_10 * this->field_C;
            }
        }
    }
    else
    {
        auto v8 = this->field_30;
        if ( v8 == 0 || v8 == 2 ) {
            return 1.0f * g_world_ptr->field_158.field_0;
        } else {
            return this->field_10 * g_world_ptr->field_158.field_0;
        }
    }
}
