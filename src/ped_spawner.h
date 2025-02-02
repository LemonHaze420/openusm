#pragma once

#include "spawnable.h"

#include "actor.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "vector3d.h"

#include <vector.hpp>

struct actor;
struct traffic_path_lane;
struct traffic_path_intersection;

struct ped_spawner : spawnable {
    int field_8;
    uint32_t field_C;
    traffic_path_lane *field_10;
    traffic_path_lane *field_14;
    traffic_path_intersection *field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    vector3d field_30;
    vhandle_type<actor> field_3C;
    bool field_40;
    int field_44;
    int field_48;

    ped_spawner(int a2);

    actor *create_ped_actor();

    //virtual
    void do_spawn(
        vector3d a2,
        vector3d a3,
        traffic_path_lane *a8,
        int a9,
        int a10,
        int a11);

    //virtual
    actor *get_my_actor();

    //virtual
    void spawn(vector3d a4, const vector3d &a2);

    void sub_6BBD30(traffic_path_lane *a2);

    void sub_6C2EA0(Float a2);

    void sub_6B9B60(Float a2);

    static void init();

    static void cleanup();

    static void advance_peds(Float a1);

    static void populate_lanes();

    static ped_spawner * assign_non_ped_actor(vhandle_type<actor> a2, int a3);

    static inline ped_spawner *& next_ped_spawner = var<ped_spawner *>(0x0096C9BC);

    static _std::vector<ped_spawner *> &ped_spawner_list;

};

extern void ped_spawner_patch();
