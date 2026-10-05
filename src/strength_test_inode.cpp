#include "strength_test_inode.h"

#include "common.h"
#include "actor.h"
#include "ai_adv_strength_test_data.h"
#include "ai_interaction_data.h"
#include "base_ai_core.h"
#include "controller_inode.h"
#include "event.h"
#include "event_manager.h"
#include "fe_mini_map_widget.h"
#include "fe_track_and_field.h"
#include "femanager.h"
#include "igofrontend.h"
#include "interact_sound_entry.h"
#include "interaction_inode.h"
#include "panelquad.h"
#include "slab_allocator.h"
#include "sound_and_pfx_interface.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

namespace ai {

VALIDATE_SIZE(strength_test_inode, 0x50);

namespace {
strength_test_inode *__fastcall strength_delete(strength_test_inode *self, void *, unsigned int flags)
{
    self->~strength_test_inode();
    if (flags & 1u) slab_allocator::deallocate(self, nullptr);
    return self;
}
int __fastcall strength_type(const strength_test_inode *) { return 152; }
bool __fastcall strength_subclass(const strength_test_inode *, void *, mash::virtual_types_enum type)
{
    return type == 537 || type == 573;
}
int __fastcall strength_size(const strength_test_inode *) { return sizeof(strength_test_inode); }
void __fastcall strength_frame(strength_test_inode *self, void *, Float dt) { self->_frame_advance(dt); }
void __fastcall strength_activate(strength_test_inode *self, void *, ai_core *core) { self->_activate(core); }
void __fastcall strength_deactivate(strength_test_inode *self, void *) { self->_deactivate(); }
double __fastcall strength_button(strength_test_inode *self, void *, Float) { return self->get_curr_button_press(); }
void __fastcall strength_ui_init(strength_test_inode *self, void *) { self->UI_Init(); }
void __fastcall strength_ui_update(strength_test_inode *self, void *) { self->UI_Update(); }
void __fastcall strength_ui_done(strength_test_inode *self, void *) { self->UI_Done(); }

void update_colors(fe_track_and_field *ui)
{
    float position = 0.03846154f;
    for (int i = 0; i != 13; ++i) {
        const float distance = std::fabs(position - ui->field_5C);
        const auto color = distance < ui->field_60 ? ui->field_68 :
            distance < ui->field_64 ? ui->field_6C : ui->field_70;
        ui->field_8[i]->SetColor(color);
        position += 0.07692308f;
    }
}

void set_shown(fe_track_and_field *ui, bool shown, float delay)
{
    if (ui->field_4) {
        ui->field_4C = shown;
        auto *map = g_femanager.IGO->field_4;
        if (shown) {
            ui->field_4D = map->field_3A8 != 0;
            if (ui->field_4D) map->SetShown(false);
            update_colors(ui);
            ui->field_48 = delay;
        } else if (ui->field_4D) {
            map->SetShown(true);
            ui->field_4D = false;
        }
    }
}

void configure_stage(fe_track_and_field *ui, const ai_adv_strength_test_data *stage)
{
    ui->field_5C = bit_cast<float>(stage->field_0);
    ui->field_60 = bit_cast<float>(stage->field_4) * 0.5f;
    ui->field_64 = (bit_cast<float>(stage->field_8) + bit_cast<float>(stage->field_4)) * 0.5f;
}
}

void *strength_test_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result{};
        std::copy_n(static_cast<void **>(info_node::native_vtable()), 12, result.data());
        result[2] = bit_cast<void *>(&strength_delete);
        result[3] = bit_cast<void *>(&strength_type);
        result[4] = bit_cast<void *>(&strength_subclass);
        result[7] = bit_cast<void *>(&strength_frame);
        result[8] = bit_cast<void *>(&strength_activate);
        result[9] = bit_cast<void *>(&strength_deactivate);
        result[11] = bit_cast<void *>(&strength_size);
        result[12] = bit_cast<void *>(&strength_button);
        result[13] = bit_cast<void *>(&strength_ui_init);
        result[14] = bit_cast<void *>(&strength_ui_update);
        result[15] = bit_cast<void *>(&strength_ui_done);
        return result;
    }();
    return table.data();
}

double strength_test_inode::get_curr_button_press()
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    int pressed = 0;
    {
        const auto left = controller->get_button(static_cast<controller_inode::eControllerButton>(16));
        if (!left.is_flagged(0x20) && left.is_flagged(GBFLAG_TRIGGERED)) pressed = 1;
    }
    {
        const auto right = controller->get_button(static_cast<controller_inode::eControllerButton>(17));
        if (!right.is_flagged(0x20) && right.is_flagged(GBFLAG_TRIGGERED)) pressed = (pressed != 0) + 2;
    }
    if (pressed > 0 && pressed <= 2) {
        strength += pressed == current_button ? -0.1f : 0.1f;
        current_button = pressed;
    } else if (pressed == 3) strength += 0.1f;
    return 0.0;
}

void strength_test_inode::UI_Init()
{
    auto *ui = g_femanager.IGO->field_14;
    if (advanced) configure_stage(ui, parameters->my_adv_str_test_list.at(advanced_stage));
    else {
        ui->field_5C = 1.0f;
        ui->field_60 = 0.30000001f;
        ui->field_64 = 0.60000002f;
    }
    set_shown(ui, true, 0.53333336f);
}

