#include "als_mocomp.h"

#include "actor.h"
#include "biped_system.h"
#include "common.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "phys_vector3d.h"
#include "rigid_body.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"
#include "als_animation_logic_system.h"
#include "animation_controller.h"
#include "event.h"
#include "event_manager.h"
#include "parse_generic_mash.h"
#include "vtbl.h"
#include "custom_math.h"
#include "mash_info_struct.h"
#include "memory.h"
#include "ai_std_combat_target.h"
#include "base_ai_core.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

namespace als {

VALIDATE_SIZE(begin_biped_physics, 0x20);

float begin_biped_physics::activate(animation_logic_system *a1)
{
    TRACE("als::begin_biped_physics::activate");

    motion_compensator::activate(a1);
    this->field_14 = true;
    this->field_18 = 0;
    assert(the_actor->has_physical_ifc() && the_actor->physical_ifc()->is_biped_physics_running() &&
           "The user of the begin_biped_physics mocomp must be running biped physics.");

    auto *v4 = the_actor->physical_ifc();
    auto *biped_system = v4->get_biped_system();

    auto *v8 = &biped_system->field_0;
    float v44 = 0.0;
    float v45 = 10.0;
    float a2 = 0.0;
    auto *v9 = the_actor->physical_ifc();
    auto v43 = v9->get_velocity();

    vector3d v50;
    if (this->field_8->does_parameter_exist(velocity_left_c_param_hash)) {
        auto *v12 = this->field_8;
        auto v14 = v12->get_pb_float(velocity_left_c_param_hash);
        v50 *= v14;
    }

    if (this->field_8->does_parameter_exist(velocity_y_left_c_param_hash)) {
        auto *v17 = this->field_8;
        auto v19 = v17->get_pb_float(velocity_y_left_c_param_hash);
        v50[1] *= v19;
    }

    auto *v22 = the_actor->physical_ifc();
    v22->set_velocity(v43, false);
    auto *v23 = this->field_8;
    this->field_1C = v23->get_optional_pb_float(velocity_left_deactivate_c_param_hash, 1.0, nullptr);
    auto *v25 = this->field_8;
    float v29;
    if (v25->does_parameter_exist(force_scale_param_hash)) {
        v29 = this->field_8->get_pb_float(force_scale_param_hash);
    } else if (not_equal(0.0f, this->field_8->get_param(this->field_4, 86u))) {
        v29 = this->field_8->get_param(this->field_4, 86u);
    }

    v44 = v29;
    if (this->field_8->does_parameter_exist(force_y_param_hash)) {
        auto *v32 = this->field_8;
        v45 = v32->get_pb_float(force_y_param_hash);
    }

    if (this->field_8->does_parameter_exist(rotate_xz_ang_param_hash)) {
        a2 = this->field_8->get_pb_float(rotate_xz_ang_param_hash) * (3.1415927 / 180.0);
    }

    if (not_equal(v44, 0.0f) || not_equal(v45, 0.0f)) {
        auto *v38 = this->field_4;
        auto *v39 = this->field_8;
        auto a3 = v39->get_vector_param(v38, 83u) * v44;
        if (not_equal(v45, 0.0f)) {
            a3.y = v45;
        }

        if (not_equal(a2, 0.0f)) {
            static Var<po> stru_91F8D8{0x0091F8D8};
            po v54 = stru_91F8D8();
            v54.set_rotate_y(a2);
            a3 = v54.non_affine_slow_xform(a3);
        }

        auto v41 = 1.0f / (*v8->m_list_rigid_body.m_data)->field_130;
        a3 *= v41;
        v8->apply_pulse(0, a3);
    }

    return this->field_8->get_param(this->field_4, 90u);
}

VALIDATE_SIZE(move_and_face_no_anim_movement, 0x74);
VALIDATE_SIZE(constant_move_and_face, 0x74);
VALIDATE_SIZE(crawl_transition, 0x74);
VALIDATE_SIZE(combat_move_and_face, 0x74);
VALIDATE_SIZE(move_and_face, 0x74);
VALIDATE_SIZE(flight_mocomp, 0x50);
VALIDATE_SIZE(feed_mocomp, 0x74);
VALIDATE_OFFSET(crawl_transition, translation_disabled, 0x5C);
VALIDATE_OFFSET(crawl_transition, orientation_disabled, 0x60);
VALIDATE_OFFSET(crawl_transition, remaining_time, 0x64);
VALIDATE_OFFSET(crawl_transition, destination_id, 0x68);
VALIDATE_OFFSET(crawl_transition, field_70, 0x70);

namespace {
using move_face = move_and_face_no_anim_movement;

void dispatch_destination(move_face *self, int offset)
{
    using fn = void(__fastcall *)(move_face *, void *);
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, offset))(self, nullptr);
}

void dispatch_move_frame(move_face *self, int offset, Float dt)
{
    using fn = void(__fastcall *)(move_face *, void *, Float);
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, offset))(self, nullptr, dt);
}

vector3d dispatch_translation(move_face *self, int offset, Float dt, const po &before, const po &animated)
{
    using fn = vector3d *(__fastcall *)(move_face *, void *, vector3d *, Float, const po *, const po *);
    vector3d result;
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, offset))(self, nullptr, &result, dt, &before, &animated);
    return result;
}

