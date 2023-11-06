#include "plr_loco_crawl_state.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_plr_loco_crawling.h"
#include "ai_std_hero.h"
#include "als_animation_logic_system.h"
#include "als_animation_logic_system_interface.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "colgeom_alter_sys.h"
#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "game_settings.h"
#include "oldmath_po.h"
#include "param_list.h"
#include "physical_interface.h"
#include "state_machine.h"
#include "trace.h"
#include "wds.h"

#include <cmath>

VALIDATE_SIZE(plr_loco_crawl_state, 0x48);

plr_loco_crawl_state::plr_loco_crawl_state() {}

void plr_loco_crawl_state::activate(
        ai::ai_state_machine *a2,
        ai::mashed_state *a3,
        ai::mashed_state *a4,
        ai::param_block *a5,
        ai::base_state::activate_flag_e a6)
{
    TRACE("plr_loco_crawl_state::activate");

    if constexpr (0) {
        ai::enhanced_state::activate(a2, a3, a4, a5, a6);
        auto *act = this->get_actor();
        this->set_player_mode(act);
        auto *the_core = this->get_core();
        auto *info_node = (ai::hero_inode *)the_core->get_info_node(ai::hero_inode::default_id, true);
        auto *v11 = this->get_actor();
        if ( v11->has_physical_ifc() ) {
            auto *the_actor = this->get_actor();
            auto *v13 = the_actor->physical_ifc();
            v13->field_C |= 0x200u;

            auto *v15 = the_actor->physical_ifc();
            v15->set_allow_manage_standing(true);

            auto *v17 = the_actor->physical_ifc();
            v17->enable(true);

            auto *v19 = the_actor->physical_ifc();
            v19->suspend(false);
        }

        auto *v20 = this->get_actor();
        if ( v20->has_physical_ifc() ) {
            auto *v21 = this->get_actor();
            auto *v22 = v21->physical_ifc();
            v22->set_gravity(false);
            auto *v23 = this->get_actor();
            auto *v24 = v23->physical_ifc();
            v24->cancel_all_velocity();
        }

        this->m_wallrun_deviation = -1.0;
        this->field_38 = ZEROVEC;
        auto *v25 = this->get_core()->field_70;
        if ( v25 != nullptr )
        {
            v25->set_avoid_floor(0);
            v25->set_mode((capsule_alter_sys::eAlterMode)2);

            vector3d v37;
            v37[0] = -0.30000001;
            v37[1] = 0.0;
            v37[2] = 0.30000001;


            vector3d v38;
            v38[0] = 0.0;
            v38[1] = 0.0;
            v38[2] = 0.30000001;
            v25->set_static_capsule(v38, v37, 0.30000001);
        }

        auto v35 = ai::als_inode::default_id;
        auto *v26 = this->get_core();
        auto *v27 = (ai::als_inode *)v26->get_info_node(v35, true);
        v27->request_category_transition(plr_loco_crawl_state::crawl_als_category_hash, (als::layer_types) 0, true, false, false);
        info_node->compute_curr_ground_plane((force_recompute_enum)1, 2.5);
        auto *player_controller = this->get_actor()->m_player_controller;
        if ( player_controller != nullptr ) {
            player_controller->frame_advance(g_world_ptr()->field_158.field_18);
        }

        info_node->update_crawl_als_params();
        this->update_wallrun(0.000099999997);
        bit_cast<als::animation_logic_system_interface *>(v27->field_1C)->force_update();
        auto *v29 = this->get_actor();
        if ( v29->has_physical_ifc() ) {
            auto *v30 = this->get_actor();
            auto *v31 = this->get_actor();
            auto *v32 = v30->physical_ifc();
            auto is_effectively_standing = v32->is_effectively_standing();
            auto *v33 = v31->physical_ifc();
            v33->manage_standing(is_effectively_standing);
        }
    } else {
        THISCALL(0x0046A080, this, a2, a3, a4, a5, a6);
    }
}

void plr_loco_crawl_state::deactivate(const ai::mashed_state *a1)
{
    TRACE("plr_loco_crawl_state::deactivate");

    THISCALL(0x0044B6A0, this, a1);
    assert(0);
}

