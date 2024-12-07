#pragma once

#include "traffic_path_lane.h"

struct region;
struct traffic_path_brew;
struct vector3d;

struct traffic_path_graph {
    struct laneInfoStruct {
        traffic_path_graph *field_0;
        traffic_path_lane *field_4;
        int field_8;
        char field_C;
        char field_D;
        bool field_E;
        char field_F;
    };

    traffic_path_graph();

    //0x005CE2D0
    traffic_path_lane *get_closest_or_farthest_lane(bool arg0,
                                                    const vector3d &a1,
                                                    const vector3d &arg8,
                                                    vector3d *a5,
                                                    traffic_path_lane::eLaneType a6,
                                                    bool a7,
                                                    float *a8);

    //0x005C7E20
    bool un_mash(char *a2, int *a3, region *a4, traffic_path_brew &a5);

    void release_mem();
};
