#pragma once

#include "singleton.h"

#include "input_device.h"

struct pc_joypad_device;

struct pc_input_mgr : singleton {
    pc_joypad_device *pad[4];

    //0x0059B930
    pc_input_mgr();

    static pc_input_mgr *& instance;

    //0x005E2C30
    static void create_inst();
};
