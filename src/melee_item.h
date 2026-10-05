#pragma once

#include "handheld_item.h"
#include "marker.h"
#include "color32.h"

struct melee_item : handheld_item {
    int field_114;
    int field_118;
    vhandle_type<marker> first_trail_marker;
    vhandle_type<marker> second_trail_marker;
    color32 first_trail_color;
    color32 second_trail_color;
    vector3d first_trail_position;
    vector3d second_trail_position;
    int trail_samples;
    bool additive_trail;
    bool trail_enabled;

    static void *native_vtable(void **handheld_table);
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    void holster(bool);
    void enable_motion_trail(bool);
    void stop_motion_trail();
};
