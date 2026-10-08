#include "ai_common_states.h"

#include "actor.h"
#include "ai_std_combat_target.h"
#include "ai_universal_soldier_inode.h"
#include "als_inode.h"
#include "combat_inode.h"
#include "event.h"
#include "state_machine.h"
#include "base_ai_core.h"
#include "common.h"
#include "damage_interface.h"
#include "controller_inode.h"
#include "entity_handle_manager.h"
#include "info_node_desc_list.h"
#include "mashed_state.h"
#include "physical_interface.h"
#include "std_default_trans_inode.h"
#include "subdued_state.h"
#include "retaliation_inode.h"
#include "vtbl.h"

#include <algorithm>
#include <array>

namespace ai {
VALIDATE_SIZE(std_interrupt_state, 0x30);
VALIDATE_SIZE(generic_target_hero_state, 0x34);
VALIDATE_SIZE(falling_state, 0x38);
VALIDATE_SIZE(unconscious_state, 0x38);
VALIDATE_SIZE(hit_avoid_state, 0x54);
VALIDATE_SIZE(prepare_combo_state, 0x30);

namespace {
template <class T, unsigned Type>
unsigned __fastcall type_of(const T *)
{
    return Type;
}
template <class T>
int __fastcall size_of(const T *)
{
    return sizeof(T);
}
template <class T>
void *__fastcall delete_state(T *self, void *, unsigned flags)
{
    self->~T();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
bool __fastcall interrupt_subclass(const std_interrupt_state *, void *, unsigned type)
{
    return type == 331 || type == 535 || type == 567 || type == 573;
}
state_trans_action *__fastcall native_interrupt_message(const std_interrupt_state *self, void *,
                                                        state_trans_action *out, Float delta,
                                                        state_trans_messages message)
{
    *out = self->interrupt_message(delta, message);
    return out;
}
state_trans_action *__fastcall native_interrupt_default(const std_interrupt_state *self, void *,
                                                        state_trans_action *out)
{
    *out = self->interrupt_default();
    return out;
}
state_trans_action *__fastcall interrupt_transition(const std_interrupt_state *self, void *, state_trans_action *out,
                                                    Float)
{
    using callback = state_trans_action *(__fastcall *)(const std_interrupt_state *, void *, state_trans_action *);
    return reinterpret_cast<callback>(get_vfunc(self->m_vtbl, 0x38))(self, nullptr, out);
}
template <class T>
void __fastcall activate_state(T *self, void *, ai_state_machine *machine, const mashed_state *state,
                               const mashed_state *previous, const param_block *params,
                               base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
template <class T>
state_trans_messages __fastcall frame_state(T *self, void *, Float delta)
{
    return self->_frame_advance(delta);
}
template <class T>
state_trans_action *__fastcall transition_state(T *self, void *, state_trans_action *out, Float delta)
{
    *out = self->_check_transition(delta);
    return out;
}
void __fastcall target_info(generic_target_hero_state *, void *, info_node_desc_list &list)
{
    list.add_entry({combat_target_inode::default_id, 351});
    list.add_entry({controller_inode::default_id, 357});
}
void __fastcall interrupt_info(std_interrupt_state *, void *, info_node_desc_list &list)
{
    list.add_entry({std_default_trans_inode::default_id, 371});
    list.add_entry({als_inode::default_id, 333});
}
void __fastcall prepare_combo_info(prepare_combo_state *, void *, info_node_desc_list &list)
{
    list.add_entry({combat_inode::default_id, 342});
    list.add_entry({combat_target_inode::default_id, 351});
}
string_hash *__fastcall prepare_combo_hash(const prepare_combo_state *self, void *, string_hash *out)
{
    *out = self->get_combo_to_use();
    return out;
}
bool has_arresting_actor(ai_core *core)
{
    auto *node = core->get_info_node(string_hash{int(to_hash("AI_ARRESTED_INODE"))}, false);
    return node &&
           reinterpret_cast<vhandle_type<actor> *>(reinterpret_cast<unsigned char *>(node) + 0x1C)->get_volatile_ptr();
}
}

void *std_interrupt_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&delete_state<std_interrupt_state>);
        result[3] = bit_cast<void *>(&type_of<std_interrupt_state, 331>);
        result[11] = bit_cast<void *>(&interrupt_transition);
        result[12] = bit_cast<void *>(&native_interrupt_message);
        result[14] = bit_cast<void *>(&native_interrupt_default);
        return result;
    }();
    return table.data();
}
std_interrupt_state::std_interrupt_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
std_interrupt_state::std_interrupt_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
state_trans_action std_interrupt_state::interrupt_message(Float delta, state_trans_messages message) const
{
    if (my_mashed_state && my_mashed_state->is_flag_set(mashed_state::IS_INTERRUPT_STATE) &&
        (message == 1 || message == 2))
        return {RETURN, string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
    return enhanced_state::process_message(delta, message);
}
state_trans_action std_interrupt_state::interrupt_default() const
{
    if (my_mashed_state && my_mashed_state->is_flag_set(mashed_state::IS_INTERRUPT_STATE))
        return {static_cast<state_trans_actions>(4), string_hash{0}, TRANS_TOTAL_MSGS, nullptr};
    return enhanced_state::get_default_return_code();
}

void *generic_target_hero_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&delete_state<generic_target_hero_state>);
        result[3] = bit_cast<void *>(&type_of<generic_target_hero_state, 352>);
        result[6] = bit_cast<void *>(&activate_state<generic_target_hero_state>);
        result[9] = bit_cast<void *>(&target_info);
        result[11] = bit_cast<void *>(&transition_state<generic_target_hero_state>);
        result[13] = bit_cast<void *>(&size_of<generic_target_hero_state>);
        return result;
    }();
    return table.data();
}
generic_target_hero_state::generic_target_hero_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
generic_target_hero_state::generic_target_hero_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
void generic_target_hero_state::_activate(ai_state_machine *machine, const mashed_state *state,
                                          const mashed_state *previous, const param_block *params,
                                          activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    failed = true;
    auto *hero =
        entity_handle_manager::find_entity(string_hash{int(to_hash("hero"))}, static_cast<entity_flavor_t>(29), true);
    if (hero) {
        auto *target =
            static_cast<combat_target_inode *>(get_core()->get_info_node(combat_target_inode::default_id, true));
        using callback = void(__fastcall *)(combat_target_inode *, void *, vhandle_type<actor>);
        reinterpret_cast<callback>(get_vfunc(target->m_vtbl, 0x60))(target, nullptr, {hero->get_my_handle().field_0});
        failed = false;
    }
}
state_trans_action generic_target_hero_state::_check_transition(Float delta)
{
    return base_state::process_message(delta, static_cast<state_trans_messages>(failed ? 2 : 1));
}

