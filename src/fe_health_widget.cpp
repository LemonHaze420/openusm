#include "fe_health_widget.h"
#include "common.h"
#include "damage_interface.h"
#include "entity.h"
#include "entity_base_vhandle.h"

#include "func_wrapper.h"
#include "panelfile.h"
#include "panelanim.h"
#include "panelanimfile.h"

// VALIDATE_SIZE(fe_health_widget, 0x58);


VALIDATE_SIZE(fe_health_widget, 0x58);
VALIDATE_OFFSET(fe_health_widget, field_30, 0x30);

fe_health_widget::fe_health_widget(int a1)
{
    this->field_30 = 0;
    this->number_of_types = a1;

    for (auto i = 0; i < this->number_of_types; ++i) {
        this->panels[i] = nullptr;
    }

    this->field_54 = 0;
    this->field_55 = 0;
    this->field_38 = this->number_of_types;
    this->field_40 = nullptr;
    this->field_44 = nullptr;
    this->field_48 = nullptr;
    this->field_4C = 1.0;
    this->field_50 = 1.0;
}

void fe_health_widget::SetShown(bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (field_38 < 0 || field_38 >= number_of_types || panels[field_38] == nullptr ||
            panels[field_38]->field_28.empty())
            return;

        field_55 = true;
        field_54 = a2;
        PanelAnimFile *animation = panels[field_38]->field_28.at(0);
        for (uint16_t i = 0; i < animation->field_0.size(); ++i) {
            if (auto *target = animation->field_0.at(i)->field_14)
                target->StartAnim(true);
        }
        animation->field_18 = bit_cast<int>(animation->field_20);
        animation->field_14 = 0.0f;
        animation->field_1C = 0;
        animation->field_28 = 0;
        animation->field_2C = false;
        animation->field_2D = true;
        animation->field_24 = a2 ? 0 : 1;
    } else {
        THISCALL(0x0061A3F0, this, a2);
    }
}

void fe_health_widget::DrawAllPanels()
{
    if (field_38 < 0 || field_38 >= number_of_types)
        return;
    auto *panel = panels[field_38];
    if (panel == nullptr)
        return;
    const bool animating = panel->field_28.at(0)->field_2D;
    if (field_54 || animating) {
        if (field_55 && !animating)
            field_55 = false;
        panel->Draw();
    }
}

void fe_health_widget::UpdateMasking()
{
    if constexpr (STANDALONE_SYSTEM) {
        const int direction = field_3C ? 1 : 2;
        if (field_55) {
            if (field_40 != nullptr)
                field_40->Mask(0.0f, direction, -1.0f);
            if (field_44 != nullptr)
                field_44->Mask(0.0f, direction, -1.0f);
            if (field_48 != nullptr)
                field_48->Mask(0.0f, direction, -1.0f);
            return;
        }

        vhandle_type<entity> source{entity_base_vhandle{static_cast<uint32_t>(field_30)}};
        if (entity *owner = source.get_volatile_ptr(); owner != nullptr && owner->has_damage_ifc()) {
            const auto &health = owner->damage_ifc()->field_1FC.field_0;
            const float range = health[2] - health[1];
            const float amount = range <= 0.0f ? 0.0f : (health[0] - health[1]) / range;
            if (field_40 != nullptr)
                field_40->Mask(amount, direction, -1.0f);
        }
        if (field_44 != nullptr)
            field_44->Mask(field_4C, direction, -1.0f);
        if (field_48 != nullptr)
            field_48->Mask(field_50, direction, -1.0f);
    } else {
        THISCALL(0x0061A5A0, this);
    }
}

void fe_health_widget::SetType(int the_type, int source_hash_code)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (panels[the_type] != nullptr) {
            field_38 = the_type;
            field_30 = source_hash_code;
            UpdateMasking();
            clear_bars();
        }
    } else {
        THISCALL(0x00641BC0, this, the_type, source_hash_code);
    }
}

void fe_health_widget::clear_bars()
{
    if constexpr (STANDALONE_SYSTEM) {
        field_4C = 1.0f;
        field_50 = 1.0f;
        field_40 = nullptr;
        field_44 = nullptr;
        field_48 = nullptr;

        if (field_38 < 0 || field_38 >= number_of_types)
            return;
        PanelFile *panel = panels[field_38];
        if (panel == nullptr)
            return;

        const auto find_quad = [panel](const char *name) -> PanelQuad * {
            for (uint16_t i = 0; i < panel->pquads.size(); ++i) {
                PanelQuad *quad = panel->pquads.at(i);
                if (strcmp(quad->field_3C.c_str(), name) == 0)
                    return quad;
            }
            return nullptr;
        };

        field_40 = find_quad("HG_boss_gauge_use");
        if (field_40 == nullptr)
            field_40 = find_quad("HG_hero_gauge_use");
        if (field_40 == nullptr)
            field_40 = panel->GetPQ("HG_TP_gauge_use");
        field_44 = find_quad("HG_hero_gauge_sick_use");
        field_48 = find_quad("HG_boss_gauge_revive_use");

        if (field_40 != nullptr)
            field_40->TurnOn(true);
        if (field_44 != nullptr)
            field_44->TurnOn(false);
        if (field_48 != nullptr)
            field_48->TurnOn(false);
    } else {
        THISCALL(0x0063B170, this);
    }
}

void fe_health_widget::Init(int type_id, const char *a3, bool a4)
{
    TRACE("fe_health_widget::Init");

    assert(type_id >= 0 && type_id < this->number_of_types);

    this->panels[type_id] = PanelFile::UnmashPanelFile(a3, static_cast<panel_layer>(7));
    this->field_3C = a4;
    this->field_55 = false;
    this->clear_bars();
}

void fe_health_widget::DeInit(int a2)
{
    this->panels[a2] = nullptr;
    if (a2 == this->field_38) {
        this->field_54 = 0;
        this->field_38 = this->number_of_types;
    }
}

void fe_health_widget::set_poison_bar_precent(float percent)
{
    if (percent < 0.0f)
        field_4C = 0.0f;
    else
        field_4C = percent > 1.0f ? 1.0f : percent;
}

void fe_health_widget::set_regen_bar_shown(bool shown)
{
    if (field_48 != nullptr)
        field_48->TurnOn(shown);
}

void fe_health_widget::set_poison_bar_shown(bool shown)
{
    if (field_44 != nullptr)
        field_44->TurnOn(shown);
}

void fe_health_widget::set_health_bar_shown(bool shown)
{
    if (field_40 != nullptr)
        field_40->TurnOn(shown);
}

void fe_health_widget_patch()
{
    REDIRECT(0x005718F4, func_address(&fe_health_widget::Init));
}