ai::state_trans_messages plr_loco_crawl_state::frame_advance(Float a2)
{
    TRACE("plr_loco_crawl_state::frame_advance");

    if constexpr (1) {
        auto result = ai::enhanced_state::frame_advance(a2);
        auto *the_core = this->get_core();
        auto *als_inode_ptr = (ai::als_inode *) the_core->get_info_node(ai::als_inode::default_id, true);
        auto *hero_inode_ptr = (ai::hero_inode *) the_core->get_info_node(ai::hero_inode::default_id, true);
        auto *player_controller = this->get_actor()->m_player_controller;
        if ( player_controller->get_spidey_loco_mode() == 14 && this->field_1C > 1.0 )
        {
            player_controller->field_10 = 14;
            player_controller->field_C = 2;
        }

        this->update_wallrun(a2);
        hero_inode_ptr->update_crawl_als_params();
        auto *the_actor = this->get_actor();
        if ( the_actor->is_frame_delta_valid() )
        {
            auto *v11 = this->get_actor();
            vector3d v35 = v11->get_frame_delta()->get_position();
            auto *v12 = this->get_actor();
            auto &abs_po = v12->get_abs_po();

            [[maybe_unused]] auto v36 = orthogonal_projection_onto_plane(v35, abs_po.get_y_facing());
            auto lateral_distance = v35.length();
            assert(lateral_distance >= 0.0f);

            if ( ai::hero_inode::get_hero_type() == 2 ) {
                g_game_ptr()->gamefile->update_miles_crawled_venom(lateral_distance);
            } else {
                g_game_ptr()->gamefile->update_miles_crawled_spidey(lateral_distance);
            }
        }

        static constexpr bool do_internal_transitions = true;
        static constexpr bool do_external_transitions = true;

        actor *v15 = nullptr;
        physical_interface *v16 = nullptr;
        auto *v14 = this->get_actor();
        if ( v14->has_physical_ifc()
            && (v15 = this->get_actor(),
            v16 = v15->physical_ifc(),
            !v16->is_effectively_standing())
            && this->can_handle_message((ai::state_trans_messages)72, false) )
        {
            result= static_cast<ai::state_trans_messages>(72);
        }
        else
        {
            auto *v17 = this->get_actor();
            if ( (v17->field_8 & 0x10000000) != 0 ) {
                v17->update_abs_po(1);
            }

            static constexpr float autocrawl_timeout = 0.44999999;

            if ( v17->my_abs_po->m[1][1] > 0.80000001
                    && this->can_handle_message((ai::state_trans_messages)71, false) )
            {
                result = static_cast<ai::state_trans_messages>(71);
            }
            else if ( hero_inode_ptr->field_238 < autocrawl_timeout || player_controller->get_spidey_loco_mode() == 14 )
            {
                if ( do_internal_transitions ) {
                    auto hero_type = ai::hero_inode::get_hero_type();
                    auto v29 = hero_type != 2;
                    auto *v19 = this->get_actor();
                    auto *v20 = check_interior_transition(
                                           v19,
                                           hero_inode_ptr->field_20C,
                                           als_inode_ptr,
                                           v29,
                                           false,
                                           false);
                    if ( v20->field_58 ) {
                        hero_inode_ptr->set_surface_info(*v20);
                        result = ai::TRANS_TRANSITION_MSG;
                    }
                }

                if ( result == ai::TRANS_TOTAL_MSGS && do_external_transitions )
                {
                    auto v21 = ai::hero_inode::get_hero_type();
                    auto v30 = v21 != 2;
                    auto *v22 = this->get_actor();
                    auto *v20 = check_exterior_transition(v22,
                                                        hero_inode_ptr->field_20C,
                                                        als_inode_ptr,
                                                        v30, false, false);
                    if ( v20->field_58 ) {
                        hero_inode_ptr->set_surface_info(*v20);
                        result = ai::TRANS_TRANSITION_MSG;
                    }
                }
            }
        }

        hero_inode_ptr->compute_curr_ground_plane((force_recompute_enum)1, 2.5);
        auto v26 = hero_inode_ptr->field_88.field_10;
        sp_log("%s", v26.to_string().c_str());

        if ( v26.length2() < 0.000099999997 ) {
            return static_cast<ai::state_trans_messages>(72);
        }

        return result;
    } else {
        ai::state_trans_messages (__fastcall *func)(void *, void *, Float) = CAST(func, 0x0047D710);
        auto result = func(this, nullptr, a2);
        printf("%d\n", result);

        return result;
    }
}