po dispatch_orientation(move_face *self, int offset, Float dt, const po &before, const po &animated)
{
    using fn = po *(__fastcall *)(move_face *, void *, po *, Float, const po *, const po *);
    po result;
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, offset))(self, nullptr, &result, dt, &before, &animated);
    return result;
}
}  // namespace

void move_face::destruct_mashed_class()
{
    if (destination_id) {
        destination_id->destruct_mashed_class();
        destination_id = nullptr;
    }
}

void move_face::unmash(mash_info_struct *info, void *)
{
    if (destination_id) {
        destination_id = reinterpret_cast<string_hash *>(info->read_from_buffer(4, 4));
        destination_id->unmash(info, this);
    }
}

void move_face::activate(animation_logic_system *system)
{
    motion_compensator::activate(system);
    movement_speed = turn_rate = 0.0f;
    translation_disabled = orientation_disabled = 0;
    translation_mode = orientation_mode = 1;
    expired_translation_mode = expired_orientation_mode = 2;
    initial_position = the_actor->get_abs_position();
    remaining_time = 0.0f;
    dispatch_destination(this, 0x60);
    dispatch_destination(this, 0x50);
    destination = the_actor->get_abs_position();
    facing = the_actor->get_abs_po().get_z_facing();
    up = the_actor->get_abs_po().get_y_facing();
    dispatch_destination(this, 0x54);
    dispatch_destination(this, 0x58);
    dispatch_destination(this, 0x5C);
}

void constant_move_and_face::activate(animation_logic_system *system)
{
    move_face::activate(system);
    using optional_fn = float(__fastcall *)(state_machine *, void *, const string_hash &, Float, bool *);
    auto optional = reinterpret_cast<optional_fn>(get_vfunc(field_8->m_vtbl, 0x64));
    movement_speed = optional(field_8, nullptr, string_hash{int(to_hash("movement_speed"))}, 5.0f, nullptr);
    turn_rate =
        optional(field_8, nullptr, string_hash{int(to_hash("turn_rate"))}, 720.0f, nullptr) * (3.1415927f / 180.0f);
}
void combat_move_and_face::activate(animation_logic_system *system)
{
    move_face::activate(system);
    turn_rate = field_8->get_optional_pb_float(string_hash{"turn_rate"}, 720.0f, nullptr) * (3.1415927f / 180.0f);
    movement_speed = field_8->get_optional_pb_float(string_hash{"movement_speed"}, -1.0f, nullptr);
}

void crawl_transition::activate(animation_logic_system *system)
{
    constant_move_and_face::activate(system);
    auto has = [this](uint32_t type) {
        return field_8->find_external_param(static_cast<external_parameter_types>(type)) != nullptr;
    };
    auto param = [this](uint32_t type) {
        return field_8->get_param(field_4, type);
    };
    up = the_actor->get_abs_po().get_y_facing();
    if (has(42) && has(7)) {
        const int transition = static_cast<int>(std::floor(param(7) + 0.5f));
        facing = field_8->get_vector_param(field_4, 42);
        if (!(transition == 2 || transition == 3 || transition == 4 || transition == 5 || transition == 7))
            facing = -facing;
    } else if (has(27)) {
        facing = vector3d{param(27), param(28), param(29)};
    } else {
        facing = the_actor->get_abs_po().get_z_facing();
    }
    facing -= up * dot(up, facing);
    if (facing.length2() < 0.01f)
        facing = the_actor->get_abs_po().get_z_facing();
    facing.normalize();
    if (has(8)) {
        po rotation;
        rotation.set_rot(up, param(8) * (3.1415927f / 180.0f));
        facing = rotation.non_affine_slow_xform(facing);
    }
    const vector3d position = the_actor->get_abs_position();
    if (has(39)) {
        const float margin = has(51) ? param(51) : 0.5f;
        destination = vector3d{param(39), param(40), param(41)} - facing * margin;
        const vector3d delta = destination - position;
        destination = position + delta - up * dot(delta, up);
    } else {
        destination = position;
    }
}

vector3d move_face::simple_linear_translation(Float dt, float fraction, const po &pose)
{
    const vector3d position = pose.get_position();
    if (std::equal_to<float>{}(destination.x, position.x) && std::equal_to<float>{}(destination.y, position.y) &&
        std::equal_to<float>{}(destination.z, position.z))
        return position;
    vector3d delta = destination - position;
    const float distance = delta.length();
    if (distance > 0.0f)
        delta *= 1.0f / distance;
    float travel = distance * fraction;
    if (movement_speed <= 0.0f) {
        if (std::equal_to<float>{}(movement_speed, -1.0f))
            movement_speed = std::fabs(travel / dt) * 1.5f + 5.0f;
        else
            movement_speed = -1.0f;
    } else if (travel / dt > movement_speed) {
        travel = dt * movement_speed;
    }
    return position + delta * travel;
}

