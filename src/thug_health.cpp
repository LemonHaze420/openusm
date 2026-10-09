#include "thug_health.h"

#include "entity_base.h"
#include "common.h"
#include "panelfile.h"
#include "panelquad.h"
#include "trace.h"
#include "ai_common_states.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "camera.h"
#include "conglom.h"
#include "damage_interface.h"
#include "geometry_manager.h"
#include "os_developer_options.h"
#include "wds.h"
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(thug_health, 0x364u);
VALIDATE_SIZE(thug_health::widget_instance, 0x1Cu);

thug_health::thug_health()
{
    this->field_0 = -1;

    this->field_4 = nullptr;

    for (int i = 0; i < 30; ++i) {
        this->field_1C[i].field_0 = false;
        this->field_1C[i].field_10 = {0};
    }
}

void thug_health::init()
{
    TRACE("thug_health::init");

    if (this->field_4 == nullptr) {
        this->field_4 = PanelFile::UnmashPanelFile("healthbar_thug", static_cast<panel_layer>(7));

        this->field_8 = this->field_4->GetPQ("thug_health_green");
        this->field_C = this->field_4->GetPQ("thug_health_yellow");
        this->field_10 = this->field_4->GetPQ("thug_health_red");
        this->field_14 = this->field_4->GetPQ("thug_health_skull");
        this->field_18 = this->field_4->GetPQ("thug_health_web");
    }
}

int thug_health::create()
{
    for (int index = 0; index < 30; ++index) {
        auto &widget = field_1C[index];
        if (!widget.field_0) {
            widget.field_10 = {0};
            widget.field_0 = true;
            widget.visible = true;
            widget.field_14 = 20;
            widget.field_18 = 0;
            return index;
        }
    }
    return field_0;
}

void thug_health::destroy(int index)
{
    if (static_cast<unsigned>(index) < 30 && field_1C[index].field_0)
        field_1C[index].field_0 = false;
}

void thug_health::set_entity(int index, entity_base *owner)
{
    if (static_cast<unsigned>(index) < 30 && field_1C[index].field_0 && owner != nullptr)
        field_1C[index].field_10 = owner->get_my_handle();
}

void thug_health::draw()
{
    if (field_4 == nullptr)
        return;
    PanelQuad *quads[]{field_8, field_C, field_10, field_14, field_18};
    for (auto &widget : field_1C) {
        if (!widget.field_0 || !widget.visible)
            continue;
        auto *owner = widget.field_10.get_volatile_ptr();
        if (owner == nullptr)
            continue;
        float hp = 1.0f;
        float maximum = 1.0f;
        auto *damage = owner->has_damage_ifc() ? owner->damage_ifc() : nullptr;
        if (damage != nullptr) {
            hp = damage->field_1FC.field_0[0];
            maximum = damage->field_1FC.field_0[2];
        }
        int shown;
        if (std::fpclassify(hp) == FP_ZERO && damage != nullptr) {
            if (damage->field_21C.field_0[0] > 0.0f) {
                shown = 3;
            } else {
                auto *state = owner->get_ai_core()->my_base_machine->my_curr_state;
                if (state != nullptr &&
                    state->m_vtbl == bit_cast<std::intptr_t>(ai::unconscious_state::native_vtable())) {
                    const auto *unconscious = static_cast<const ai::unconscious_state *>(state);
                    widget.field_14 = std::max(
                        4, static_cast<int>((1.0f - unconscious->field_1C / unconscious->unconscious_time) * 20.0f));
                } else {
                    widget.field_14 = 20;
                }
                shown = widget.field_18 < widget.field_14 / 2 ? 4 : -1;
            }
        } else {
            const double fraction = hp / maximum;
            shown = fraction < 0.33 ? 2 : fraction < 0.66 ? 1 : 0;
        }
        for (int i = 0; i < 5; ++i)
            quads[i]->TurnOn(i == shown);
        auto *camera = g_world_ptr->get_chase_cam_ptr(0);
        const auto camera_position = camera->get_abs_position();
        widget.field_4 = owner->get_abs_position();
        float near_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(58));
        float far_distance = os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(59));
        float minimum_scale =
            os_developer_options::instance->get_int(static_cast<os_developer_options::ints_t>(60)) * 0.01f;
        if (near_distance < 1.0f || far_distance <= near_distance || minimum_scale <= 0.0f || minimum_scale > 1.0f) {
            near_distance = 1.0f;
            far_distance = 11.0f;
            minimum_scale = 0.5f;
        }
        const float distance = std::clamp((widget.field_4 - camera_position).length(), near_distance, far_distance);
        const float scale = (1.0f - (distance - near_distance) / far_distance) * (1.0f - minimum_scale) + minimum_scale;
        auto position = widget.field_4;
        entity_base *head = nullptr;
        if (owner->field_4 & 4)
            head = static_cast<conglomerate *>(owner)->get_bone(bip01_head, true);
        if (head != nullptr) {
            position = head->get_abs_position();
            position.y -= 0.3f;
        } else {
            position.y += owner->is_an_actor() ? owner->get_visual_radius() * 0.5f : 1.2f;
        }
        const auto projected = geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(4)) * position;
        const auto screen =
            sub_501B20(geometry_manager::get_xform(static_cast<geometry_manager::xform_t>(5)), projected);
        for (auto *quad : quads) {
            quad->ResetToInitialXY();
            quad->SetCenterPos(screen.x, screen.y);
            quad->Scale(scale, true);
            if (projected.z <= 0.0f)
                quad->TurnOn(false);
        }
        field_4->Draw();
    }
}

void thug_health_patch()
{
    {
        FUNC_ADDRESS(address, &thug_health::init);
        REDIRECT(0x00647E32, address);
    }
}
