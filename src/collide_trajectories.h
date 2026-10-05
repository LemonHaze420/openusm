#pragma once

#include "sphere.h"

struct intraframe_trajectory_t;
struct capsule;
namespace local_collision {
struct primitive_list_t;
struct closest_points_pair_t;
}  // namespace local_collision

extern bool __fastcall swept_capsule_intersection(const capsule &end, const capsule &start, float duration,
                                                  float radius, local_collision::primitive_list_t *primitives,
                                                  float *time, local_collision::closest_points_pair_t *pair,
                                                  bool *tunnelled, bool skip_base);

extern sphere compute_bounding_sphere_for_trajectory_and_intersected_trajectories(intraframe_trajectory_t *trj);

extern void resolve_rotations(intraframe_trajectory_t *trj, int a1);

extern void resolve_collisions(intraframe_trajectory_t **a1, Float a2);

extern void resolve_moving_pendulums(intraframe_trajectory_t *a1, Float a2);
