#include "venom_states.h"

#include "actor.h"
#include "ai_std_combat_target.h"
#include "ai_std_jump_inode.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_ai_state_machine.h"
#include "combat_inode.h"
#include "common.h"
#include "damage_interface.h"
#include "info_node_desc_list.h"
#include "local_collision.h"
#include "physical_interface.h"
#include "retaliation_inode.h"
#include "state_machine.h"
#include "utility.h"
#include "variable.h"
#include "venom_inode.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ai {

VALIDATE_SIZE(retaliation_state, 0x30);
VALIDATE_SIZE(venom_combat_idle_state, 0x30);
VALIDATE_SIZE(venom_feed_check_state, 0x30);
VALIDATE_SIZE(venom_phase_check_state, 0x30);
VALIDATE_SIZE(venom_jump_attack_state, 0x60);
VALIDATE_SIZE(venom_jump_chase_state, 0x60);
VALIDATE_OFFSET(venom_jump_attack_state, target, 0x58);
VALIDATE_OFFSET(venom_jump_attack_state, saved_gravity_multiplier, 0x5C);

namespace {
const string_hash venom_id{int(to_hash("venom"))};
const string_hash retaliation_id{int(to_hash("RETALIATION"))};
const string_hash jump_id{int(to_hash("ai_std_jump_inode"))};
constexpr auto primary_layer = static_cast<als::layer_types>(0);

venom_inode *venom_node(const base_state *state)
{
    return static_cast<venom_inode *>(state->get_core()->get_info_node(venom_id, true));
}
venom_combat_inode *venom_combat(const base_state *state)
{
    return static_cast<venom_combat_inode *>(state->get_core()->get_info_node(combat_inode::default_id, true));
}
combat_target_inode *target_node(const base_state *state)
{
    return static_cast<combat_target_inode *>(state->get_core()->get_info_node(combat_target_inode::default_id, true));
}
entity_base_vhandle target_handle(combat_target_inode *node)
{
    entity_base_vhandle handle;
    reinterpret_cast<entity_base_vhandle *(__fastcall *)(combat_target_inode *, void *, entity_base_vhandle *)>(
        get_vfunc(node->m_vtbl, 0x38))(node, nullptr, &handle);
    return handle;
}
bool target_valid(combat_target_inode *node)
{
    return reinterpret_cast<bool(__fastcall *)(combat_target_inode *, void *)>(get_vfunc(node->m_vtbl, 0x6C))(node,
                                                                                                              nullptr);
}

template <class T, unsigned Type>
unsigned __fastcall state_type(T *, void *)
{
    return Type;
}
template <class T>
int __fastcall state_size(T *, void *)
{
    return sizeof(T);
}
bool __fastcall enhanced_subclass(const enhanced_state *, void *, unsigned type)
{
    return type == 535 || type == 567 || type == 573;
}
bool __fastcall jump_subclass(const ai_std_jump_state *, void *, unsigned type)
{
    return type == 387 || type == 535 || type == 567 || type == 573;
}
template <class T>
void __fastcall state_activate(T *self, void *, ai_state_machine *machine, const mashed_state *state,
                               const mashed_state *previous, const param_block *params,
                               base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
template <class T>
void __fastcall state_deactivate(T *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
template <class T>
void __fastcall state_info(T *self, void *, info_node_desc_list &list)
{
    self->_get_info_node_list(list);
}
template <class T>
state_trans_action *__fastcall state_transition(T *self, void *, state_trans_action *out, Float dt)
{
    *out = self->_check_transition(dt);
    return out;
}
state_trans_messages __fastcall idle_frame(venom_combat_idle_state *self, void *, Float dt)
{
    return self->_frame_advance(dt);
}
template <class T>
void __fastcall jump_setup(T *self, void *, ai_state_machine *machine)
{
    self->setup_jump(machine);
}
void __fastcall chase_prediction(venom_jump_chase_state *self, void *)
{
    self->update_target_prediction();
}

template <class T, unsigned Type, unsigned Slots = 16>
std::array<void *, Slots> state_table(void *base)
{
    std::array<void *, Slots> result{};
    std::copy_n(static_cast<void **>(base), Slots, result.data());
    result[3] = reinterpret_cast<void *>(&state_type<T, Type>);
    if constexpr (Slots == 16)
        result[4] = reinterpret_cast<void *>(&enhanced_subclass);
    result[9] = reinterpret_cast<void *>(&state_info<T>);
    result[11] = reinterpret_cast<void *>(&state_transition<T>);
    result[13] = reinterpret_cast<void *>(&state_size<T>);
    return result;
}
float interpolate(float minimum, float minimum_value, float maximum, float maximum_value, float value)
{
    double fraction = (double(value) - minimum) / (double(maximum) - minimum);
    fraction = std::clamp(fraction, 0.0, 1.0);
    return static_cast<float>(fraction * (maximum_value - minimum_value) + minimum_value);
}
}

void *retaliation_state::native_vtable()
{
    static auto table = [] {
        auto result = state_table<retaliation_state, 48>(enhanced_state::native_vtable());
        result[6] = reinterpret_cast<void *>(&state_activate<retaliation_state>);
        return result;
    }();
    return table.data();
}
void *venom_combat_idle_state::native_vtable()
{
    static auto table = [] {
        auto result = state_table<venom_combat_idle_state, 434>(enhanced_state::native_vtable());
        result[6] = reinterpret_cast<void *>(&state_activate<venom_combat_idle_state>);
        result[7] = reinterpret_cast<void *>(&state_deactivate<venom_combat_idle_state>);
        result[8] = reinterpret_cast<void *>(&idle_frame);
        return result;
    }();
    return table.data();
}
void *venom_feed_check_state::native_vtable()
{
    static auto table = state_table<venom_feed_check_state, 438>(enhanced_state::native_vtable());
    return table.data();
}
void *venom_phase_check_state::native_vtable()
{
    static auto table = state_table<venom_phase_check_state, 444>(enhanced_state::native_vtable());
    return table.data();
}
void *venom_jump_attack_state::native_vtable()
{
    static auto table = [] {
        auto result = state_table<venom_jump_attack_state, 441, 20>(ai_std_jump_state::native_vtable());
        result[4] = reinterpret_cast<void *>(&jump_subclass);
        result[6] = reinterpret_cast<void *>(&state_activate<venom_jump_attack_state>);
        result[7] = reinterpret_cast<void *>(&state_deactivate<venom_jump_attack_state>);
        result[16] = reinterpret_cast<void *>(&jump_setup<venom_jump_attack_state>);
        return result;
    }();
    return table.data();
}
void *venom_jump_chase_state::native_vtable()
{
    static auto table = [] {
        auto result = state_table<venom_jump_chase_state, 454, 20>(venom_jump_attack_state::native_vtable());
        result[16] = reinterpret_cast<void *>(&jump_setup<venom_jump_chase_state>);
        result[19] = reinterpret_cast<void *>(&chase_prediction);
        return result;
    }();
    return table.data();
}


retaliation_state::retaliation_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

venom_combat_idle_state::venom_combat_idle_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

venom_feed_check_state::venom_feed_check_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

venom_phase_check_state::venom_phase_check_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

venom_jump_attack_state::venom_jump_attack_state(from_mash_in_place_constructor *tag) : ai_std_jump_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    target.field_0 = 0;
}

venom_jump_chase_state::venom_jump_chase_state(from_mash_in_place_constructor *tag) : venom_jump_attack_state(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}


void retaliation_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                  const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    static_cast<retaliation_inode *>(get_core()->get_info_node(retaliation_id, true))->configure(my_mashed_state);
}

