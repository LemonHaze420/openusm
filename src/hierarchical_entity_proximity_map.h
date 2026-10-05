#pragma once

#include "float.hpp"
#include "proximity_map_stack.h"
#include <vector.hpp>
#include <cstdint>

template <typename T, uint32_t N>
struct fixed_vector;
struct vector2d;

struct entity;
struct entity_proximity_map_data;
struct dynamic_proximity_map;
struct subdivision_visitor;
struct vector3d;


dynamic_proximity_map_stack *acquire_district_proximity_map_stack();
void release_district_proximity_map_stack(dynamic_proximity_map_stack *stack);

struct hierarchical_entity_proximity_map {
    struct {
        entity_proximity_map_data *entries[256];
    } entity_data_lookup;
    dynamic_proximity_map *maps[5];
    int number_of_levels;

    void init(dynamic_proximity_map_stack &allocator, int sphere_kind, const vector3d &min, const vector3d &max,
              const _std::vector<int> &levels);

    int traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4);

    int traverse_point(const vector3d &position, subdivision_visitor &visitor);

    int traverse_convex_hull_raster(const fixed_vector<vector2d, 14> &points, subdivision_visitor &visitor);

    bool remove_entity(entity *a2);

    void remove_entity(entity *a2, entity_proximity_map_data *data);

    void update_entity(entity *ent);

    bool sub_55E9B0(entity **a2, entity_proximity_map_data **a3);
};

extern void hierarchical_entity_proximity_map_patch();
