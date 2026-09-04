#pragma once

#include "entity.h"

struct mic : entity {
    vector3d field_68;
    vector3d field_74;

    //0x0050B930
    mic(entity *a2, const string_hash &a3);

    void *operator new(size_t size);

    //0x0051D9A0
    //virtual
    void frame_advance(Float a2);
};
