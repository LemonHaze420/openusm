#include "glass_house.h"

#include "color32.h"
#include "common.h"
#include "debug_render.h"
#include "mash_info_struct.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "vector3d.h"

VALIDATE_SIZE(glass_house, 0x14);

glass_house::glass_house(from_mash_in_place_constructor *a2) : field_0(a2), obbs(a2) {
}

void glass_house::render()
{
    TRACE("glass_house::render");

    for (auto i = 0; i < obbs.size(); ++i)
    {
        auto &v7 = obbs.at(i);

        vector3d a1[8]{};
        v7.get_vertices(a1);
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

void glass_house::destruct_mashed_class()
{
    this->field_0.destruct_mashed_class();
    this->obbs.destruct_mashed_class();
}

void glass_house::unmash(mash_info_struct *a1, void *) {
    TRACE("glass_house::unmash");

    a1->unmash_class_in_place(this->field_0, nullptr);
    a1->unmash_class_in_place(this->obbs, nullptr);
}