void retaliation_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({retaliation_id, 54});
}

state_trans_action retaliation_state::_check_transition(Float dt)
{
    return base_state::process_message(dt, static_cast<state_trans_messages>(1));
}


void venom_combat_idle_state::_activate(ai_state_machine *machine, const mashed_state *state,
                                        const mashed_state *previous, const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    auto *animation = static_cast<als_inode *>(get_core()->get_info_node(als_inode::default_id, true));
    if (animation->is_layer_interruptable(primary_layer))
        animation->request_category_transition(
            string_hash{int(to_hash("idle_walk_run"))}, primary_layer, true, false, false);
    get_core()->stop_movement();
    auto *venom = venom_node(this);
    if (flags != ACTIVATE_FLAG_FROM_INTERUPT) {
        venom->optional_pb_float =
            get_core()->field_50.get_optional_pb_float(string_hash{int(to_hash("time_to_attack"))}, 0.5f, nullptr);
        auto *retaliation = static_cast<retaliation_inode *>(get_core()->get_info_node(retaliation_id, false));
        if (retaliation)
            retaliation->record_retaliation(0);
    }
    venom->float_NULL =
        get_core()->field_50.get_optional_pb_float(string_hash{int(to_hash("engage_timer"))}, 0.0f, nullptr);
    venom->field_23 = 0;
    if (!venom->field_24)
        venom_combat(this)->allow_hit_react = true;
}

void venom_combat_idle_state::_deactivate(const mashed_state *next)
{
    base_state::_deactivate(next);
    venom_combat(this)->allow_hit_react = false;
}

