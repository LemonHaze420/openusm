#include "fe_mini_map_dot.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "panelquad.h"
#include "vtbl.h"

VALIDATE_SIZE(fe_mini_map_dot, 0x2C);

fe_mini_map_dot::fe_mini_map_dot(mini_map_dot_type a2, vector3d a3)
{
    THISCALL(0x0063AB90, this, a2, a3);
}

void fe_mini_map_dot::Draw()
{
    if (this->field_24 && this->field_25) {
        this->field_0->Draw();

        if (this->field_8 != nullptr) {
            nglListAddQuad(this->field_8);
        }

        if (this->field_C != nullptr) {
            nglListAddQuad(this->field_C);
        }

        if (this->field_10 != nullptr) {
            nglListAddQuad(this->field_10);
        }

        if (this->highlight_circle_count_down != 0) {
            this->field_4->Draw();
        }
    }
}
