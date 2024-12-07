#pragma once

#include "spawnable.h"

#include "entity.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "variable.h"
#include "vector3d.h"
#include "vehicle.h"

#include <vector.hpp>

struct traffic_path_lane;
struct actor;

struct traffic : spawnable {
    int field_8;
    vehicle field_C;
    int field_13C;
    traffic_path_lane *field_140;
    traffic_path_lane *field_144;
    traffic_path_lane *field_148;
    vector3d field_14C;

    bool field_158;
    int field_15C[41];

    bool field_200;
    bool field_201;
    bool field_202;
    bool field_203;
    int field_204;
    int field_208;
    int field_20C;
    int field_210;
    int field_214;
    int field_218;
    int field_21C;
    int field_220;
    int field_224;
    int field_228;

    traffic_path_lane * get_current_lane()
    {
        return field_140;
    }

    void sub_6DA3B0(Float a2, Float a3, Float a4);

    void set_current_lane(traffic_path_lane *a2, int a3, bool a4);

    //virtual
    //0x006D9070
    void _do_spawn(
            vector3d a4,
            vector3d a2,
            traffic_path_lane *lane,
            int a9,
            bool a10,
            bool a11);

    static void initialize_traffic();

    static void terminate_traffic();

    static void enable_traffic(bool a1, bool a2);

    //0x006D0910
    static void get_closest_point_on_lane_with_facing(vector3d *a1, vector3d *a2, bool a3);

    //0x004ADE20
    static bool is_unanimated_car(actor *a1);

    //0x006C3510
    static traffic *get_traffic_from_entity(vhandle_type<entity> a1);

    static inline Var<bool> traffic_enabled {0x00937FF4};

    static inline Var<bool> traffic_initialized {0x0096C9E8};

    static inline Var<_std::vector<traffic *>> traffic_list {0x0096D2A8};
};

extern void traffic_patch();
