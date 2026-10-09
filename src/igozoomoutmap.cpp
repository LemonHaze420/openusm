#include "igozoomoutmap.h"

#include "common.h"
#include "func_wrapper.h"
#include "femanager.h"
#include "game.h"
#include "input_mgr.h"
#include "pausemenusystem.h"
#include "marky_camera.h"
#include "sound_instance_id.h"
#include "string_hash.h"
#include "variable.h"
#include "wds.h"
#include "vtbl.h"
#include "cursor.h"
#include "geometry_manager.h"
#include "panelfile.h"
#include "panelquad.h"

#include <utility.h>

VALIDATE_SIZE(IGOZoomOutMap, 0x82Cu);
VALIDATE_SIZE(IGOZoomOutMap::internal, 0x1Cu);
VALIDATE_SIZE(IGOZoomPOI, 0x14);
VALIDATE_SIZE(zoom_map_ui, 0x248u);
VALIDATE_SIZE(zoom_map_ui::marker, 0x2Cu);
VALIDATE_OFFSET(IGOZoomOutMap, field_5CC, 0x5CC);
VALIDATE_OFFSET(IGOZoomOutMap, field_5C4, 0x5C4);

// 0x006489A0
IGOZoomOutMap::IGOZoomOutMap()
{
    field_5BC = 0;
    field_5BD = 0;
    field_5BE = 0;
    field_5BF = 0;
    field_5C0 = 0;
    field_5C1 = 0;
    field_5C2 = 0;
    field_5C3 = false;
    field_5C4 = false;
    field_5C5 = 0;
    field_5C6 = 0;
    field_5C7 = 0;
    field_5C8 = 5.0f;
    field_5B0 = 0;
    field_5B4 = 0;
    field_5B8 = 0;
    field_818 = 0;
    field_824 = 0;
    field_828 = 0;
}

IGOZoomOutMap::~IGOZoomOutMap()
{
    for (auto &entry : field_0) {
        auto *object = entry.field_0.field_10;
        if (object != nullptr) {
            using destroy_t = void(__fastcall *)(void *, void *, int);
            auto destroy = reinterpret_cast<destroy_t>(get_vfunc(*object, 8));
            destroy(object, nullptr, 1);
        }
    }
}

void IGOZoomOutMap::UpdateInScene()
{
    if (this->field_5C5) {
        for (int i = 0; i < this->field_5B4; ++i) {
            if (this->field_5B8 == this->field_0[i].field_14) {
                this->field_0[i].field_0.UpdateInScene();
            }
        }
    }
}

void zoom_map_ui::Draw()
{
    auto *panel = reinterpret_cast<PanelFile *>(field_0[3]);
    if (panel == nullptr)
        return;
    panel->Draw();
    const float height = bit_cast<float>(field_0[0x234 / 4]);
    const float scale = height > 500.0f ? 1.0f - (height - 500.0f) * 0.002f * 0.25f : 1.0f;
    const bool large = height > 1850.0f;
    const auto *bytes = reinterpret_cast<const uint8_t *>(field_0);
    for (const auto &entry : field_23C) {
        int filter = -1;
        switch (entry.type) {
        case 2:
            filter = 0x1E7;
            break;
        case 6:
            filter = 0x1E6;
            break;
        case 13:
        case 14:
            filter = 0x1E8;
            break;
        case 3:
        case 4:
            filter = 0x1E9;
            break;
        case 11:
        case 12:
            filter = 0x1EA;
            break;
        case 17:
        case 18:
            filter = 0x1ED;
            break;
        case 19:
            filter = 0x1EC;
            break;
        case 1:
            filter = 0x1EB;
            break;
        }
        if (filter >= 0 && !bytes[filter])
            continue;
        const auto position =
            sub_501B20(geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(7)), entry.position);
        auto *selected = large ? entry.large : entry.small_quad;
        selected->SetCenterPos(position.x, position.y);
        entry.overlay->SetCenterPos(position.x, position.y);
        if (!large)
            selected->Scale(scale, true);
        entry.large->TurnOn(large);
        entry.small_quad->TurnOn(!large);
        entry.overlay->TurnOn(true);
        entry.large->SetZvalue(1000.0f - entry.depth, static_cast<panel_layer>(7));
        entry.small_quad->SetZvalue(1000.0f - entry.depth, static_cast<panel_layer>(7));
        entry.large->Draw();
        entry.small_quad->Draw();
        entry.overlay->Scale(scale, true);
        entry.overlay->SetAlpha(entry.alpha);
        entry.overlay->SetColor(color32{entry.color});
        entry.overlay->SetZvalue(999.0f, static_cast<panel_layer>(7));
        entry.overlay->Draw();
    }
    const auto position = sub_501B20(geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(7)),
                                     g_world_ptr->get_hero_ptr(0)->get_abs_position());
    auto *small_icon = reinterpret_cast<PanelQuad *>(field_0[0x218 / 4]);
    auto *large_icon = reinterpret_cast<PanelQuad *>(field_0[0x21C / 4]);
    large_icon->TurnOn(large);
    small_icon->TurnOn(!large);
    auto *selected = large ? large_icon : small_icon;
    selected->SetCenterPos(position.x, position.y);
    if (!large)
        selected->Scale(scale, true);
    small_icon->Draw();
    large_icon->Draw();
    g_cursor->Draw();
}

