#pragma once

struct limited_timer;
struct traffic_path_intersection;
struct intersection_manager_brew {
    int field_0{};
    limited_timer *timer{};
    int header_state{};
    int intersection_state{};
    int road_state{};
    int special_state{};
    char *cursor{};
    traffic_path_intersection ***special_output{};
    int index{};
};
