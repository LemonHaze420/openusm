#pragma once

#include "actor.h"

struct visual_item : actor {
    bool field_C0;
    entity_base *field_C4;

    visual_item(const string_hash &, uint32_t);
    static void *native_vtable(void **actor_table);
    void attach(entity_base *, string_hash bone, float scale, const vector3d &position, const vector3d &rotation,
                bool drawn);
    void _render(Float);
};