po move_face::simple_linear_orientation(Float dt, float fraction, const po &pose)
{
    vector3d axis = up;
    if (axis.length2() > 9.999999439624929e-11f)
        axis *= 1.0f / axis.length();
    vector3d desired = facing - axis * dot(facing, axis);
    vector3d current = the_actor->get_abs_po().get_z_facing();
    current -= axis * dot(current, axis);
    if (desired.length2() <= 0.0001f || current.length2() <= 0.0001f)
        return pose;
    desired.normalize();
    current.normalize();
    vector3d right = -the_actor->get_abs_po().get_x_facing();
    right -= axis * dot(right, axis);
    right.normalize();
    const float angle = sub_48A720(dot(desired, right), dot(desired, current));
    if (angle <= 0.01f && angle >= -0.01f)
        return pose;
    float step = angle * fraction;
    const float angular_speed = step / dt;
    if (turn_rate > 0.0f && std::fabs(angular_speed) > turn_rate) {
        step = dt * turn_rate;
        if (angular_speed < 0.0f)
            step = -step;
    }
    po rotation;
    rotation.set_rot(up, step);
    po result;
    result.set_from_ptr_to_po_world(ptr_to_po{&pose.m, &rotation.m});
    return result;
}

vector3d move_face::compute_translation(Float dt, const po &before, const po &)
{
    if (translation_disabled)
        return before.get_position();
    using fn = vector3d *(__fastcall *)(move_face *, void *, vector3d *, Float, float, const po *);
    vector3d result;
    reinterpret_cast<fn>(get_vfunc(m_vtbl, 0x78))(this, nullptr, &result, dt, dt / remaining_time, &before);
    return result;
}

po move_face::compute_orientation(Float dt, const po &before, const po &)
{
    if (orientation_disabled)
        return before;
    using fn = po *(__fastcall *)(move_face *, void *, po *, Float, float, const po *);
    po result;
    reinterpret_cast<fn>(get_vfunc(m_vtbl, 0x7C))(this, nullptr, &result, dt, dt / remaining_time, &before);
    return result;
}

vector3d move_face::select_translation(Float dt, const po &before, const po &animated)
{
    const int mode = remaining_time <= 0.0f ? expired_translation_mode : translation_mode;
    switch (mode) {
    case 0:
        return before.get_position();
    case 1:
        return dispatch_translation(this, 0x80, dt, before, animated);
    case 2:
        return animated.get_position();
    case 3:
        return dispatch_translation(this, 0x84, dt, before, animated);
    default:
#ifdef _MSC_VER
        __assume(0);
#else
        __builtin_unreachable();
#endif
    }
}

po move_face::select_orientation(Float dt, const po &before, const po &animated)
{
    const int mode = remaining_time <= 0.0f ? expired_orientation_mode : orientation_mode;
    switch (mode) {
    case 0:
        return before;
    case 1:
        return dispatch_orientation(this, 0x8C, dt, before, animated);
    case 2:
        return animated;
    case 3:
        return dispatch_orientation(this, 0x90, dt, before, animated);
    default:
        return po{};
    }
}

void move_face::post_anim_action(Float dt)
{
    if (field_8->find_external_param(static_cast<external_parameter_types>(16))) {
        using get_fn = double(__fastcall *)(move_face *, void *);
        using set_fn = void(__fastcall *)(move_face *, void *, Float);
        const Float speed = reinterpret_cast<get_fn>(get_vfunc(m_vtbl, 0x48))(this, nullptr);
        reinterpret_cast<set_fn>(get_vfunc(m_vtbl, 0x40))(this, nullptr, speed);
    }
    dispatch_move_frame(this, 0x74, dt);
    dispatch_move_frame(this, 0xA0, dt);
    dispatch_move_frame(this, 0xA8, dt);
    dispatch_move_frame(this, 0xA4, dt);
    dispatch_move_frame(this, 0x9C, dt);
}

void move_face::face_and_arrive_by(Float dt)
{
    const po before = the_actor->get_abs_po();
    using fn = po *(__fastcall *)(move_face *, void *, po *);
    po animated;
    reinterpret_cast<fn>(get_vfunc(m_vtbl, 0x98))(this, nullptr, &animated);
    const vector3d position = dispatch_translation(this, 0x88, dt, before, animated);
    po result = dispatch_orientation(this, 0x94, dt, before, animated);
    result.set_position(position);
    entity_set_abs_po(the_actor, result);
    the_actor->set_frame_delta_trans(the_actor->get_abs_position() - vector3d{before.get_position()}, dt);
}

po move_face::apply_animation_offset()
{
    po animation_pose;
    field_4->get_animation_controller()->get_curr_po_offset(animation_pose);
    animation_pose.set_from_ptr_to_po_world(ptr_to_po{&animation_pose.m, &the_actor->get_rel_po().m});
    animation_pose.sub_48D840();
    the_actor->get_rel_po() = animation_pose;
    the_actor->dirty_family(false);
    if (the_actor->is_conglom_member() || the_actor->is_a_conglomerate())
        the_actor->dirty_model_po_family();
    using changed_fn = void(__fastcall *)(actor *, void *);
    reinterpret_cast<changed_fn>(get_vfunc(the_actor->m_vtbl, 0x34))(the_actor, nullptr);
    return the_actor->get_abs_po();
}