void strength_test_inode::UI_Update()
{
    g_femanager.IGO->field_14->field_50 = std::clamp(strength, 0.0f, 1.0f);
}

void strength_test_inode::UI_Done()
{
    set_shown(g_femanager.IGO->field_14, false, 0.0f);
}

void strength_test_inode::set_new_adv_stage()
{

    if (++advanced_stage == 0) ++field_32;
    auto *ui = g_femanager.IGO->field_14;
    configure_stage(ui, parameters->my_adv_str_test_list.at(advanced_stage));
    update_colors(ui);
}

void strength_test_inode::update_strength(float time_step)
{
    const float previous = strength;
    strength += time_step * bit_cast<float>(parameters->field_34) * -0.1f;
    using callback = double (__fastcall *)(strength_test_inode *, void *, Float);
    const double input = reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x30))(this, nullptr, Float{time_step});
    strength = static_cast<float>(input + strength);
    strength = std::clamp(strength, 0.0f, 1.0f);
    update_sounds(time_step, previous);
}

void strength_test_inode::update_advanced_strength(float time_step)
{
    const auto *stage = parameters->my_adv_str_test_list.at(advanced_stage);
    const float center = bit_cast<float>(stage->field_0);
    const float green = bit_cast<float>(stage->field_4) * 0.5f;
    const float yellow = (bit_cast<float>(stage->field_8) + bit_cast<float>(stage->field_4)) * 0.5f;
    if (center - green <= strength && center + green >= strength)
        advanced_strength = std::clamp(advanced_strength + time_step / bit_cast<float>(stage->field_C), 0.0f, 1.0f);
    else if (center - yellow > strength || center + yellow < strength)
        advanced_strength = std::clamp(advanced_strength - time_step / bit_cast<float>(stage->field_10), 0.0f, 1.0f);
}

void strength_test_inode::update_sounds(float time_step, float previous_strength)
{
    field_34->value = (1.0f - field_34->weight) * field_34->value +
        (strength - previous_strength) / time_step * field_34->weight;
    const float speed = std::fabs(field_34->value);
    auto *interaction = static_cast<interaction_inode *>(field_8->get_info_node(interaction_inode::default_id, true));
    auto *data = interaction->field_2C;
    for (int i = 0; i != data->field_54.m_size; ++i) {
        auto *entry = data->field_54.m_data[i];
        const float threshold = bit_cast<float>(entry->field_4);
        const bool crossed = entry->field_11 ?
            previous_strength <= threshold && strength > threshold :
            strength <= threshold && previous_strength > threshold;
        if (!crossed || speed < bit_cast<float>(entry->field_8) || speed > entry->field_C) continue;

        if (entry->field_10 && !field_38->empty()) continue;
        auto *target = interaction->target_handle.get_volatile_ptr();
        if (target) target = interaction->target_handle.get_volatile_ptr();
        target->my_sound_and_pfx_interface->play_sound_grp(entry->field_0, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);
        if (entry->field_10) field_38->push_back(entry);
    }
}

void strength_test_inode::_frame_advance(Float time_step)
{
    if (!active) return;
    using callback = void (__fastcall *)(strength_test_inode *, void *);
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x38))(this, nullptr);
    elapsed += time_step;
    if (state == 1) {
        if (elapsed >= parameters->field_3C) {
            state = static_cast<int>(advanced) + 2;
            elapsed = parameters->field_3C;
        }
        strength = elapsed / parameters->field_3C * parameters->field_38;
        if (advanced)
            advanced_strength = elapsed / parameters->field_3C *
                bit_cast<float>(parameters->my_adv_str_test_list.at(0)->field_14);
    } else if (state == 2) {
        update_strength(time_step);
        if (std::equal_to<float>{}(strength, 0.0f) || std::equal_to<float>{}(strength, 1.0f)) {
            event_manager::raise_event(event::META_ANIM_UTILITY, get_actor()->my_handle);
            state = 4;
        }
    } else if (state == 3) {
        update_strength(time_step);
        update_advanced_strength(time_step);
        if (std::equal_to<float>{}(advanced_strength, 0.0f) ||
            std::equal_to<float>{}(advanced_strength, 1.0f)) {
            event_manager::raise_event(event::META_ANIM_UTILITY, get_actor()->my_handle);
            state = 4;
        }
        const float next_threshold = bit_cast<float>(parameters->my_adv_str_test_list.at(advanced_stage)->field_18);
        if (std::not_equal_to<float>{}(next_threshold, -1.0f) &&
            advanced_strength >= next_threshold) set_new_adv_stage();
    }
}

strength_test_inode::strength_test_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[152]);
#else
    m_vtbl = 0x0087CE40;
#endif
    active = false;
    advanced = false;
    sound = 0;
}

strength_test_inode::strength_test_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[152]);
#else
    m_vtbl = 0x0087CE40;
#endif
    sound = 0;
}

void strength_test_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    active = false;
    field_34 = nullptr;
    field_38 = nullptr;
    sound = 0;
    field_40 = 0;
    field_44 = 0;
    parameters = nullptr;
}

void strength_test_inode::_deactivate()
{

}

}
