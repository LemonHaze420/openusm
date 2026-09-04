#include "convex_box.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(convex_box, 0x78u);

bounding_box::bounding_box()
{
    this->field_0[0] = vector3d{3.4028235e38, 3.4028235e38, 3.4028235e38};
    this->field_0[1] = vector3d{-3.4028235e38, -3.4028235e38, -3.4028235e38};
}

bool convex_box::sub_55EDB0(const vector3d &a2, const vector3d &a3)
{
    return (bool)THISCALL(0x0055EDB0, this, &a2, &a3);
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