namespace {
void __fastcall crawl_destruct(move_face *self, void *)
{
    self->destruct_mashed_class();
}
void __fastcall crawl_unmash(move_face *self, void *, mash_info_struct *info, void *context)
{
    self->unmash(info, context);
}
void *__fastcall crawl_delete(crawl_transition *self, void *, unsigned char flags)
{
    self->~crawl_transition();
    if (flags & 1)
        mem_dealloc(self, sizeof(*self));
    return self;
}
int __fastcall crawl_type(crawl_transition *, void *)
{
    return 503;
}
bool __fastcall crawl_parent(crawl_transition *, void *, uint32_t type)
{
    return type == 498 || type == 512 || type == 490 || type == 573;
}
void __fastcall crawl_activate(crawl_transition *self, void *, animation_logic_system *system)
{
    self->activate(system);
}
void __fastcall crawl_post(move_face *self, void *, Float dt)
{
    self->post_anim_action(dt);
}
int __fastcall crawl_size(crawl_transition *, void *)
{
    return sizeof(crawl_transition);
}

void __fastcall move_reset_time(move_face *self, void *)
{
    self->remaining_time = 0.0001f;
}
void __fastcall move_destination_empty(move_face *, void *) {}
int __fastcall move_reset_destination(move_face *self, void *)
{
    self->destination_id = nullptr;
    self->field_6C = 0;
    self->field_70 = false;
    return 0;
}
void __fastcall move_update_position(move_face *self, void *)
{
    dispatch_destination(self, 0x54);
}
void __fastcall move_update_facing(move_face *self, void *)
{
    dispatch_destination(self, 0x58);
}
void __fastcall move_update_up(move_face *self, void *)
{
    dispatch_destination(self, 0x5C);
}
void __fastcall move_update_destination(move_face *self, void *, Float)
{
    dispatch_destination(self, 0x64);
    dispatch_destination(self, 0x68);
    dispatch_destination(self, 0x6C);
    dispatch_destination(self, 0x70);
}
vector3d *__fastcall move_linear_translation(move_face *self, void *, vector3d *out, Float dt, float fraction,
                                             const po *pose)
{
    *out = self->simple_linear_translation(dt, fraction, *pose);
    return out;
}
po *__fastcall move_linear_orientation(move_face *self, void *, po *out, Float dt, float fraction, const po *pose)
{
    *out = self->simple_linear_orientation(dt, fraction, *pose);
    return out;
}
vector3d *__fastcall move_compute_translation(move_face *self, void *, vector3d *out, Float dt, const po *before,
                                              const po *animated)
{
    *out = self->compute_translation(dt, *before, *animated);
    return out;
}
vector3d *__fastcall move_hybrid_translation(move_face *self, void *, vector3d *out, Float dt, const po *,
                                             const po *animated)
{
    *out = dispatch_translation(self, 0x80, dt, *animated, *animated);
    return out;
}
vector3d *__fastcall move_select_translation(move_face *self, void *, vector3d *out, Float dt, const po *before,
                                             const po *animated)
{
    *out = self->select_translation(dt, *before, *animated);
    return out;
}
po *__fastcall move_compute_orientation(move_face *self, void *, po *out, Float dt, const po *before,
                                        const po *animated)
{
    *out = self->compute_orientation(dt, *before, *animated);
    return out;
}
po *__fastcall move_hybrid_orientation(move_face *self, void *, po *out, Float dt, const po *, const po *animated)
{
    *out = dispatch_orientation(self, 0x8C, dt, *animated, *animated);
    return out;
}
po *__fastcall move_select_orientation(move_face *self, void *, po *out, Float dt, const po *before, const po *animated)
{
    *out = self->select_orientation(dt, *before, *animated);
    return out;
}
po *__fastcall move_animation_offset(move_face *self, void *, po *out)
{
    *out = self->apply_animation_offset();
    return out;
}
void __fastcall move_arrival_event(move_face *self, void *, Float)
{
    if (std::fabs(self->remaining_time) < 0.0001f)
        event_manager::raise_event(event::ANIM_DEST_REACHED, self->the_actor->my_handle);
}
void __fastcall move_clamp_time(move_face *self, void *, Float dt)
{
    if (self->remaining_time > 0.0f && self->remaining_time < dt)
        self->remaining_time = dt;
}
void __fastcall move_decrement_time(move_face *self, void *, Float dt)
{
    self->remaining_time -= dt;
}
void __fastcall move_arrive(move_face *self, void *, Float dt)
{
    self->face_and_arrive_by(dt);
}
void *__fastcall arrive_delete(combat_move_and_face *self, void *, unsigned char flags)
{
    self->~combat_move_and_face();
    if (flags & 1)
        mem_dealloc(self, sizeof(*self));
    return self;
}
int __fastcall arrive_type(combat_move_and_face *, void *)
{
    return 496;
}
bool __fastcall arrive_parent(combat_move_and_face *, void *, uint32_t type)
{
    return type == 512 || type == 490 || type == 573;
}
void __fastcall arrive_activate(combat_move_and_face *self, void *, animation_logic_system *system)
{
    self->activate(system);
}
void __fastcall arrive_reset(move_face *self, void *)
{
    static string_hash signals[]{event::ATTACK, event::ANIM_ACTION};
    self->destination_id = signals;
    self->field_6C = 2;
    self->field_70 = true;
}
void __fastcall arrive_time(move_face *self, void *)
{
    for (int i = 0; i < self->field_6C; ++i) {
        self->remaining_time = self->field_8->get_time_to_signal(self->destination_id[i]);
        if (self->remaining_time >= 0.0f)
            break;
    }
    if (self->remaining_time <= 0.0f)
        self->remaining_time = self->field_8->get_time_to_end_of_anim();
    if (self->remaining_time < EPSILON)
        self->remaining_time = 0.0001f;
}
template <unsigned Parameter, vector3d move_face::*Member>
void __fastcall arrive_destination(move_face *self, void *)
{
    if (self->field_8->find_external_param(static_cast<external_parameter_types>(Parameter)))
        self->*Member = self->field_8->get_vector_param(self->field_4, Parameter);
}
}  // namespace

