#pragma once

#include "fixed_pool.h"
#include "local_collision.h"
#include "sphere.h"
#include "variable.h"

struct intraframe_trajectory_t;
struct dirty_sphere_t;

struct trajectory_cluster_t {
    intraframe_trajectory_t *trajectories;
    trajectory_cluster_t *next;
    float remaining_time;
    float collision_time;
    intraframe_trajectory_t *first_collision;
    local_collision::closest_points_pair_t pair;
    bool tunnelled;
    int iteration;
    dirty_sphere_t *dirty_spheres;

    trajectory_cluster_t(intraframe_trajectory_t *list, float remaining, float collision, int iteration = 0);
    static inline Var<fixed_pool> pool{0x009375A4};
};
