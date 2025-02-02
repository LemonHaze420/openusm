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
        vector3d &a4);

    //virtual
    void do_spawn(
            vector3d a4,
            vector3d a2,
            traffic_path_lane *lane,
            bool a10,
            bool a11);

    //virtual
    void un_spawn();

    //virtual
    bool is_viable_pos(const vector3d &a2);

    //virtual
    actor *get_my_actor();

    //0x006C9990
    traffic_path_graph::laneInfoStruct * get_spawnable_lane(
        traffic_path_lane::eLaneType arg0,
        vector3d *arg4,
        traffic_path_graph **arg8,
        traffic_path_graph **argC,
        int *a6,
        bool a7,
        bool a8,
        bool a9);

    //0x006CCD10
    traffic_path_graph::laneInfoStruct * get_new_spawn_pos(
        traffic_path_lane::eLaneType a1,
        vector3d *a2,
        traffic_path_graph **a3,
        traffic_path_graph **a4,
        int *a5);

    void sub_6B9B60(Float a2);

    static bool should_update_spawn_lanes(po &a1, entity_base *ent);

    static void update_spawn_lanes();

    //0x006D8610
    static void advance_traffic_and_peds(Float a1);

    static _std::vector<traffic_path_graph::laneInfoStruct> *& spawnable_lanes;

    static traffic_path_graph::laneInfoStruct *&last_spawn_lane_info;

    static po &last_camera_po;

    static float &spawn_spacing;
};

extern void sub_6C2E10(Float a1);

extern void spawnable_patch();
