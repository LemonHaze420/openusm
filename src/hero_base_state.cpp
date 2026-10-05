#include "hero_base_state.h"
#include "venom_base_state.h"

#include "actor.h"
#include "ai_state_jump.h"
#include "ai_state_run.h"
#include "ai_state_web_zip.h"
#include "ai_std_hero.h"
#include "ai_std_combat_target.h"
#include "ai_team.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "combat_inode.h"
#include "combat_state.h"
#include "common.h"
#include "controller_inode.h"
#include "damage_interface.h"
#include "func_wrapper.h"
#include "glass_house_inode.h"
#include "hit_react_state.h"
#include "info_node_desc_list.h"
#include "interaction_inode.h"
#include "physics_inode.h"
#include "plr_loco_crawl_transition_state.h"
#include "spider_monkey.h"
#include "std_default_trans_inode.h"
#include "subdued_state.h"
#include "utility.h"
#include "vtbl.h"

#include <algorithm>
#include <array>

namespace ai {
VALIDATE_SIZE(hero_base_state, 0x1C);
VALIDATE_SIZE(venom_base_state, 0x1C);

namespace {
uint32_t __fastcall hero_type(const hero_base_state *)
{
    return 323;
}
uint32_t __fastcall venom_type(const venom_base_state *)
{
    return 325;
}
bool __fastcall hero_subclass(const hero_base_state *, void *, mash::virtual_types_enum type)
{
    return type == 567 || type == 573;
}
bool __fastcall venom_subclass(const venom_base_state *, void *, mash::virtual_types_enum type)
{
    return type == 323 || type == 567 || type == 573;
}
state_trans_messages __fastcall hero_frame(hero_base_state *, void *, Float)
{
    return TRANS_TOTAL_MSGS;
}
state_trans_action *__fastcall hero_transition(hero_base_state *self, void *, state_trans_action *out, Float dt)
{
    *out = self->_check_transition(dt);
    return out;
}
state_trans_action *__fastcall hero_message(hero_base_state *self, void *, state_trans_action *out, Float dt,
                                            state_trans_messages message)
{
    *out = self->_process_message(dt, message);
    return out;
}
string_hash *__fastcall hero_desired(hero_base_state *self, void *, string_hash *out, Float dt)
{
    *out = self->hero_base_state::get_desired_state_id(dt);
    return out;
}
string_hash *__fastcall venom_desired(venom_base_state *self, void *, string_hash *out, Float dt)
{
    *out = self->get_desired_state_id(dt);
    return out;
}
void __fastcall venom_nodes(venom_base_state *self, void *, info_node_desc_list &nodes)
{
    self->_get_info_node_list(nodes);
}
state_trans_action go_to(string_hash state)
{
    return {GOTO_STATE, state, TRANS_TOTAL_MSGS, nullptr};
}
state_trans_action no_transition()
{
    return {NO_ACTION, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
}
}  // namespace

void *hero_base_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 15> result{};
        std::copy_n(static_cast<void **>(base_state::native_vtable()), 14, result.data());
        result[3] = bit_cast<void *>(&hero_type);
        result[4] = bit_cast<void *>(&hero_subclass);
        result[8] = bit_cast<void *>(&hero_frame);
        result[11] = bit_cast<void *>(&hero_transition);
        result[12] = bit_cast<void *>(&hero_message);
        result[14] = bit_cast<void *>(&hero_desired);
        return result;
    }();
    return table.data();
}

void *venom_base_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 15> result;
        std::copy_n(static_cast<void **>(hero_base_state::native_vtable()), 15, result.data());
        result[3] = bit_cast<void *>(&venom_type);
        result[4] = bit_cast<void *>(&venom_subclass);
        result[9] = bit_cast<void *>(&venom_nodes);
        result[14] = bit_cast<void *>(&venom_desired);
        return result;
    }();
    return table.data();
}

hero_base_state::hero_base_state() : hero_base_state(0) {}
hero_base_state::hero_base_state(int mode) : base_state(mode)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x8774F8;
}
venom_base_state::venom_base_state() : venom_base_state(0) {}
venom_base_state::venom_base_state(int mode) : hero_base_state(mode)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x877570;
}