void *prepare_combo_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 17> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), 16, result.data());
        result[2] = bit_cast<void *>(&delete_state<prepare_combo_state>);
        result[3] = bit_cast<void *>(&type_of<prepare_combo_state, 277>);
        result[6] = bit_cast<void *>(&activate_state<prepare_combo_state>);
        result[8] = bit_cast<void *>(&frame_state<prepare_combo_state>);
        result[9] = bit_cast<void *>(&prepare_combo_info);
        result[13] = bit_cast<void *>(&size_of<prepare_combo_state>);
        result[16] = bit_cast<void *>(&prepare_combo_hash);
        return result;
    }();
    return table.data();
}
prepare_combo_state::prepare_combo_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
prepare_combo_state::prepare_combo_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
void prepare_combo_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                    const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *target = static_cast<combat_target_inode *>(get_core()->get_info_node(combat_target_inode::default_id, true));
    if (target->is_target_known()) {
        using callback = vhandle_type<actor> *(__fastcall *)(combat_target_inode *, void *, vhandle_type<actor> *);
        vhandle_type<actor> handle;
        reinterpret_cast<callback>(get_vfunc(target->m_vtbl, 0x38))(target, nullptr, &handle);
        get_core()->set_facing_point(handle.get_volatile_ptr()->get_abs_position());
    }
    const int retaliation =
        my_mashed_state->field_0.get_optional_pb_int(string_hash{int(to_hash("retaliation"))}, 0, nullptr);
    if (retaliation > 0)
        static_cast<retaliation_inode *>(get_core()->get_info_node(string_hash{int(to_hash("RETALIATION"))}, true))
            ->record_retaliation(retaliation - 1);
}
string_hash prepare_combo_state::get_combo_to_use() const
{
    return my_mashed_state->field_0.get_pb_hash(string_hash{int(to_hash("combo_to_use"))});
}
state_trans_messages prepare_combo_state::_frame_advance(Float delta)
{
    enhanced_state::frame_advance(delta);
    auto *combat = static_cast<combat_inode *>(get_core()->get_info_node(combat_inode::default_id, true));
    if (combat->has_next_move())
        return static_cast<state_trans_messages>(1);
    using callback = string_hash *(__fastcall *)(prepare_combo_state *, void *, string_hash *);
    string_hash attack;
    reinterpret_cast<callback>(get_vfunc(m_vtbl, 0x40))(this, nullptr, &attack);
    return static_cast<state_trans_messages>(combat->set_attack(attack) ? 1 : 2);
}

