#include "spidey_base_state.h"

#include "actor.h"
#include "ai_interaction_data.h"
#include "ai_state_jump.h"
#include "ai_std_hero.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "combat_state.h"
#include "common.h"
#include "controller_inode.h"
#include "func_wrapper.h"
#include "hit_react_state.h"
#include "interaction_inode.h"
#include "physics_inode.h"
#include "plr_loco_crawl_transition_state.h"
#include "pole_swing_inode.h"
#include "pole_swing_state.h"

#include "ai_state_run.h"
#include "ai_state_swing.h"
#include "ai_state_web_zip.h"
#include "spidey_combat_inode.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"

namespace ai {

VALIDATE_SIZE(spidey_base_state, 0x1C);

spidey_base_state::spidey_base_state()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x00438E80, this);
    }
}

void spidey_base_state::_unmash(mash_info_struct *a1, void *a2)
{
    hero_base_state::_unmash(a1, a2);
}

string_hash spidey_base_state::get_desired_state_id(Float a3) const
{
    TRACE("spidey_base_state::get_desired_state_id");

    {
        string_hash a2 = this->field_C->my_curr_state->get_name();
        sp_log("%s", a2.to_string());
    }

    if constexpr (0) {
        string_hash a2 = this->field_C->my_curr_state->get_name();

        auto *v4 = this->get_core();
        auto *hero_inode_ptr = bit_cast<hero_inode *>(v4->get_info_node(hero_inode::default_id, true));
        auto *web_zip_inode_ptr = hero_inode_ptr->field_3C;
        auto *combat_inode_ptr = hero_inode_ptr->field_2C;
        auto *interaction_inode_ptr = hero_inode_ptr->field_34;
        auto *pole_swing_inode_ptr = hero_inode_ptr->field_48;
        auto *swing_inode_ptr = hero_inode_ptr->field_40;

        if (a2 == run_state::default_id) {
            if (interaction_inode_ptr->is_eligible(a2, false)) {
                if (hero_inode_ptr->run_can_go_to(interaction_inode_ptr->get_chosen_interact_state_id())) {
                    return interaction_inode_ptr->get_chosen_interact_state_id();
                }

                interaction_inode_ptr->clear_interaction(static_cast<interaction_result_enum>(2));
            }

            auto *v13 = combat_inode_ptr;

            auto *v14 = vhandle_type<actor>{combat_inode_ptr->field_20}.get_volatile_ptr();

            this->get_core()->field_50.get_optional_pb_int(loco_allow_web_tie_id(),
                                                           combat_inode_ptr->field_20,
                                                           nullptr);
            this->get_actor();
            if (v14 != nullptr) {
                if (v13->has_next_move()) {
                    if (hero_inode_ptr->run_can_go_to(combat_state::default_id)) {
                        return combat_state::default_id;
                    }
                }
            }

            if (hero_inode_ptr->run_can_go_to(swing_state::default_id)
                    && swing_inode_ptr->is_eligible(a2, a3))
            {
                hero_inode_ptr->set_jump_type(static_cast<ai::eJumpType>(9), false);
                return jump_state::default_id;
            }

            if (hero_inode_ptr->run_can_go_to(plr_loco_crawl_transition_state::default_id)
                    && hero_inode_ptr->crawl_is_eligible(a2, true))
            {
                return plr_loco_crawl_transition_state::default_id;
            }

            if (hero_inode_ptr->run_can_go_to(combat_state::default_id) && v13->has_next_move()) {
                return combat_state::default_id;
            }

            if (v13->needs_hit_react(a3)
                    && hero_inode_ptr->run_can_go_to(hit_react_state::default_id))
            {
                return hit_react_state::default_id;
            }

            if (hero_inode_ptr->run_can_go_to(jump_state::default_id)
                    && hero_inode_ptr->jump_is_eligible(a2))
            {
                return jump_state::default_id;
            }

            if (hero_inode_ptr->run_can_go_to(web_zip_state::default_id)
                    && web_zip_inode_ptr->is_eligible(a2))
            {
                return web_zip_state::default_id;
            }

            return hero_base_state::NO_TRANS;
        }
        
        if (a2 == jump_state::default_id) {
            vhandle_type<actor> v51;
            v51.field_0 = combat_inode_ptr->field_20;
            actor *v19 = v51.get_volatile_ptr();
            if (v19 != nullptr) {
                auto *v20 = this->get_core();
                auto *v21 = (controller_inode *) v20->get_info_node(controller_inode::default_id,
                                                                    true);

                vector3d axis = v21->get_axis(static_cast<controller_inode::eControllerAxis>(2));
                auto *v22 = this->get_actor();
                auto v53 = v19->get_abs_position() - v22->get_abs_position();
                v53.normalize();
                if (dot(axis, v53) != 0.0f) {
                    hero_inode_ptr->jump_can_go_to(pole_swing_state::default_id);
                }

                if (hero_inode_ptr->jump_can_go_to(combat_state::default_id)
                        && combat_inode_ptr->has_next_move())
                {
                    return combat_state::default_id;
                }
            }

            if (interaction_inode_ptr->is_eligible(a2, false)) {
                if (string_hash v46 = interaction_inode_ptr->get_chosen_interact_state_id();
                    hero_inode_ptr->jump_can_go_to(v46)) {
                    return interaction_inode_ptr->get_chosen_interact_state_id();
                }

                interaction_inode_ptr->clear_interaction(static_cast<interaction_result_enum>(2));
            }

            if (hero_inode_ptr->jump_can_go_to(swing_state::default_id)
                    && swing_inode_ptr->is_eligible(a2, a3))
            {
                return swing_state::default_id;
            }

            if (hero_inode_ptr->jump_can_go_to(pole_swing_state::default_id)) {
                if (0) {
                    return pole_swing_state::default_id;
                }
            }

            if (hero_inode_ptr->jump_can_go_to(plr_loco_crawl_transition_state::default_id)
                    && hero_inode_ptr->crawl_is_eligible(a2, true))
            {
                return plr_loco_crawl_transition_state::default_id;
            }

            if (hero_inode_ptr->jump_can_go_to(run_state::default_id)
                    && hero_inode_ptr->run_is_eligible(a2))
            {
                return run_state::default_id;
            }

            if (hero_inode_ptr->jump_can_go_to(web_zip_state::default_id) &&
                web_zip_inode_ptr->is_eligible(a2)) {
                return web_zip_state::default_id;
            }

            if (hero_inode_ptr->jump_can_go_to(hit_react_state::default_id)
                    && combat_inode_ptr->needs_hit_react(a3))
            {
                return hit_react_state::default_id;
            }

            return hero_base_state::NO_TRANS;
        }

        if (hero_inode::is_a_crawl_state(a2, false)) {
            if (vhandle_type<actor>{combat_inode_ptr->field_20}.get_volatile_ptr()) {
                if (hero_inode_ptr->crawl_can_go_to(combat_state::default_id, a2) &&
                    combat_inode_ptr->has_next_move()) {
                    auto *v34 = bit_cast<physics_inode *>(
                            this->get_core()->get_info_node(physics_inode::default_id, true));
                    v34->setup_for_bounce();
                    return combat_state::default_id;
                }
            }

            if (interaction_inode_ptr->is_eligible(a2, false)) {
                if (string_hash v45 = interaction_inode_ptr->get_chosen_interact_state_id();
                    hero_inode_ptr->crawl_can_go_to(v45, a2)) {
                    return interaction_inode_ptr->get_chosen_interact_state_id();
                }

                interaction_inode_ptr->clear_interaction(static_cast<interaction_result_enum>(2));
            }

            if (hero_inode_ptr->crawl_can_go_to(run_state::default_id, a2)
                    && hero_inode_ptr->run_is_eligible(a2))
            {
                return run_state::default_id;
            }

            if (hero_inode_ptr->crawl_can_go_to(jump_state::default_id, a2)
                    && hero_inode_ptr->jump_is_eligible(a2))
            {
                return jump_state::default_id;
            }

            if (hero_inode_ptr->crawl_can_go_to(web_zip_state::default_id, a2)
                    && web_zip_inode_ptr->is_eligible(a2))
            {
                return web_zip_state::default_id;
            }

            if (hero_inode_ptr->crawl_can_go_to(hit_react_state::default_id, a2)
                    && combat_inode_ptr->needs_hit_react(a3))
            {
                hero_inode_ptr->set_jump_type(static_cast<ai::eJumpType>(5), false);

                return jump_state::default_id;
            }

            return hero_base_state::NO_TRANS;
        }

        if (a2 == swing_state::default_id) {
            if (interaction_inode_ptr->is_eligible(a2, false)) {
                if (string_hash v46 = interaction_inode_ptr->get_chosen_interact_state_id();
                    swing_inode_ptr->can_go_to(v46)) {

                    return interaction_inode_ptr->get_chosen_interact_state_id();
                }

                interaction_inode_ptr->clear_interaction(static_cast<interaction_result_enum>(2));
            }

            if (swing_inode_ptr->can_go_to(plr_loco_crawl_transition_state::default_id)) {
                if (hero_inode_ptr->crawl_is_eligible(a2, true)) {

                    return plr_loco_crawl_transition_state::default_id;
                }
            }

            if (swing_inode_ptr->can_go_to(jump_state::default_id)) {
                if (hero_inode_ptr->jump_is_eligible(a2)) {
                    return jump_state::default_id;
                }
            }

            if (swing_inode_ptr->can_go_to(run_state::default_id)
                    && hero_inode_ptr->run_is_eligible(a2))
            {
                return run_state::default_id;
            }

            if (swing_inode_ptr->can_go_to(hit_react_state::default_id)
                    && combat_inode_ptr->needs_hit_react(a3))
            {
                return hit_react_state::default_id;
            }

            return hero_base_state::NO_TRANS;

        } else if (a2 == web_zip_state::default_id) {

            if (web_zip_inode_ptr->can_go_to(plr_loco_crawl_transition_state::default_id)
                    && hero_inode_ptr->crawl_is_eligible(a2, true))
            {
                return plr_loco_crawl_transition_state::default_id;
            }

            if (web_zip_inode_ptr->can_go_to(run_state::default_id)
                    && hero_inode_ptr->run_is_eligible(a2))
            {
                return run_state::default_id;
            }

            if (web_zip_inode_ptr->can_go_to(jump_state::default_id)
                    && hero_inode_ptr->jump_is_eligible(a2))
            {
                return jump_state::default_id;
            }

            if (web_zip_inode_ptr->can_go_to(hit_react_state::default_id)
                    && combat_inode_ptr->needs_hit_react(a3))
            {
                return hit_react_state::default_id;
            }

            return hero_base_state::NO_TRANS;

        } else if (a2 == pole_swing_state::default_id) {
            if (pole_swing_inode_ptr->can_go_to(jump_state::default_id)
                    && hero_inode_ptr->jump_is_eligible(a2))
            {
                return jump_state::default_id;
            }

            return hero_base_state::NO_TRANS;
        } else if (a2 == combat_state::default_id) {
            if (combat_inode_ptr->needs_hit_react(a3)) {
                if (combat_inode_ptr->get_cur_move()->field_80.field_4.field_8 != 16) {
                    return hit_react_state::default_id;
                }
            }

            return hero_base_state::NO_TRANS;
        } else {
            return hero_base_state::NO_TRANS;
        }
    } else {
        string_hash result;
        THISCALL(0x00488680, this, &result, a3);

        sp_log("%s", result.to_string());
        return result;
    }
}

} // namespace ai


string_hash * __fastcall get_desired_state_id(ai::spidey_base_state *self, void *, string_hash *a2, Float a3)
{
    *a2 = self->get_desired_state_id(a3);
    return a2;
}

void spidey_base_state_patch()
{
        set_vfunc(0x0087756C, &get_desired_state_id);
}
