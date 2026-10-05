#include "rumble_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "memory.h"
#include "input_device.h"

#include <algorithm>
#include <cstdint>

VALIDATE_SIZE(rumble_manager, 0x64);

rumble_manager::rumble_manager()
{
    this->field_C = 0;
    this->field_10 = 0;
    this->field_14 = -1;
    this->field_1C = 0;
    this->field_20 = 0;
    this->field_4 = -1.0;
    this->field_8 = -1.0;
    this->field_18 = -1.0;
    this->field_21 = 1;
    this->field_24 = 0;
    this->field_45 = 1;
    this->field_28 = -1.0;
    this->field_2C = -1.0;
    this->field_30 = 0;
    this->field_34 = 0;
    this->field_38 = -1;
    this->field_3C = -1.0;
    this->field_40 = 0;
    this->field_44 = 0;
    this->field_48 = 0;
    this->field_4C = 0;
    this->field_50 = 0;
    this->field_54 = 0;

    this->field_5D = 1;
    this->field_60 = 1;
    this->field_0 = 0;
    this->field_5C = 0;
    this->field_5F = false;
    this->field_61 = 0;
    this->field_62 = 0;
    this->field_58 = 15.0;
}

void rumble_manager::start_vibration(float amplitude, float duration, float attack, float release, unsigned int pulses,
                                     float interval)
{
    if ((input_mgr::instance->field_20 & 2) != 0 || !field_60)
        return;
    auto *device = input_mgr::instance->get_device_from_map(input_mgr::instance->field_58);
    field_0 = reinterpret_cast<std::intptr_t>(device);
    if (device == nullptr)
        return;
    field_58 = 15.0f;
    if (field_20 && field_21) {
        get_current_rumble_info(*reinterpret_cast<rumble_struct *>(&field_28));
        field_61 = true;
    }
    field_4 = std::clamp(amplitude, 0.0f, 1.0f);
    field_8 = std::clamp(duration, 0.0f, 1.0f);
    field_C = attack;
    field_10 = release;
    field_14 = static_cast<int>(pulses);
    field_18 = interval;
    field_24 = 0.0f;
    field_4C = field_8;
    field_20 = false;
    field_5D = false;
    field_5C = true;
    field_1C = attack <= 0.0f ? field_4 : 0.0f;
    device->m_vtbl->vibrate_0(device, nullptr, Float{field_1C});
    field_54 = 5.0f;
}

void rumble_manager::stop_vibration()
{
    THISCALL(0x005BA4E0, this);
}

void rumble_manager::enable_vibration()
{
    input_mgr::instance->field_20 &= 0xFFFFFFFD;
}

void rumble_manager::disable_vibration()
{
    this->field_21 = 1;
    this->field_1C = 0.0;
    this->field_4 = -1.0;
    this->field_8 = -1.0;
    this->field_14 = -1;
    this->field_18 = -1.0;
    this->field_C = 0.0;
    this->field_10 = 0;
    this->field_4C = 0.0;
    this->field_5C = 0;
    this->field_5D = 1;
    this->field_20 = 0;
    int v1 = this->field_0;
    if (v1) {
        (*(void (**)(void))(*(uint32_t *)v1 + 60))();
    }

    input_mgr::instance->field_20 |= 2u;
}

void rumble_manager::vibrate(rumble_struct a2)
{
    THISCALL(0x005D78F0, this, a2);
}

void rumble_manager::get_current_rumble_info(rumble_struct &a2)
{
    if (!this->field_5C) {
        this->field_20 = false;
        this->field_21 = true;
        this->field_1C = 0.0;
        this->field_4 = -1.0;
        this->field_8 = -1.0;
        this->field_14 = -1;
        this->field_18 = -1.0;
        this->field_C = 0.0;
        this->field_10 = 0.0;
    }

    a2.field_0 = this->field_4;
    a2.field_4 = this->field_8;
    a2.field_8 = this->field_C;
    a2.field_C = this->field_10;
    a2.field_10 = this->field_14;
    a2.field_14 = this->field_18;
    a2.field_18 = this->field_1C;
    a2.field_1C = this->field_20;
    a2.field_1D = this->field_21;
    a2.field_20 = this->field_24;
}
