#include "panelquadsection.h"

#include "func_wrapper.h"
#include "utility.h"
#include "vector2d.h"
#include "color32.h"
#include "common.h"
#include "matrix4x4.h"
#include "vector3d.h"

VALIDATE_SIZE(PanelQuadSection, 0x7C);


PanelQuadSection::PanelQuadSection() : field_78(false)
{
    nglInitQuad(reinterpret_cast<nglQuad *>(&field_14));
    field_78 = true;
}

PanelQuadSection::PanelQuadSection(from_mash_in_place_constructor *)
{
    this->field_78 = true;
}

void PanelQuadSection::Init(vector2d *pos, vector2d *uv, color32 *a4, Float a5)
{
    if constexpr (1) {
        for (auto i = 0; i < 4; ++i) {
            this->field_10[i] = a4[i].get_alpha();
            this->field_0[i] = pos[i][0] + 0.5f;
            this->field_8[i] = pos[i][1] + 0.5f;
            nglSetQuadVPos((nglQuad *)&this->field_14, i, pos[i][0], pos[i][1]);
            nglSetQuadVUV((nglQuad *)&this->field_14, i, uv[i][0], uv[i][1]);
            nglSetQuadVColor((nglQuad *)&this->field_14, i, color32::to_int(a4[i]));
        }

        nglSetQuadZ((nglQuad *)&this->field_14, a5);
        nglSetQuadMapFlags((nglQuad *)&this->field_14, 194u);

    } else {
        THISCALL(0x00615FB0, this, pos, uv, a4, a5);
    }
}

color32 PanelQuadSection::GetColor(int a3) const
{
    color32 result = this->field_14.field_0[a3].m_color;
    return result;
}

void PanelQuadSection::sub_608EF0(float *a2, float *a3)
{
    a2[0] = (float)this->field_0[0];
    a2[1] = (float)this->field_0[1];
    a2[2] = (float)this->field_0[2];
    a2[3] = (float)this->field_0[3];

    a3[0] = (float)this->field_8[0];
    a3[1] = (float)this->field_8[1];
    a3[2] = (float)this->field_8[2];
    a3[3] = (float)this->field_8[3];
}

void PanelQuadSection::Animate(const matrix4x4 &transform, float z, bool relative)
{
    auto *quad = reinterpret_cast<nglQuad *>(&field_14);
    for (int vertex = 0; vertex < 4; ++vertex) {
        const float x = relative ? quad->field_0[vertex].pos.x : static_cast<float>(field_0[vertex]);
        const float y = relative ? quad->field_0[vertex].pos.y : static_cast<float>(field_8[vertex]);
        const vector3d transformed = transform * vector3d{x, y, z};
        nglSetQuadVPos(quad, vertex, transformed[0], transformed[1]);
    }
}

// 0x006081B0
void PanelQuadSection::Mask(float amount, int direction, float uv_extent, float scale)
{
    if (direction == 0)
        return;
    if (amount > 1.0f)
        amount = 2.0f - amount;

    auto *quad = reinterpret_cast<nglQuad *>(&field_14);
    const float left = quad->field_0[0].pos.x;
    const float top = quad->field_0[0].pos.y;
    const float right = quad->field_0[3].pos.x;
    const float bottom = quad->field_0[3].pos.y;
    const float extent = (direction == 1 || direction == 2) ? static_cast<float>(field_0[3] - field_0[0]) * scale
                                                            : static_cast<float>(field_8[3] - field_8[0]) * scale;

    float masked_left = left;
    float masked_right = right;
    float masked_top = top;
    float masked_bottom = bottom;
    if (direction == 1)
        masked_left = right - amount * extent;
    else if (direction == 2)
        masked_right = left + amount * extent;
    else if (direction == 3)
        masked_bottom = top + amount * extent;
    else if (direction == 4)
        masked_top = bottom - amount * extent;

    nglSetQuadVPos(quad, 0, masked_left, masked_top);
    nglSetQuadVPos(quad, 1, masked_right, masked_top);
    nglSetQuadVPos(quad, 2, masked_left, masked_bottom);
    nglSetQuadVPos(quad, 3, masked_right, masked_bottom);

    if (uv_extent <= 0.0f)
        return;
    const float u_left = quad->field_0[0].uv.field_0;
    const float v_top = quad->field_0[0].uv.field_4;
    const float u_right = quad->field_0[3].uv.field_0;
    const float v_bottom = quad->field_0[3].uv.field_4;
    float masked_u_left = u_left;
    float masked_u_right = u_right;
    float masked_v_top = v_top;
    float masked_v_bottom = v_bottom;
    if (direction == 1)
        masked_u_left = u_right - amount * uv_extent;
    else if (direction == 2)
        masked_u_right = u_left + amount * uv_extent;
    else if (direction == 3)
        masked_v_bottom = v_top + amount * uv_extent;
    else if (direction == 4)
        masked_v_top = v_bottom - amount * uv_extent;

    nglSetQuadVUV(quad, 0, masked_u_left, masked_v_top);
    nglSetQuadVUV(quad, 1, masked_u_right, masked_v_top);
    nglSetQuadVUV(quad, 2, masked_u_left, masked_v_bottom);
    nglSetQuadVUV(quad, 3, masked_u_right, masked_v_bottom);
}

void PanelQuadSection_patch()
{
    FUNC_ADDRESS(address, &PanelQuadSection::Init);
    REDIRECT(0x0062E416, address);
}
