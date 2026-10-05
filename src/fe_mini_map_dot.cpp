#include "fe_mini_map_dot.h"

#include "common.h"
#include "femanager.h"
#include "fe_mini_map_widget.h"
#include "igofrontend.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "panelquad.h"
#include "vtbl.h"

VALIDATE_SIZE(fe_mini_map_dot, 0x2C);

// 0x0063AB90
fe_mini_map_dot::fe_mini_map_dot(mini_map_dot_type a2, vector3d)
    : field_0(nullptr), field_4(nullptr), field_8(nullptr), field_C(nullptr), field_10(nullptr), field_14(),
      field_20(a2), field_24(false), field_25(false), field_26(false), field_27(false), highlight_circle_count_down(0)
{
    uint32_t packed_color = 0xFFC8C8C8;
    switch (static_cast<int>(a2)) {
    case 0:
        packed_color = 0xFF527AB7;
        break;
    case 1:
        packed_color = 0xFFE6D03F;
        break;
    case 2:
        packed_color = 0xFF962826;
        break;
    case 3:
    case 4:
        packed_color = 0xFF6D31C7;
        break;
    case 5:
        packed_color = 0xFFE589EB;
        break;
    case 6:
        packed_color = 0xFFC936A8;
        break;
    case 7:
        packed_color = 0xFFD9ED6C;
        break;
    case 8:
        packed_color = 0xFFEFD841;
        break;
    case 9:
        packed_color = 0xFF60571A;
        break;
    case 10:
        packed_color = 0xFF74C5C4;
        break;
    case 11:
    case 12:
        packed_color = 0xFFE57918;
        break;
    case 13:
    case 14:
        packed_color = 0xFF24DD26;
        break;
    case 15:
    case 16:
        packed_color = 0xFFD53B3B;
        break;
    case 17:
    case 18:
        packed_color = 0xFF20D2CE;
        break;
    case 19:
        packed_color = 0xFFD1BABA;
        break;
    default:
        break;
    }

    auto *mini_map = g_femanager.IGO->field_4;
    field_0 = new PanelQuad{};
    field_0->CopyFrom(a2 == static_cast<mini_map_dot_type>(0) ? mini_map->map_icon_spidey : mini_map->map_icon_others);
    if (a2 == static_cast<mini_map_dot_type>(15) || a2 == static_cast<mini_map_dot_type>(16)) {
        field_4 = new PanelQuad{};
        field_4->CopyFrom(mini_map->minimap_ring);
        highlight_circle_count_down = 12;
    }

    field_0->SetColor(color32{packed_color});
    field_0->TurnOn(true);

    field_8 = new nglQuad{};
    nglInitQuad(field_8);
    nglSetQuadColor(field_8, packed_color);
    field_C = new nglQuad{};
    nglInitQuad(field_C);
    nglSetQuadColor(field_C, 0xFF000000);
    field_10 = new nglQuad{};
    nglInitQuad(field_10);
    nglSetQuadColor(field_10, 0xFF000000);

    static int current_z_value = 72;
    const float z = a2 == static_cast<mini_map_dot_type>(0) ? 101.0f : static_cast<float>(current_z_value);
    field_0->SetZvalue(z, static_cast<panel_layer>(7));
    const float absolute_z = static_cast<float>(field_0->GetZvalue());
    nglSetQuadZ(field_8, absolute_z - 0.5f);
    nglSetQuadZ(field_C, absolute_z + 0.5f);
    nglSetQuadZ(field_10, absolute_z + 0.5f);
    field_0->SetZvalueAbs(absolute_z);
    if (a2 != static_cast<mini_map_dot_type>(0) && ++current_z_value > 100)
        current_z_value = 72;

    mini_map->field_364.push_back(this);
    field_24 = true;
    field_25 = true;
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
