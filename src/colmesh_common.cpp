#include "colmesh_common.h"
#include "subdivision_obb.h"

bool segment_mesh_box_overlap(const vector3d &start, const vector3d &end, const collision_obb_t &box)
{
    const vector3d to_center = box.field_0 - start;
    const vector3d from_center = end - box.field_0;
    const vector3d &x = box.field_10;
    const vector3d &y = box.axis_y;
    const vector3d &z = box.axis_z;

    return collision_segment_box_overlap({dot(to_center, x), dot(to_center, y), dot(to_center, z)},
                                         {dot(from_center, x), dot(from_center, y), dot(from_center, z)},
                                         {x.length2(), y.length2(), z.length2()});
}
