#include "panelquadsection.h"

#include "func_wrapper.h"
#include "utility.h"
#include "vector2d.h"
#include "color32.h"
#include "common.h"

VALIDATE_SIZE(PanelQuadSection, 0x7C);

PanelQuadSection::PanelQuadSection()
{

}

void PanelQuadSection::Init(vector2d *pos, vector2d *uv, color32 *a4, Float a5) {
    if constexpr (1) {
        for (auto i = 0; i < 4; ++i) {
            this->field_10[i] = a4[i].get_alpha();
            this->field_0[i] = pos[i][0] + 0.5f;
            this->field_8[i] = pos[i][1] + 0.5f;
            nglSetQuadVPos((nglQuad *) &this->field_14, i, pos[i][0], pos[i][1]);
            nglSetQuadVUV((nglQuad *) &this->field_14, i, uv[i][0], uv[i][1]);
            nglSetQuadVColor((nglQuad *) &this->field_14, i, color32::to_int(a4[i]));
        }

        nglSetQuadZ((nglQuad *) &this->field_14, a5);
        nglSetQuadMapFlags((nglQuad *) &this->field_14, 194u);

    } else {
        THISCALL(0x00615FB0, this, pos, uv, a4, a5);
    }
}

void PanelQuadSection_patch() {
    FUNC_ADDRESS(address, &PanelQuadSection::Init);
    REDIRECT(0x0062E416, address);
}
