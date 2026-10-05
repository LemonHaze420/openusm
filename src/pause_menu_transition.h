#pragma once

#include "femenu.h"

#include "float.hpp"

struct PauseMenuSystem;

struct pause_menu_transition : FEMenu {
    PauseMenuSystem *field_2C;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;

    pause_menu_transition(FEMenuSystem *a2, int a3, int a4);

    void set_transition(int a1);

    //0x0061C640
    //virtual
    void _Load();

    //0x0061C680
    //virtual
    void Update(Float a2);

    void OnActivate();
    void Draw();
};

extern void pause_menu_transition_patch();
