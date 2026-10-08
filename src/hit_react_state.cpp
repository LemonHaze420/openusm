#include "hit_react_state.h"

#include "actor.h"
#include "ai_universal_soldier_inode.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "combat_inode.h"
#include "common.h"
#include "damage_interface.h"
#include "event.h"
#include "femanager.h"
#include "igofrontend.h"
#include "info_node_desc_list.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "state_machine.h"
#include "std_default_trans_inode.h"
#include "subdued_state.h"
#include "targeting_reticle.h"
#include "track_field_inode.h"
#include "vtbl.h"
#include "wds.h"
#include "web_interface.h"

#include <algorithm>
#include <array>
#include <cstdlib>

namespace ai {
VALIDATE_SIZE(hit_react_state, 0xDC);
VALIDATE_OFFSET(hit_react_state, reaction, 0x50);
VALIDATE_OFFSET(hit_react_state, direction, 0xCC);
namespace {
unsigned __fastcall react_type(const hit_react_state *)
{
    return 369;
}
int __fastcall react_size(const hit_react_state *)
{
    return sizeof(hit_react_state);
}
bool __fastcall react_subclass(const hit_react_state *, void *, unsigned type)
{
    return type == 331 || type == 535 || type == 567 || type == 573;
}
void __fastcall react_destruct(hit_react_state *self, void *)
{
    self->category.destruct_mashed_class();
    self->next_state.destruct_mashed_class();
    self->reaction.field_4.destruct_mashed_class();
    self->reaction.field_8.destruct_mashed_class();
    self->reaction.field_C.destruct_mashed_class();
    self->reaction.field_10.destruct_mashed_class();
    using callback = void(__fastcall *)(enhanced_state *, void *);
    reinterpret_cast<callback>(static_cast<void **>(enhanced_state::native_vtable())[0])(self, nullptr);
}
void __fastcall react_unmash(hit_react_state *self, void *, mash_info_struct *info, void *owner)
{
    using callback = void(__fastcall *)(enhanced_state *, void *, mash_info_struct *, void *);
    reinterpret_cast<callback>(static_cast<void **>(enhanced_state::native_vtable())[1])(self, nullptr, info, owner);
    self->category.unmash(info, self);
    self->next_state.unmash(info, self);
    mash_virtual_base::fixup_vtable(&self->reaction);
    self->reaction._unmash(info, self);
}
void *__fastcall delete_react(hit_react_state *self, void *, unsigned flags)
{
    self->~hit_react_state();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
void __fastcall activate_react(hit_react_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                               const mashed_state *previous, const param_block *params,
                               base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
void __fastcall deactivate_react(hit_react_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
state_trans_messages __fastcall frame_react(hit_react_state *self, void *, Float delta)
{
    return self->_frame_advance(delta);
}
state_trans_action *__fastcall transition_react(hit_react_state *self, void *, state_trans_action *out, Float delta)
{
    *out = self->_check_transition(delta);
    return out;
}
bool __fastcall react_done(hit_react_state *self, void *)
{
    return self->is_done_hit_reacting();
}
bool __fastcall react_signal(hit_react_state *self, void *, Float delta)
{
    return self->is_event_raised(delta);
}
void __fastcall react_info(hit_react_state *, void *, info_node_desc_list &list)
{
    list.add_entry({std_default_trans_inode::default_id, 371});
    list.add_entry({combat_inode::default_id, 342});
    list.add_entry({als_inode::default_id, 333});
}
int current_attack_id(combat_inode *node)
{
    using callback = int(__fastcall *)(combat_inode *, void *);
    return reinterpret_cast<callback>(get_vfunc(node->m_vtbl, 0xC0))(node, nullptr);
}
combat_inode *actor_combat(actor *owner)
{
    return static_cast<combat_inode *>(owner->get_ai_core()->get_info_node(combat_inode::default_id, true));
}
combat_inode::incoming_move *incoming_at(combat_inode *node, int index)
{
    using callback = combat_inode::incoming_move *(__fastcall *)(combat_inode *, void *, int);
    return reinterpret_cast<callback>(get_vfunc(node->m_vtbl, 0xCC))(node, nullptr, index);
}
bool feeding(combat_inode *node)
{
    using callback = bool(__fastcall *)(combat_inode *, void *);
    return reinterpret_cast<callback>(get_vfunc(node->m_vtbl, 0x120))(node, nullptr);
}
track_field_inode *track_test(ai_core *core)
{
    return static_cast<track_field_inode *>(core->get_info_node(string_hash{int(to_hash("track_field"))}, false));
}
void update_webbing(actor *owner)
{
    if (owner->field_88 && owner->field_88->field_18 != 1 && owner->has_damage_ifc() &&
        owner->damage_ifc()->field_21C.field_0[0] > 0.75f)
        owner->field_88->set_webbed(true);
}
void show_reticle(bool shown)
{
    if (auto *reticle = g_femanager.IGO->m_targeting_reticle)
        reticle->field_0 = shown;
}
void add_wall_position(als::param_list &params, actor *owner, actor *source, const combo_system_move::results &result)
{
    if (result.field_2C == 8) {
        const auto position = owner->get_abs_position();
        const auto offset = source->get_abs_position() - position;
        const float distance = offset.length();
        params.add_param(0x21, position + offset / distance * (distance - bit_cast<float>(result.field_30)));
    }
}
}

void *hit_react_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 18> result;
        std::copy_n(static_cast<void **>(std_interrupt_state::native_vtable()), 16, result.data());
        result[0] = bit_cast<void *>(&react_destruct);
        result[1] = bit_cast<void *>(&react_unmash);
        result[2] = bit_cast<void *>(&delete_react);
        result[3] = bit_cast<void *>(&react_type);
        result[4] = bit_cast<void *>(&react_subclass);
        result[6] = bit_cast<void *>(&activate_react);
        result[7] = bit_cast<void *>(&deactivate_react);
        result[8] = bit_cast<void *>(&frame_react);
        result[9] = bit_cast<void *>(&react_info);
        result[11] = bit_cast<void *>(&transition_react);
        result[13] = bit_cast<void *>(&react_size);
        result[16] = bit_cast<void *>(&react_done);
        result[17] = bit_cast<void *>(&react_signal);
        return result;
    }();
    return table.data();
}
hit_react_state::hit_react_state()
    : category_entered{false}, attacker{0}, web_reaction{false}, was_webbed{false}, transition_pending{false},
      field_D8{false}
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
hit_react_state::hit_react_state(from_mash_in_place_constructor *tag)
    : std_interrupt_state(tag), category(tag), attacker{0}, next_state(tag), reaction(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
hit_react_state::~hit_react_state()
{
    if (get_machine()) {
        if (auto *source = static_cast<actor *>(attacker.get_volatile_ptr()))
            source->remove_collision_ignorance(get_actor()->get_my_handle());
    }
}
void hit_react_state::update_source_parameters()
{
    als::param_list params;
    params.add_param(0x53, direction);
    (void)get_actor()->get_abs_po();
    animation->set_desired_params(params, static_cast<als::layer_types>(0));
    params.clear();
}
void hit_react_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *core = get_core();
    static_cast<std_default_trans_inode *>(core->get_info_node(std_default_trans_inode::default_id, true))->field_30 =
        this;
    transition_pending = false;
    core->stop_movement();
    combat = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    animation = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
    using direction_fn = vector3d *(__fastcall *)(combat_inode *, void *, vector3d *);
    reinterpret_cast<direction_fn>(get_vfunc(combat->m_vtbl, 0x110))(combat, nullptr, &direction);
    combat->clear_cur_move();
    combat->clear_next_move();
    auto *owner = get_actor();
    if (animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        using category_fn = string_hash *(__fastcall *)(combat_inode *, void *, string_hash *);
        reinterpret_cast<category_fn>(get_vfunc(combat->m_vtbl, 0xF0))(combat, nullptr, &category);
        const int index = combat->find_incoming_move(category);
        if (index >= 0)
            reaction = incoming_at(combat, index)->field_14;
        animation->request_category_transition(category, static_cast<als::layer_types>(0), true, false, true);
        attack_id = -1;
        als::param_list desired;
        desired.add_param(0x37, direction);
        if (index >= 0)
            add_wall_position(
                desired,
                owner,
                static_cast<actor *>(
                    entity_base_vhandle{static_cast<unsigned>(incoming_at(combat, index)->field_4)}.get_volatile_ptr()),
                reaction);
        update_source_parameters();
        animation->set_desired_params(desired, static_cast<als::layer_types>(0));
        desired.clear();
        category_entered = false;
        web_reaction = index >= 0 && reaction.field_28 == 1;
        if (reaction.field_28 == 2)
            show_reticle(false);
    } else {
        category_entered = true;
        category = string_hash{0};
    }
    if (auto *soldier =
            static_cast<universal_soldier_inode *>(core->get_info_node(universal_soldier_inode::default_id, false)))
        soldier->say_gab(
            string_hash{int(
                to_hash(category == string_hash{int(to_hash("Fd_Web_Splat_By_Spider"))} ? "THUG_WBEY" : "THUG_TDMG"))},
            0,
            0);
    attacker = entity_base_vhandle{0};
    if (!category_entered) {
        if (combat->field_88 != -1) {
            if ((combat->field_B0 & 0xFF) != 0)
                attacker = entity_base_vhandle{static_cast<unsigned>(combat->field_A0)};
        } else {
            const int index = combat->find_incoming_move(category);
            if (index >= 0) {
                const auto *incoming = incoming_at(combat, index);
                attacker = entity_base_vhandle{static_cast<unsigned>(incoming->field_4)};
                owner->add_collision_ignorance(attacker);
                if (incoming->field_14.field_24 >= 0 && incoming->field_14.field_24 <= 2 &&
                    static_cast<actor *>(attacker.get_volatile_ptr())->is_hero())
                    var<int>(0x0095C8F4) = 5;
            }
        }
    }
    combat->field_82 = true;
    combat->clear_forced_react_needed();
    update_webbing(owner);
    if (auto *source = static_cast<actor *>(attacker.get_volatile_ptr())) {
        auto *other_core = source->get_ai_core();
        const bool is_feeding = feeding(actor_combat(source));
        if (auto *test = track_test(core)) {
            if (is_feeding && !test->field_1C) {
                test->begin(other_core->field_50.get_optional_pb_float(
                    string_hash{int(to_hash("feed_test_difficulty"))}, 1.0f, nullptr));
                test->start_advanced();
            } else if (!is_feeding && test->field_1C) {
                test->end();
            }
        }
    }
}
void hit_react_state::_deactivate(const mashed_state *next)
{
    auto *core = get_core();
    static_cast<std_default_trans_inode *>(core->get_info_node(std_default_trans_inode::default_id, true))->field_30 =
        nullptr;
    auto *node = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    auto *owner = get_actor();
    if (owner->field_88 && owner->field_88->field_18 == 1) {
        owner->field_88->destroy_web_effects();
        if (!transition_pending && owner->has_damage_ifc())
            owner->damage_ifc()->field_21C.sub_48BFB0(0.0f);
    }
    if (owner->has_physical_ifc()) {
        auto *physics = owner->physical_ifc();
        if (auto *constraint = physics->get_pendulum(4)) {
            combat_pendulum_manager::release_pendulum(constraint);
            physics->set_pendulum(4, nullptr);
        }
    }
    node->field_82 = false;
    owner->remove_collision_ignorance(attacker);
    if (auto *test = track_test(core)) {
        if ((!transition_pending || next_state != default_id) && test->field_1C)
            test->end();
    }
    show_reticle(true);
    base_state::_deactivate(next);
}
state_trans_action hit_react_state::_check_transition(Float delta)
{
    if (transition_pending)
        return {GOTO_STATE, next_state, TRANS_TOTAL_MSGS, nullptr};
    using callback = state_trans_action *(__fastcall *)(hit_react_state *, void *, state_trans_action *, Float);
    state_trans_action out;
    reinterpret_cast<callback>(static_cast<void **>(std_interrupt_state::native_vtable())[11])(
        this, nullptr, &out, delta);
    return out;
}
bool hit_react_state::is_done_hit_reacting()
{
    const auto current = animation->get_category_id(static_cast<als::layer_types>(0));
    if (!category_entered) {
        if (current == category) {
            category_entered = true;
            return false;
        }
        return field_1C > 0.25f;
    }
    if (current != category)
        get_actor()->remove_collision_ignorance(attacker);
    if (current == category || current == string_hash{int(to_hash("Combat_Recover"))} ||
        current == string_hash{int(to_hash("Combat_Knock_Down"))} ||
        current == string_hash{int(to_hash("Combat_Web_Recover"))}) {
        auto *node = static_cast<combat_inode *>(get_core()->get_info_node(combat_inode::default_id, true));
        auto *owner = get_actor();
        const bool conscious =
            !owner->has_damage_ifc() || (!owner->damage_ifc()->is_subdued() && owner->damage_ifc()->is_alive());
        const float delta = g_world_ptr->time_manager.field_18;
        if (node->needs_hit_avoid(delta) && conscious) {
            transition_pending = true;
            next_state = string_hash{int(to_hash("hit_avoid"))};
            return true;
        }
        if (!node->needs_hit_react(delta) || !conscious)
            return false;
        const int index = node->get_react_index();
        if (index >= 0) {
            auto *source = static_cast<actor *>(
                entity_base_vhandle{static_cast<unsigned>(incoming_at(combat, index)->field_4)}.get_volatile_ptr());
            if (current_attack_id(actor_combat(source)) == attack_id)
                return false;
        }
        transition_pending = true;
        next_state = default_id;
    }
    return true;
}
bool hit_react_state::is_event_raised(Float delta)
{
    const float signal =
        animation->get_als_layer(static_cast<als::layer_types>(0))->get_time_to_signal(event::CMBT_CHAIN);
    if (signal < 0.0f) {
        if (auto *source = static_cast<actor *>(attacker.get_volatile_ptr())) {
            if (current_attack_id(actor_combat(source)) != attack_id)
                get_actor()->remove_collision_ignorance(attacker);
        }
    }
    return signal > -0.0001f && signal < delta;
}
bool hit_react_state::resist_feeding()
{
    auto *source = static_cast<actor *>(attacker.get_volatile_ptr());
    if (!source)
        return false;
    auto *other = actor_combat(source);
    if (!feeding(other))
        return false;
    auto *test = track_test(get_core());
    if (!test || test->field_20 <= 0.9f)
        return false;
    if (test->field_1C) {
        using callback = void(__fastcall *)(track_field_inode *, void *);
        reinterpret_cast<callback>(get_vfunc(test->m_vtbl, 0x3C))(test, nullptr);
    }
    using clear_fn = void(__fastcall *)(combat_inode *, void *);
    reinterpret_cast<clear_fn>(get_vfunc(other->m_vtbl, 0x10C))(other, nullptr);
    other->clear_next_move();
    return true;
}
bool hit_react_state::pending_attack(combo_system_move::results &out)
{
    auto *source = static_cast<actor *>(attacker.get_volatile_ptr());
    if (!source)
        return false;
    auto *other = actor_combat(source);
    if (other->has_next_move()) {
        out = other->get_next_move()->field_4;
        return true;
    }
    if (other->has_cur_move() &&
        entity_base_vhandle{static_cast<unsigned>(other->field_20)}.get_volatile_ptr() == get_actor()) {
        const auto &move = other->get_cur_move()->field_4;
        if (move.field_8 != animation->get_category_id(static_cast<als::layer_types>(0))) {
            out = move;
            return true;
        }
    }
    return false;
}
bool hit_react_state::forced_response(combo_system_move::results &out, vector3d &out_direction)
{
    if (!category_entered || combat->field_88 == -1 || !combat->field_85 ||
        (attacker.get_volatile_ptr() && attacker.field_0 != static_cast<unsigned>(combat->field_A0)))
        return false;
    out.field_4 = string_hash{combat->field_98};
    using callback = string_hash *(__fastcall *)(combat_inode *, void *, string_hash *, string_hash, string_hash, int);
    reinterpret_cast<callback>(get_vfunc(combat->m_vtbl, 0xF4))(
        combat, nullptr, &out.field_8, string_hash{combat->field_90}, out.field_4, combat->field_9C);
    reinterpret_cast<callback>(get_vfunc(combat->m_vtbl, 0xFC))(
        combat, nullptr, &out.field_C, string_hash{combat->field_94}, out.field_4, combat->field_9C);
    out.field_24 = combat->field_9C;
    out.field_2C = 0;
    out_direction = vector3d{
        bit_cast<float>(combat->field_A4), bit_cast<float>(combat->field_A8), bit_cast<float>(combat->field_AC)};
    return true;
}
state_trans_messages hit_react_state::_frame_advance(Float delta)
{
    if (attack_id == -1) {
        auto *source = static_cast<actor *>(attacker.get_volatile_ptr());
        attack_id = source && source->get_ai_core() ? current_attack_id(actor_combat(source))
                                                    : static_cast<int>(std::rand() * 3.0518509447574615e-5f);
    }
    const auto message = enhanced_state::frame_advance(delta);
    auto *owner = get_actor();
    auto *physics = owner->has_physical_ifc() ? owner->physical_ifc() : nullptr;
    als::param_list desired;
    if (owner->has_damage_ifc()) {
        auto *damage = owner->damage_ifc();
        if (web_reaction && damage->field_21C.field_0[0] > 0.0f)
            was_webbed = true;
        float reaction_mode;
        if (!damage->is_alive())
            reaction_mode = 2.0f;
        else if ((web_reaction && damage->field_21C.field_0[0] <= 0.0f) || (!web_reaction && field_1C >= 4.5f))
            reaction_mode = 0.0f;
        else
            reaction_mode = physics && physics->get_pendulum(4) ? 4.0f : 3.0f;
        desired.add_param({0x36, reaction_mode});
        animation->set_desired_params(desired, static_cast<als::layer_types>(0));
    }
    combo_system_move::results move;
    vector3d response_direction = ZEROVEC;
    using signal_fn = bool(__fastcall *)(hit_react_state *, void *, Float);
    bool change = false;
    if (reinterpret_cast<signal_fn>(get_vfunc(m_vtbl, 0x44))(this, nullptr, delta) && !resist_feeding() &&
        pending_attack(move)) {
        change = !combat->consider_forced_responses(
            move.field_8, move.field_4, move.field_C, move.field_24, {attacker}, ZEROVEC, true, false);
    }
    if (!change)
        change = forced_response(move, response_direction);
    if (category_entered && physics) {
        if (auto *constraint = physics->get_pendulum(4)) {
            constraint->m_active = true;
            if (constraint->m_constraint > 6.0f)
                constraint->m_constraint = (1.0f - 2.0f * delta) * constraint->m_constraint + delta * 8.0f;
        }
    }
    if (change) {
        if (combat->check_avoid_category(move.field_8)) {
            transition_pending = true;
            next_state = string_hash{int(to_hash("hit_avoid"))};
            combat->try_set_forced_avoid_needed(
                move.field_8, move.field_4, move.field_24, {attacker}, response_direction, true, false);
        } else if (animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
            update_source_parameters();
            category = move.field_8;
            animation->request_category_transition(category, static_cast<als::layer_types>(0), true, false, true);
            attack_id = -1;
            desired.add_param(0x37, direction);
            if (auto *source = static_cast<actor *>(attacker.get_volatile_ptr()))
                add_wall_position(desired, owner, source, move);
            animation->set_desired_params(desired, static_cast<als::layer_types>(0));
            category_entered = false;
            field_1C = 0.0f;
            if (move.field_28 == 2)
                show_reticle(false);
        }
        desired.clear();
        return message;
    }
    using done_fn = bool(__fastcall *)(hit_react_state *, void *);
    auto result = message;
    if (reinterpret_cast<done_fn>(get_vfunc(m_vtbl, 0x40))(this, nullptr)) {
        if (owner->has_damage_ifc()) {
            auto *damage = owner->damage_ifc();
            if (damage->is_subdued()) {
                transition_pending = true;
                next_state = subdued_state::default_id;
            } else if (!damage->is_alive()) {
                transition_pending = true;
                next_state = string_hash{int(to_hash("unconscious"))};
            }
        }
        if (!transition_pending)
            result = static_cast<state_trans_messages>(1);
    }
    update_webbing(owner);
    desired.clear();
    return result;
}
}  // namespace ai
