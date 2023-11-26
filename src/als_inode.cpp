#include "als_inode.h"

#include "als_animation_logic_system.h"
#include "als_inode_render_debug.h"
#include "base_ai_core.h"
#include "common.h"
#include "event.h"
#include "func_wrapper.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(als_inode, 0x2C);

als_inode::als_inode() {}

als::state_machine *als_inode::get_als_layer(als::layer_types a2) {
    if constexpr (1) {
        assert(get_system() != nullptr);

        return this->get_system()->get_als_layer(a2);
    } else {
        return (als::state_machine *) THISCALL(0x00689BA0, this, a2);
    }
}

string_hash als_inode::sub_48B100(als::layer_types a3) {
    auto *v3 = this->get_als_layer(a3);

    string_hash id = v3->get_state_id();
    return id;
}

bool als_inode::is_layer_interruptable(als::layer_types a1)
{
    auto *als_layer = this->get_als_layer(a1);
    return als_layer->is_interruptable();
}

string_hash als_inode::get_category_id(als::layer_types a3) {
    auto *v3 = this->get_als_layer(a3);

    string_hash id = v3->get_category_id();
    return id;
}

void als_inode::request_category_transition(
    string_hash a2, als::layer_types a3, bool a4, bool a5, bool a6) {
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

void als_inode::activate(ai_core *a2) {
    THISCALL(0x00693770, this, a2);
}

void als_inode::set_known_combat_signal_time_and_category(Float a2, string_hash a3) {
    this->field_24 = a2;
    this->field_28 = a3;
}

float als_inode::get_eta_of_combat_signal(als::layer_types a2) {
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

            auto tmp = g_world_ptr()->field_158.get_level_time() + a2a;
            this->set_known_combat_signal_time_and_category(tmp, category_id);
        }
        return a2a;
    } else {
        return (float) THISCALL(0x00689C20, this, a2);
    }
}

bool als_inode::sub_48B140(string_hash a2, als::layer_types a3)
{
    auto *als_layer = this->get_als_layer(a3);
    bool (__fastcall *is_cat_our_prev_cat)(const void *, void *, string_hash) = CAST(is_cat_our_prev_cat,get_vfunc(als_layer->m_vtbl, 0x58));
    return is_cat_our_prev_cat(als_layer, nullptr, a2);
}

bool als_inode::anim_finished(string_hash a2, als::layer_types a3)
{
    TRACE("als_inode::anim_finished");

    auto *v3 = this->field_1C->get_als_layer(a3);
    if ( v3->is_cat_our_prev_cat(a2) ) {
        return true;
    }

    if ( v3->get_category_id() == a2 )
    {
        return std::abs(v3->get_time_to_end_of_anim()) < EPSILON;
    }
    else
    {
        return !v3->is_requesting_category(a2);
    }
}

} // namespace ai

void als_inode_patch() {
    {
        FUNC_ADDRESS(address, &ai::als_inode::get_eta_of_combat_signal);
        //set_vfunc(0x0087CEF8, address);
    }

    {
        FUNC_ADDRESS(address, &ai::als_inode::anim_finished);
        SET_JUMP(0x00689E10, address);
    }
}
