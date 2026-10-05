#include "trajectory_cluster.h"
#include "common.h"
#include "dirty_sphere.h"

VALIDATE_SIZE(trajectory_cluster_t, 0x5C);
VALIDATE_SIZE(dirty_sphere_t, 0x18);

trajectory_cluster_t::trajectory_cluster_t(intraframe_trajectory_t *list, float remaining, float collision, int count)
    : trajectories(list), next(nullptr), remaining_time(remaining), collision_time(collision), first_collision(nullptr),
      pair{}, tunnelled(false), iteration(count), dirty_spheres(nullptr)
{}
