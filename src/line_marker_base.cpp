#include "line_marker_base.h"

#include "common.h"
#include "func_wrapper.h"
#include "oldmath_po.h"

VALIDATE_SIZE(line_marker_base, 0x84);

line_marker_base::line_marker_base(const string_hash &a2, uint32_t a3) : marker(a2, a3)
{
    THISCALL(0x0050A6E0, this, &a2, a3);
}

vector3d line_marker_base::get_target()
{
    auto &abs_po = this->get_abs_po();

    auto result = abs_po.slow_xform(this->field_68);
    return result;
}

vector3d line_marker_base::get_origin()
{
    auto &abs_po = this->get_abs_po();

    auto result = abs_po.slow_xform(this->field_74);
    return result;
}
