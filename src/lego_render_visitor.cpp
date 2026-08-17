#include "lego_render_visitor.h"

#include "func_wrapper.h"
#include "hull.h"
#include "region.h"
#include "vector3d.h"

lego_render_visitor::lego_render_visitor(region *a2, const hull *a3, const vector3d &a4, Float a5) {
    void (__fastcall *func)(lego_render_visitor *, void *edx, region *a2, const hull *a3, const vector3d *a4, Float a5) = CAST(func, 0x0052DEC0);
    func(this, nullptr, a2, a3, &a4, a5);
}

