#include "game_axis.h"

#include "common.h"
#include "func_wrapper.h"
#include "input_mgr.h"
#include "os_developer_options.h"

#include <cmath>
#include <functional>
#include "trace.h"

VALIDATE_SIZE(game_axis, 0x30);

game_axis::game_axis()
{
    this->field_2C = false;
    this->field_2D = false;
    this->field_0 = input_mgr::instance->field_58;
    this->field_8 = 0.0;
    this->field_4 = 5;

    this->clear();
}

void game_axis::clear()
{
    this->field_2D = true;
    this->field_10 = 0.0;
    this->field_C = 0.0;
    this->field_18 = 0.0;
    this->m_timeout = default_timeout;
    this->m_threshold = default_threshold;
    this->field_1C = 0.0;
    this->field_28 = 0;
    this->field_24 = 0;
    this->field_2C = false;
}

void game_axis::update(Float a2)
{
    TRACE("game_axis::update");

    if constexpr (STANDALONE_SYSTEM) {
        const auto device = static_cast<device_id_t>(field_0);
        const auto value = input_mgr::instance->get_control_state(field_4, device);
        float delta;
        if (os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(12)) == 3
            && (field_4 == 110 || field_4 == 111)) {
            delta = value - field_10;
            if (std::not_equal_to<float>{}(std::abs(delta), 1.0f)) {
                delta = 0.0f;
            }
        } else {
            delta = input_mgr::instance->get_control_delta(field_4, device);
        }
        override(a2, value, delta);
        update_multitap(a2);
    } else {
        THISCALL(0x0051D900, this, a2);
    }
}


void game_axis::override(Float dt, float value, float delta)
{
    field_10 = value;
    field_C = delta;
    field_2D = value <= field_8 && value >= -field_8;
    field_18 = field_2D ? 0.0f : field_18 + dt;
}



void game_axis::update_multitap(Float dt)
{
    float delta = field_C;
    int direction = 0;
    if (delta > m_threshold) {
        direction = 1;
    } else if (delta < -m_threshold) {
        direction = -1;
        delta = -delta;
    }
    const int tap_count = field_28;
    if (tap_count == 0) {
        if ((direction == -1 && field_10 < -m_threshold)
            || (direction == 1 && field_10 > m_threshold)) {
            field_1C = 0.0f;
            field_28 = 1;
            field_24 = direction;
            field_2C = false;
            return;
        }
    } else if (field_1C > m_timeout) {
        field_28 = 0;
        field_1C = 0.0f;
        field_24 = 0;
        field_2C = false;
        return;
    }
    if (field_1C <= m_timeout && tap_count > 0) {
        if (delta > m_threshold) {
            if (direction == -field_24) {
                field_2C = true;
                field_1C += dt;
                return;
            }
            if (direction == field_24 && field_2C) {
                field_28 = tap_count + 1;
                field_1C = 0.0f;
                field_2C = false;
            }
        }
        field_1C += dt;
    }
}
