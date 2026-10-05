#pragma once

#include "ngl_math.h"

struct rigid_body;
struct phys_vector3d;

struct nuge {
    static void calc_box_inertia(const phys_vector3d &dimensions,
        phys_vector3d &inertia, float &mass);
    static void calc_velocities(const matrix4x4 &previous, const matrix4x4 &current,
        const phys_vector3d &local_pivot, float elapsed,
        phys_vector3d &linear, phys_vector3d &angular);
    static void calc_velocities(const matrix4x4 &previous, const matrix4x4 &current,
        float elapsed, phys_vector3d &linear, phys_vector3d &angular);
    static void get_ballistic_info(rigid_body *const *a1, int a2, math::VecClass<3, 1> *a3, math::VecClass<3, 0> *a4,
                                   float *a5);
};
