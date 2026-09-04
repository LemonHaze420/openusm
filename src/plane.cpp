#include "plane.h"

#include "common.h"
#include "vector3d.h"

#include <cmath>

VALIDATE_SIZE(plane, 0x10);

plane::plane(const vector3d &a2, const vector3d &a3)
{
    auto v1 = a3.normalized();

    this->arr[0] = v1[0];
    this->arr[1] = v1[1];
    this->arr[2] = v1[2];
    this->arr[3] = -dot(v1, a2);
}
