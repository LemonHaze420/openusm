#pragma once

#include "femenu.h"
#include "mstring.h"

#include <vector.hpp>

struct main_menu_keyboard : FEMenu {
    char field_2C[0xB8];
    mString field_E4;
    _std::vector<char> field_F4;
    char field_104[0x34];
    int16_t field_138;
    bool field_13A;
    bool field_13B;
    bool field_13C;
    bool field_13D;
    char field_13E[2];
    FEMenuSystem *field_140;
    int field_144;

    main_menu_keyboard(FEMenuSystem *a2, int a3, int a4);

    void _Init();

    //0x006241E0
    /* virtual */ void OnActivate();

    //0x00613D10
    /* virtual */ void OnDeactivate(FEMenu *a2);
};

extern void main_menu_keyboard_patch();