state_trans_messages venom_combat_idle_state::_frame_advance(Float dt)
{
    const auto message = enhanced_state::frame_advance(dt);
    auto *venom = venom_node(this);
    auto position = venom->base_1;
    get_core()->set_facing_point(position);
    if ((venom->n11 == 1 && get_core()->field_50.get_pb_int(string_hash{int(to_hash("venom_s08"))}) == 1) ||
        position.y - get_actor()->get_abs_position().y < 6.0f) {
        position.y = get_actor()->get_abs_position().y;
        const double factor =
            std::clamp((std::sqrt(double(venom->float_NULL_2)) - 3.5) * 0.06060606241226196, 0.0, 1.0);
        get_core()->goto_position(
            position, Float{float(factor * 15.0 + 1.0)}, Float{venom->optional_pb_float_2}, Float{1.0f}, 6);
    }
    return message;
}

void venom_combat_idle_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({combat_target_inode::default_id, 351});
    list.add_entry({venom_id, 440});
    list.add_entry({combat_inode::default_id, 435});
}

state_trans_action venom_combat_idle_state::_check_transition(Float dt)
{
    auto result = enhanced_state::check_transition(dt);
    auto *venom = venom_node(this);
    auto attack_in_range = [&](combat_inode *combat) {
        const float minimum = combat->get_attack_min_distance();
        const float maximum = combat->get_attack_max_distance();
        const double height = double(venom->base_1.y) - get_actor()->get_abs_position().y;
        return venom->a3b >= double(minimum) * minimum && venom->a3b <= double(maximum) * maximum && height >= -3.0 &&
               height <= 3.0;
    };
    auto attack_transition = [&] {
        get_core()->stop_movement();
        get_core()->set_facing_point(venom->base_1);
        return base_state::process_message(dt, static_cast<state_trans_messages>(1));
    };
    if (venom->field_24) {
        venom->field_24 = false;
        if (target_valid(target_node(this))) {
            auto *combat = venom_combat(this);
            combat->set_attack(string_hash{int(to_hash("Right_Claw"))});
            if (attack_in_range(combat))
                return attack_transition();
        }
    }
    if (venom->should_throw_prop())
        return base_state::process_message(dt, static_cast<state_trans_messages>(27));
    if ((venom->n11 != 1 || get_core()->field_50.get_pb_int(string_hash{int(to_hash("venom_s08"))}) != 1) &&
        venom->float_NULL <= 0.0f) {
        venom->field_23 = 1;
        return base_state::process_message(dt, static_cast<state_trans_messages>(9));
    }
    if (venom->should_chase())
        return base_state::process_message(dt, static_cast<state_trans_messages>(8));
    if (venom->should_jump_attack()) {
        venom->reset_attack_timer();
        return base_state::process_message(dt, static_cast<state_trans_messages>(6));
    }
    if (venom->optional_pb_float <= 0.0f) {
        auto *target = target_node(this);
        if (target_valid(target)) {
            auto *combat = venom_combat(this);
            if (combat->choose_attack(static_cast<actor *>(target_handle(target).get_volatile_ptr())) &&
                attack_in_range(combat))
                return attack_transition();
        }
    }
    return result;
}


void venom_feed_check_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({venom_id, 440});
}

state_trans_action venom_feed_check_state::_check_transition(Float dt)
{
    enhanced_state::check_transition(dt);
    auto *venom = venom_node(this);
    venom->record_target_damage();
    return base_state::process_message(dt, static_cast<state_trans_messages>(venom->should_feed() ? 1 : 2));
}

void venom_phase_check_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({venom_id, 440});
}

state_trans_action venom_phase_check_state::_check_transition(Float dt)
{
    enhanced_state::check_transition(dt);
    auto *venom = venom_node(this);
    const bool first = venom->n11 == 1 && get_core()->field_50.get_pb_int(string_hash{int(to_hash("phase"))}) == 2;
    return base_state::process_message(dt, static_cast<state_trans_messages>(first ? 1 : 2));
}


void venom_jump_attack_state::_activate(ai_state_machine *machine, const mashed_state *state,
                                        const mashed_state *previous, const param_block *params, activate_flag_e flags)
{
    auto *node =
        static_cast<combat_target_inode *>(machine->get_core()->get_info_node(combat_target_inode::default_id, true));
    target = target_handle(node);
    reinterpret_cast<void(__fastcall *)(venom_jump_attack_state *, void *, ai_state_machine *)>(
        get_vfunc(m_vtbl, 0x40))(this, nullptr, machine);
    ai_std_jump_state::_activate(machine, state, previous, params, flags);
    saved_gravity_multiplier = get_actor()->physical_ifc()->m_gravity_multiplier;
}

