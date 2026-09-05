#pragma once

#include "mstring.h"

struct pause_menu_awards {
    mString field_0;
    mString field_10;
    mString field_20;
    mString field_30[17];
    mString field_140[17];

    //0x0060EB90
    void initialize();
};
