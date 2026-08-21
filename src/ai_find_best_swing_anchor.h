#pragma once

#include "float.hpp"

#include "fixed_vector.h"
#include "quick_anchor_info.h"
#include "variable.h"
#include "vector4d.h"

struct conglomerate_clone;
struct entity;
struct find_best_anchor_result_t;
struct vector3d;

struct sweet_cone_t;

namespace local_collision {
struct primitive_list_t;
}

struct swing_anchor_finder {
    float field_0;
    float field_4;
    float sweet_spot_distance;
    float field_C;
    float field_10;
    float web_min_length;
    float web_max_length;
    float field_1C;
    float field_20;

    swing_anchor_finder() = default;

    swing_anchor_finder(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6);

    void set_max_pull_length(Float length);

    //0x00464590
    bool accept_swing_point(const quick_anchor_info &info, const sweet_cone_t &sweet_cone, const Float &a4, Float a5,
                            float *arg10, local_collision::primitive_list_t **a7,
                            local_collision::primitive_list_t ***a8) const;

    //0x00486280
    bool find_best_offset_anchor(entity *self, const vector3d &a3, const vector3d &a4,
                                 find_best_anchor_result_t *result) const;

    //0x00487D20
    bool find_best_anchor(entity *a1, const vector3d &a2, find_best_anchor_result_t *a3) const;

    //0x004647D0
    void find_sweet_spot(entity *ent, const vector3d &a5, const vector3d &a6, vector3d *sweet_spot) const;

    static void remove_all_anchors();
};

inline constexpr auto MAX_3DGRID_BITS = 32;

struct occupancy_voxels_t {
    vector4d field_0;
    vector4d field_10;
    vector4d field_20;
    vector4d field_30;
    vector4d field_40;
    vector4d field_50;
    uint32_t field_60[MAX_3DGRID_BITS * MAX_3DGRID_BITS];

    void init(const vector3d &a2, const vector3d &a3);

    bool is_occupied_if_not_occupy(const vector4d &a2);

    void map_vector3d(const vector4d &a2, int &minx, int &miny, int &minz, int &maxx, int &maxy, int &maxz);
};

struct quick_anchor_container_t {
    fixed_vector<quick_anchor_info, 100> field_0;

    void add_anchor(occupancy_voxels_t *grid, const vector3d &a3, const vector3d &a4, const vector3d &a5, Float a6,
                    entity *a7, conglomerate_clone *a8);
};

inline Var<bool> g_anchor_finding_enabled{0x0091F4D0};

extern void swing_anchor_finder_patch();