void venom_jump_attack_state::_deactivate(const mashed_state *next)
{
    ai_std_jump_state::_deactivate(next);
    get_actor()->physical_ifc()->m_gravity_multiplier = saved_gravity_multiplier;
}

void venom_jump_attack_state::_get_info_node_list(info_node_desc_list &list)
{
    ai_std_jump_state::_get_info_node_list(list);
    list.add_entry({jump_id, 386});
    list.add_entry({combat_target_inode::default_id, 351});
    list.add_entry({combat_inode::default_id, 435});
}

void venom_jump_attack_state::setup_jump(ai_state_machine *machine)
{
    auto *actor = machine->field_8;
    const auto position = actor->get_abs_position();
    auto destination = target.get_volatile_ptr()->get_abs_position();
    actor->get_primary_region();
    vector3d point;
    vector3d normal;
    if (find_intersection(destination,
                          destination - YVEC * 95.0f,
                          *local_collision::entfilter_reject_all,
                          *local_collision::obbfilter_lineseg_test,
                          &point,
                          &normal,
                          nullptr,
                          nullptr,
                          nullptr,
                          false)) {
        destination = point;
        destination.y += actor->physical_ifc()->get_floor_offset();
    } else {
        destination = ZEROVEC;
    }
    const auto difference = destination - position;
    const float distance = std::sqrt(difference.length2());
    const float time = interpolate(10.0f, 0.5f, 60.0f, 1.5f, distance);
    const float height = interpolate(5.0f, 4.0f, 60.0f, 8.0f, distance);
    const float vertical_speed = (height + height) / (time * 0.5f);
    const float gravity = (vertical_speed + vertical_speed) / time * 0.10204081237316132f;
    const vector3d velocity{difference.x / time, vertical_speed, difference.z / time};
    const float speed = std::sqrt(velocity.length2());
    auto *jump = static_cast<ai_std_jump_inode *>(machine->get_core()->get_info_node(jump_id, true));
    jump->set_jump_velocity(velocity / speed, speed);
    jump->set_jump_parameters(true, 1.0f, 10.0f, string_hash{0});
    jump->field_30 = destination;
    actor->physical_ifc()->m_gravity_multiplier = gravity;
}


state_trans_action venom_jump_chase_state::_check_transition(Float dt)
{
    auto result = ai_std_jump_state::_check_transition(dt);
    auto *venom = venom_node(this);
    if (venom->field_23 && venom->float_NULL_2 < 9.0f)
        result = base_state::process_message(dt, static_cast<state_trans_messages>(1));
    return result;
}

void venom_jump_chase_state::setup_jump(ai_state_machine *machine)
{
    auto *actor = machine->field_8;
    const auto position = actor->get_abs_position();
    const auto destination = target.get_volatile_ptr()->get_abs_position();
    const float difference = std::fabs(position.y - destination.y) * 0.3f;
    const float maximum_y = std::max(position.y, destination.y) + (difference > 8.0f ? difference : 8.0f);
    auto *jump = static_cast<ai_std_jump_inode *>(machine->get_core()->get_info_node(jump_id, true));
    jump->set_jump_target(position, destination, maximum_y);
    jump->set_jump_parameters(true, 1.0f, 10.0f, string_hash{0});
    jump->field_30 = destination;
    actor->physical_ifc()->m_gravity_multiplier = 3.0f;
}

void venom_jump_chase_state::update_target_prediction()
{
    if (jump_phase == 1)
        return;
    auto *jump = static_cast<ai_std_jump_inode *>(get_core()->get_info_node(jump_id, true));
    if (!jump->field_3C)
        return;
    auto *target = static_cast<actor *>(target_handle(target_node(this)).get_volatile_ptr());
    if (!target)
        return;
    auto destination = target->get_abs_position();
    const float distance = std::sqrt((destination - get_actor()->get_abs_position()).length2());
    const float time = interpolate(0.0f, 0.05f, 15.0f, 0.5f, distance);
    auto velocity = target->get_velocity();
    const double squared =
        double(velocity.x) * velocity.x + double(velocity.y) * velocity.y + double(velocity.z) * velocity.z;
    if (squared > 100.0)
        velocity *= static_cast<float>(10.0 / std::sqrt(squared));
    destination += velocity * time;
    als::param_list parameters;
    parameters.add_param(33, destination);
    static_cast<als_inode *>(get_core()->get_info_node(als_inode::default_id, true))
        ->set_desired_params(parameters, primary_layer);
    parameters.clear();
}

}
