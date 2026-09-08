#include <algorithm>

#include "time_interface.h"

#include "common.h"
#include "entity.h"
#include "func_wrapper.h"
#include "variables.h"
#include "wds.h"

VALIDATE_SIZE(time_interface, 0x34);

namespace {
_std::vector<time_interface *> all_time_interfaces;

void decrement_to_zero(float &value, float amount)
{
    value -= amount;
    if (value < 0.0f) {
        value = 0.0f;
    }
}
}

time_interface::time_interface(entity *a2)
{
    this->m_vtbl = 0x00883850;
    this->field_8 = false;
    this->field_4 = a2;
    this->field_8 = true;
    this->field_18 = 0;
    this->field_1C = 0;
    this->field_20 = 0;
    if (!g_generating_vtables) {
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
time_interface::~time_interface()
{
#if STANDALONE_SYSTEM
    const auto it = std::find(all_time_interfaces.begin(), all_time_interfaces.end(), this);
    if (it != all_time_interfaces.end()) {
        all_time_interfaces.erase(it);
    }
#else
    THISCALL(0x004D5530, this);
#endif
}

void time_interface::frame_advance_all_time_interfaces(Float a1)
{
#if STANDALONE_SYSTEM
    for (auto *time_ifc : all_time_interfaces) {
        time_ifc->frame_advance(a1);
    }
#else
    CDECL_CALL(0x004D18D0, a1);
#endif
}

void time_interface::add_to_time_ifc_list()
{
#if STANDALONE_SYSTEM
    all_time_interfaces.push_back(this);
#else
    THISCALL(0x004D9870, this);
#endif
}

void time_interface::set_state(int state, Float elapsed)
{
    field_30 = state;
    switch (state) {
    case 0:
        field_10 = 1.0f;
        break;
    case 1:
        if (field_18 <= 0.0f || elapsed >= field_18) {
            set_state(4, elapsed - field_18);
        } else {
            decrement_to_zero(field_18, elapsed);
            field_10 = field_14;
        }
        break;
    case 3:
        if (field_24 <= 0.0f || elapsed >= field_24) {
            set_state(1, elapsed - field_24);
        } else {
            field_20 = field_24 - elapsed;
            field_10 =
                (1.0f - field_20 / field_24) *
                    (field_14 - 1.0f) +
                1.0f;
        }
        break;
    case 4:
        if (field_28 <= 0.0f || elapsed >= field_28) {
            set_state(0, elapsed - field_28);
        } else {
            field_20 = field_28 - elapsed;
            field_10 =
                (1.0f - field_20 / field_28) *
                    (1.0f - field_14) +
                field_14;
        }
        break;
    default:
        break;
    }
}

void time_interface::frame_advance(Float elapsed)
{
    switch (field_30) {
    case 1:
        if (elapsed < field_18) {
            decrement_to_zero(field_18, elapsed);
        } else {
            const float remainder = elapsed - field_18;
            field_18 = 0;
            set_state(4, remainder);
        }
        break;
    case 2:
        if (elapsed < field_1C) {
            decrement_to_zero(field_1C, elapsed);
        } else {
            const float remainder = elapsed - field_1C;
            field_1C = 0;
            field_30 = 3;
            if (field_24 <= 0.0f || remainder >= field_24) {
                set_state(1, remainder - field_24);
            } else {
                field_20 = field_24 - remainder;
                field_10 =
                    (1.0f - field_20 / field_24) *
                        (field_14 - 1.0f) +
                    1.0f;
            }
        }
        break;
    case 3:
        if (elapsed < field_20) {
            decrement_to_zero(field_20, elapsed);
            field_10 =
                (1.0f - field_20 / field_24) *
                    (field_14 - 1.0f) +
                1.0f;
        } else {
            const float remainder = elapsed - field_20;
            field_20 = 0;
            set_state(1, remainder);
        }
        break;
    case 4:
        if (elapsed < field_20) {
            decrement_to_zero(field_20, elapsed);
            field_10 =
                (1.0f - field_20 / field_28) *
                    (1.0f - field_14) +
                field_14;
        } else {
            field_30 = 0;
            field_10 = 1.0f;
        }
        break;
    default:
        field_10 = 1.0f;
        break;
    }
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
