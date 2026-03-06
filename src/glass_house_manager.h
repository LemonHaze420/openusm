#pragma once

#include "string_hash.h"

#include <vector.hpp>

struct vector3d;
struct glass_house;

struct glass_house_manager {
    //0x00520890
    static bool is_enabled();

    //0x00538570
    static bool is_point_in_glass_house(const vector3d &a1);

    static void show_glass_houses();

    static void remove_glass_house(string_hash a2);

    static bool & enabled;

    static _std::vector<glass_house *> (& glass_houses)[3];
};

extern void glass_house_manager_patch();
