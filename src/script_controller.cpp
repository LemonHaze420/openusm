#include "script_controller.h"

#include "common.h"
#include "entity.h"
#include "event.h"
#include "fe_controller_disconnect.h"
#include "femanager.h"
#include "func_wrapper.h"
#include "game.h"
#include "input_device.h"
#include "input_mgr.h"
#include "log.h"
#include "pausemenusystem.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(script_controller, 0x48);

#if !STANDALONE_SYSTEM

script_controller *&script_pad = var<script_controller *>(0x0096BB40);

#else

script_controller *&script_pad = []() -> auto & {
    static script_controller *g_script_pad{};
    return g_script_pad;
}();

#endif


script_controller::script_controller() : signaller(false)
{
    this->m_vtbl = 0x0089BD50;
}

void script_controller::update()
{
    TRACE("script_controller::update");

    if (!fe_controller_disconnect::get_currently_plugged_in()) {
        return;
    }

    if (g_femanager.m_pause_menu_system != nullptr &&
        g_femanager.m_pause_menu_system->IsDialogActivated()) {
        return;
    }

    if (g_game_ptr != nullptr && g_game_ptr->flag.game_paused) {
        return;
    }

    if constexpr (STANDALONE_SYSTEM) {
        const auto device_id = input_mgr::instance->field_58;
        if (device_id == INVALID_DEVICE_ID) {
            return;
        }

        auto *device = input_mgr::instance->get_device_from_map(device_id);
        if (device == nullptr || equal(device->get_axis_state(22, 0), 1.0f)) {
            return;
        }

        struct button_event {
            int axis;
            string_hash pressed;
            string_hash released;
        };

        static const button_event buttons[] = {
            {20, event::START_PRESSED, event::START_RELEASED},
            {21, event::SELECT_PRESSED, event::SELECT_RELEASED},
            {10, event::X_PRESSED, event::X_RELEASED},
            {11, event::SQUARE_PRESSED, event::SQUARE_RELEASED},
            {13, event::TRIANGLE_PRESSED, event::TRIANGLE_RELEASED},
            {14, event::CIRCLE_PRESSED, event::CIRCLE_RELEASED},
            {16, event::L1_PRESSED, event::L1_RELEASED},
            {18, event::L2_PRESSED, event::L2_RELEASED},
            {17, event::R1_PRESSED, event::R1_RELEASED},
            {19, event::R2_PRESSED, event::R2_RELEASED},
        };

        for (const auto &binding : buttons) {
            const auto axis = device->get_axis_id(binding.axis);
            const auto delta = device->get_axis_delta(axis, 0);
            if (equal(delta, 1.0f)) {
                raise_event(binding.pressed);
                return;
            }
            if (equal(delta, -1.0f)) {
                raise_event(binding.released);
                return;
            }
        }

        constexpr float threshold = 0.7f;
        const auto dispatch_direction = [this, device](
                                            int axis,
                                            string_hash negative_pressed,
                                            string_hash negative_released,
                                            string_hash positive_pressed,
                                            string_hash positive_released) {
            const auto mapped_axis = device->get_axis_id(axis);
            const auto current = device->get_axis_state(mapped_axis, 0);
            const auto previous = device->get_axis_old_state(mapped_axis, 0);

            if (previous > -threshold && current <= -threshold) {
                raise_event(negative_pressed);
                return true;
            }
            if (previous <= -threshold && current > -threshold) {
                raise_event(negative_released);
                return true;
            }
            if (previous < threshold && current >= threshold) {
                raise_event(positive_pressed);
                return true;
            }
            if (previous >= threshold && current < threshold) {
                raise_event(positive_released);
                return true;
            }
            return false;
        };

        if (dispatch_direction(2, event::LEFT_PRESSED, event::LEFT_RELEASED,
                               event::RIGHT_PRESSED, event::RIGHT_RELEASED) ||
            dispatch_direction(3, event::UP_PRESSED, event::UP_RELEASED,
                               event::DOWN_PRESSED, event::DOWN_RELEASED) ||
            dispatch_direction(7, event::RSTICK_LEFT_PRESSED, event::RSTICK_LEFT_RELEASED,
                               event::RSTICK_RIGHT_PRESSED, event::RSTICK_RIGHT_RELEASED) ||
            dispatch_direction(8, event::RSTICK_UP_PRESSED, event::RSTICK_UP_RELEASED,
                               event::RSTICK_DOWN_PRESSED, event::RSTICK_DOWN_RELEASED) ||
            dispatch_direction(4, event::LSTICK_LEFT_PRESSED, event::LSTICK_LEFT_RELEASED,
                               event::LSTICK_RIGHT_PRESSED, event::LSTICK_RIGHT_RELEASED)) {
            return;
        }

        dispatch_direction(5, event::LSTICK_UP_PRESSED, event::LSTICK_UP_RELEASED,
                           event::LSTICK_DOWN_PRESSED, event::LSTICK_DOWN_RELEASED);
    } else {
        THISCALL(0x0065F8A0, this);
    }
}

bool script_controller::is_button_pressed(int a1) const
{
    auto v3 = input_mgr::instance->field_58;
    if ( v3 == -1 ) {
        return false;
    }

    bool result = false;
    auto *device = input_mgr::instance->get_device_from_map(v3);
    if (device != nullptr) {
        if (not_equal(1.0f, device->get_axis_state(22, 0))) {
            auto v7 = device->get_axis_id(a1);
            if (equal(1.0f, device->get_axis_delta(v7, 0))) {
                return true;
            }
        }
    }

    return result;
}

float script_controller::get_axis_position(int a1) const
{
    auto v3 = input_mgr::instance->field_58;
    if ( v3 == -1 ) {
        return 0.0f;
    }
    
    auto *device = input_mgr::instance->get_device_from_map_internal(v3);
    if (device != nullptr && device->get_id() != -1 && not_equal(1.0f, device->get_axis_state(22, 0))) {
        auto v6 = device->get_axis_id(a1);
        return device->get_axis_state(v6, 0);
    }

    return 0.0f;
}

void script_controller_patch()
{
    {
        FUNC_ADDRESS(address, &script_controller::update);
        set_vfunc(0x0089BD50 + 0x168, address);
    }
}