void *crawl_transition::native_vtable()
{
    static auto table = [] {
        std::array<void *, 43> result{};
        std::copy_n(static_cast<void **>(motion_compensator::native_vtable(490)), 20, result.begin());
        result[0] = reinterpret_cast<void *>(crawl_destruct);
        result[1] = reinterpret_cast<void *>(crawl_unmash);
        result[2] = reinterpret_cast<void *>(crawl_delete);
        result[3] = reinterpret_cast<void *>(crawl_type);
        result[4] = reinterpret_cast<void *>(crawl_parent);
        result[6] = reinterpret_cast<void *>(crawl_activate);
        result[9] = reinterpret_cast<void *>(crawl_post);
        result[19] = reinterpret_cast<void *>(crawl_size);
        result[20] = reinterpret_cast<void *>(move_reset_time);
        result[21] = reinterpret_cast<void *>(move_destination_empty);
        result[22] = reinterpret_cast<void *>(move_destination_empty);
        result[23] = reinterpret_cast<void *>(move_destination_empty);
        result[24] = reinterpret_cast<void *>(move_reset_destination);
        result[25] = reinterpret_cast<void *>(move_reset_time);
        result[26] = reinterpret_cast<void *>(move_update_position);
        result[27] = reinterpret_cast<void *>(move_update_facing);
        result[28] = reinterpret_cast<void *>(move_update_up);
        result[29] = reinterpret_cast<void *>(move_update_destination);
        result[30] = reinterpret_cast<void *>(move_linear_translation);
        result[31] = reinterpret_cast<void *>(move_linear_orientation);
        result[32] = reinterpret_cast<void *>(move_compute_translation);
        result[33] = reinterpret_cast<void *>(move_hybrid_translation);
        result[34] = reinterpret_cast<void *>(move_select_translation);
        result[35] = reinterpret_cast<void *>(move_compute_orientation);
        result[36] = reinterpret_cast<void *>(move_hybrid_orientation);
        result[37] = reinterpret_cast<void *>(move_select_orientation);
        result[38] = reinterpret_cast<void *>(move_animation_offset);
        result[39] = reinterpret_cast<void *>(move_arrival_event);
        result[40] = reinterpret_cast<void *>(move_clamp_time);
        result[41] = reinterpret_cast<void *>(move_decrement_time);
        result[42] = reinterpret_cast<void *>(move_arrive);
        return result;
    }();
    return table.data();
}
void *combat_move_and_face::native_vtable()
{
    static auto table = [] {
        std::array<void *, 43> result;
        std::copy_n(static_cast<void **>(crawl_transition::native_vtable()), result.size(), result.begin());
        result[2] = reinterpret_cast<void *>(&arrive_delete);
        result[3] = reinterpret_cast<void *>(&arrive_type);
        result[4] = reinterpret_cast<void *>(&arrive_parent);
        result[6] = reinterpret_cast<void *>(&arrive_activate);
        result[20] = reinterpret_cast<void *>(&arrive_time);
        result[21] = reinterpret_cast<void *>(&arrive_destination<33, &move_face::destination>);
        result[22] = reinterpret_cast<void *>(&arrive_destination<27, &move_face::facing>);
        result[23] = reinterpret_cast<void *>(&arrive_destination<24, &move_face::up>);
        result[24] = reinterpret_cast<void *>(&arrive_reset);
        result[25] = reinterpret_cast<void *>(&move_destination_empty);
        return result;
    }();
    return table.data();
}

void feed_mocomp::activate(animation_logic_system *)
{
    move_and_face_no_anim_movement::activate(field_4);
    expired_translation_mode = expired_orientation_mode = 0;
}

namespace {
int __fastcall feed_type(feed_mocomp *, void *)
{
    return 513;
}
void __fastcall feed_activate(feed_mocomp *self, void *, animation_logic_system *system)
{
    self->activate(system);
}
}

void *feed_mocomp::native_vtable()
{
    static auto table = [] {
        std::array<void *, 43> result;
        std::copy_n(static_cast<void **>(combat_move_and_face::native_vtable()), result.size(), result.begin());
        result[3] = reinterpret_cast<void *>(&feed_type);
        result[6] = reinterpret_cast<void *>(&feed_activate);
        result[24] = reinterpret_cast<void *>(&move_reset_destination);
        return result;
    }();
    return table.data();
}

