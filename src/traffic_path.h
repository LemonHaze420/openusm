#pragma once

#include <cstdint>

#include "actor.h"

struct traffic_path_lane;

struct traffic_path_intersection {

    enum eDirection {};

    bool has_stopsign(bool a1);

};

struct traffic_path_road {

    traffic_path_lane **in_lanes;
    uint32_t total_in_lanes;
    traffic_path_lane **out_lanes;
    uint32_t total_out_lanes;
    traffic_path_lane **all_lanes;
    int field_14;
    traffic_path_intersection *field_18;
    traffic_path_intersection *field_1C;

    traffic_path_intersection * get_previous_intersection()
    {
        return this->field_1C;
    }

    traffic_path_intersection * get_next_intersection()
    {
        return this->field_18;
    }

    //0x005C0EE0
    bool is_an_in_lane(const traffic_path_lane *a2) const;

    static bool road_is_valid(const traffic_path_road *a1);
};

struct traffic_ai_list {

    uint16_t num_ais;
    uint16_t field_2;
    vhandle_type<actor> ais[20];

    int get_ai_index(vhandle_type<actor> me);

    static traffic_ai_list (&ai_lists)[1];
};
