#pragma once

#include "float.hpp"
#include "color.h"
#include "variable.h"

struct region;

struct light_manager {
    int field_0;
    light_manager *field_4;
    light_manager *field_8;
    int field_C;
    color field_10;
    color field_20;
    float field_30;

    light_manager();

    light_manager(int);

    //0x00534980
    void frame_advance(region *a2, Float a3, bool a4);

    //0x0053B040
    static void frame_advance_all_light_managers(Float a1);

    void remove_from_list();

    static inline Var<light_manager *> active_light_managers{0x0095C900};
};