namespace {
void suppress_motion_physics(physical_interface *physical)
{
    physical->field_C &= ~0x200u;
    physical->set_gravity(false);
    physical->field_C |= 0x800u;
}

void restore_motion_physics(actor *owner, bool collision, bool gravity, bool motion)
{
    auto *physical = owner->physical_ifc();
    physical->enable(true);
    physical->field_C = collision ? physical->field_C | 0x200u : physical->field_C & ~0x200u;
    physical->set_gravity(gravity);
    physical->field_C = motion ? physical->field_C | 0x800u : physical->field_C & ~0x800u;
    physical->set_velocity(ZEROVEC, false);
}

void update_playback_speed(motion_compensator *self)
{
    using get_fn = double(__fastcall *)(motion_compensator *, void *);
    using set_fn = void(__fastcall *)(motion_compensator *, void *, Float);
    const float speed = reinterpret_cast<get_fn>(get_vfunc(self->m_vtbl, 0x48))(self, nullptr);
    reinterpret_cast<set_fn>(get_vfunc(self->m_vtbl, 0x40))(self, nullptr, speed);
}

bool set_motion_position(motion_compensator *self, vector3d &position)
{
    using fn = bool(__fastcall *)(motion_compensator *, void *, vector3d *);
    return reinterpret_cast<fn>(get_vfunc(self->m_vtbl, 0x30))(self, nullptr, &position);
}

void apply_motion_offset(motion_compensator *self, po &offset)
{
    using fn = void(__fastcall *)(motion_compensator *, void *, actor *, po *);
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, 0x2C))(self, nullptr, self->the_actor, &offset);
}

void face_motion_direction(motion_compensator *self, const vector3d &direction, Float dt)
{
    using fn = void(__fastcall *)(motion_compensator *, void *, actor *, vector3d, vector3d, Float, Float, Float);
    reinterpret_cast<fn>(get_vfunc(self->m_vtbl, 0x34))(self,
                                                        nullptr,
                                                        self->the_actor,
                                                        self->the_actor->get_abs_po().get_z_facing(),
                                                        direction,
                                                        self->get_turn_rate(10.0f),
                                                        std::cos(0.08726646502812704),
                                                        dt);
}

vhandle_type<actor> flight_target(actor *owner, bool *has_node = nullptr)
{
    auto *node = owner->get_ai_core()->get_info_node(ai::combat_target_inode::default_id, true);
    if (has_node)
        *has_node = node != nullptr;
    if (node == nullptr)
        return {};
    using fn = vhandle_type<actor> *(__fastcall *)(ai::info_node *, void *, vhandle_type<actor> *);
    vhandle_type<actor> result;
    reinterpret_cast<fn>(get_vfunc(node->m_vtbl, 0x38))(node, nullptr, &result);
    return result;
}

template <typename T, unsigned Type>
int __fastcall motion_type(T *, void *)
{
    return Type;
}

template <typename T>
int __fastcall motion_size(T *, void *)
{
    return sizeof(T);
}

template <typename T>
void *__fastcall motion_delete(T *self, void *, unsigned flags)
{
    self->~T();
    if (flags & 1)
        mem_dealloc(self, sizeof(T));
    return self;
}

template <typename T>
void __fastcall motion_activate(T *self, void *, animation_logic_system *system)
{
    self->activate(system);
}

template <typename T>
void __fastcall motion_deactivate(T *self, void *)
{
    self->deactivate();
}

template <typename T>
void __fastcall motion_post(T *self, void *, Float dt)
{
    self->post_anim_action(dt);
}

template <typename T, unsigned Type>
void *movement_vtable()
{
    static auto table = [] {
        std::array<void *, 20> result;
        std::copy_n(static_cast<void **>(motion_compensator::native_vtable(490)), result.size(), result.begin());
        result[2] = reinterpret_cast<void *>(&motion_delete<T>);
        result[3] = reinterpret_cast<void *>(&motion_type<T, Type>);
        result[6] = reinterpret_cast<void *>(&motion_activate<T>);
        if constexpr (Type != 502)
            result[7] = reinterpret_cast<void *>(&motion_deactivate<T>);
        result[9] = reinterpret_cast<void *>(&motion_post<T>);
        result[19] = reinterpret_cast<void *>(&motion_size<T>);
        return result;
    }();
    return table.data();
}
}

VALIDATE_SIZE(crawl_orient, 0x14);

void *crawl_orient::native_vtable()
{
    return movement_vtable<crawl_orient, 502>();
}

void crawl_orient::post_anim_action(Float dt)
{
    update_playback_speed(this);
    po offset{};
    field_4->get_animation_controller()->get_curr_po_offset(offset);
    static const string_hash turn_rate_hash{int(to_hash("turn_rate"))};
    const float turn_rate = field_8->get_optional_pb_float(turn_rate_hash, 10.0f, nullptr);
    vector3d direction = field_8->get_vector_param(field_4, 27);
    auto *owner = field_4->get_actor();
    auto up = owner->get_abs_po().get_y_facing();
    direction = orthogonal_projection_onto_plane(direction, up);
    const float length2 = direction.length2();
    if (length2 > LARGE_EPSILON)
        direction *= static_cast<float>(1.0 / std::sqrt(static_cast<double>(length2)));
    if (direction.length2() > EPSILON) {
        vector3d forward;
        reorient_vectors(
            owner->get_abs_po().get_z_facing(), up, direction, up, forward, up, Float{turn_rate * dt.value});
        po transform{};
        transform.set_po(forward, up, the_actor->get_abs_position());
        entity_set_abs_po(the_actor, transform);
    }
    const auto before = the_actor->get_abs_position();
    apply_motion_offset(this, offset);
    the_actor->set_frame_delta_trans(the_actor->get_abs_position() - before, dt);
}

