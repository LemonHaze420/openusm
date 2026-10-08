#pragma once

#include <cstdint>

struct region;
struct ai_quad_path_cell;
struct vector3d;

struct ai_quad_path {
    int field_0[6];

    region *field_18;
    uint16_t field_1C;
    uint16_t field_1E;
    int field_20;
    ai_quad_path_cell *field_24;
    uint16_t field_28;
    uint16_t field_2A;
    int field_2C;
    int field_30;

    ai_quad_path();
    bool find_exit_to_district(int district, int path, const vector3d &position, vector3d &exit_position,
                               ai_quad_path_cell *&exit_cell) const;
    ai_quad_path_cell *find_exit_cell_to_path(const ai_quad_path &path, const vector3d &position,
                                              vector3d &exit_position) const;
    bool check_points_in_cells(const vector3d &position, float tolerance, ai_quad_path_cell **inside,
                               ai_quad_path **nearest_path, ai_quad_path_cell **nearest_cell, float *nearest_distance);

    //0x00464FA0
    void un_mash(void *buffer_ptr, region *reg, int a4, int a5, int a6);
};
