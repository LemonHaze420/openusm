#include "ai_std_jump_state.h"

#include "actor.h"
#include "ai_std_combat_target.h"
#include "ai_std_jump_inode.h"
#include "als_inode.h"
#include "anim_event.h"
#include "base_ai_core.h"
#include "common.h"
#include "conglom.h"
#include "damage_inode.h"
#include "event_manager.h"
#include "info_node_desc_list.h"
#include "mashed_state.h"
#include "physical_interface.h"
#include "state_machine.h"
#include "std_default_trans_inode.h"
#include "utility.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ai {

VALIDATE_SIZE(ai_std_jump_state, 0x58);
VALIDATE_OFFSET(ai_std_jump_state, jump_phase, 0x30);
VALIDATE_OFFSET(ai_std_jump_state, jump_category, 0x4C);

namespace {
const string_hash jump_id{int(to_hash("ai_std_jump_inode"))};
constexpr auto primary_layer = static_cast<als::layer_types>(0);

ai_std_jump_inode *jump_node(const base_state *state)
{
    return static_cast<ai_std_jump_inode *>(state->get_core()->get_info_node(jump_id, true));
}
als_inode *animation_node(const base_state *state)
{
    return static_cast<als_inode *>(state->get_core()->get_info_node(als_inode::default_id, true));
}
void dispatch_setup(ai_std_jump_state *state, ai_state_machine *machine)
{
    reinterpret_cast<void(__fastcall *)(ai_std_jump_state *, void *, ai_state_machine *)>(
        get_vfunc(state->m_vtbl, 0x40))(state, nullptr, machine);
}
void dispatch_phase(ai_std_jump_state *state, int phase)
{
    reinterpret_cast<void(__fastcall *)(ai_std_jump_state *, void *, int)>(get_vfunc(state->m_vtbl, 0x48))(
        state, nullptr, phase);
}
void animation_action(event *, entity_base_vhandle, void *context)
{
    static_cast<ai_std_jump_state *>(context)->on_animation_action();
}
void attack_begin(event *event, entity_base_vhandle owner, void *context)
{
    auto *state = static_cast<ai_std_jump_state *>(context);
    state->attack_bone = static_cast<conglomerate *>(owner.get_volatile_ptr())
                             ->get_bone(static_cast<anim_event *>(event)->field_C, true);
}
void attack_end(event *, entity_base_vhandle, void *context)
{
    static_cast<ai_std_jump_state *>(context)->attack_bone = nullptr;
}
unsigned __fastcall jump_type(ai_std_jump_state *, void *)
{
    return 387;
}
int __fastcall jump_size(ai_std_jump_state *, void *)
{
    return sizeof(ai_std_jump_state);
}
bool __fastcall jump_subclass(ai_std_jump_state *, void *, unsigned type)
{
    return type == 535 || type == 567 || type == 573;
}
void __fastcall jump_activate(ai_std_jump_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                              const mashed_state *previous, const param_block *params,
                              base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
void __fastcall jump_deactivate(ai_std_jump_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
state_trans_messages __fastcall jump_frame(ai_std_jump_state *self, void *, Float dt)
{
    return self->_frame_advance(dt);
}
void __fastcall jump_info(ai_std_jump_state *self, void *, info_node_desc_list &list)
{
    self->_get_info_node_list(list);
}
state_trans_action *__fastcall jump_transition(ai_std_jump_state *self, void *, state_trans_action *out, Float dt)
{
    *out = self->_check_transition(dt);
    return out;
}
void __fastcall jump_setup(ai_std_jump_state *, void *, ai_state_machine *) {}
void __fastcall jump_action(ai_std_jump_state *, void *) {}
void __fastcall native_jump_phase(ai_std_jump_state *self, void *, int phase)
{
    self->set_jump_phase(phase);
}
}

void *ai_std_jump_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 20> result{};
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), 16, result.data());
        result[3] = reinterpret_cast<void *>(&jump_type);
        result[4] = reinterpret_cast<void *>(&jump_subclass);
        result[6] = reinterpret_cast<void *>(&jump_activate);
        result[7] = reinterpret_cast<void *>(&jump_deactivate);
        result[8] = reinterpret_cast<void *>(&jump_frame);
        result[9] = reinterpret_cast<void *>(&jump_info);
        result[11] = reinterpret_cast<void *>(&jump_transition);
        result[13] = reinterpret_cast<void *>(&jump_size);
        result[16] = reinterpret_cast<void *>(&jump_setup);
        result[17] = reinterpret_cast<void *>(&jump_action);
        result[18] = reinterpret_cast<void *>(&native_jump_phase);
        return result;
    }();
    return table.data();
}


