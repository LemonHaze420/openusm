#include "als_simple_orientation.h"

#include "actor.h"
#include "ai_pedestrian.h"
#include "ai_std_avoidance.h"
#include "als_animation_logic_system.h"
#include "animation_controller.h"
#include "base_ai_core.h"
#include "common.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace als {

VALIDATE_SIZE(simple_orientation, 0x14);
VALIDATE_SIZE(simple_orientation_ped, 0x14);
VALIDATE_SIZE(set_orient_mocomp, 0x14);

float motion_compensator::get_turn_rate(float fallback) const
{
    using optional_fn = float(__fastcall *)(state_machine *, void *, const string_hash &, Float, bool *);
    using required_fn = float(__fastcall *)(state_machine *, void *, string_hash);
    auto optional = reinterpret_cast<optional_fn>(get_vfunc(field_8->m_vtbl, 0x64));
    bool found = false;
    float rate = optional(field_8, nullptr, string_hash{int(to_hash("turn_rate"))}, fallback, &found);
    if (found)
        return rate;
    const float time_0 = optional(field_8, nullptr, string_hash{int(to_hash("turn_time_0"))}, 0.0f, &found);
    if (found) {
        auto required = reinterpret_cast<required_fn>(get_vfunc(field_8->m_vtbl, 0x30));
        const float rate_0 = required(field_8, nullptr, string_hash{int(to_hash("turn_rate_0"))});
        const float time_1 = required(field_8, nullptr, string_hash{int(to_hash("turn_time_1"))});
        const float rate_1 = required(field_8, nullptr, string_hash{int(to_hash("turn_rate_1"))});
        const float time = field_8->get_param(field_4, 0x5D);
        double fraction = (static_cast<double>(time) - time_0) / (static_cast<double>(time_1) - time_0);
        if (fraction < 0.0f)
            fraction = 0.0f;
        else if (fraction > 1.0f)
            fraction = 1.0f;
        return fraction * (rate_1 - rate_0) + rate_0;
    }
    const float external_rate = field_8->get_param(field_4, 1);
    return external_rate > 0.0f ? external_rate : rate;
}

void simple_orientation::get_directions(animation_logic_system *system, state_machine *machine, vector3d &facing,
                                        vector3d &up)
{
    if (machine->find_external_param(static_cast<external_parameter_types>(0x1B))) {
        const float z = machine->get_param(system, 0x1D);
        const float y = machine->get_param(system, 0x1C);
        const float x = machine->get_param(system, 0x1B);
        facing = {x, y, z};
    } else {
        facing = the_actor->get_abs_po().get_z_facing();
    }
    if (machine->find_external_param(static_cast<external_parameter_types>(0x18))) {
        const float z = machine->get_param(system, 0x1A);
        const float y = machine->get_param(system, 0x19);
        const float x = machine->get_param(system, 0x18);
        up = {x, y, z};
    } else {
        up = the_actor->get_abs_po().get_y_facing();
    }
}

namespace {
void get_controller_offset(animation_logic_system *system, po &offset)
{
    auto *controller = system->get_animation_controller();
    using offset_fn = void(__fastcall *)(animation_controller *, void *, po *);
    reinterpret_cast<offset_fn>(get_vfunc(controller->m_vtbl, 0x74))(controller, nullptr, &offset);
}
}  // namespace

