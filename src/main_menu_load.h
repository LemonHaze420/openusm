#pragma once

#include "femenu.h"

struct main_menu_load : FEMenu {
    char field_2C[0x148];

    main_menu_load(FEMenuSystem *a2, int a4, int a5);

    void _Init();
};