void *falling_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(std_interrupt_state::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&delete_state<falling_state>);
        result[3] = bit_cast<void *>(&type_of<falling_state, 367>);
        result[4] = bit_cast<void *>(&interrupt_subclass);
        result[6] = bit_cast<void *>(&activate_state<falling_state>);
        result[8] = bit_cast<void *>(&frame_state<falling_state>);
        result[9] = bit_cast<void *>(&interrupt_info);
        result[13] = bit_cast<void *>(&size_of<falling_state>);
        return result;
    }();
    return table.data();
}
falling_state::falling_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
falling_state::falling_state(from_mash_in_place_constructor *tag) : std_interrupt_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
void falling_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                              const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    get_core()->stop_movement();
    static_cast<als_inode *>(get_core()->get_info_node(als_inode::default_id, true))
        ->request_category_transition(
            string_hash{int(to_hash("Falling"))}, static_cast<als::layer_types>(0), true, false, false);
}
state_trans_messages falling_state::_frame_advance(Float)
{
    return static_cast<state_trans_messages>(get_actor()->physical_ifc()->is_effectively_standing() ? 1 : 75);
}

void *unconscious_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(std_interrupt_state::native_vtable()), result.size(), result.data());
        result[2] = bit_cast<void *>(&delete_state<unconscious_state>);
        result[3] = bit_cast<void *>(&type_of<unconscious_state, 373>);
        result[4] = bit_cast<void *>(&interrupt_subclass);
        result[6] = bit_cast<void *>(&activate_state<unconscious_state>);
        result[8] = bit_cast<void *>(&frame_state<unconscious_state>);
        result[9] = bit_cast<void *>(&interrupt_info);
        result[11] = bit_cast<void *>(&transition_state<unconscious_state>);
        result[13] = bit_cast<void *>(&size_of<unconscious_state>);
        return result;
    }();
    return table.data();
}
unconscious_state::unconscious_state()
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
unconscious_state::unconscious_state(from_mash_in_place_constructor *tag) : std_interrupt_state(tag)
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
void unconscious_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                  const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    get_core()->stop_movement();
    if (auto *damage = get_actor()->damage_ifc())
        damage->field_22C.field_0[0] =
            std::clamp(damage->field_22C.field_0[0] + 1, damage->field_22C.field_0[1], damage->field_22C.field_0[2]);
    phase = 0;
    unconscious_time =
        get_core()->field_50.get_optional_pb_float(string_hash{int(to_hash("unconscious_time"))}, 8.0f, nullptr);
    auto *soldier =
        static_cast<universal_soldier_inode *>(get_core()->get_info_node(universal_soldier_inode::default_id, false));
    if (soldier)
        soldier->say_gab(string_hash{int(to_hash("THUG_SBDU"))}, 0, 0);
}
state_trans_messages unconscious_state::_frame_advance(Float delta)
{
    auto message = enhanced_state::frame_advance(delta);
    auto *animation = static_cast<als_inode *>(get_core()->get_info_node(als_inode::default_id, true));
    auto category = animation->get_category_id(static_cast<als::layer_types>(0));
    if (phase == 0 && animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        animation->request_category_transition(
            string_hash{int(to_hash("Unconscious"))}, static_cast<als::layer_types>(0), true, false, false);
        phase = 1;
    } else if (phase == 1 && category == string_hash{int(to_hash("Unconscious"))}) {
        if (has_arresting_actor(get_core())) {
            if (field_1C > std::max(unconscious_time * 6.0f, 2.0f)) {
                get_core()->field_64->field_8 &= ~0x4000u;
                get_core()->field_64->damage_ifc()->apply_subdue(nullptr, 15.0f);
            }
        } else if (unconscious_time >= 0.0f && field_1C > unconscious_time) {
            animation->request_category_transition(
                string_hash{int(to_hash("Idle_Walk_Run"))}, static_cast<als::layer_types>(0), true, false, false);
            phase = 2;
        }
    } else if (phase == 2) {
        auto *damage = get_actor()->damage_ifc();
        if (damage->field_1FC.field_0[0] <= 0.0f)
            damage->field_1FC.sub_48BFB0(damage->field_1FC.field_0[2]);
        return static_cast<state_trans_messages>(1);
    }
    return message;
}
state_trans_action unconscious_state::_check_transition(Float)
{
    auto *defaults =
        static_cast<std_default_trans_inode *>(get_core()->get_info_node(std_default_trans_inode::default_id, true));
    auto *damage = get_core()->field_64->damage_ifc();
    if (defaults->field_26 && damage && damage->is_subdued())
        return {GOTO_STATE, subdued_state::default_id, TRANS_TOTAL_MSGS, nullptr};
    return interrupt_default();
}