void set_orient_mocomp::post_anim_action(Float elapsed)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x004A3200, this, elapsed);
    } else {
        using scalar_fn = double(__fastcall *)(motion_compensator *, void *);
        using speed_fn = void(__fastcall *)(motion_compensator *, void *, Float);
        const Float speed = reinterpret_cast<scalar_fn>(get_vfunc(m_vtbl, 0x48))(this, nullptr);
        reinterpret_cast<speed_fn>(get_vfunc(m_vtbl, 0x40))(this, nullptr, speed);
        po offset{};
        get_controller_offset(field_4, offset);
        const auto position = the_actor->get_abs_position();
        const float facing_z = field_8->get_param(field_4, 29);
        const float facing_y = field_8->get_param(field_4, 28);
        const float facing_x = field_8->get_param(field_4, 27);
        vector3d facing{facing_x, facing_y, facing_z};
        const float up_z = field_8->get_param(field_4, 26);
        const float up_y = field_8->get_param(field_4, 25);
        const float up_x = field_8->get_param(field_4, 24);
        vector3d up{up_x, up_y, up_z};
        if (up.length2() <= EPSILON)
            up = the_actor->get_abs_po().get_y_facing();
        if (is_colinear(facing, up, 0.0099999998f)) {
            const auto &transform = the_actor->get_abs_po();
            facing = is_colinear(transform.get_z_facing(), up, 0.0099999998f) ? transform.get_y_facing()
                                                                              : transform.get_z_facing();
        }
        po transform;
        transform.set_po(facing, up, position);
        entity_set_abs_po(the_actor, transform);
        const auto previous_position = the_actor->get_abs_position();
        using offset_fn = void(__fastcall *)(motion_compensator *, void *, actor *, po *);
        reinterpret_cast<offset_fn>(get_vfunc(m_vtbl, 0x2C))(this, nullptr, the_actor, &offset);
        the_actor->set_frame_delta_trans(the_actor->get_abs_position() - previous_position, elapsed);
    }
}

namespace {
int __fastcall set_orient_type(set_orient_mocomp *, void *)
{
    return 519;
}
void __fastcall set_orient_post(set_orient_mocomp *self, void *, Float elapsed)
{
    self->post_anim_action(elapsed);
}
}

void *set_orient_mocomp::native_vtable()
{
    static auto table = [] {
        std::array<void *, 20> result;
        std::copy_n(static_cast<void **>(motion_compensator::native_vtable(490)), result.size(), result.begin());
        result[3] = reinterpret_cast<void *>(&set_orient_type);
        result[9] = reinterpret_cast<void *>(&set_orient_post);
        return result;
    }();
    return table.data();
}

void simple_orientation::post_anim_action(Float elapsed)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x004A13B0, this, elapsed);
    } else {
        using scalar_fn = double(__fastcall *)(simple_orientation *, void *);
        using set_speed_fn = void(__fastcall *)(simple_orientation *, void *, Float);
        const float speed = reinterpret_cast<scalar_fn>(get_vfunc(m_vtbl, 0x48))(this, nullptr);
        reinterpret_cast<set_speed_fn>(get_vfunc(m_vtbl, 0x40))(this, nullptr, speed);

        po offset{};
        get_controller_offset(field_4, offset);
        vector3d desired_facing, desired_up;
        using directions_fn = void(__fastcall *)(
            simple_orientation *, void *, animation_logic_system *, state_machine *, vector3d &, vector3d &);
        reinterpret_cast<directions_fn>(get_vfunc(m_vtbl, 0x50))(
            this, nullptr, field_4, field_8, desired_facing, desired_up);
        const float up_length_squared = desired_up.length2();
        if (up_length_squared > 1.0e-10f)
            desired_up /= std::sqrt(up_length_squared);
        desired_facing -= desired_up * dot(desired_facing, desired_up);
        const auto current_facing = field_4->get_actor()->get_abs_po().get_z_facing();
        const float turn_rate = get_turn_rate(10.0f);
        const float heading_change_max = reinterpret_cast<scalar_fn>(get_vfunc(m_vtbl, 0x58))(this, nullptr);
        using facing_fn = void(__fastcall *)(
            simple_orientation *, void *, actor *, vector3d, vector3d, vector3d, Float, Float, Float);
        reinterpret_cast<facing_fn>(get_vfunc(m_vtbl, 0x38))(this,
                                                             nullptr,
                                                             the_actor,
                                                             current_facing,
                                                             desired_facing,
                                                             desired_up,
                                                             elapsed,
                                                             heading_change_max,
                                                             turn_rate);

        const auto previous_position = the_actor->get_abs_position();
        using changes_fn = void(__fastcall *)(simple_orientation *, void *, Float);
        reinterpret_cast<changes_fn>(get_vfunc(m_vtbl, 0x54))(this, nullptr, elapsed);
        using offset_fn = void(__fastcall *)(simple_orientation *, void *, actor *, po *);
        reinterpret_cast<offset_fn>(get_vfunc(m_vtbl, 0x2C))(this, nullptr, the_actor, &offset);
        const auto translation = the_actor->get_abs_position() - previous_position;
        entity_set_abs_position(the_actor, previous_position + translation);
        the_actor->set_frame_delta_trans(translation, elapsed);
    }
}