void venom_base_state::_get_info_node_list(info_node_desc_list &nodes)
{
    nodes.add_entry({physics_inode::default_id, 402});
    nodes.add_entry({hero_inode::default_id, 384});
    nodes.add_entry({web_zip_inode::default_id, 326});
    nodes.add_entry({controller_inode::default_id, 358});
    nodes.add_entry({combat_inode::default_id, 342});
    nodes.add_entry({glass_house_inode::default_id, 383});
}

void hero_base_state::combat_inode_transition_notification(Float dt, string_hash desired)
{
    auto *hero = static_cast<hero_inode *>(get_core()->get_info_node(hero_inode::default_id, true));
    if (desired != combat_state::default_id) {
        hero->field_2C->clear_next_move();
    } else if (mega_god_mode_cheat()) {
        if constexpr (!STANDALONE_SYSTEM) {
            THISCALL(0x00474040, this, dt, desired);
        } else {
            auto *core = get_core();
            const auto team_key = combat_target_inode::team_hash();
            if (!core->field_50.does_parameter_exist(team_key))
                return;
            const auto observer_team = team::manager::get_team_enum_by_hash(core->field_50.get_pb_hash(team_key));
            auto *owner = core->field_64;
            if (damage_interface::find_damageable(owner->get_abs_position(), 30.0f, 3, true) == 0)
                return;
            for (auto *damage : *damage_interface::found_damageable) {
                auto *target = damage->field_4;
                if (target == owner)
                    continue;
                bool enemy;
                if (auto *target_core = target->get_ai_core()) {
                    if (!target_core->field_50.does_parameter_exist(team_key))
                        continue;
                    enemy = team::manager::is_enemy(
                        observer_team,
                        team::manager::get_team_enum_by_hash(target_core->field_50.get_pb_hash(team_key)));
                } else {
                    enemy = (target->field_4 & 0x1000u) != 0;
                }
                if (!enemy)
                    continue;
                auto direction = target->get_abs_position() - owner->get_abs_position();
                direction.normalize();
                const string_hash no_hash{};
                damage->apply_damage(owner,
                                     100000.0f,
                                     6,
                                     owner->get_abs_position(),
                                     direction,
                                     0,
                                     no_hash,
                                     no_hash,
                                     no_hash,
                                     false,
                                     vector3d{},
                                     17,
                                     false);
            }
        }
    }
}

string_hash hero_base_state::get_desired_state_id(Float)
{
    return NO_TRANS;
}

state_trans_action hero_base_state::_check_transition(Float dt)
{
    auto *core = get_core();
    auto *hero = static_cast<hero_inode *>(core->get_info_node(hero_inode::default_id, true));
    const auto current = get_machine()->my_curr_state->get_name();
    auto *defaults =
        static_cast<std_default_trans_inode *>(core->get_info_node(std_default_trans_inode::default_id, true));
    if (current != subdued_state::default_id && defaults->field_26) {
        auto *damage = get_actor()->damage_ifc();
        if (damage && damage->field_1FC.field_0[0] <= 0.0f)
            return go_to(subdued_state::default_id);
    }
    auto *glass = hero->field_44;
    if (glass->field_20 && (current != jump_state::default_id || hero->field_50 != static_cast<eJumpType>(14))) {
        hero->set_jump_type(static_cast<eJumpType>(14), false);
        glass->show_glass_house_message();
        glass->field_20 = false;
        return go_to(jump_state::default_id);
    }
    if (hero->field_23C.get_volatile_ptr()) {
        hero->engage_water_exit();
        return go_to(plr_loco_crawl_transition_state::default_id);
    }
    auto *animation = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
    if (!animation->is_layer_interruptable(static_cast<als::layer_types>(0)) && current != web_zip_state::default_id)
        return no_transition();
    string_hash desired;
    auto callback = reinterpret_cast<string_hash *(__fastcall *)(hero_base_state *, void *, string_hash *, Float)>(
        get_vfunc(m_vtbl, 0x38));
    callback(this, nullptr, &desired, dt);
    if (desired == NO_TRANS)
        return no_transition();
    combat_inode_transition_notification(dt, desired);
    return go_to(desired);
}