namespace {
void __fastcall avoid_destruct(hit_avoid_state *self, void *)
{
    self->category.destruct_mashed_class();
    using callback = void(__fastcall *)(enhanced_state *, void *);
    reinterpret_cast<callback>(static_cast<void **>(enhanced_state::native_vtable())[0])(self, nullptr);
}
void __fastcall avoid_unmash(hit_avoid_state *self, void *, mash_info_struct *info, void *owner)
{
    using callback = void(__fastcall *)(enhanced_state *, void *, mash_info_struct *, void *);
    reinterpret_cast<callback>(static_cast<void **>(enhanced_state::native_vtable())[1])(self, nullptr, info, owner);
    self->category.unmash(info, self);
}
void __fastcall avoid_deactivate(hit_avoid_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
void __fastcall avoid_info(hit_avoid_state *, void *, info_node_desc_list &list)
{
    list.add_entry({std_default_trans_inode::default_id, 371});
    list.add_entry({combat_inode::default_id, 342});
    list.add_entry({als_inode::default_id, 333});
}
combat_inode::incoming_move *incoming_at(combat_inode *node, int index)
{
    using callback = combat_inode::incoming_move *(__fastcall *)(combat_inode *, void *, int);
    return reinterpret_cast<callback>(get_vfunc(node->m_vtbl, 0xCC))(node, nullptr, index);
}
string_hash avoid_category_for(combat_inode *node, string_hash attack, string_hash reaction, unsigned kind)
{
    string_hash out{0};
    using callback =
        string_hash *(__fastcall *)(combat_inode *, void *, string_hash *, string_hash, string_hash, unsigned);
    reinterpret_cast<callback>(get_vfunc(node->m_vtbl, 0xFC))(node, nullptr, &out, attack, reaction, kind);
    return out;
}
void set_avoid_direction(als_inode *animation, const vector3d &direction)
{
    als::param_list params;
    params.add_param(0x37, direction);
    animation->set_desired_params(params, static_cast<als::layer_types>(0));
    params.clear();
}
}

void *hit_avoid_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(std_interrupt_state::native_vtable()), result.size(), result.data());
        result[0] = bit_cast<void *>(&avoid_destruct);
        result[1] = bit_cast<void *>(&avoid_unmash);
        result[2] = bit_cast<void *>(&delete_state<hit_avoid_state>);
        result[3] = bit_cast<void *>(&type_of<hit_avoid_state, 368>);
        result[4] = bit_cast<void *>(&interrupt_subclass);
        result[6] = bit_cast<void *>(&activate_state<hit_avoid_state>);
        result[7] = bit_cast<void *>(&avoid_deactivate);
        result[8] = bit_cast<void *>(&frame_state<hit_avoid_state>);
        result[9] = bit_cast<void *>(&avoid_info);
        result[13] = bit_cast<void *>(&size_of<hit_avoid_state>);
        return result;
    }();
    return table.data();
}
hit_avoid_state::hit_avoid_state() : category_entered{false}, attacker{0}
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
hit_avoid_state::hit_avoid_state(from_mash_in_place_constructor *tag)
    : std_interrupt_state(tag), category(tag), attacker{0}
{
    m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}