namespace {
bool pedestrian_translation_blocked(actor *owner, const vector3d &translation)
{
    if (!(translation.length2() > EPSILON))
        return false;
    auto *core = owner->get_ai_core();
    if (core == nullptr)
        return false;
    auto *pedestrian =
        static_cast<ai::pedestrian_inode *>(core->get_info_node(ai::pedestrian_inode::default_id, false));
    if (pedestrian == nullptr || (pedestrian->field_1C & 0x401) != 0x401)
        return false;
    auto *avoidance =
        static_cast<ai::ped_avoidance_inode *>(core->get_info_node(ai::ped_avoidance_inode::default_id, false));
    return avoidance != nullptr && avoidance->blocks_translation(translation);
}
}  // namespace

void simple_orientation_ped::other_po_changes(Float)
{
    auto *machine = field_4->get_als_layer_internal(static_cast<layer_types>(0));
    auto *owner = field_4->get_actor();
    vector3d position = owner->get_abs_position();
    po offset{};
    get_controller_offset(field_4, offset);
    bool changed = false;
    if (!owner->has_physical_ifc()) {
        const float horizontal_distance = std::sqrt(offset.m[3].x * offset.m[3].x + offset.m[3].z * offset.m[3].z);
        const float slope = machine->get_param(field_4, 0x4B);
        if (std::equal_to<float>{}(slope, 0.0f)) {
            const float elevation = machine->get_param(field_4, 0x4A);
            const float difference = elevation - position.y;
            if (std::not_equal_to<float>{}(elevation, 0.0f) && std::fabs(difference) > 0.0f) {
                position.y += difference * 0.8f;
                changed = true;
            }
        } else if (horizontal_distance > EPSILON) {
            float difference = machine->get_param(field_4, 0x4A) - position.y;
            if ((difference > 0.0f && slope > 0.0f) || (difference < 0.0f && slope < 0.0f))
                difference = slope * horizontal_distance;
            if (std::fabs(difference) > 0.0f) {
                position.y += difference;
                changed = true;
            }
        }
    }
    const float normal_z = machine->get_param(field_4, 0x51);
    const float normal_y = machine->get_param(field_4, 0x50);
    const float normal_x = machine->get_param(field_4, 0x4F);
    const vector3d normal{normal_x, normal_y, normal_z};
    if (normal.length2() > EPSILON) {
        const float anchor_z = machine->get_param(field_4, 0x4E);
        machine->get_param(field_4, 0x4D);
        const float anchor_x = machine->get_param(field_4, 0x4C);
        const float margin = std::equal_to<float>{}(machine->get_param(field_4, 0x52), 1.0f) ? 0.7f : 0.35f;
        const float distance = -normal_x * (position.x - (normal_x * margin + anchor_x)) -
                               normal_z * (position.z - (normal_z * margin + anchor_z)) - normal_y * 0.0f;
        if (distance < 0.0f) {
            if (distance > -1.0f) {
                position += normal * distance;
                changed = true;
            }
        } else if (distance > 1.05f) {
            const float correction = distance - 1.05f;
            if (correction < 1.0f) {
                position += normal * correction;
                changed = true;
            }
        }
    }
    const vector3d translation{offset.m[3].x, offset.m[3].y, offset.m[3].z};
    if (pedestrian_translation_blocked(owner, translation)) {
        position -= translation;
        changed = true;
    }
    if (changed)
        entity_set_abs_position(owner, position);
}

