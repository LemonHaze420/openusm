#include "hero_base_state.h"

#include "actor.h"
#include "ai_state_jump.h"
#include "ai_state_web_zip.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "damage_interface.h"
#include "func_wrapper.h"
#include "glass_house_inode.h"
#include "plr_loco_crawl_transition_state.h"
#include "state_machine.h"
#include "state_trans_action.h"
#include "std_default_trans_inode.h"
#include "subdued_state.h"
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


state_trans_action hero_base_state::check_transition(Float a3)
{
    TRACE("hero_base_state::check_transition");

    state_trans_action result;

    if constexpr (0) {
        auto *v4 = this->get_core();
        auto *hero_inode_ptr = (hero_inode *)v4->get_info_node(hero_inode::default_id, true);

        string_hash v21 = this->field_C->my_curr_state->get_name();

        actor *v9;
        actor *v8;

        auto *v6 = this->get_core();
        auto *v7 = (std_default_trans_inode *)v6->get_info_node(std_default_trans_inode::default_id, true);
        if (v21 != subdued_state::default_id) {
            if (v7->field_26) {
                v8 = this->get_actor();
                if (v8->damage_ifc()) {
                    v9 = this->get_actor();
                    if (v9->damage_ifc()->field_1FC.field_0[0] <= 0.0f) {
                        result = state_trans_action{
                            state_trans_actions::TRANSITION, subdued_state::default_id, TRANS_TOTAL_MSGS, nullptr};
                        return result;
                    }
                }
            }
        }

        auto *glass_house_inode_ptr = hero_inode_ptr->field_44;
        if (glass_house_inode_ptr->field_20 &&
            (v21 != ai::jump_state::default_id || hero_inode_ptr->field_50 != static_cast<ai::eJumpType>(14))) {
            hero_inode_ptr->set_jump_type(static_cast<ai::eJumpType>(14), false);
            glass_house_inode_ptr->show_glass_house_message();
            glass_house_inode_ptr->field_20 = 0;
            return result = state_trans_action{
                       state_trans_actions::TRANSITION, ai::jump_state::default_id, TRANS_TOTAL_MSGS, nullptr};
        }

        if (hero_inode_ptr->field_23C.get_volatile_ptr() != nullptr) {
            hero_inode_ptr->engage_water_exit();
            result = state_trans_action{state_trans_actions::TRANSITION,
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
                result = state_trans_action{state_trans_actions::TRANSITION, v20, TRANS_TOTAL_MSGS, nullptr};
                return result;
            } else {
                result =
                    state_trans_action{static_cast<state_trans_actions>(3), string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
                return result;
            }

        } else {
            result = state_trans_action{static_cast<state_trans_actions>(3), string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
            return result;
        }

    } else {
        THISCALL(0x00478D80, this, &result, a3);
        return result;
    }
}

}  // namespace ai

void *__fastcall check_transition(ai::hero_base_state *self, void *, ai::state_trans_action *a2, Float a3)
{
    *a2 = self->check_transition(a3);
    return a2;
}


void hero_base_state_patch()
{
    set_vfunc(0x00877560, &check_transition);

    {
        FUNC_ADDRESS(address, &ai::hero_base_state::combat_inode_transition_notification);
        REDIRECT(0x00478F55, address);
    }
}
