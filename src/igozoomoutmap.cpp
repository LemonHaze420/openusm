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

#include <utility.h>

VALIDATE_SIZE(IGOZoomOutMap, 0x82Cu);
VALIDATE_SIZE(IGOZoomOutMap::internal, 0x1Cu);
VALIDATE_SIZE(IGOZoomPOI, 0x14);
VALIDATE_SIZE(zoom_map_ui, 0x240u);
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
