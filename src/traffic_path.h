#pragma once

#include <cstdint>

#include "actor.h"

struct traffic_path_lane;
struct traffic_ai_list;
struct traffic_path_graph;
struct traffic_path_road;

struct traffic_path_intersection {
    enum eDirection {};
    traffic_path_road *roads[4];
    traffic_path_graph *field_10;
    uint8_t field_14;
    uint8_t next_direction;
    int16_t field_16;

    traffic_ai_list *get_ai_list();
    bool remove_ai_from_intersection(vhandle_type<actor> actor_handle, int direction);
    void release_semaphore(bool direction);
    bool reserve_turn(int direction, bool incoming);
    void release_turn(int direction, bool incoming);
    int get_ai_index(vhandle_type<actor> actor_handle);
    bool add_ai(vhandle_type<actor> actor_handle);
    bool reserve_stopsign(vhandle_type<actor> actor_handle, int direction);
    int get_next_direction(traffic_path_lane *lane, traffic_path_lane **next_lane,
                           int orientation, int excluded_direction, const vector3d &target,
                           bool ignore_restrictions, bool randomize);
    traffic_path_lane *get_next_lane(vector3d position, int direction, traffic_path_lane *lane,
                                     traffic_path_graph **graph, int orientation, bool flag);
    bool get_allowed_ai_roads(traffic_path_lane *lane, traffic_path_road **out_roads);
    int get_direction_to_lane(traffic_path_lane *lane, traffic_path_lane *next_lane);
    float evaluate_road_chance(traffic_path_lane *lane, traffic_path_lane **next_lane, float bias,
                               int road_index, int excluded_direction, int direction,
                               const vector3d &target, bool weigh_flags, bool randomize);

    bool has_stopsign(bool a1);
};

struct traffic_path_road {
    traffic_path_lane **in_lanes;
    uint32_t total_in_lanes;
    traffic_path_lane **out_lanes;
    uint32_t total_out_lanes;
    traffic_path_lane **all_lanes;
    uint8_t n2;
    uint8_t n2_1;
    uint8_t flags;
    uint8_t padding;
    traffic_path_intersection *field_18;
    traffic_path_intersection *field_1C;

    traffic_path_intersection *get_previous_intersection()
    {
        return this->field_1C;
    }

    traffic_path_intersection *get_next_intersection()
    {
        return this->field_18;
    }

    //0x005C0EE0
    bool is_an_in_lane(const traffic_path_lane *a2) const;
    traffic_path_lane *get_lane(int index, int type, bool incoming) const;
    int count_lanes(int type, bool incoming) const;
    int get_lane_index(const traffic_path_lane *lane) const;
    int get_lane_position(const traffic_path_lane *lane) const;
    traffic_path_lane *get_indexed_lane(int index, int type) const;
    traffic_path_lane *get_closest_lane(const vector3d &position, int type) const;
    int map_lane_index(traffic_path_lane *lane, const traffic_path_road *next_road,
                       int index, bool allow_incoming) const;

    static bool road_is_valid(const traffic_path_road *a1);
};

struct traffic_ai_list {
    uint16_t num_ais;
    uint16_t field_2;
    vhandle_type<actor> ais[20];
    void *owner;
    uint16_t type;
    uint16_t padding;

    void remove_ai(vhandle_type<actor> actor_handle);
    void release();
    static int allocate(uint16_t type, void *owner);

    int get_ai_index(vhandle_type<actor> me);

    static traffic_ai_list (&ai_lists)[64];
};
