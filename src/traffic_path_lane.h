#pragma once

#include <cstdint>

#include "actor.h"
#include "entity_base_vhandle.h"

struct entity;
struct vector3d;
struct traffic_path_graph;
struct traffic_path_road;
struct traffic_path_intersection;

struct traffic_path_lane {
    enum eLaneType {};

    vector3d *nodes;
    traffic_path_road *my_road;
    float lane_length;
    uint16_t total_nodes;
    uint16_t flags;
    int field_10;

    bool is_valid(traffic_path_graph *a2) const;

    entity_base_vhandle get_ai_by_index(int a3) const;

    int get_ai_index(vhandle_type<actor> a2);

    //0x005C8320
    void remove_ai_from_lane(vhandle_type<actor> a2);

    //0x005BFEA0
    void update_lane_indexes();

    traffic_path_intersection *get_next_intersection(int a2);

    int get_type() const;

    int get_num_nodes() const
    {
        return this->total_nodes;
    }

    int get_num_ais();

    traffic_path_intersection *sub_5C8460();

    vector3d get_node(int a3) const;

    //0x005E2040
    vector3d get_directional_node() const;

    void sub_5CCFB0(entity *ent, int a3);

    vector3d get_node_before_point(const vector3d &a3, int *a4);

    void seed_with_pedestrians(entity &ent, int a3);

    static bool lane_is_valid(const traffic_path_lane *a1);
};

extern void traffic_path_lane_patch();
