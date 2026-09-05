#pragma once

#include "femenu.h"
#include "mstring.h"

struct FEMenuSystem;
struct pause_menu_options_data {
    int field_0[8];
    mString field_20[8];
    mString field_A0[8];

    //0x006106B0
    explicit pause_menu_options_data(int a2);

    //0x00610840
    void initialize();
};


struct pause_menu_options_display : FEMenu {
    int field_2C[49];
    int field_F0[15];
    int field_12C[5];
    FEMenuSystem *field_140;
    pause_menu_options_data *field_144;
    int field_148;
    int field_14C;
    int field_150;
    int field_154;
    int field_158;
    int field_15C[18];

    //0x0061D970
    pause_menu_options_display(FEMenuSystem *a2, int a3, int a4);

    //0x0063BE80
    //virtual
    void _Load();
};

extern void pause_menu_options_display_patch();
