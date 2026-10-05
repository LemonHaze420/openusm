#pragma once

#include "float.hpp"
#include "string_hash.h"
#include "vector3d.h"

struct entity_base;

struct decal_morphs {
    decal_morphs();

    static bool create_decal(string_hash name, vector3d position, float lifetime,
        vector3d direction, entity_base *parent);

    //0x004CE1E0
    static void frame_advance(Float a1);
};
