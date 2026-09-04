#include "interaction_inode.h"

#include "ai_interaction_data.h"
#include "anim_record.h"
#include "attach_state.h"
#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "generic_interaction.h"
#include "interaction_state.h"
#include "pick_up_state.h"
#include "put_down_state.h"
#include "throw_state.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

namespace ai {

VALIDATE_SIZE(interaction_inode, 0x4C);

interaction_inode::interaction_inode(from_mash_in_place_constructor *a2) : info_node(a2)
{
    this->field_40 = 0;
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->field_28 = 2;
    this->target_handle.field_0 = 0;
    this->field_34 = nullptr;
    this->target_interaction_type = 4;
    this->field_49 = 0;
    this->curr_status = 0;
}

interaction_inode::interaction_inode() : info_node()
{
    this->field_48 = 0;
    this->field_49 = 0;
    this->field_40 = 0;
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->field_28 = 2;
    this->target_handle = {0};
    this->field_34 = nullptr;
    this->target_interaction_type = 4;
    this->field_49 = 0;
    this->curr_status = 0;
}

bool interaction_inode::is_eligible(string_hash a2, bool a3)
{
    TRACE("ai::interaction_inode::is_eligible");

    return THISCALL(0x00479850, this, a2, a3);
}

void interaction_inode::set_curr_anim(anim_key *the_anim_key)
{
    assert(the_anim_key != nullptr);
    this->field_34 = the_anim_key;
}

void interaction_inode::set_curr_anim(enum_anim_key::key_enum a2)
{
    TRACE("interaction_inode::set_curr_anim:");

    if constexpr (0) {
        auto *the_record = this->field_2C->does_anim_exist(a2, false);
        if (the_record != nullptr) {
            assert(the_record->my_key != nullptr);
            this->set_curr_anim(the_record->my_key);
        } else {
            this->field_34 = nullptr;
        }
    } else {
        THISCALL(0x00451340, this, a2);
    }
}

bool interaction_inode::is_in_master_mode()
{
    return this->curr_status == 2;
}

void interaction_inode::init_interaction()
{
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->target_handle.field_0 = 0;
    this->target_interaction_type = 4;
    this->field_34 = nullptr;
    this->field_49 = false;
    this->curr_status = 0;
}

void interaction_inode::clear_interaction(interaction_result_enum a2)
{
    if constexpr (0) {
        string_hash v6;
        entity_base_vhandle v7;
        this->field_28 = a2;
        if (a2 != 0) {
            if (a2 == 1) {
                auto *v3 = this->field_30;
                if (v3 != nullptr && v3->field_45) {
                    v3->set_enabled(false);
                }

                v7 = this->target_handle.field_0;
                v6 = event::INTERACTION_SUCCESS;

            } else if (a2 == 2 && !this->is_interacting()) {
                return;
            }
        } else {
            v7 = this->target_handle.field_0;
            v6 = event::INTERACTION_FAILURE;
        }

        event_manager::raise_event(v6, v7);

        auto *target = this->get_target();
        if (target != nullptr) {
            target->kill_interact_anim();
        }

        if (!this->is_in_master_mode()) {
            auto *v5 = this->field_2C;
            if (v5 != nullptr) {
                v5->unregister_interactor(vhandle_type<actor>{this->field_C->get_my_vhandle().field_0});
            }

            this->init_interaction();
        }
    } else {
        THISCALL(0x0046E3E0, this, a2);
    }
}


string_hash interaction_inode::get_chosen_interact_state_id()
{
    static constexpr auto NUM_INTERACT_TYPES = 4;

    assert(target_interaction_type != NUM_INTERACT_TYPES);

    string_hash result;

    switch (this->target_interaction_type) {
    case 0:
        result = interaction_state::default_id;
        break;
    case 1:

        if (this->is_in_master_mode()) {
            result = put_down_state::default_id;
        } else {
            result = pick_up_state::default_id;
        }

        break;
    case 2:
        result = ai::attach_state::default_id;
        break;
    case 3:
        result = throw_state::default_id;
        break;
    default:
        assert(0 && "Unhandled interaction type.");

        result = string_hash{};

        break;
    }

    return result;
}

actor *interaction_inode::get_target()
{
    actor *target = nullptr;

    if (this->target_handle.get_volatile_ptr()) {
        target = this->target_handle.get_volatile_ptr();
    }

    return target;
}

void interaction_inode::set_flag_from_param(const string_hash &a2, interaction_type_enum a3)
{
    THISCALL(0x00463980, this, &a2, a3);
}

void interaction_inode::set_interaction(const ai_interaction_data *a2, actor *a3, interaction_type_enum a4,
                                        interaction *a5)
{
    assert(is_in_master_mode() || !is_interacting());

    if (!this->is_in_master_mode()) {
        this->curr_status = 1;
    }

    this->target_handle = {a3->my_handle};
    this->field_2C = CAST(this->field_2C, a2);
    if (this->field_2C != nullptr) {
        this->field_2C->register_interactor(this->target_handle);
    }

    this->field_30 = a5;
    this->target_interaction_type = a4;
    this->field_28 = 2;
}

void interaction_inode::set_scripted_start(actor *a2, generic_interaction *a3)
{
    if constexpr (1) {
        this->field_48 = true;
        interaction_type_enum v4 = a3->field_28;

        ai_interaction_data *(__fastcall * get_interaction_data)(void *) =
            CAST(get_interaction_data, get_vfunc(a3->m_vtbl, 0x20));

        auto *v5 = get_interaction_data(a3);
        this->set_interaction(v5, a2, v4, a3);
    } else {
        THISCALL(0x0046E380, this, a2, a3);
    }
}

}  // namespace ai

void interaction_inode_patch()
{
    void (ai::interaction_inode::*func)(enum_anim_key::key_enum) = &ai::interaction_inode::set_curr_anim;
    FUNC_ADDRESS(address, func);
    REDIRECT(0x00463C59, address);

    {
        FUNC_ADDRESS(address, &ai::interaction_inode::is_eligible);
        REDIRECT(0x00488711, address);
        REDIRECT(0x00488B15, address);
        REDIRECT(0x00488DCD, address);
        REDIRECT(0x00488FEE, address);
    }
}
