#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "traffic_path_graph.h"
#include "variable.h"

#include <cstdint>

struct actor;

struct spawnable {
    std::intptr_t m_vtbl;
    bool field_4;
    bool field_5;

    spawnable(vhandle_type<entity>);

    vector3d prepare_for_spawn(
        traffic_path_graph::laneInfoStruct *next_lane_struct,
        vector3d &a4,
        int a5);

    //virtual
    void do_spawn(
            vector3d a4,
            vector3d a2,
            traffic_path_lane *lane,
            int a9,
            bool a10,
            bool a11);

    //virtual
    void un_spawn();

    //virtual
    bool is_viable_pos(const vector3d &a2);

    //virtual
    actor *get_my_actor();

    //0x006D8610
    static void advance_traffic_and_peds(Float a1);

    static inline Var<_std::vector<traffic_path_graph::laneInfoStruct> *> 
        spawnable_lanes {0x0096C9B0};

    static traffic_path_graph::laneInfoStruct *&last_spawn_lane_info;
};

extern void spawnable_patch();
