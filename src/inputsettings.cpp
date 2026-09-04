#include "inputsettings.h"

#include "common.h"
#include "func_wrapper.h"
#include "input.h"
#include "settings.h"
#include "trace.h"
#include "utility.h"

InputSettings *&g_inputSettingsMenu = var<InputSettings *>(0x00965C0C);
InputSettings *&g_inputSettingsInGame = var<InputSettings *>(0x00965C10);
InputSettings *&g_inputSettings2 = var<InputSettings *>(0x00965C14);
InputSettings *&g_inputSettings3 = var<InputSettings *>(0x00965C18);
InputSettings *&g_inputSettings4 = var<InputSettings *>(0x00965C1C);

VALIDATE_SIZE(InputSettings, 0xE2Cu);

InputSettings::InputSettings()
{
    this->field_1 = false;
    this->field_2 = true;
    this->field_3 = false;
    this->field_4 = 0.99000001;
    this->field_8 = 10.0;
    this->field_C = 0;
    this->field_10 = 0;
    this->field_14 = 1.0;
    this->field_18 = {};
}

void InputSettings::internal_struct::sub_821E70()
{
    this->m_size = 0;
    memset(&this->field_4, 0, sizeof(this->field_4));
}

void InputSettings::internal_struct::set_key(InputAction a2, uint32_t a3, int value)
{
    this->set(a2, a3, InputType::Key, value);
}

void InputSettings::internal_struct::set_mouse(InputAction a2, uint32_t a3, InputMouse value)
{
    this->set(a2, a3, InputType::Mouse, static_cast<int>(value));
}

const char *to_string(InputAction action)
{
    static const char *arr[60]{"",
                               "",
                               "",
                               "",
                               "Jump",
                               "StickToWalls",
                               "Punch",
                               "Kick",
                               "BlackButton",
                               "ThrowWeb",
                               "",
                               "",
                               "Pause",
                               "BackButton",
                               "",
                               "CameraCenter",
                               "Forward",
                               "Backward",
                               "TurnLeft",
                               "TurnRight",

                               "CameraUp",
                               "CameraDown",
                               "CameraLeft",
                               "CameraRight",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "",
                               "ScreenShot"};

    return arr[static_cast<uint32_t>(action)];
}

void InputSettings::internal_struct::set(InputAction a2, uint32_t a3, InputType input_type, int value)
{
    TRACE("InputSettings::internal_struct::set");

    sp_log("%s, idx = %d, input_type = %d, value = %d", to_string(a2), a3, input_type, value);

    if constexpr (1) {
        auto idx = static_cast<uint32_t>(a2);
        auto size = idx + 1;
        if (size <= this->m_size) {
            size = this->m_size;
        }

        this->m_size = size;
        auto &v6 = this->field_4[idx][a3];
        v6.m_input_type = input_type;
        v6.m_value = value;
        v6.field_8 = 0.0f;

    } else {
        THISCALL(0x00821FD0, this, a2, a3, input_type, value);
    }
}

void InputSettings::internal_struct::operator=(const InputSettings::internal_struct &a2)
{
    this->m_size = a2.m_size;
    std::memcpy(this->field_4, a2.field_4, sizeof(this->field_4));
}

void InputSettings::internal_struct::find_and_clear(InputType input_type, int value)
{
    for (auto v3 = 0u; v3 < this->m_size; ++v3) {
        for (auto i = 0u; i < 6u; ++i) {
            auto &v4 = this->field_4[v3][i];

            if (v4.m_input_type == input_type && v4.m_value == value) {
                v4.m_input_type = InputType::None;
                v4.m_value = 0;
                v4.field_8 = 0.0f;
            }
        }
    }
}

void InputSettings::internal_struct::find_and_clear(InputType input_type)
{
    for (auto v2 = 0u; v2 < this->m_size; ++v2) {
        for (auto i = 0u; i < 6u; ++i) {
            auto &v3 = this->field_4[v2][i];

            if (v3.m_input_type == input_type) {
                v3.m_input_type = InputType::None;
                v3.m_value = 0;
                v3.field_8 = 0.0f;
            }
        }
    }
}

void InputSettings::internal_struct::clear(int a2, int a3)
{
    auto &v3 = this->field_4[a2][a3];
    v3.m_input_type = InputType::None;
    v3.m_value = 0;
    v3.field_8 = 0.0f;
}

float InputSettings::internal_struct::get_state(InputAction a2) const
{
    TRACE("get_state", to_string(a2));

    if constexpr (1) {
        auto idx = static_cast<uint32_t>(a2);
        if (idx >= this->m_size) {
            return 0.0f;
        }

        float result = 0.0f;
        float v17 = 0.0;

        for (uint32_t i{0}; i < 6; ++i) {
            const auto &v1 = this->field_4[idx][i];
            if (v1.m_input_type != InputType::None) {
                auto v2 = v1.field_8;
                auto v5 = std::abs(v2);
                if (v5 > v17) {
                    result = v2;
                    v17 = v5;
                }
            }
        }

        return result;
    } else {
        float(__fastcall * func)(const void *, void *, uint32_t) = CAST(func, 0x00821E90);

        return func(this, nullptr, static_cast<uint32_t>(a2));
    }
}

void sub_5828B0()
{
    auto v0 = Input::instance->sub_820080();
    if (v0 > 4) {
        v0 = 4;
    }

    auto v1 = 3;
    if (v0 > 0) {
        auto v2 = v0;
        do {
            g_inputSettingsMenu->field_18.set(InputAction::Jump, v1, (InputType)v1, 21);
            g_inputSettingsMenu->field_18.set(InputAction::Kick, v1, (InputType)v1, 24);
            g_inputSettingsMenu->field_18.set(InputAction::Forward, v1, (InputType)v1, 4);
            g_inputSettingsMenu->field_18.set(InputAction::Backward, v1, (InputType)v1, 3);
            g_inputSettingsMenu->field_18.set(InputAction::TurnLeft, v1, (InputType)v1, 2);
            g_inputSettingsMenu->field_18.set(InputAction::TurnRight, v1, (InputType)v1, 1);
            ++v1;
            --v2;
        } while (v2);
    }
}

bool sub_582630(BOOL Data)
{
    auto *v1 = &g_inputSettingsInGame->field_18;
    Settings::MouseLook = Data;
    if (Data) {
        v1->set_mouse(static_cast<InputAction>(22u), 3u, InputMouse::LookLeft);
        v1->set_mouse(static_cast<InputAction>(23u), 3u, InputMouse::LookRight);
        v1->set_mouse(static_cast<InputAction>(20u), 3u, InputMouse::LookUp);
        v1->set_mouse(static_cast<InputAction>(21u), 3u, InputMouse::LookDown);
        return g_settings->sub_81CF80("Settings\\MouseLook", Data);
    } else {
        v1->clear(22, 3);
        v1->clear(23, 3);
        v1->clear(20, 3);
        v1->clear(21, 3);
        return g_settings->sub_81CF80("Settings\\MouseLook", 0);
    }
}

void input_settings_patch()
{
    {
        FUNC_ADDRESS(address, &InputSettings::internal_struct::get_state);
        SET_JUMP(0x00821E90, address);
    }

    {
        FUNC_ADDRESS(address, &InputSettings::internal_struct::set);
        SET_JUMP(0x00821FD0, address);
    }
}
