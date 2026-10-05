#pragma once

#include "entity_base_vhandle.h"

struct entity;
struct fe_mini_map_dot;
struct mini_map_dot_type;

struct entity_tracker {
    entity_base_vhandle field_0;
    fe_mini_map_dot *field_4;
    int field_8;
    int field_C;

    entity_tracker();
    explicit entity_tracker(entity_base_vhandle handle);

    // 0x0060BC40
    entity *get_entity();

    // 0x00641120
    void set_poi_icon(mini_map_dot_type type);
    void set_health_widget_active(bool enabled);
};
