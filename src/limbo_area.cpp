#include "limbo_area.h"

#include "utility.h"

bool limbo_area::sphere_intersects_unsafe_area(const vector3d &a1, Float a2)
{
    bool(__cdecl * func)(const vector3d *, Float) = CAST(func, 0x0052EB60);
    return func(&a1, a2);
}
