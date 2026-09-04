#pragma once

#include "entity_base_vhandle.h"
#include "float.hpp"
#include "mstring.h"
#include "vector3d.h"

#include <cstdint>

#include <vector.hpp>

struct region;
struct ai_quad_path;
struct ai_quad_path_cell;

struct ai_path {
    struct eAIPathStatus {
        uint32_t field_0;
    };

    _std::vector<region *> field_0;
    _std::vector<void *> field_10;
    _std::vector<ai_quad_path *> field_20;
    _std::vector<ai_quad_path_cell *> field_30;

    vector3d field_40;
    vector3d field_4C;
    vector3d field_58;
    vector3d field_64;
    vector3d field_70;
    float field_7C;
    float field_80;
    ai_quad_path_cell *field_84;
    entity_base_vhandle field_88;
    eAIPathStatus m_pathStatus;
    mString field_90;
    bool field_A0;

    //0x0047F7D0
    ai_path();

    //0x00479B10
    ~ai_path();

    ai_quad_path_cell *advance_to_next_cell();

    //0x0048A170
    ai_quad_path_cell *advance_to_farthest_direct_cell();

    //0x004899A0
    bool populate_quad_path_cell_route();

    //0x00489C70
    void setup(entity_base_vhandle a2, const vector3d &a3, const vector3d &a4, bool a5, Float a6);

    //0x0048A420
    vector3d get_next_point();

    //0x00479ED0
    static void frame_advance_all_ai_paths(Float a1);

    //0x0048A540
    static bool can_path_between_points(const vector3d &a1, const vector3d &a2, Float a6);

    //0x00487F10
    static bool find_region_route(region *a1, region *a2, _std::vector<region *> *route);

    //0x00479C50
    static region *find_region_for_point(const vector3d &a1, Float a2);

    //0x00452CF0
    static void set_status(ai_path *a1, ai_path::eAIPathStatus a2, const char *Format, ...);
};

extern void ai_path_patch();