void *move_and_face::native_vtable()
{
    return movement_vtable<move_and_face, 511>();
}

void move_and_face::activate(animation_logic_system *system)
{
    motion_compensator::activate(system);
    reverse_facing = field_8->get_optional_pb_int(string_hash{int(to_hash("reverse_facing"))}, 0, nullptr) != 0;
    land_max_slowdown_rate =
        field_8->get_optional_pb_float(string_hash{int(to_hash("land_max_slowdown_rate"))}, 500.0f, nullptr);
    target = field_8->get_vector_param(system, 33);
    has_target = target != ZEROVEC;
    target_initialized = has_target;
    if (has_target)
        initial_target = target;
    if (the_actor->has_physical_ifc()) {
        auto *physical = the_actor->physical_ifc();
        saved_collision = (physical->field_C & 0x200u) != 0;
        saved_gravity = (physical->field_C & 4u) != 0;
        saved_motion = (physical->field_C & 0x800u) != 0;
        suppress_motion_physics(physical);
        velocity = added_velocity = ZEROVEC;
        acceleration = {0.0f, g_gravity * physical->m_gravity_multiplier, 0.0f};
    }
    if (field_8->does_parameter_exist(string_hash{int(to_hash("target_cheat_k"))}))
        animation_target_offset.z = field_8->get_pb_float(string_hash{int(to_hash("target_cheat_k"))});
    else
        target_cheat_k = 200.0f;
    grounded = false;
}

void move_and_face::deactivate()
{
    if (the_actor->has_physical_ifc())
        restore_motion_physics(the_actor, saved_collision, saved_gravity, saved_motion);
}

void move_and_face::post_anim_action(Float dt)
{
    update_playback_speed(this);
    auto *physical = the_actor->physical_ifc();
    physical->enable(false);
    suppress_motion_physics(physical);
    acceleration = {0.0f, -9.81f * physical->m_gravity_multiplier, 0.0f};
    if (!grounded)
        velocity += acceleration * dt;
    const auto external_velocity = field_8->get_vector_param(field_4, 83);
    if (external_velocity != added_velocity) {
        added_velocity = external_velocity;
        velocity += external_velocity;
    }
    const auto before = the_actor->get_abs_position();
    po offset{};
    field_4->get_animation_controller()->get_curr_po_offset(offset);
    apply_motion_offset(this, offset);
    auto position = the_actor->get_abs_position() + velocity * dt;
    grounded = set_motion_position(this, position);
    the_actor->set_frame_delta_trans(the_actor->get_abs_position() - before, dt);
    physical->set_velocity(velocity, false);
    if (grounded) {
        if (added_velocity == ZEROVEC) {
            velocity = ZEROVEC;
        } else {
            const float change = dt * land_max_slowdown_rate;
            if (std::fabs(velocity.y) >= land_max_slowdown_rate)
                velocity.y -= velocity.y > 0.0f ? change : -change;
            else
                velocity.y = 0.0f;
            const float horizontal2 = velocity.x * velocity.x + velocity.z * velocity.z;
            if (change * change <= horizontal2) {
                const float fraction = change / std::sqrt(horizontal2);
                velocity.x -= velocity.x * fraction;
                velocity.z -= velocity.z * fraction;
            } else {
                velocity.x = velocity.z = 0.0f;
            }
        }
        return;
    }
    target = field_8->get_vector_param(field_4, 33);
    has_target = target != ZEROVEC;
    if (has_target) {
        if (!target_initialized) {
            initial_target = target;
            target_initialized = true;
        }
        const float radius = field_8->get_param(field_4, 61);
        auto distance = target - initial_target;
        if (radius > 0.0f && distance.length2() > radius * radius) {
            distance.set_length(radius);
            target = initial_target + distance;
        }
        animation_target_offset = ZEROVEC;
        if (field_8->does_parameter_exist(string_hash{int(to_hash("delta_pos_z"))}))
            animation_target_offset.z = field_8->get_pb_float(string_hash{int(to_hash("delta_pos_z"))});
        const auto origin = the_actor->get_abs_po().slow_xform(animation_target_offset);
        const double quadratic = -0.5 * g_gravity * physical->m_gravity_multiplier;
        const double constant = origin.y - target.y;
        const double discriminant = velocity.y * velocity.y - 4.0 * quadratic * constant;
        double time = -1.0;
        if (std::not_equal_to<double>{}(quadratic, 0.0) && discriminant >= 0.0) {
            const double root = std::sqrt(discriminant);
            time = std::max((-velocity.y - root) / (2.0 * quadratic), (root - velocity.y) / (2.0 * quadratic));
        }
        const float error_x = target.x - (origin.x + velocity.x * time);
        const float error_z = target.z - (origin.z + velocity.z * time);
        if (time > 0.1 && error_x * error_x + error_z * error_z > 0.1f) {
            vector3d correction{error_x / static_cast<float>(time), 0.0f, error_z / static_cast<float>(time)};
            const float maximum = dt * target_cheat_k;
            if (correction.length2() > maximum * maximum)
                correction.set_length(maximum);
            velocity += correction;
        }
    }
    if (velocity.x * velocity.x + velocity.z * velocity.z > 1.0f)
        face_motion_direction(this, velocity * (reverse_facing ? -1.0f : 1.0f), dt);
}

