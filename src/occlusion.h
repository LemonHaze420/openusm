#pragma once

#include "plane.h"
#include "variable.h"
#include "vector3d.h"

namespace occlusion {
struct quad {
    vector3d points[4];
};

struct quad_shadow_volume {
    vector3d field_0[4];
    plane field_30[5];
    int field_80;
    vector3d field_84;
};

void init();

void term();

//0x00510800
void add_quad_to_database(const occlusion::quad &a1);

void debug_render_occluders();

void reset_active_occluders();

//0x00510840
void empty_quad_database();

void init_frame(const vector3d &a1);

void term_frame();

void update_based_on_scores(const vector3d &a1);


void build_shadow_volume(quad_shadow_volume &volume, const vector3d &camera_position);
void add_active_occluder(const quad &occluder, const vector3d &camera_position);


bool find_occluding_volume(const vector3d &center, float radius, int &volume_index);
bool sphere_occluded(const vector3d &center, float radius, int score = 0);

extern Var<int> minimum_occluder_score;
extern Var<int> quad_database_update_index;

//0x0095C880
extern Var<quad *> quad_database;

//0x0095C884
extern Var<int> quad_database_count;

//0x0095C87C
extern Var<bool> initialized;

//0x0095C88C
extern Var<int> num_active_shadow_volumes;

inline Var<quad_shadow_volume[14]> active_shadow_volumes{0x0095CC08};

inline Var<quad_shadow_volume *> active_shadow_volumes_scratchpad_mirror{0x0095C888};

}  // namespace occlusion
