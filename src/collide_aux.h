#pragma once

#include "float.hpp"

struct capsule;
struct sphere;
struct vector3d;

//0x005C3900
extern void compute_bounding_sphere_for_two_capsules(const capsule &cap0, const capsule &cap1, sphere *a3);

extern void merge_spheres(const vector3d &a1, Float a2, const vector3d &a3, Float a4, vector3d &center, float &radius);

extern bool sub_5B8F40(const vector3d &a1, const vector3d &a2, const vector3d &a3, const vector3d &a4, float *t);

extern void closest_point_line_segment_line_segment(const vector3d &start_a, const vector3d &end_a,
    const vector3d &start_b, const vector3d &end_b, float *time_a, float *time_b);
extern void closest_point_line_segment_plane(const vector3d &start, const vector3d &end,
    const vector3d &plane_point, const vector3d &plane_normal,
    vector3d *segment_point, vector3d *plane_point_out);

extern void collide_aux_patch();
