#include "glass_house.h"

#include "color32.h"
#include "common.h"
#include "debug_render.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "vector3d.h"

void glass_house::render()
{
    TRACE("glass_house::render");

    for (auto i = 0; i < obbs.size(); ++i)
    {
        auto *v7 = obbs.at(i);

        vector3d a1[8]{};
        v7->get_vertices(a1);
        color32 a3{255, 0, 0, 196};
        color32 v4{255, 0, 255, 64};
        render_beam(a1[0], a1[1], a3, 0.2, 0);
        render_beam(a1[1], a1[2], a3, 0.2, 0);
        render_beam(a1[2], a1[3], a3, 0.2, 0);
        render_beam(a1[3], a1[0], a3, 0.2, 0);
        render_beam(a1[4], a1[5], a3, 0.2, 0);
        render_beam(a1[5], a1[6], a3, 0.2, 0);
        render_beam(a1[6], a1[7], a3, 0.2, 0);
        render_beam(a1[7], a1[4], a3, 0.2, 0);
        render_beam(a1[0], a1[4], a3, 0.2, 0);
        render_beam(a1[1], a1[5], a3, 0.2, 0);
        render_beam(a1[2], a1[6], a3, 0.2, 0);
        render_beam(a1[3], a1[7], a3, 0.2, 0);

#if 0
        sub_684B49((int)a1, (int)&a1[1], (int)&a1[2], v4.field_0[0], 1);
        sub_684B49((int)a1, (int)&a1[2], (int)&a1[3], v4.field_0[0], 1);
        sub_684B49((int)&a1[4], (int)&a1[5], (int)&a1[6], v4.field_0[0], 1);
        sub_684B49((int)&a1[4], (int)&a1[6], (int)&a1[7], v4.field_0[0], 1);
        sub_684B49((int)a1, (int)&a1[1], (int)&a1[5], v4.field_0[0], 1);
        sub_684B49((int)a1, (int)&a1[5], (int)&a1[4], v4.field_0[0], 1);
        sub_684B49((int)&a1[2], (int)&a1[6], (int)&a1[7], v4.field_0[0], 1);
        sub_684B49((int)&a1[2], (int)&a1[3], (int)&a1[7], v4.field_0[0], 1);
        sub_684B49((int)a1, (int)&a1[3], (int)&a1[7], v4.field_0[0], 1);
        sub_684B49((int)a1, (int)&a1[7], (int)&a1[4], v4.field_0[0], 1);
        sub_684B49((int)&a1[1], (int)&a1[2], (int)&a1[6], v4.field_0[0], 1);
        sub_684B49((int)&a1[1], (int)&a1[5], (int)&a1[6], v4.field_0[0], 1);
        sub_6A5D08(v2, 0);
#endif
    }
}

