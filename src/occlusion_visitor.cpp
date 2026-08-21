#include "occlusion_visitor.h"

occlusion_visitor::occlusion_visitor(const vector3d &a1, const vector3d &a3, Float a4, region *)
    : field_4(a1), field_10(a3), field_1C(a4)
{
    this->field_10.normalize();
}
