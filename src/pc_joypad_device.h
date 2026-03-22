#pragma once

#include "input_device.h"

struct InputState {
    int field_0;
    uint8_t m_flags;
    char field_5;
    uint8_t m_jump;
    uint8_t m_stick_to_walls;
    uint8_t m_punch;
    uint8_t m_kick;
    uint8_t m_black_button;
    uint8_t m_throw_web;
    uint8_t field_C;
    uint8_t field_D;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
};

struct InputCapabilities {
    uint16_t field_0;
    uint16_t field_2;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    uint32_t field_20;
    uint32_t field_24;
};

struct pc_joypad_device : input_device {
    enum Axis {
        Jump = 10,
    };

    InputState m_axis_state;
    InputState m_axis_old_state;

    InputCapabilities field_48;

    int field_70;
    int field_74[5];
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    char field_98;

    pc_joypad_device();

    //0x005991E0
    pc_joypad_device(int in_port);

    //0x0058E450
    //virtual
    int _get_id();

    //0x0058E560
    /* virtual */ float _get_axis_state(Axis a2, int a3);

    //0x0058E7B0
    /* virtual */ float _get_axis_old_state(Axis a2, int a3);

    //0x0058E740
    /* virtual */ float _get_axis_delta(Axis a2, int a3);

    //0x0058E6F0
    //virtual
    bool _is_connected();

    //0x0058E700
    //virtual
    void _clear_state();

    //0x0058E5C0
    /* virtual */ void _poll();

    //0x0058E870
    /* virtual */ float _get_axis_state(Axis axis, InputState input_state);
};

extern int InputOpen(int a1, unsigned int a2);

extern void InputGetCapabilities(int a1, InputCapabilities *pCapabilities);

extern void pc_joypad_device_patch();
