#pragma once

#include "vector3d.h"
#include <cstdint>

struct ai_quad_path_cell {
    vector3d field_0[4];
    ai_quad_path_cell **neighbors[4];
    std::uint8_t neighbor_counts[4];
    int field_44;
    int field_48;
    std::uint8_t field_4C;
    std::uint8_t field_4D;
    std::uint16_t field_4E;

    ai_quad_path_cell();

    //0x004648C0
    vector3d get_edge_midpoint(ai_quad_path_cell *a3);
    vector3d get_midpoint() const;
    float fast_distance_check(const ai_quad_path_cell &other) const;
    ai_quad_path_cell *get_edge_neighbor(int edge, int index) const;
    bool is_point_in_cell(const vector3d &position, float radius) const;
    float is_point_near_cell(const vector3d &position) const;
    bool find_intersection_point_in_cell_along_line(const vector3d &start, const vector3d &end, vector3d *intersection,
                                                    vector3d *vertex) const;
    vector3d closest_point(const vector3d &position) const;

    //0x00452C60
    void un_mash(void *a2, int a3, int a4);
};
