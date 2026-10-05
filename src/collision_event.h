#pragma once

#include "event.h"
#include "entity_base_vhandle.h"
#include "vector3d.h"

struct subdivision_node;
struct vector3d;

struct collision_event : event {
    entity_base_vhandle other;
    const subdivision_node *obb;
    vector3d position;
    vector3d normal;
    //0x005B1870
    collision_event(entity_base_vhandle arg0, const subdivision_node *a3, const vector3d &a4, const vector3d &a5);

    static inline string_hash type_id = to_hash("collision_event");
};
