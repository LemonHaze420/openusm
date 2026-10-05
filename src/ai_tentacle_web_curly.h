#pragma once

#include "actor.h"
#include "ai_tentacle_dangle.h"
#include "entity_base_vhandle.h"

struct ai_tentacle_info;

struct ai_tentacle_web_curly : ai_tentacle_dangle {
    vhandle_type<actor> field_24;
    entity_base *field_28;
    _std::vector<vector3d> field_2C;
    vector3d field_3C;
    float field_48;
    float field_4C;
    int field_50;

    //0x00483DE0
    ai_tentacle_web_curly(ai_tentacle_info *a2);
    static void *native_vtable();
    bool frame_advance(Float dt, bool modifier);

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    void reset_curl();

    //0x00487470
    void setup(vhandle_type<actor> a2, entity_base *a3);
};