ai_std_jump_state::ai_std_jump_state(from_mash_in_place_constructor *tag) : enhanced_state(tag), jump_category{0}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    attack_target.field_0 = 0;
}


void ai_std_jump_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({jump_id, 386});
    list.add_entry({als_inode::default_id, 333});
    list.add_entry({string_hash{int(to_hash("physics"))}, 402});
}


void ai_std_jump_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                  const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *defaults =
        static_cast<std_default_trans_inode *>(get_core()->get_info_node(std_default_trans_inode::default_id, true));
    saved_default_transition = defaults->field_24;
    defaults->field_24 = false;
    if (get_actor()->has_physical_ifc()) {
        auto *physical = get_actor()->physical_ifc();
        saved_physical_parameter = physical->field_F0;
        physical->field_F0 = 10.0f;
    }
    if (flags == ACTIVATE_FLAG_FROM_INTERUPT)
        return;
    jump_category = string_hash{int(to_hash("Jump"))};
    if (state->field_0.does_parameter_exist(string_hash{int(to_hash("jump_als_category"))}))
        jump_category = state->field_0.get_pb_hash(string_hash{int(to_hash("jump_als_category"))});
    auto *animation = animation_node(this);
    auto *jump = jump_node(this);
    jump->field_1C = 0.0f;
    allow_early_exit = false;
    attack_applied = false;
    if (state->field_0.does_parameter_exist(string_hash{int(to_hash("allow_early_exit"))}))
        allow_early_exit =
            state->field_0.get_optional_pb_int(string_hash{int(to_hash("allow_early_exit"))}, 0, nullptr) != 0;
    dispatch_setup(this, machine);
    jump->field_3D = true;
    jump_phase = 0;
    dispatch_phase(this, 1);
    als::param_list parameters;
    parameters.add_param({52, jump->field_3C ? 1.0f : 0.0f});
    parameters.add_param({61, jump->field_1C});
    anim_callback = get_actor()->add_callback(event::ANIM_ACTION, &animation_action, this, false);
    if (jump->field_3C) {
        auto *target =
            static_cast<combat_target_inode *>(get_core()->get_info_node(combat_target_inode::default_id, true));
        using target_fn = entity_base_vhandle *(__fastcall *)(combat_target_inode *, void *, entity_base_vhandle *);
        reinterpret_cast<target_fn>(get_vfunc(target->m_vtbl, 0x38))(target, nullptr, &attack_target);
        attack_bone = nullptr;
        attack_begin_callback = get_actor()->add_callback(event::ATTACK_BEGIN, &attack_begin, this, false);
        attack_end_callback = get_actor()->add_callback(event::ATTACK_END, &attack_end, this, false);
        get_actor()->add_collision_ignorance(attack_target);
    } else {
        attack_target.field_0 = 0;
        attack_bone = nullptr;
        attack_begin_callback = 0;
        attack_end_callback = 0;
    }
    animation->set_desired_params(parameters, primary_layer);
    parameters.clear();
    animation->request_category_transition(jump_category, primary_layer, true, false, false);
    get_core()->stop_movement();
}