state_trans_action hero_base_state::_process_message(Float, state_trans_messages message)
{
    const auto current = get_machine()->my_curr_state->get_name();
    auto *hero = static_cast<hero_inode *>(get_core()->get_info_node(hero_inode::default_id, true));
    if (message == TRANS_SUCCESS_MSG || message == TRANS_FAILURE_MSG) {
        if (hero->field_2C->has_next_move())
            return go_to(combat_state::default_id);
        if (hero->run_is_eligible(current))
            return go_to(run_state::default_id);
        if (hero->jump_is_eligible(current))
            return go_to(jump_state::default_id);
    }
    return no_transition();
}


string_hash venom_base_state::get_desired_state_id(Float dt) const
{
    const auto current = get_machine()->my_curr_state->get_name();
    auto *core = get_core();
    auto *hero = static_cast<hero_inode *>(core->get_info_node(hero_inode::default_id, true));
    auto *combat = hero->field_2C;
    auto *interaction = hero->field_34;
    auto *zip = hero->field_3C;

    if (current == run_state::default_id) {
        if (interaction->is_eligible(current, false)) {
            const auto chosen = interaction->get_chosen_interact_state_id();
            if (hero->run_can_go_to(chosen))
                return chosen;
            interaction->clear_interaction(static_cast<interaction_result_enum>(2));
        }
        if (hero->run_can_go_to(plr_loco_crawl_transition_state::default_id) && hero->crawl_is_eligible(current, true))
            return plr_loco_crawl_transition_state::default_id;
        if (hero->run_can_go_to(jump_state::default_id) && hero->jump_is_eligible(current))
            return jump_state::default_id;
        if (hero->run_can_go_to(web_zip_state::default_id) && zip->is_eligible(current))
            return web_zip_state::default_id;
        if (hero->run_can_go_to(combat_state::default_id) && combat->has_next_move())
            return combat_state::default_id;
    } else if (current == jump_state::default_id) {
        auto *target = vhandle_type<actor>{combat->field_20}.get_volatile_ptr();
        if (target) {
            auto *controller = static_cast<controller_inode *>(core->get_info_node(controller_inode::default_id, true));
            (void)controller->get_axis(static_cast<controller_inode::eControllerAxis>(2));
            get_actor()->get_abs_position();
            target->get_abs_position();
            if (hero->jump_can_go_to(combat_state::default_id) && combat->has_next_move())
                return combat_state::default_id;
        }
        if (hero->jump_can_go_to(plr_loco_crawl_transition_state::default_id) && hero->crawl_is_eligible(current, true))
            return plr_loco_crawl_transition_state::default_id;
        if (hero->jump_can_go_to(run_state::default_id) && hero->run_is_eligible(current))
            return run_state::default_id;
        if (hero->jump_can_go_to(web_zip_state::default_id) && zip->is_eligible(current))
            return web_zip_state::default_id;
        if (hero->jump_can_go_to(combat_state::default_id) && combat->has_next_move())
            return combat_state::default_id;
    } else if (hero_inode::is_a_crawl_state(current, false)) {
        if (hero->crawl_can_go_to(combat_state::default_id, current) && combat->has_next_move()) {
            auto *physics = static_cast<physics_inode *>(core->get_info_node(physics_inode::default_id, true));
            physics->setup_for_bounce();
            return combat_state::default_id;
        }
        if (hero->crawl_can_go_to(run_state::default_id, current) && hero->run_is_eligible(current))
            return run_state::default_id;
        if (hero->crawl_can_go_to(jump_state::default_id, current) && hero->jump_is_eligible(current))
            return jump_state::default_id;
        if (hero->crawl_can_go_to(web_zip_state::default_id, current) && zip->is_eligible(current))
            return web_zip_state::default_id;
    } else if (current == web_zip_state::default_id) {
        if (zip->can_go_to(plr_loco_crawl_transition_state::default_id) && hero->crawl_is_eligible(current, true))
            return plr_loco_crawl_transition_state::default_id;
        if (zip->can_go_to(run_state::default_id) && hero->run_is_eligible(current))
            return run_state::default_id;
        if (zip->can_go_to(jump_state::default_id) && hero->jump_is_eligible(current))
            return jump_state::default_id;
    } else if (current == combat_state::default_id) {
        return combat->needs_hit_react(dt) ? hit_react_state::default_id : NO_TRANS;
    } else {
        return NO_TRANS;
    }
    if (hero->run_can_go_to(hit_react_state::default_id) && combat->needs_hit_react(dt))
        return hit_react_state::default_id;
    return NO_TRANS;
}
}  // namespace ai

void hero_base_state_patch() {}
