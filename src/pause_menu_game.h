#pragma once

#include "mstring.h"

struct pause_menu_game {
    mString field_0;
    mString field_10[21];
    mString field_160[21];

    //0x0060F1A0
    void initialize();
};
