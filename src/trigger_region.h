#pragma once

#include "mash_virtual_base.h"
#include "string_hash.h"
#include "vector3d.h"

struct actor;

struct trigger_region : mash_virtual_base {
    //virtual
    void unmash(mash_info_struct *info, void *context);

    //virtual
    int get_mash_sizeof() const;
    bool is_inside_trigger_region(const vector3d *position, actor *owner) const;
    void destruct_mashed_class();
    void delete_owned();
};

struct box_region : trigger_region {
    vector3d upper;
    vector3d lower;
    bool actor_relative;

    box_region(from_mash_in_place_constructor *tag);
    bool contains(const vector3d *position, actor *owner) const;
    static void *native_vtable();
};

struct named_trigger_box_region : trigger_region {
    string_hash name;

    named_trigger_box_region(from_mash_in_place_constructor *tag);
    void _unmash(mash_info_struct *info, void *context);
    bool contains(const vector3d *position, actor *owner) const;
    static void *native_vtable();
};

struct point_dist_region : trigger_region {
    float radius_squared;
    vector3d center;
    bool actor_relative;

    point_dist_region(from_mash_in_place_constructor *tag);
    bool contains(const vector3d *position, actor *owner) const;
    static void *native_vtable();
};