void ai_std_jump_state::_deactivate(const mashed_state *next)
{
    base_state::_deactivate(next);
    auto *jump = jump_node(this);
    jump->field_1C = 0.0f;
    jump->field_3D = false;
    const auto owner = get_actor()->get_my_vhandle();
    event_manager::remove_callback(anim_callback, event::ANIM_ACTION, owner);
    event_manager::remove_callback(attack_begin_callback, event::ATTACK_BEGIN, owner);
    event_manager::remove_callback(attack_end_callback, event::ATTACK_END, owner);
    get_actor()->remove_collision_ignorance(attack_target);
    auto *defaults =
        static_cast<std_default_trans_inode *>(get_core()->get_info_node(std_default_trans_inode::default_id, true));
    defaults->field_24 = saved_default_transition;
    if (get_actor()->has_physical_ifc())
        get_actor()->physical_ifc()->field_F0 = saved_physical_parameter;
}


void ai_std_jump_state::set_jump_phase(int phase)
{
    if (jump_phase != phase) {
        jump_phase = phase;
        phase_time = 0.0f;
    }
}


state_trans_messages ai_std_jump_state::_frame_advance(Float dt)
{
    jump_node(this)->field_3D = jump_phase == 1 || jump_phase == 2;
    phase_time += dt;
    apply_jump_damage();
    update_jump_parameters();
    return enhanced_state::frame_advance(dt);
}


state_trans_action ai_std_jump_state::_check_transition(Float dt)
{
    if (field_4 == ACTIVATE_FLAG_FROM_INTERUPT)
        return base_state::process_message(dt, static_cast<state_trans_messages>(2));
    auto result = animation_node(this)->get_category_id(primary_layer) == jump_category
                      ? enhanced_state::check_transition(dt)
                      : base_state::process_message(dt, static_cast<state_trans_messages>(1));
    if ((result.the_action == 3 || result.the_action == 4) && jump_node(this)->field_4C < field_1C)
        result = base_state::process_message(dt, static_cast<state_trans_messages>(2));
    return result;
}


void ai_std_jump_state::on_animation_action()
{
    auto *animation = animation_node(this);
    const auto state = animation->get_state_id(primary_layer);
    if (state == string_hash{int(to_hash("jump_fly_up_state"))} ||
        state == string_hash{int(to_hash("jump_fly_down_state"))})
        return;
    if (jump_phase == 1) {
        auto *jump = jump_node(this);
        reinterpret_cast<void(__fastcall *)(ai_std_jump_state *, void *)>(get_vfunc(m_vtbl, 0x44))(this, nullptr);
        als::param_list parameters;
        parameters.add_param(83, jump->field_20);
        animation->set_desired_params(parameters, primary_layer);
        parameters.clear();
        dispatch_phase(this, 2);
    } else if (jump_phase == 2) {
        dispatch_phase(this, 3);
    }
}


void ai_std_jump_state::apply_jump_damage()
{
    if (attack_applied || !attack_target.get_volatile_ptr() || !attack_bone)
        return;
    auto *jump = jump_node(this);
    auto *target = static_cast<actor *>(attack_target.get_volatile_ptr());
    const auto difference = target->get_abs_position() - attack_bone->get_abs_position();
    const double squared =
        double(difference.x) * difference.x + double(difference.y) * difference.y + double(difference.z) * difference.z;
    if (squared < double(jump->field_44) * jump->field_44) {
        auto *damage =
            static_cast<damage_inode *>(target->get_ai_core()->get_info_node(damage_inode::default_id, true));
        const auto direction = difference / static_cast<float>(std::sqrt(squared));
        damage->apply_damage(get_actor(),
                             static_cast<int>(jump->field_40),
                             direction,
                             jump->field_48,
                             string_hash{0},
                             string_hash{0},
                             false,
                             17,
                             false);
        attack_applied = true;
        animation_node(this)->get_als_layer(primary_layer)->set_desired_param({54, 2.0f});
    }
}


void ai_std_jump_state::update_jump_parameters()
{
    auto *jump = jump_node(this);
    if (jump_phase != 3) {
        als::param_list parameters;
        parameters.add_param(33, jump->field_30);
        parameters.add_param(27, (jump->field_30 - get_actor()->get_abs_position()).normalized());
        animation_node(this)->set_desired_params(parameters, primary_layer);
        parameters.clear();
    }
}

}