int plr_loco_crawl_state::set_player_mode(actor *a1)
{
    TRACE("plr_loco_crawl_state::set_player_mode");

    auto *player_controller = a1->m_player_controller;
    auto result = player_controller->field_C;
    if ( result != 14 )
    {
        if ( result != 2 ) {
            player_controller->field_10 = result;
        }

        player_controller->field_C = 2;
    }

    return result;
}

void plr_loco_crawl_state::update_wallrun([[maybe_unused]] Float a2) {
    TRACE("plr_loco_crawl_state::update_wallrun");

    auto *v3 = this->get_core();
    auto *v4 = (ai::als_inode *) v3->get_info_node(ai::als_inode::default_id, true);
    auto v5 = this->m_wallrun_deviation;
    auto *v6 = v4;
    float v21 = 1.0;

    static constexpr auto deviance_forgiveness_timeout = 0.1f;

    if (v5 < 0.0f || this->field_1C < deviance_forgiveness_timeout) {
        auto *v9 = this->get_actor();
        if ((v9->field_8 & 0x10000000) != 0) {
            v9->update_abs_po(1);
        }

        this->field_38 = v9->my_abs_po->m[2];
        this->m_wallrun_deviation = 0.0;
    } else {
        auto *v7 = this->get_actor();
        auto *v8 = v7;
        if ((v7->field_8 & 0x10000000) != 0) {
            v7->update_abs_po(1);
        }

        this->m_wallrun_deviation = 1.0f -
            std::abs(this->field_38[2] * v8->my_abs_po->m[2][2] +
                     this->field_38[1] * v8->my_abs_po->m[2][1] +
                     this->field_38[0] * v8->my_abs_po->m[2][0]);
    }

    auto *v10 = this->get_actor();
    if ((v10->field_8 & 0x10000000) != 0) {
        v10->update_abs_po(true);
    }

    auto v11 = -YVEC[0] * v10->my_abs_po->m[2][0] +
        -YVEC[1] * v10->my_abs_po->m[2][1] +
        -YVEC[2] * v10->my_abs_po->m[2][2];
    if (v11 >= 0.0f) {
        this->m_wallrun_deviation = v11 * v11 + this->m_wallrun_deviation;
    }

    auto *v12 = this->get_actor()->m_player_controller;
    if ((v12->gb_grab.m_flags & 0x20) == 0) {
        if (v12->gb_grab.m_flags & 1) {
            if (this->field_1C > 0.12f)
                this->m_wallrun_deviation = 1.0;
        }
    }

    auto *v13 = v6->get_als_layer({0});

    string_hash tmp = v13->get_state_id();

    static Var<string_hash> wall_run_state_hash {0x009585DC};

    if (tmp == wall_run_state_hash()) {
        auto v14 = this->field_1C / 2.0f;
        if (v14 > 1.0f) {
            v14 = 1.0f;
        }

        constexpr auto flt_91F67C = 0.69999999f;

        v21 = (1.0f - flt_91F67C) * (1.0f - v14) + flt_91F67C;
    }

    als::param_list list;

    list.add_param({72, this->m_wallrun_deviation});
    list.add_param({16, v21});

    auto *v15 = v6->get_als_layer({0});
    v15->set_desired_params(list);

    auto *v16 = this->get_core();
    auto *v17 = (ai::hero_inode *) v16->get_info_node(ai::hero_inode::default_id, true);
    v17->update_wall_run_als_params();
}

void plr_loco_crawl_state_patch()
{
    REDIRECT(0x0047D98B, check_interior_transition);

    REDIRECT(0x0047D9D8, check_exterior_transition);

    {
        FUNC_ADDRESS(address, &plr_loco_crawl_state::update_wallrun);
        set_vfunc(0x00875E10, address);
    }

    {
        FUNC_ADDRESS(address, &plr_loco_crawl_state::activate);
        set_vfunc(0x00875DE0, address);
    }

    {
        FUNC_ADDRESS(address, &plr_loco_crawl_state::deactivate);
        set_vfunc(0x00875DE4, address);
    }

    {
        FUNC_ADDRESS(address, &plr_loco_crawl_state::frame_advance);
        set_vfunc(0x00875DE8, address);
    }
}