hit_avoid_state::~hit_avoid_state()
{
    if (get_machine()) {
        if (auto *source = static_cast<actor *>(attacker.get_volatile_ptr()))
            source->remove_collision_ignorance(get_actor()->get_my_handle());
    }
}
void hit_avoid_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *core = get_core();
    core->stop_movement();
    combat = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    animation = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
    using direction_fn = vector3d *(__fastcall *)(combat_inode *, void *, vector3d *);
    reinterpret_cast<direction_fn>(get_vfunc(combat->m_vtbl, 0x110))(combat, nullptr, &direction);
    combat->field_83 = true;
    using category_fn = string_hash *(__fastcall *)(combat_inode *, void *, string_hash *);
    reinterpret_cast<category_fn>(get_vfunc(combat->m_vtbl, 0xF8))(combat, nullptr, &category);
    const int index = combat->find_incoming_move(category);
    if (index >= 0)
        attacker = entity_base_vhandle{static_cast<unsigned>(incoming_at(combat, index)->field_4)};
    if (animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        animation->request_category_transition(category, static_cast<als::layer_types>(0), true, false, false);
        if (combat->field_8C == -1)
            set_avoid_direction(animation, direction);
        category_entered = false;
        combat->avoid_attack();
    } else {
        category_entered = true;
        category = string_hash{0};
    }
    combat->clear_forced_avoid_needed();
}
void hit_avoid_state::_deactivate(const mashed_state *)
{
    combat->field_83 = false;
}
bool hit_avoid_state::finished()
{
    const auto current = animation->get_category_id(static_cast<als::layer_types>(0));
    if (!category_entered) {
        if (current == category)
            category_entered = true;
        return false;
    }
    return current != category && animation->is_layer_interruptable(static_cast<als::layer_types>(0));
}
state_trans_messages hit_avoid_state::_frame_advance(Float delta)
{
    auto complete = [&] {
        if (!finished())
            return static_cast<state_trans_messages>(75);
        get_core()->get_info_node(combat_inode::default_id, true);
        return static_cast<state_trans_messages>(1);
    };
    if (!category_entered || !combat->needs_hit_avoid(delta)) {
        const float signal_time =
            animation->get_als_layer(static_cast<als::layer_types>(0))->get_time_to_signal(event::CMBT_CHAIN);
        if (signal_time <= -delta || signal_time >= delta)
            return complete();
    }
    entity_base_vhandle source{0};
    if (combat->field_8C != -1 && combat->field_86) {
        source = entity_base_vhandle{static_cast<unsigned>(combat->field_A0)};
    } else {
        const int index = combat->find_incoming_move(animation->get_category_id(static_cast<als::layer_types>(0)));
        if (index >= 0)
            source = entity_base_vhandle{static_cast<unsigned>(incoming_at(combat, index)->field_4)};
    }
    auto *attacking_actor = static_cast<actor *>(source.get_volatile_ptr());
    if (!attacking_actor || !attacking_actor->get_ai_core())
        return static_cast<state_trans_messages>(75);
    auto *other =
        static_cast<combat_inode *>(attacking_actor->get_ai_core()->get_info_node(combat_inode::default_id, true));
    string_hash next{0};
    if (combat->field_8C != -1 && combat->field_86) {
        next = avoid_category_for(combat,
                                  string_hash{combat->field_94},
                                  string_hash{combat->field_98},
                                  static_cast<unsigned>(combat->field_9C));
    } else {
        combo_system_move *move = nullptr;
        if (other->has_next_move())
            move = other->get_next_move();
        else if (other->has_cur_move() &&
                 entity_base_vhandle{static_cast<unsigned>(other->field_20)}.get_volatile_ptr() == get_actor())
            move = other->get_cur_move();
        if (!move)
            return complete();
        next = avoid_category_for(
            combat, move->field_4.field_C, move->field_4.field_4, static_cast<unsigned>(move->field_4.field_24));
    }
    combat->avoid_attack();
    if (animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        category = next;
        animation->request_category_transition(next, static_cast<als::layer_types>(0), true, false, true);
        set_avoid_direction(animation, direction);
        category_entered = false;
    }
    return static_cast<state_trans_messages>(75);
}

}
