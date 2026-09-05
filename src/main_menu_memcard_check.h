#pragma once

#include "femenu.h"

struct main_menu_memcard_check : FEMenu {
    char field_2C[0xD4];
    int field_100;
    char field_104[0x9C];

    main_menu_memcard_check(FEMenuSystem *a2, int a4, int a5);

    void _Init();
};
