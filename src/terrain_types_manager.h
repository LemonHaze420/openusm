#pragma once

#include "string_hash.h"

struct terrain_types_manager {
    //0x005BA680
    static void delete_inst();

    //0x005C54B0
    static void create_inst();


    static string_hash get_terrain_type_by_index(int index);
};
