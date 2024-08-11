#include "ai_pedestrian.h"

#include "ai_std_combat_target.h"
#include "ai_std_avoidance.h"
#include "ai_team.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "common.h"
#include "game_clock.h"
#include "ped_spawner.h"
#include "physical_interface.h"
#include "std_default_trans_inode.h"
#include "std_fear_inode.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(pedestrian_inode, 0xD8);

VALIDATE_SIZE(pedestrian_idle_state, 0x30);

pedestrian_inode::pedestrian_inode()
{
    this->m_vtbl = 0x0087CF80;
    this->initialize(mash::ALLOCATED);
}

void pedestrian_inode::initialize(mash::allocation_scope a2)
{
    if ( a2 == mash::FROM_MASH ) {
        this->m_ped_spawner = ped_spawner::next_ped_spawner;
    }

    this->m_ped_spawner_cleared = false;
    this->field_D1 = false;
}

void pedestrian_inode::set_ped_spawner(ped_spawner *a2)
{
    if ( this->m_ped_spawner != nullptr )
    {
        if ( a2 == nullptr )
        {
            this->m_ped_spawner_cleared = true;
            this->m_ped_spawner = nullptr;
            return;
        }
    }
    else if ( a2 != nullptr )
    {
        this->m_ped_spawner_cleared = false;
    }

    this->m_ped_spawner = a2;
}

traffic_path_lane * pedestrian_inode::get_cur_lane() const
{
    auto *v1 = this->m_ped_spawner;
    if ( v1 != nullptr ) {
        return v1->field_10;
    } else {
        return nullptr;
    }
}

void pedestrian_inode::set_flag(uint32_t a3, bool a4)
{
    if ( a4 ) {
        this->field_1C |= a3;
    } else {
        this->field_1C &= ~a3;
    }
}

void pedestrian_inode::set_can_dodge(int a1)
{
    static const string_hash can_dodge_hash {int(to_hash("can_dodge"))};
    this->my_param_block.set_pb_int(can_dodge_hash, a1, true);
}

void pedestrian_inode::restore_hit_pts()
{
    float a3 = 10.0;
    this->m_hit_points = this->field_8->get_param_block()->get_optional_pb_float(
            pedestrian_inode::hit_points_hash,
            a3,
            nullptr);

    this->field_D1 = false;
}

void pedestrian_inode::sub_696AF0(Float a2)
{
    this->field_C4 = a2;
    this->field_C8 = 0;
    this->field_CC = g_world_ptr->field_158.field_C;
    auto *v2 = this->field_20;

    auto *v3 = v2->field_1C->get_als_layer(static_cast<als::layer_types>(0));

    als::param v4 {74, this->field_C4};
    v3->set_desired_param(v4);
}

void pedestrian_inode::activate(ai::ai_core *a2)
{
    info_node::activate(a2);
    if ( is_a_pedestrian(this->field_8) )
    {
        auto *trans_inode = (std_default_trans_inode *) a2->get_info_node(
                std_default_trans_inode::default_id,
                false);
        if ( trans_inode != nullptr ) {
            trans_inode->set_enabled(false);
        }
    }

    auto *v5 = this->field_8;
    this->field_20 = (ai::als_inode *)v5->get_info_node(als_inode::default_id, true);

    this->field_48 = nullptr;
    this->restore_hit_pts();

    float v2 = 0;
    auto *v7 = this->field_8->get_param_block();
    this->m_elevation_adj = v7->get_optional_pb_float(
            pedestrian_inode::elevation_adj_hash,
            v2,
            nullptr);

    this->reset();
    timer = game_clock::ticks;
    auto *v8 = this->get_actor();
    this->sub_696AF0(v8->get_abs_position().y);
    this->set_can_dodge(this->is_flagged(1u));
}

bool pedestrian_inode::is_a_pedestrian(ai::ai_core *a1)
{
    if ( a1 != nullptr )
    {
        auto *p_pb = a1->get_param_block();
        if ( p_pb->does_parameter_exist(combat_target_inode::team_hash()) )
        {
            auto pb_hash = p_pb->get_pb_hash(combat_target_inode::team_hash());
            if ( team::manager::get_team_enum_by_hash(pb_hash) == 14 ) {
                return true;
            }
        }
    }

    return false;
}

void pedestrian_inode::unregister_non_ped(vhandle_type<actor> a1)
{
    CDECL_CALL(0x006AE2B0, a1);
}

void pedestrian_idle_state::pedize_non_pedestrian()
{
    assert(!pedestrian_inode::is_a_pedestrian(this->get_core())
            && "This is a pedestrian already");

    auto *my_core = this->field_C->my_core;
    auto *v3 = my_core->field_64;
    if ( v3 != nullptr )
    {
        if ( v3->has_physical_ifc() )
        {
            auto *v5 = v3->physical_ifc();
            v5->set_gravity(true);
        }

        auto *v6 = this->field_C->my_core;
        auto *ped_inode = (pedestrian_inode *)v6->get_info_node(pedestrian_inode::default_id, true);
        auto *v8 = ped_spawner::assign_non_ped_actor(vhandle_type<actor>{v3->my_handle.field_0}, 1);
        if ( v8 != nullptr )
        {
            ped_inode->set_ped_spawner(v8);
            ped_inode->reset();
            pedestrian_inode::unregister_non_ped(vhandle_type<actor>{v3->my_handle.field_0});
            if ( ped_inode->get_cur_lane() != nullptr )
            {
                ped_inode->set_flag(0x20, true);
                ped_inode->set_flag(0x10, true);

                auto *the_core = this->get_core();
                auto *v11 = (ped_avoidance_inode *) the_core->get_info_node(ped_avoidance_inode::default_id, true);
                v11->set_respect_obbs(true);
            }
        }

        auto *v12 = this->get_core();
        auto *v13 = (std_fear_inode *) v12->get_info_node(std_fear_inode::default_id, false);
        if ( v13 != nullptr ) {
            v13->set_cowering_enabled(false);
        }
    }
}

}
