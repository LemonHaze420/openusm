#pragma once

#include "mstring.h"

struct pause_menu_save_load_display;

struct pause_menu_save_load_hookup {
    struct save_slot {
        mString field_0;
        mString field_10;
        bool field_20;
        char field_21[3];
    };

    int field_0[9];
    bool field_24;
    bool field_25;
    bool field_26;
    char field_27;
    int field_28;
    save_slot field_2C[3];
    mString field_98[32];

    //0x0061F2B0
    void initialize();

    //0x0062C970
    void SaveFile(int a2);

    //0x0062C9B0
    void LoadFile(int a2);

    //0x0062BF90
    void set_current_state(int a2);
};
