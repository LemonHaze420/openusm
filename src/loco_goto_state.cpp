#include "loco_goto_state.h"
#include "actor.h"
#include "ai_path.h"
#include "ai_quad_path_inode.h"
#include "ai_std_avoidance.h"
#include "ai_std_combat_target.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "game.h"
#include "info_node_desc_list.h"
#include "loco_inode.h"
#include "mashed_state.h"
#include "message_board.h"
#include "param_list.h"
#include "oldmath_po.h"
#include "state_machine.h"
#include "vtbl.h"
#include "wds.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace ai {
VALIDATE_SIZE(loco_goto_state, 0x38);
VALIDATE_SIZE(nonpathed_goto_state, 0x48);
VALIDATE_SIZE(pathed_goto_state, 0x4C);
namespace {
const string_hash biped_id{int(to_hash("biped_layer"))};
const string_hash quad_id{int(to_hash("QUAD_PATH_INODE"))};
const string_hash avoidance_id{int(to_hash("AVOIDANCE_INODE"))};
const string_hash loco_key{int(to_hash("loco_inode"))};
template <class T>
T *node(const base_state *state, const string_hash &id, bool required = true)
{
    return static_cast<T *>(state->get_core()->get_info_node(id, required));
}
template <class T, unsigned Type>
unsigned __fastcall type(const T *)
{
    return Type;
}
template <class T>
int __fastcall size(const T *)
{
    return sizeof(T);
}
template <class T, unsigned Type>
bool __fastcall subclass(const T *, void *, unsigned value)
{
    return value == Type || value == 390 || (Type == 340 && value == 339) || value == 535 || value == 567 ||
           value == 573;
}
template <class T>
void *__fastcall destroy(T *self, void *, unsigned flags)
{
    self->~T();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
template <class T>
void __fastcall activate(T *self, void *, ai_state_machine *machine, const mashed_state *state,
                         const mashed_state *previous, const param_block *params, base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, params, flags);
}
template <class T>
state_trans_messages __fastcall frame(T *self, void *, Float delta)
{
    return self->_frame_advance(delta);
}
template <class T>
loco_inode *__fastcall select(const T *self)
{
    return self->select_inode();
}
vector3d *__fastcall steer(loco_goto_state *self, void *, vector3d *out, const vector3d &direction, Float delta)
{
    *out = self->steer_direction(direction, delta);
    return out;
}
void __fastcall animation(loco_goto_state *self, void *, const vector3d &direction, float speed)
{
    self->update_animation(direction, speed);
}
bool __fastcall at_destination(loco_goto_state *self, void *, float distance, bool last)
{
    return self->is_at_destination(distance, last);
}
template <class T>
bool __fastcall stuck(T *self, void *, Float delta)
{
    return self->is_stuck(delta);
}
template <class T>
state_trans_messages __fastcall checks(T *self, void *, const vector3d &destination, float distance, Float delta)
{
    return self->do_checks(destination, distance, delta);
}
template <class T>
const vector3d *__fastcall destination(const T *self)
{
    return &self->get_destination();
}
template <class T>
bool __fastcall last_destination(const T *self)
{
    return self->is_last_destination();
}
template <class T>
void __fastcall repath(T *self)
{
    self->repath();
}
void __fastcall category(loco_goto_state *self)
{
    self->update_category();
}
template <unsigned Type>
void __fastcall info(loco_goto_state *, void *, info_node_desc_list &list)
{
    list.add_entry({als_inode::default_id, 333});
    if constexpr (Type != 390)
        list.add_entry({biped_id, 338});
    if constexpr (Type == 340) {
        list.add_entry({quad_id, 341});
        list.add_entry({avoidance_id, 336});
    }
}
void __fastcall deactivate(pathed_goto_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
state_trans_action *__fastcall transition(const pathed_goto_state *self, void *, state_trans_action *out, Float delta)
{
    *out = self->_check_transition(delta);
    return out;
}
state_trans_action *__fastcall message(pathed_goto_state *self, void *, state_trans_action *out, Float delta,
                                       state_trans_messages msg)
{
    *out = self->_process_message(delta, msg);
    return out;
}
template <class T, unsigned Type>
std::array<std::uintptr_t, 26> make_table()
{
    std::array<std::uintptr_t, 26> table{};
    std::copy_n(static_cast<const std::uintptr_t *>(enhanced_state::native_vtable()), 16, table.begin());
    table[2] = reinterpret_cast<std::uintptr_t>(&destroy<T>);
    table[3] = reinterpret_cast<std::uintptr_t>(&type<T, Type>);
    table[4] = reinterpret_cast<std::uintptr_t>(&subclass<T, Type>);
    table[6] = reinterpret_cast<std::uintptr_t>(&activate<T>);
    table[8] = reinterpret_cast<std::uintptr_t>(&frame<T>);
    table[9] = reinterpret_cast<std::uintptr_t>(&info<Type>);
    table[13] = reinterpret_cast<std::uintptr_t>(&size<T>);
    table[16] = reinterpret_cast<std::uintptr_t>(&select<T>);
    table[17] = reinterpret_cast<std::uintptr_t>(&steer);
    table[18] = reinterpret_cast<std::uintptr_t>(&animation);
    table[19] = reinterpret_cast<std::uintptr_t>(&at_destination);
    table[20] = reinterpret_cast<std::uintptr_t>(&stuck<T>);
    table[21] = reinterpret_cast<std::uintptr_t>(&checks<T>);
    table[22] = reinterpret_cast<std::uintptr_t>(&destination<T>);
    table[23] = reinterpret_cast<std::uintptr_t>(&last_destination<T>);
    table[24] = reinterpret_cast<std::uintptr_t>(&repath<T>);
    table[25] = reinterpret_cast<std::uintptr_t>(&category);
    if constexpr (Type == 340) {
        table[7] = reinterpret_cast<std::uintptr_t>(&deactivate);
        table[11] = reinterpret_cast<std::uintptr_t>(&transition);
        table[12] = reinterpret_cast<std::uintptr_t>(&message);
    }
    return table;
}
template <class Result, class... Args>
Result dispatch(loco_goto_state *state, int slot, Args... args)
{
    using callback = Result(__fastcall *)(loco_goto_state *, void *, Args...);
    return reinterpret_cast<callback>(get_vfunc(state->m_vtbl, slot * 4))(state, nullptr, args...);
}
state_trans_messages result(unsigned value)
{
    return static_cast<state_trans_messages>(value);
}
}
void *loco_goto_state::native_vtable()
{
    static auto table = make_table<loco_goto_state, 390>();
    return table.data();
}
void *nonpathed_goto_state::native_vtable()
{
    static auto table = make_table<nonpathed_goto_state, 339>();
    return table.data();
}
void *pathed_goto_state::native_vtable()
{
    static auto table = make_table<pathed_goto_state, 340>();
    return table.data();
}
loco_goto_state::loco_goto_state() : locomotion(nullptr), avoidance(nullptr)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
nonpathed_goto_state::nonpathed_goto_state() : stuck_time(0.0f), last_position{}
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
pathed_goto_state::pathed_goto_state() : path(nullptr)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
loco_goto_state::loco_goto_state(from_mash_in_place_constructor *tag) : enhanced_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
nonpathed_goto_state::nonpathed_goto_state(from_mash_in_place_constructor *tag) : loco_goto_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
pathed_goto_state::pathed_goto_state(from_mash_in_place_constructor *tag) : nonpathed_goto_state(tag)
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}
loco_inode *loco_goto_state::select_inode() const
{
    return node<loco_inode>(this, my_mashed_state->field_0.get_pb_hash(loco_key));
}
loco_inode *nonpathed_goto_state::select_inode() const
{
    const auto &params = my_mashed_state->field_0;
    return node<loco_inode>(this, params.does_parameter_exist(loco_key) ? params.get_pb_hash(loco_key) : biped_id);
}
void loco_goto_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    locomotion = dispatch<loco_inode *>(this, 16);
    avoidance = node<avoidance_inode>(this, avoidance_id, false);
    locomotion->needs_repathfind = false;
    if (!locomotion->field_54)
        node<als_inode>(this, als_inode::default_id)
            ->request_category_transition(
                locomotion->als_category, static_cast<als::layer_types>(0), true, false, false);
}
void nonpathed_goto_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                     const param_block *params, activate_flag_e flags)
{
    loco_goto_state::_activate(machine, state, previous, params, flags);
    last_position = vector3d{0.0f, 0.0f, 0.0f};
    stuck_time = 0.0f;
    locomotion->needs_repathfind = false;
}
void pathed_goto_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                                  const param_block *params, activate_flag_e flags)
{
    nonpathed_goto_state::_activate(machine, state, previous, params, flags);
    locomotion->needs_repathfind = true;
}
void loco_goto_state::repath()
{
    locomotion->needs_repathfind = false;
}
void pathed_goto_state::repath()
{
    path = nullptr;
    auto *quad = node<quad_path_inode>(this, quad_id);
    quad->path->setup(quad->field_C->get_my_handle(),
                      get_actor()->get_abs_position(),
                      *reinterpret_cast<const vector3d *>(locomotion->goto_destination),
                      false,
                      -1.0f);
    path = quad->path;
    locomotion->needs_repathfind = false;
}
const vector3d &loco_goto_state::get_destination() const
{
    return *reinterpret_cast<const vector3d *>(locomotion->goto_destination);
}
const vector3d &pathed_goto_state::get_destination() const
{
    return path->field_64;
}
bool pathed_goto_state::is_last_destination() const
{
    return !path->has_more_points();
}
bool nonpathed_goto_state::is_stuck(Float delta)
{
    const auto &position = get_actor()->get_abs_position();
    if ((last_position - position).length2() > delta * delta) {
        last_position = position;
        stuck_time = 0.0f;
    } else {
        stuck_time += delta;
    }
    return stuck_time > 0.5f;
}
bool loco_goto_state::is_at_destination(float distance, bool last)
{
    float radius = 0.5f;
    if (last || locomotion->field_5F) {
        if (!locomotion->explicit_goto_radius)
            locomotion->set_goto_radius(-1.0f);
        radius = locomotion->goto_radius;
    }
    return radius * radius > distance;
}
state_trans_messages loco_goto_state::do_checks(const vector3d &, float distance, Float delta)
{
    if (dispatch<bool, float, bool>(this, 19, distance, true)) {
        locomotion->field_40 = 0;
        return result(1);
    }
    if (dispatch<bool, Float>(this, 20, delta)) {
        ++locomotion->field_40;
        return result(2);
    }
    return result(75);
}
state_trans_messages pathed_goto_state::do_checks(const vector3d &, float distance, Float delta)
{
    if (is_at_destination(distance, !path->has_more_points())) {
        locomotion->field_40 = 0;
        if (!path->has_more_points())
            return result(1);
        path->get_next_point();
        return result(75);
    }
    if (is_stuck(delta)) {
        ++locomotion->field_40;
        return result(2);
    }
    auto *avoid = node<avoidance_inode>(this, avoidance_id, false);
    if (path->has_more_points() &&
        ((avoid != nullptr && (avoid->field_30 & 4) != 0) || (!path->field_A0 && path->can_see_next_point())))
        path->get_next_point();
    return result(75);
}
vector3d loco_goto_state::steer_direction(const vector3d &direction, Float delta)
{
    if (avoidance == nullptr)
        return direction;
    if (!locomotion->explicit_goto_radius)
        locomotion->set_goto_radius(-1.0f);
    const bool last = dispatch<bool>(this, 23);
    const auto *target = dispatch<const vector3d *>(this, 22);
    using callback =
        void(__fastcall *)(avoidance_inode *, void *, const vector3d &, const vector3d &, bool, float, float);
    reinterpret_cast<callback>(get_vfunc(avoidance->m_vtbl, 0x34))(
        avoidance, nullptr, direction, *target, last, locomotion->goto_radius, delta);
    return (avoidance->field_30 & 2) != 0 ? avoidance->desired_direction : direction;
}
void loco_goto_state::update_animation(const vector3d &direction, float speed)
{
    auto *animation = node<als_inode>(this, als_inode::default_id);
    auto *layer = animation->get_als_layer(static_cast<als::layer_types>(0));
    using predicate = bool(__fastcall *)(als::state_machine *, void *);
    if (!reinterpret_cast<predicate>(get_vfunc(layer->m_vtbl, 0x14))(layer, nullptr))
        return;
    als::param_list params;
    auto *target = node<combat_target_inode>(this, combat_target_inode::default_id, false);
    if (target != nullptr) {
        using get_target = vhandle_type<actor> *(__fastcall *)(combat_target_inode *, void *, vhandle_type<actor> *);
        vhandle_type<actor> handle;
        reinterpret_cast<get_target>(get_vfunc(target->m_vtbl, 0x38))(target, nullptr, &handle);
        params.add_param({52, handle.get_volatile_ptr() != nullptr ? 1.0f : 0.0f});
    }
    params.add_param({0, speed});
    const auto &facing = get_actor()->get_abs_po().get_z_facing();
    const bool zero = direction == vector3d{0.0f, 0.0f, 0.0f};
    params.add_param(30, zero ? facing : direction);
    params.add_param(locomotion->allow_facing_change ? 55 : 27,
                     locomotion->allow_facing_change || !zero ? direction : facing);
    params.add_param(24, UP);
    layer->set_desired_params(params);
    params.clear();
}
void loco_goto_state::update_category()
{
    auto *animation = node<als_inode>(this, als_inode::default_id);
    const auto layer = static_cast<als::layer_types>(0);
    if (locomotion->als_category != animation->get_category_id(layer))
        animation->request_category_transition(locomotion->als_category, layer, true, false, false);
    locomotion->field_55 = false;
}
state_trans_messages loco_goto_state::_frame_advance(Float delta)
{
    const auto message = enhanced_state::frame_advance(delta);
    if (message != result(75))
        return message;
    if (!locomotion->field_54 && locomotion->field_55)
        dispatch<void>(this, 25);
    if (locomotion->needs_repathfind)
        dispatch<void>(this, 24);
    if (!locomotion->explicit_goto_speed)
        locomotion->set_goto_speed(-1.0f);
    const auto *destination = dispatch<const vector3d *>(this, 22);
    auto direction = *destination - get_actor()->get_abs_position();
    if (locomotion->field_58)
        direction.y = 0.0f;
    const float distance = direction.length2();
    if (distance > 0.1f * 0.1f) {
        const float now = g_world_ptr->time_manager.field_0;
        const float interval =
            std::sqrt(std::clamp((distance - 10.0f) / 40.0f, 0.0f, 1.0f)) * (1.0f / 6.0f - 1.0f / 30.0f) + 1.0f / 30.0f;
        if (locomotion->always_update_als || (avoidance != nullptr && (avoidance->field_30 & 2) != 0) ||
            now - locomotion->field_20 > interval) {
            direction.normalize();
            vector3d steered;
            using steering = vector3d *(__fastcall *)(loco_goto_state *, void *, vector3d *, const vector3d &, Float);
            reinterpret_cast<steering>(get_vfunc(m_vtbl, 0x44))(this, nullptr, &steered, direction, delta);
            using update = void(__fastcall *)(loco_goto_state *, void *, const vector3d &, float);
            reinterpret_cast<update>(get_vfunc(m_vtbl, 0x48))(this, nullptr, steered, locomotion->goto_speed);
            locomotion->field_20 = now;
        }
    }
    using check = state_trans_messages(__fastcall *)(loco_goto_state *, void *, const vector3d &, float, Float);
    return reinterpret_cast<check>(get_vfunc(m_vtbl, 0x54))(this, nullptr, *destination, distance, delta);
}
state_trans_messages pathed_goto_state::_frame_advance(Float delta)
{
    if (locomotion->needs_repathfind)
        repath();
    return path->m_pathStatus.field_0 != 0 ? result(2) : loco_goto_state::_frame_advance(delta);
}
void pathed_goto_state::_deactivate(const mashed_state *)
{
    if (!locomotion->field_59)
        return;
    auto *quad = node<quad_path_inode>(this, quad_id);
    if (quad->path->m_pathStatus.field_0 != 0) {
        auto *animation = node<als_inode>(this, als_inode::default_id, false);
        if (animation != nullptr) {
            als::param_list params;
            params.add_param({0, 0.0f});
            animation->get_als_layer(static_cast<als::layer_types>(0))->set_desired_params(params);
            params.clear();
        }
    }
}
state_trans_action pathed_goto_state::_check_transition(Float) const
{
    return {NO_ACTION, {0}, result(75), nullptr};
}
state_trans_action pathed_goto_state::_process_message(Float, state_trans_messages msg)
{
    if (msg == result(1))
        return {MACHINE_EXIT, {0}, msg, nullptr};
    if (msg != result(2))
        return {NO_ACTION, {0}, result(75), nullptr};
    auto *biped = node<loco_inode>(this, biped_id);
    if (biped->field_40 == 1 || biped->field_40 >= 3) {
        mString text{biped->field_40 == 1 ? "I got stuck once"
                                          : "I got stuck three times, screw this, I'm going home!"};
        g_game_ptr->mb->post(*bit_cast<message_board::string *>(&text), 2.0f, color32{0xFFFFFFFF});
    }
    if (biped->field_40 == 1)
        return {GOTO_STATE, string_hash{int(to_hash("pathed_goto"))}, result(75), nullptr};
    if (biped->field_40 < 3 && biped->field_56)
        return {GOTO_STATE, string_hash{int(to_hash("nonpathed_goto"))}, result(75), nullptr};
    return {MACHINE_EXIT, {0}, msg, nullptr};
}
}
