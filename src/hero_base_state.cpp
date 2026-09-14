#include "hero_base_state.h"

#include "actor.h"
#include "aimed_throw_state.h"
#include "ai_state_jump.h"
#include "ai_state_run.h"
#include "ai_state_web_zip.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "attach_state.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "combat_inode.h"
#include "combat_state.h"
#include "common.h"
#include "damage_interface.h"
#include "func_wrapper.h"
#include "glass_house_inode.h"
#include "hit_react_state.h"
#include "interaction_state.h"
#include "physical_interface.h"
#include "pick_up_state.h"
#include "plr_loco_crawl_transition_state.h"
#include "put_down_state.h"
#include "state_machine.h"
#include "state_trans_action.h"
#include "std_default_trans_inode.h"
#include "subdued_state.h"
#include "throw_state.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

namespace ai {

VALIDATE_SIZE(hero_base_state, 0x1C);

hero_base_state::hero_base_state() {}

hero_base_state::hero_base_state(int a2) : base_state(a2) {}

void hero_base_state::combat_inode_transition_notification(Float a2, string_hash a3)
{
    TRACE("hero_base_state::combat_inode_transition_notification");

    THISCALL(0x00474040, this, a2, a3);
}

string_hash hero_base_state::get_desired_state_id(Float)
{
    return hero_base_state::NO_TRANS;
}


state_trans_action hero_base_state::_check_transition(Float a3)
{
    TRACE("hero_base_state::check_transition");

    state_trans_action result;

    if constexpr (0) {
        auto *v4 = this->get_core();
        auto *hero_inode_ptr = (hero_inode *)v4->get_info_node(hero_inode::default_id, true);

        string_hash v21 = this->field_C->my_curr_state->get_name();

        auto *v6 = this->get_core();
        auto *v7 = (std_default_trans_inode *)v6->get_info_node(std_default_trans_inode::default_id, true);
        if (v21 != subdued_state::default_id) {
            if (v7->field_26) {
                auto *v8 = this->get_actor();
                if (v8->damage_ifc() != nullptr) {
                    auto *v9 = this->get_actor();
                    if (v9->damage_ifc()->field_1FC.field_0[0] <= 0.0f) {
                        result = state_trans_action{
                            state_trans_actions::GOTO_STATE, subdued_state::default_id, TRANS_TOTAL_MSGS, nullptr};
                        return result;
                    }
                }
            }
        }

        auto *glass_house_inode_ptr = hero_inode_ptr->field_44;
        if (glass_house_inode_ptr->field_20 &&
            (v21 != jump_state::default_id || hero_inode_ptr->field_50 != static_cast<eJumpType>(14))) {
            hero_inode_ptr->set_jump_type(static_cast<eJumpType>(14), false);
            glass_house_inode_ptr->show_glass_house_message();
            glass_house_inode_ptr->field_20 = 0;
            return result = state_trans_action{
                       state_trans_actions::GOTO_STATE, jump_state::default_id, TRANS_TOTAL_MSGS, nullptr};
        }

        if (hero_inode_ptr->field_23C.get_volatile_ptr() != nullptr) {
            hero_inode_ptr->engage_water_exit();
            result = state_trans_action{state_trans_actions::GOTO_STATE,
                                        plr_loco_crawl_transition_state::default_id,
                                        TRANS_TOTAL_MSGS,
                                        nullptr};
            return result;
        }

        ai_core *v13 = this->get_core();
        auto *the_als_i = (als_inode *)v13->get_info_node(als_inode::default_id, true);
        if (the_als_i->is_layer_interruptable(static_cast<als::layer_types>(0)) || v21 == web_zip_state::default_id) {
            auto v20 = this->get_desired_state_id(a3);
            if (v20 != NO_TRANS) {
                auto name = this->field_C->my_curr_state->get_name();

                this->combat_inode_transition_notification(a3, v20);
                printf("%s => %s", name.to_string(), v20.to_string());
                result = state_trans_action{state_trans_actions::GOTO_STATE, v20, TRANS_TOTAL_MSGS, nullptr};
                return result;
            } else {
                result = state_trans_action{state_trans_actions::GOTO_STATE, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
                return result;
            }

        } else {
            result = state_trans_action{state_trans_actions::NO_ACTION, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
            return result;
        }

    } else {
        THISCALL(0x00478D80, this, &result, a3);
        return result;
    }
}

#define AI_STATE_ID(state_name) state_name##_state::default_id

state_trans_action hero_base_state::_process_message(Float, state_trans_messages a4)
{
    TRACE("hero_base_state::process_message");

    auto curr_state = this->get_machine()->get_curr_state();
    auto state_name = curr_state->get_name();
    auto *the_core = this->get_core();
    auto *hero_inode_ptr = (hero_inode *)the_core->get_info_node(hero_inode::default_id, true);

    string_hash v8;
    if (a4 == 1 || a4 == 2) {
        assert(state_name == AI_STATE_ID(combat) || state_name == AI_STATE_ID(hit_react) ||
               state_name == AI_STATE_ID(throw) || state_name == AI_STATE_ID(aimed_throw) ||
               state_name == AI_STATE_ID(pick_up) || state_name == AI_STATE_ID(put_down) ||
               state_name == AI_STATE_ID(interaction) || state_name == AI_STATE_ID(attach));

        if (hero_inode_ptr->field_2C->has_next_move()) {
            state_trans_action result{state_trans_actions::GOTO_STATE,
                                      combat_state::default_id,
                                      state_trans_messages::TRANS_TOTAL_MSGS,
                                      nullptr};
            return result;
        }

        if (hero_inode_ptr->run_is_eligible(state_name)) {
            state_trans_action result{state_trans_actions::GOTO_STATE,
                                      run_state::default_id,
                                      state_trans_messages::TRANS_TOTAL_MSGS,
                                      nullptr};
            return result;
        }

        if (hero_inode_ptr->jump_is_eligible(state_name)) {
            state_trans_action result{state_trans_actions::GOTO_STATE,
                                      jump_state::default_id,
                                      state_trans_messages::TRANS_TOTAL_MSGS,
                                      nullptr};
            return result;
        }

        assert(this->get_actor()->physical_ifc()->allow_manage_standing() &&
               "Manage standing needs to be re-enabled for the hero before exiting AI state.");

        assert(0 && "hero_base_state: combat finished, but could not find a state to transition to.");
    }

    state_trans_action result{
        state_trans_actions::NO_ACTION, string_hash{0}, state_trans_messages::TRANS_TOTAL_MSGS, nullptr};
    return result;
}

}  // namespace ai

void *__fastcall check_transition(ai::hero_base_state *self, void *, ai::state_trans_action *a2, Float a3)
{
    *a2 = self->_check_transition(a3);
    return a2;
}

ai::state_trans_action *__fastcall process_message(ai::hero_base_state *self, void *, ai::state_trans_action *out,
                                                   Float a3, ai::state_trans_messages a4)
{
    *out = self->_process_message(a3, a4);
    return out;
}


void hero_base_state_patch()
{
    {
        FUNC_ADDRESS(address, &ai::base_state::_activate);
        set_vfunc(0x00877510, address);
        set_vfunc(0x0087754C, address);
    }

    {
        set_vfunc(0x00877560, &check_transition);
    }

    {
        set_vfunc(0x00877528, &process_message);
        set_vfunc(0x00877564, &process_message);
        set_vfunc(0x008775A0, &process_message);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_base_state::combat_inode_transition_notification);
        REDIRECT(0x00478F55, address);
    }
}
