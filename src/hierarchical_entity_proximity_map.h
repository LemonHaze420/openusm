#pragma once

#include "float.hpp"

struct entity;
struct entity_proximity_map_data;
struct proximity_map;
struct subdivision_visitor;
struct vector3d;

struct hierarchical_entity_proximity_map {
    struct {
        void *entries[256];
    } entity_data_lookup;
    proximity_map *maps[1];
    int field_404[4];
    int number_of_levels;

    int traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4);

    bool remove_entity(entity *a2);

    void remove_entity(entity *a2, entity_proximity_map_data *data);

    void update_entity(entity *ent);

    bool sub_55E9B0(entity **a2, entity_proximity_map_data **a3);
};

extern void hierarchical_entity_proximity_map_patch();
