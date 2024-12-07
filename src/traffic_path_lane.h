#pragma once

#include <cstdint>

struct entity;
struct vector3d;
struct traffic_path_graph;
struct traffic_path_road;

struct traffic_path_lane {
    enum eLaneType {
    };

    vector3d *nodes;
    traffic_path_road *my_road;
    uint16_t total_nodes;
    uint16_t flags;

    bool is_valid(traffic_path_graph *a2) const;

    int get_type() const;

    int get_num_nodes() const {
        return this->total_nodes;
    }

    vector3d get_node(int a3) const;

    //0x005E2040
    vector3d get_directional_node() const;

    void sub_5CCFB0(entity *ent, int a3);

    vector3d get_node_before_point(const vector3d &a3, int *a4);

    void seed_with_pedestrians(entity &ent, int a3);

    static bool lane_is_valid(const traffic_path_lane *a1);
};

extern void traffic_path_lane_patch();
