#pragma once

#include "variable.h"

#include "input_action.h"
#include "input_mouse.h"

#include <cstdint>
#include <cstring>

enum InputType {
    None = 0,
    Key = 1,
    Mouse = 2,
    Joy = 3,
    Joy1 = 4,
    Joy2 = 5,
    Joy3 = 6,
    Joy4 = 7,
    Joy5 = 8,
    Joy6 = 9,
    Joy7 = 10,
    Joy8 = 11,
    Joy9 = 12

};

struct InputSettings {

    struct internal_struct {
        uint32_t m_size {};

        struct {
            InputType m_input_type;
            int m_value;
            float field_8;
        } field_4[50][6];

        internal_struct() {
            std::memset(&this->field_4, 0, sizeof(field_4));
        }

        void operator=(const InputSettings::internal_struct &a2);

        void find_and_clear(InputType input_type, int value);

        void find_and_clear(InputType input_type);

        void clear(int a2, int a3);

        void sub_821E70();

        void set_key(InputAction a2, uint32_t a3, int value);

        void set_mouse(InputAction a2, uint32_t a3, InputMouse value);

        void set(InputAction a2, uint32_t a3, InputType input_type, int value);

        float get_state(InputAction a2) const;
    };

    bool field_0;
    bool field_1;
    bool field_2;
    bool field_3;
    float field_4;
    float field_8;
    float field_C;
    float field_10;
    float field_14;

    internal_struct field_18;

    //0x00822010
    InputSettings();
};

extern void input_settings_patch();

extern InputSettings *& g_inputSettingsMenu;
extern InputSettings *& g_inputSettingsInGame;
extern InputSettings *& g_inputSettings2;
extern InputSettings *& g_inputSettings3;
extern InputSettings *& g_inputSettings4;

extern void sub_5828B0();
