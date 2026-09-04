#pragma once

#include "float.hpp"

struct capsule;
struct sphere;
struct vector3d;

//0x005C3900
extern void compute_bounding_sphere_for_two_capsules(const capsule &cap0, const capsule &cap1, sphere *a3);

extern void merge_spheres(const vector3d &a1, Float a2, const vector3d &a3, Float a4, vector3d &center, float &radius);

extern bool sub_5B8F40(const vector3d &a1, const vector3d &a2, const vector3d &a3, const vector3d &a4, float *t);

extern void collide_aux_patch();