void IGOZoomOutMap::Draw()
{
    if (!field_5C4)
        return;
    field_5CC.Draw();
    if (field_5C5) {
        for (int i = 0; i < field_5B4; ++i) {
            auto &entry = field_0[i];
            if (entry.field_14 == field_5B8 && entry.field_0.field_10 != nullptr)
                reinterpret_cast<PanelQuad *>(entry.field_0.field_10)->Draw();
        }
    }
}

void IGOZoomOutMap::DoneZoomingBack()
{
    g_game_ptr->enable_marky_cam(false, false, -1000.0, 0.0);
    g_world_ptr->field_28.field_44->set_affixed_x_facing(false);
    g_game_ptr->unpause();
    g_game_ptr->zoomInactive = false;
}

bool IGOZoomOutMap::sub_55F320()
{
    return this->field_5C4 || this->field_5C3;
}

void IGOZoomOutMap::sub_638AD0(int a2, int a3, int a4)
{
    THISCALL(0x00638AD0, this, a2, a3, a4);
}

// 0x0063A760
void IGOZoomOutMap::Update(Float)
{
    UpdateSelectButton();
}

// 0x006386E0
void IGOZoomOutMap::UpdateSelectButton()
{
    const float select = input_mgr::instance->get_control_state(115, INVALID_DEVICE_ID);
    if (!field_5BC && (select < 0.0f || select > 0.0f)) {
        field_5BC = true;
        OnSelectPress();
        if (input_mgr::instance->get_control_state(99, INVALID_DEVICE_ID) >= 0.5f)
            field_5C0 = true;
    }
    if (field_5BC && !(select < 0.0f || select > 0.0f))
        field_5BC = false;
}

// 0x00638570
void IGOZoomOutMap::OnSelectPress()
{
    if (g_femanager.m_pause_menu_system->m_index >= 0 || field_5C6)
        return;
    entity *hero = g_world_ptr->get_hero_ptr(0);
    if (hero == nullptr)
        return;

    field_5C4 = !field_5C4;
    if (field_5C4) {
        SetZoomLevel(0);
    } else {
        field_5C3 = true;
        field_5C7 = true;
    }
}

void IGOZoomOutMap::SetZoomLevel(int a2)
{
    auto v2 = a2;
    if (a2 >= 1) {
        if (a2 > 4) {
            v2 = 4;
        }

    } else {
        v2 = 1;
    }

    if (this->field_5B0 != v2) {
        static string_hash sfx_id_hash{"FE_GENERIC_LRSCROLL"};

        [[maybe_unused]] sound_instance_id id = sub_60B960(sfx_id_hash, 1.0, 1.0);
    }

    this->field_5B0 = v2;
    auto *v4 = g_world_ptr->field_28.field_44;

    this->field_578 = v4->get_abs_position();

    this->field_578[1] = this->field_5B0 * 500.0f;
    this->field_5C3 = true;
}

void IGOZoomPOI::UpdateInScene()
{
    THISCALL(0x0062A160, this);
}

void IGOZoomOutMap_patch()
{
    {
        FUNC_ADDRESS(address, &IGOZoomOutMap::SetZoomLevel);
        SET_JUMP(0x00619550, address);
    }
}
