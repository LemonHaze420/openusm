#pragma once

#include "float.hpp"

struct scene_entity;
struct matrix4x4;
struct region;

void render_generated_building(const scene_entity &building, Float fade, Float distance_squared, region &owner,
                               const matrix4x4 &local_to_world);

#if STANDALONE_SYSTEM
void initialize_building_mesh_shaders();
#endif
