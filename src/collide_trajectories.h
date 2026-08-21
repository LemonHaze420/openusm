#pragma once

#include "sphere.h"

struct intraframe_trajectory_t;

extern sphere compute_bounding_sphere_for_trajectory_and_intersected_trajectories(intraframe_trajectory_t *trj);

extern void resolve_rotations(intraframe_trajectory_t *trj, int a1);

extern void resolve_collisions(intraframe_trajectory_t **a1, Float a2);

extern void resolve_moving_pendulums(intraframe_trajectory_t *a1, Float a2);
