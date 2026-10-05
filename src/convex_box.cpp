#include "convex_box.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(convex_box, 0x78u);

bounding_box::bounding_box()
{
    this->field_0[0] = vector3d{3.4028235e38f, 3.4028235e38f, 3.4028235e38f};
    this->field_0[1] = vector3d{-3.4028235e38f, -3.4028235e38f, -3.4028235e38f};
}

bool convex_box::sub_55EDB0(const vector3d &a2, const vector3d &a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        const auto local_point = a2 - a3;
        for (const auto &plane : field_0) {
            if (local_point.z * plane.z + local_point.x * plane.x +
                local_point.y * plane.y - plane.w > 0.0f)
                return false;
        }
        return true;
    } else {
        return (bool)THISCALL(0x0055EDB0, this, &a2, &a3);
    }
}

bool convex_box::sub_55EE20(const vector3d &a2, const po &a3)
{
    return (bool)THISCALL(0x0055EE20, this, &a2, &a3);
}

bool convex_box::set_box_coords(const vector3d *a2)
{
    bool(__fastcall * func)(void *, void *edx, const vector3d *) = CAST(func, 0x00516810);
    return func(this, nullptr, a2);
}
