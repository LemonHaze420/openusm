#include "als_inode.h"

#include "als_animation_logic_system.h"
#include "als_inode_render_debug.h"
#include "base_ai_core.h"
#include "common.h"
#include "conglom.h"
#include "event.h"
#include "func_wrapper.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"
#include "native_info_node_table.h"

namespace ai {

VALIDATE_SIZE(als_inode, 0x2C);

namespace {
void __fastcall native_destruct(als_inode *self, void *)
{
    self->field_28.destruct_mashed_class();
    self->_destruct_mashed_class();
}
void __fastcall native_unmash(als_inode *self, void *, mash_info_struct *info, void *base)
{
    self->_unmash(info, base);
}
void __fastcall native_activate(als_inode *self, void *, ai_core *core) { self->activate(core); }
void __fastcall native_deactivate(als_inode *self, void *) { self->deactivate(); }
void __fastcall native_advance(als_inode *self, void *, Float dt) { self->frame_advance(dt); }
void __fastcall native_set_signal(als_inode *self, void *, Float time, string_hash category)
{
    self->set_known_combat_signal_time_and_category(time, category);
}
void __fastcall native_get_signal(const als_inode *self, void *, Float &time, string_hash &category)
{
    self->get_known_combat_signal_time_and_category(time, category);
}
float __fastcall native_signal_eta(als_inode *self, void *, als::layer_types layer)
{
    return self->get_eta_of_combat_signal(layer);
}
}

void *als_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<als_inode, 333, 537, 15> result;
        result[0] = reinterpret_cast<void *>(&native_destruct);
        result[1] = reinterpret_cast<void *>(&native_unmash);
        result[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&native_advance);
        result[8] = reinterpret_cast<void *>(&native_activate);
        result[9] = reinterpret_cast<void *>(&native_deactivate);
        result[12] = reinterpret_cast<void *>(&native_set_signal);
        result[13] = reinterpret_cast<void *>(&native_get_signal);
        result[14] = reinterpret_cast<void *>(&native_signal_eta);
        return result;
    }();
    return table.data();
}

void als_inode::get_known_combat_signal_time_and_category(Float &time, string_hash &category) const
{
    time = field_24;
    category = field_28;
}

als_inode::als_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[333]);
}

als_inode::als_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), field_28(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[333]);
}

void als_inode::_unmash(mash_info_struct *info, void *context)
{
    info_node::_unmash(info, context);
    info->unmash_class_in_place(field_28, this);
}

void als_inode::deactivate()
{

    field_20 = nullptr;
}

void als_inode::frame_advance(Float)
{

}

als::state_machine *als_inode::get_als_layer(als::layer_types a2)
{
    if constexpr (1) {
        assert(get_system() != nullptr);

        return this->get_system()->get_als_layer(a2);
    } else {
        return (als::state_machine *)THISCALL(0x00689BA0, this, a2);
    }
}

void als_inode::kill_layer(als::layer_types layer_type)
{
    if (static_cast<conglomerate *>(field_C)->field_114 != nullptr) {
        if (auto *layer = field_1C->get_als_layer(layer_type)) {
            layer->kill_layer();
        }
    }
}

string_hash als_inode::get_state_id(als::layer_types a3)
{
    auto *the_layer = this->get_als_layer(a3);
    return the_layer->get_state_id();
}

bool als_inode::is_layer_interruptable(als::layer_types a1)
{
    auto *als_layer = this->get_als_layer(a1);
    return als_layer->is_interruptable();
}

string_hash als_inode::get_category_id(als::layer_types a3)
{
    auto *v3 = this->get_als_layer(a3);

    string_hash id = v3->get_category_id();
    return id;
}

void als_inode::set_desired_params(als::param_list &a2, als::layer_types a3)
{
    auto *the_layer = this->get_als_layer(a3);
    the_layer->set_desired_params(a2);
}

void als_inode::request_category_transition(string_hash a2, als::layer_types a3, bool a4, bool a5, bool a6)
{
    if constexpr (1) {
        als::state_machine *v7 = nullptr;

        if (a6 || (v7 = this->field_1C->get_als_layer(a3), a2 != v7->get_category_id())) {
            auto *v8 = this->field_1C->get_als_layer(a3);
            v8->request_category_transition(a2);
        }

    } else {
        THISCALL(0x00689D80, this, a2, a3, a4, a5, a6);
    }
}

void als_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    field_1C = static_cast<conglomerate *>(core->field_64)->get_my_als();
    field_20 = nullptr;
}

void als_inode::set_known_combat_signal_time_and_category(Float a2, string_hash a3)
{
    this->field_24 = a2;
    this->field_28 = a3;
}

float als_inode::get_eta_of_combat_signal(als::layer_types a2)
{
    if constexpr (1) {
        string_hash v16 = event::ATTACK;
        auto *v3 = this->field_1C;

        auto *v6 = v3->get_als_layer(a2);

        auto v7 = v6->get_time_to_signal(v16);
        float a2a = v7;
        if (v7 < 0.0f) {
            v16 = event::ANIM_ACTION;
            auto *v8 = this->field_1C;

            auto *v10 = v8->get_als_layer(a2);

            a2a = v10->get_time_to_signal(v16);
        }

        if (a2a > 0.0f) {
            auto *v11 = this->field_1C->get_als_layer(a2);

            string_hash category_id = v11->get_category_id();

            auto tmp = g_world_ptr->time_manager.get_level_time() + a2a;
            this->set_known_combat_signal_time_and_category(tmp, category_id);
        }
        return a2a;
    } else {
        float(__fastcall * func)(void *, void *edx, als::layer_types a2) = CAST(func, 0x00689C20);
        return func(this, nullptr, a2);
    }
}

bool als_inode::is_cat_our_prev_cat(string_hash a2, als::layer_types a3)
{
    auto *als_layer = this->get_als_layer(a3);
    return als_layer->is_cat_our_prev_cat(a2);
}

bool als_inode::anim_finished(string_hash a2, als::layer_types a3)
{
    TRACE("als_inode::anim_finished");

    if constexpr (STANDALONE_SYSTEM) {
        auto *the_layer = this->field_1C->get_als_layer(a3);
        if (the_layer->is_cat_our_prev_cat(a2)) {
            return true;
        }

        if (the_layer->get_category_id() == a2) {
            return std::abs(the_layer->get_time_to_end_of_anim()) < EPSILON;
        } else {
            return !the_layer->is_requesting_category(a2);
        }
    } else {
        bool(__fastcall * func)(void *, void *, string_hash, als::layer_types) = CAST(func, 0x00689E10);
        auto result = func(this, nullptr, a2, a3);

        return result;
    }
}

}  // namespace ai

void als_inode_patch()
{
    {
        FUNC_ADDRESS(address, &ai::als_inode::get_eta_of_combat_signal);
        //set_vfunc(0x0087CEF8, address);
    }

    {
        FUNC_ADDRESS(address, &ai::als_inode::anim_finished);
        REDIRECT(0x0045AB02, address);
        REDIRECT(0x006ABB52, address);
    }
}
