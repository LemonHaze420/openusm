#pragma once

#include "config.h"

#include "rumble_struct.h"

struct rumble_manager {
    int field_0;
    float field_4;
    float field_8;
    int field_C;
    int field_10;
    int field_14;
    float field_18;
    int field_1C;
    bool field_20;
    bool field_21;
    int field_24;
    float field_28;
    float field_2C;
    int field_30;
    int field_34;
    int field_38;
    float field_3C;
    int field_40;
    char field_44;
    char field_45;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;
    float field_58;
    bool field_5C;
    bool field_5D;
    bool field_5E;
    bool field_5F;
    bool field_60;
    bool field_61;
    bool field_62;

    rumble_manager();

    ~rumble_manager() = default;

    //0x005BA4E0
    void stop_vibration();

    void enable_vibration();

    void disable_vibration();

    //0x005D78F0
    void vibrate(rumble_struct a2);

    //0x005BA520
    void get_current_rumble_info(rumble_struct &a2);
};