void *flight_mocomp::native_vtable()
{
    return movement_vtable<flight_mocomp, 508>();
}

void flight_mocomp::activate(animation_logic_system *system)
{
    motion_compensator::activate(system);
    target_offset = {field_8->get_optional_pb_float(string_hash{int(to_hash("target_offset_x"))}, 0.0f, nullptr),
                     field_8->get_optional_pb_float(string_hash{int(to_hash("target_offset_y"))}, 0.0f, nullptr),
                     field_8->get_optional_pb_float(string_hash{int(to_hash("target_offset_z"))}, 0.0f, nullptr)};
    var<vector3d>(0x009595D4) = target_offset;
    target_cheat_k = std::clamp(
        field_8->get_optional_pb_float(string_hash{int(to_hash("target_cheat_k"))}, 0.25f, nullptr), 0.0001f, 1.0f);
    restore_visible_timer =
        field_8->get_optional_pb_float(string_hash{int(to_hash("restore_visible_timer"))}, 0.0f, nullptr);
    restored_visible = false;
    const auto target_handle = flight_target(the_actor);
    if (target_handle.get_volatile_ptr())
        the_actor->add_collision_ignorance(target_handle.field_0);
    if (the_actor->has_physical_ifc()) {
        auto *physical = the_actor->physical_ifc();
        saved_collision = (physical->field_C & 0x200u) != 0;
        saved_gravity = (physical->field_C & 4u) != 0;
        saved_motion = (physical->field_C & 0x800u) != 0;
        suppress_motion_physics(physical);
    }
}

void flight_mocomp::deactivate()
{
    the_actor->set_visible(true, false);
    if (the_actor->get_ai_core()) {
        const auto target_handle = flight_target(the_actor);
        if (target_handle.get_volatile_ptr())
            the_actor->remove_collision_ignorance(target_handle.field_0);
    }
    if (the_actor->has_physical_ifc())
        restore_motion_physics(the_actor, saved_collision, saved_gravity, saved_motion);
}

void flight_mocomp::post_anim_action(Float dt)
{
    if (restored_visible) {
        update_playback_speed(this);
        const auto before = the_actor->get_abs_position();
        po offset{};
        field_4->get_animation_controller()->get_curr_po_offset(offset);
        offset.set_from_ptr_to_po_world({&offset.m, &the_actor->get_rel_po().m});
        offset.sub_48D840();
        the_actor->get_rel_po() = offset;
        the_actor->dirty_family(false);
        if (the_actor->is_ext_flagged(0x8004u))
            the_actor->dirty_model_po_family();
        the_actor->po_changed();
        the_actor->set_frame_delta_trans(the_actor->get_abs_position() - before, dt);
        return;
    }
    auto *physical = the_actor->physical_ifc();
    physical->enable(false);
    suppress_motion_physics(physical);
    if (restore_visible_timer > 0.0f) {
        restore_visible_timer -= dt;
        if (restore_visible_timer <= 0.0f) {
            restored_visible = true;
            the_actor->set_visible(true, false);
            if (the_actor->has_physical_ifc()) {
                restore_motion_physics(the_actor, saved_collision, saved_gravity, saved_motion);
                return;
            }
        } else {
            the_actor->set_visible(false, false);
        }
    }
    bool has_node;
    const auto target_handle = flight_target(the_actor, &has_node);
    if (has_node) {
        if (auto *target_actor = target_handle.get_volatile_ptr()) {
            destination = target_actor->get_abs_po().slow_xform(target_offset);
            target_direction = destination - the_actor->get_abs_position();
            target_facing = -target_actor->get_abs_po().get_z_facing();
            if (target_direction.length2() > 1.0f)
                target_direction.normalize();
            else
                target_direction *= 0.0f;
        } else {
            destination = the_actor->get_abs_position();
            target_facing = the_actor->get_abs_po().get_z_facing();
        }
    }
    const auto before = the_actor->get_abs_position();
    auto position = before + (destination - before) * target_cheat_k;
    const auto distance = before - destination;
    if (distance.x * distance.x + distance.z * distance.z < 0.25f)
        the_actor->set_visible(false, false);
    set_motion_position(this, position);
    const auto translation = position - before;
    the_actor->set_frame_delta_trans(translation, dt);
    physical->set_velocity(translation / dt, false);
    face_motion_direction(this, target_facing, dt);
}

}  // namespace als

void als_mocomp_patch()
{
    {
        FUNC_ADDRESS(address, &als::begin_biped_physics::activate);
        //SET_JUMP(0x004A48C0, address);
    }
}