namespace {
int __fastcall orientation_type(simple_orientation_ped *, void *)
{
    return 523;
}
bool __fastcall orientation_parent(simple_orientation_ped *, void *, int type)
{
    return type == 522 || type == 490 || type == 573;
}
void __fastcall orientation_post(simple_orientation_ped *self, void *, Float elapsed)
{
    self->post_anim_action(elapsed);
}
void __fastcall orientation_directions(simple_orientation_ped *self, void *, animation_logic_system *system,
                                       state_machine *machine, vector3d &facing, vector3d &up)
{
    self->get_directions(system, machine, facing, up);
}
void __fastcall orientation_changes(simple_orientation_ped *self, void *, Float elapsed)
{
    self->other_po_changes(elapsed);
}
double __fastcall orientation_heading(simple_orientation_ped *, void *)
{
    return 1.0;
}
int __fastcall base_orientation_type(simple_orientation *, void *)
{
    return 522;
}
bool __fastcall base_orientation_parent(simple_orientation *, void *, int type)
{
    return type == 490 || type == 573;
}
void __fastcall base_orientation_post(simple_orientation *self, void *, Float elapsed)
{
    self->post_anim_action(elapsed);
}
void __fastcall base_orientation_directions(simple_orientation *self, void *, animation_logic_system *system,
                                            state_machine *machine, vector3d &facing, vector3d &up)
{
    self->get_directions(system, machine, facing, up);
}
void __fastcall base_orientation_changes(simple_orientation *, void *, Float) {}
double __fastcall base_orientation_heading(simple_orientation *, void *)
{
    return 0.99000001f;
}
int __fastcall relative_orientation_type(relative_orientation *, void *)
{
    return 518;
}
bool __fastcall relative_orientation_parent(relative_orientation *, void *, int type)
{
    return type == 522 || type == 490 || type == 573;
}
void __fastcall relative_orientation_directions(relative_orientation *self, void *, animation_logic_system *system,
                                                state_machine *machine, vector3d &facing, vector3d &up)
{
    facing = machine->find_external_param(static_cast<external_parameter_types>(27))
                 ? machine->get_vector_param(system, 27)
                 : self->the_actor->get_abs_po().get_z_facing();
    up = self->the_actor->get_abs_po().get_y_facing();
}
}  // namespace

void *simple_orientation::native_vtable()
{
    static auto table = [] {
        std::array<void *, 23> result{};
        auto *base = static_cast<void **>(motion_compensator::native_vtable(490));
        std::copy_n(base, 20, result.begin());
        result[0xC / 4] = reinterpret_cast<void *>(base_orientation_type);
        result[0x10 / 4] = reinterpret_cast<void *>(base_orientation_parent);
        result[0x24 / 4] = reinterpret_cast<void *>(base_orientation_post);
        result[0x50 / 4] = reinterpret_cast<void *>(base_orientation_directions);
        result[0x54 / 4] = reinterpret_cast<void *>(base_orientation_changes);
        result[0x58 / 4] = reinterpret_cast<void *>(base_orientation_heading);
        return result;
    }();
    return table.data();
}

void *relative_orientation::native_vtable()
{
    static_assert(sizeof(relative_orientation) == 0x14);
    static auto table = [] {
        std::array<void *, 23> result;
        std::copy_n(static_cast<void **>(simple_orientation::native_vtable()), result.size(), result.begin());
        result[0xC / 4] = reinterpret_cast<void *>(relative_orientation_type);
        result[0x10 / 4] = reinterpret_cast<void *>(relative_orientation_parent);
        result[0x50 / 4] = reinterpret_cast<void *>(relative_orientation_directions);
        return result;
    }();
    return table.data();
}

void *simple_orientation_ped::native_vtable()
{
    static auto table = [] {
        std::array<void *, 23> result{};
        auto *base = static_cast<void **>(motion_compensator::native_vtable(490));
        std::copy_n(base, 20, result.begin());
        result[0xC / 4] = reinterpret_cast<void *>(orientation_type);
        result[0x10 / 4] = reinterpret_cast<void *>(orientation_parent);
        result[0x24 / 4] = reinterpret_cast<void *>(orientation_post);
        result[0x50 / 4] = reinterpret_cast<void *>(orientation_directions);
        result[0x54 / 4] = reinterpret_cast<void *>(orientation_changes);
        result[0x58 / 4] = reinterpret_cast<void *>(orientation_heading);
        return result;
    }();
    return table.data();
}

}  // namespace als

void als_simple_orientation_patch()
{
    FUNC_ADDRESS(address, &als::simple_orientation::post_anim_action);
    set_vfunc(0x008785F4, address);
}
