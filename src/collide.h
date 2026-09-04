#pragma once

#include "float.hpp"

struct vector3d;
struct po;

namespace local_collision {
struct entfilter_base;
struct obbfilter_base;
}  // namespace local_collision

struct subdivision_node_obb_base;
struct entity;

//0x005B9F30
extern bool find_sphere_intersection(const vector3d &a1, Float a2, const local_collision::entfilter_base &a3,
                                     const local_collision::obbfilter_base &a4, vector3d *arg10, vector3d *a5,
                                     entity **a7, subdivision_node_obb_base **a8);

//0x005B9430
extern bool collide_segment_solid_sphere(const vector3d &a1, const vector3d &a2, const vector3d &a3, Float radius,
                                         vector3d *hit_loc);

//0x005B9660
extern int collide_segment_hollow_sphere(const vector3d &a1, const vector3d &a2, const vector3d &a3, Float a4,
                                         vector3d *a5);

//0x005CB740
extern bool collide_segment_entity(const vector3d &a2, const vector3d &a3, const entity *a4, const po &a5, vector3d *a6,
                                   vector3d *a7);

extern bool collide_sphere_entity(const vector3d &a1, Float a2, const entity *ent, vector3d *a4, vector3d *a5, po *a6);

//0x005C47A0
extern bool collide_capsule_capsule(const vector3d &a1, const vector3d &a2, Float radius1, const vector3d &a4,
                                    const vector3d &a5, Float radius2, vector3d &cp1, vector3d &cp2, vector3d &normal);

extern bool closest_point_segment(const vector3d &a1, const vector3d &a2, const vector3d &a3, vector3d &a4);

extern void collide_unit_test();
