#include "track_field_inode.h"

#include "common.h"
#include "func_wrapper.h"
#include "base_ai_core.h"
#include "controller_inode.h"
#include "femanager.h"
#include "igofrontend.h"
#include "fe_track_and_field.h"
#include "fe_mini_map_widget.h"
#include "game_button.h"
#include "panelquad.h"
#include "vtbl.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace ai {

VALIDATE_SIZE(track_field_inode, 0x3C);

namespace {
void *__fastcall track_delete(track_field_inode *self, void *, unsigned flags)
{
    self->~track_field_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(track_field_inode));
    return self;
}
unsigned __fastcall track_type(track_field_inode *, void *)
{
    return 420;
}
bool __fastcall track_subclass(track_field_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall track_needs_advance(track_field_inode *, void *)
{
    return true;
}
void __fastcall track_advance(track_field_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
void __fastcall track_activate(track_field_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
int __fastcall track_size(track_field_inode *, void *)
{
    return sizeof(track_field_inode);
}
int __fastcall track_button(track_field_inode *self, void *)
{
    return self->get_curr_button_press();
}
void __fastcall track_ui_init(track_field_inode *self, void *)
{
    self->ui_init();
}
void __fastcall track_ui_update(track_field_inode *self, void *)
{
    self->ui_update();
}
}  // namespace

void *track_field_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 15> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), 12, result.data());
        result[2] = reinterpret_cast<void *>(&track_delete);
        result[3] = reinterpret_cast<void *>(&track_type);
        result[4] = reinterpret_cast<void *>(&track_subclass);
        result[6] = reinterpret_cast<void *>(&track_needs_advance);
        result[7] = reinterpret_cast<void *>(&track_advance);
        result[8] = reinterpret_cast<void *>(&track_activate);
        result[11] = reinterpret_cast<void *>(&track_size);
        result[12] = reinterpret_cast<void *>(&track_button);
        result[13] = reinterpret_cast<void *>(&track_ui_init);
        result[14] = reinterpret_cast<void *>(&track_ui_update);
        return result;
    }();
    return table.data();
}

int track_field_inode::get_curr_button_press()
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    const auto first = controller->get_button(static_cast<controller_inode::eControllerButton>(16));
    const auto second = controller->get_button(static_cast<controller_inode::eControllerButton>(17));
    const bool left = !first.is_flagged(0x20) && first.is_flagged(2);
    const bool right = !second.is_flagged(0x20) && second.is_flagged(2);
    return (left ? 1 : 0) + (right ? 2 : 0);
}

void track_field_inode::compute_strength_value(Float delta)
{
    field_20 = static_cast<float>(static_cast<double>(delta) * field_24 * -0.1f + field_20);
    using button_fn = int(__fastcall *)(track_field_inode *);
    const int button = reinterpret_cast<button_fn>(get_vfunc(m_vtbl, 0x30))(this);
    if (button > 0 && button <= 2) {
        field_20 += button == field_34 ? -0.1f : 0.1f;
        field_34 = button;
    } else if (button == 3) {
        field_20 += 0.1f;
    }
    field_20 = std::clamp(field_20, 0.0f, 1.0f);
}

void track_field_inode::_frame_advance(Float delta)
{
    if (field_1C) {
        using update_fn = void(__fastcall *)(track_field_inode *);
        reinterpret_cast<update_fn>(get_vfunc(m_vtbl, 0x38))(this);
        if (field_38 == 1) {
            compute_strength_value(delta);
            if (field_20 <= 1.0f && field_20 >= 1.0f)
                field_38 = 2;
        }
    }
}

void track_field_inode::ui_init()
{
    auto *widget = g_femanager.IGO->field_14;
    widget->field_5C = 1.0f;
    widget->field_60 = 0.3f;
    widget->field_64 = 0.6f;
    if (widget->field_4) {
        widget->field_4C = true;
        widget->field_4D = g_femanager.IGO->field_4->field_3A8;
        if (widget->field_4D)
            g_femanager.IGO->field_4->SetShown(false);
        float center = 0.03846154f;
        for (int i = 0; i < 13; ++i) {
            const float distance = std::fabs(center - widget->field_5C);
            const auto color = distance < widget->field_60
                                   ? widget->field_68
                                   : (distance < widget->field_64 ? widget->field_6C : widget->field_70);
            widget->field_8[i]->SetColor(color);
            center += 0.07692307978868484f;
        }
        widget->field_48 = 0.53333336f;
    }
}

void track_field_inode::ui_update()
{
    g_femanager.IGO->field_14->field_50 = std::clamp(field_20, 0.0f, 1.0f);
}

ai::track_field_inode::track_field_inode()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[420]);
#else
    m_vtbl = 0x0087DD68;
#endif
    field_1C = false;
}

track_field_inode::track_field_inode(from_mash_in_place_constructor *a2) : info_node(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[420]);
#else
    m_vtbl = 0x0087DD68;
#endif
}

void track_field_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    field_1C = false;
    field_28 = field_2C = field_30 = 0;
}

void track_field_inode::_deactivate() {}

}  // namespace ai
