#pragma once

#include <vector.hpp>

struct proximity_map;
struct proximity_map_construction_leaf;
struct stack_allocator;
struct subdivision_node_builder;
struct vector3d;

proximity_map *create_static_proximity_map_on_the_stack(stack_allocator &a1,
                                                        _std::vector<proximity_map_construction_leaf> &a2,
                                                        subdivision_node_builder &a3, const vector3d &a4,
                                                        const vector3d &a5, int a6);
