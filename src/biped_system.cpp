#include "biped_system.h"

#include "func_wrapper.h"
#include "phys_vector3d.h"
#include "actor.h"
#include "capsule.h"
#include "common.h"
#include "conglom.h"
#include "oldmath_po.h"
#include "variable.h"
#include "bone_mass_info.h"
#include "rbc_ragdoll.h"
#include "physics_system.h"
#include "phys_vector3d.h"
#include "time_interface.h"
#include "pendulum.h"
#include "rbc_def_distance.h"
#include "nuge.h"
#include "game.h"
#include "game_settings.h"
#include "wds.h"
#include "moved_entities.h"
#include "local_collision.h"
#include "line_segment.h"
#include "utility.h"
#include "collide.h"
#include "colmesh.h"
#include "subdivision_obb.h"
#include "collide_aux.h"
#include "collision_event.h"
#include "event_manager.h"
#include "rb_capsule_pair.h"
#include "rbc_def_contact.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#include <cassert>

biped_system::biped_system() {}

biped_system_pool *&g_biped_system_pool = []() -> biped_system_pool *& {
    static biped_system_pool native_pool;
    native_pool.slot_array = native_pool.slots;
    native_pool.count = 0;
    for (int i = 0; i < 9; ++i) {
        native_pool.allocated[i] = &native_pool.slots[i];
        native_pool.indices[i] = i;
    }
    auto &pool = var<biped_system_pool *>(0x00923820);
    pool = &native_pool;
    return pool;
}();
VALIDATE_OFFSET(biped_system, restored_position, 0x2120);
VALIDATE_SIZE(biped_system, 0x2140);
VALIDATE_OFFSET(biped_system_pool, allocated, 0x12B40);
VALIDATE_OFFSET(biped_system_pool, slot_array, 0x12B88);


void biped_system_pool::destroy_member(biped_system *biped)
{
    const int slot = static_cast<int>(biped - slot_array);
    const int index = indices[slot];
    if (count == 1) {
        for (int i = 0; i < count; ++i)
            allocated[i]->field_0.destroy_bodies();
        for (int i = 0; i < 9; ++i) {
            indices[i] = i;
            allocated[i] = &slot_array[i];
        }
        count = 0;
    } else {
        biped->field_0.destroy_bodies();
        auto *last = allocated[--count];
        allocated[index] = last;
        allocated[count] = biped;
        indices[slot] = count;
        indices[last - slot_array] = index;
    }
}

void destroy_biped_ragdoll(biped_system *biped)
{
    auto &bones = biped->field_0.field_460;
    po restored = *bones.owner->my_abs_po;
    restored.set_position(biped->restored_position);
    const float radius = biped->collision_diameter * 0.5f;
    capsule shape;
    shape.base = vector3d(0.0f, -radius, 0.0f);
    shape.end = vector3d(0.0f, radius, 0.0f);
    shape.radius = radius;
    bones.owner->save_last_collision_free_state(restored, shape, 0.033333335f);
    biped->field_0.rdbi_calc_bone_mat_from_rb();
    bones.copy_back_bones_recurse();
    bones.update_owner_matrix(biped->restored_position);
    g_biped_system_pool->destroy_member(biped);
}

void biped_system::create_bps(conglomerate *a2, int a3, physical_interface::biped_physics_body_types arg4a)
{
    this->field_2130 = a3;
    this->setup_physics(a2, arg4a);
    this->field_2134 = 0.0;
    this->field_2138 = 0.15000001;
    this->field_0.reset_state_variables();
}


namespace {
vector3d xyz(const vector4d &v) { return vector3d(v.x, v.y, v.z); }
vector4d xyzw(const vector3d &v) { return vector4d(v.x, v.y, v.z, 0.0f); }
const po &body_pose(const rigid_body *body)
{
    return *reinterpret_cast<const po *>(&body->field_0);
}
vector3d body_direction(const rigid_body *body, const vector3d &direction)
{
    if (body->field_144 & 0x10)
        return direction;
    const auto &m = body->field_0;
    return vector3d(m[0][0] * direction.x + m[1][0] * direction.y + m[2][0] * direction.z,
                    m[0][1] * direction.x + m[1][1] * direction.y + m[2][1] * direction.z,
                    m[0][2] * direction.x + m[1][2] * direction.y + m[2][2] * direction.z);
}
vector3d direction(const po &pose, const vector3d &v)
{
    const auto &m = pose.m;
    return vector3d(m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
                    m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
                    m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z);
}
vector3d inverse_direction(const po &pose, const vector3d &v)
{
    const auto &m = pose.m;
    return vector3d(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
}
vector3d normalized(vector3d v)
{
    const float squared_length = v.length2();
    return v * (squared_length <= 0.0f && squared_length >= 0.0f ? 0.0f : 1.0f / std::sqrt(std::fabs(squared_length)));
}
vector3d rotated(const vector3d &v, const vector3d &axis, float angle)
{
    const float length = axis.length();
    if (length < 0.00001f)
        return v;
    const vector3d unit = axis / length;
    const float sine = std::sin(length * angle);
    const float cosine = std::cos(length * angle);
    return v * cosine + vector3d::cross(unit, v) * sine + unit * (dot(unit, v) * (1.0f - cosine));
}
}

VALIDATE_SIZE(bone_mass_info, 0x138);
VALIDATE_SIZE(rigid_body_constraint_ragdoll, 0x140);
VALIDATE_OFFSET(rigid_body_constraint_ragdoll, flags, 0x2C);
VALIDATE_OFFSET(rigid_body_constraint_ragdoll, axis1, 0x80);
VALIDATE_SIZE(rb_collision_sphere, 0x34);
VALIDATE_SIZE(rigid_body_sphere_list, 0x110);
VALIDATE_OFFSET(biped_system, sphere_slots, 0x1580);
VALIDATE_OFFSET(biped_system, contact, 0x2100);

void rigid_body_constraint_ragdoll::reset()
{
    for (int i = 0; i < 20; i += 2)
        caches[i] = -1;
    limit_data = limits;
    limit_count = 0;
    flags = 0;
}

void rigid_body_constraint_ragdoll::set(const vector3d &first, const vector3d &second)
{
    anchor1 = xyzw(first);
    anchor2 = xyzw(second);
}

void rigid_body_constraint_ragdoll::set_damp_k(float value)
{
    damping = value;
    flags = value > 0.0f ? flags | 0x40u : flags & ~0x40u;
}

void rigid_body_constraint_ragdoll::set_hinge(const vector3d &first, const vector3d &second,
    const vector3d &reference1, const vector3d &reference2, float lower, float upper)
{
    axis1 = xyzw(normalized(first));
    axis2 = xyzw(normalized(second));
    reference = xyzw(normalized(reference1));
    flags |= 4;
    vector3d perpendicular(0.0f, axis1.z, -axis1.y);
    if (perpendicular.length() < EPSILON)
        perpendicular = vector3d(-axis1.z, 0.0f, axis1.x);
    if (perpendicular.length() < EPSILON)
        perpendicular = vector3d(axis1.y, -axis1.x, 0.0f);
    tangent = xyzw(normalized(perpendicular));
    bitangent = xyzw(vector3d::cross(xyz(axis1), xyz(tangent)));
    minimum = xyzw(normalized(rotated(reference2, xyz(axis2), lower)));
    maximum = xyzw(normalized(rotated(reference2, xyz(axis2), upper)));
}

void rigid_body_constraint_ragdoll::set_swivel(const vector3d &first, const vector3d &second,
    const vector3d &reference1, const vector3d &reference2, float lower, float upper)
{
    axis1 = xyzw(normalized(first));
    axis2 = xyzw(normalized(second));
    reference = xyzw(normalized(reference1));
    flags |= 8;
    minimum = xyzw(normalized(rotated(reference2, xyz(axis2), lower)));
    maximum = xyzw(normalized(rotated(reference2, xyz(axis2), upper)));
}

void rigid_body_constraint_ragdoll::add_joint_limit(const vector3d &axis, float angle)
{
    assert(limit_count < 2);
    auto &limit = limit_data[limit_count++];
    limit.axis = xyzw(normalized(axis));
    limit.cosine = std::cos(angle);
    limit.sine = std::sin(angle);
}

float rigid_body_constraint_ragdoll::relax()
{
    vector3d first = body_direction(b1, xyz(anchor1));
    vector3d second = body_direction(b2, xyz(anchor2));
    if ((b1->field_144 & 0x10) == 0)
        first += body_pose(b1).get_position();
    if ((b2->field_144 & 0x10) == 0)
        second += body_pose(b2).get_position();
    const auto correction = first - second;
    b2->field_0[3][0] += correction.x;
    b2->field_0[3][1] += correction.y;
    b2->field_0[3][2] += correction.z;
    return correction.length2();
}

void rigid_body_sphere_list::initialize()
{
    sphere_data = spheres;
    sphere_count = 0;
    capsule.m_radius = 0.0f;
    body = nullptr;
}

void rigid_body_sphere_list::calc_bounding_sphere()
{
    bounds = sphere_data[0];
    for (int i = 0; i < sphere_count; ++i) {
        const auto &sphere = sphere_data[i];
        const auto delta = bounds.local_position - sphere.local_position;
        const float distance = delta.length();
        if (distance + bounds.radius <= sphere.radius) {
            bounds.local_position = sphere.local_position;
            bounds.radius = sphere.radius;
        } else if (distance + sphere.radius > bounds.radius) {
            bounds.local_position = (bounds.local_position + sphere.local_position +
                delta * ((bounds.radius - sphere.radius) / distance)) * 0.5f;
            bounds.radius = (distance + bounds.radius + sphere.radius) * 0.5f;
        }
    }
}

void rigid_body_sphere_list::set(rigid_body *rigid, float scale, const vector3d &position)
{
    body = rigid;
    previous_position = position;
    collision_scale = smallest_radius = scale;
    tunnel_count = 0;
    for (int i = 0; i < sphere_count; ++i)
        smallest_radius = std::min(smallest_radius, sphere_data[i].radius);
    calc_bounding_sphere();
}

const char *biped_bone_name(int index)
{
    static const char *names[] = {
        "BIP01 HEAD", "BIP01 SPINE", "BIP01 PELVIS", "BIP01 SPINE1", "BIP01 SPINE2",
        "BIP01 R UPPERARM", "BIP01 R FOREARM", "BIP01 R HAND", "BIP01 R THIGH",
        "BIP01 R CALF", "BIP01 R FOOT", "BIP01 R CLAVICLE", "BIP01 L UPPERARM",
        "BIP01 L FOREARM", "BIP01 L HAND", "BIP01 L THIGH", "BIP01 L CALF",
        "BIP01 L FOOT", "BIP01 L CLAVICLE"};
    return names[index];
}

void bone_mass_info::calc_stuff(conglomerate *owner)
{
    const string_hash start_name{biped_bone_name(start_bone)};
    const string_hash end_name{biped_bone_name(end_bone)};
    auto *first = owner->get_bone(start_name, true);
    auto *second = owner->get_bone(end_name, true);
    const auto &first_pose = first->get_abs_po();
    const auto &second_pose = second->get_abs_po();
    start = first_pose.get_position();
    end = second_pose.get_position() + direction(
        offset_in_end_frame ? second_pose : first_pose, end_offset);
    local_center = first_pose.inverse_xform(
        start * (1.0f - center_fraction) + end * center_fraction);
    joint_world1 = first_pose.get_position();
    joint_world2 = second_pose.get_position() + direction(first_pose, joint_offset);
    const auto axis = (end - start) / (end - start).length();
    start += axis * start_trim;
    end -= axis * end_trim;

    const float sphere_mass = radius * radius * radius * 4.188790321f;
    const float sphere_inertia = sphere_mass * radius * radius * 0.4f;
    const float component = sphere_inertia * (mass / sphere_mass);
    inertia = vector3d(component, component, component);
    bone = owner->get_bone(start_name, true);
    parent = bone->m_parent;
    if (parent) {
        if (bone->field_8 & 0x08000000u)
            bone->compute_rel_po_from_model();
        anchor1 = bone->my_rel_po->get_position();
        anchor2 = ZEROVEC;
    }
}

void setup_bone_mass_info(bone_mass_info (&bones)[10], conglomerate *owner, int type)
{
    for (auto &bone : bones)
        bone = bone_mass_info{};
    const bool venom = type == 2;
    const int starts[10] = {2, 0, 12, 13, 5, 6, 15, 16, 8, 9};
    const int ends[10] = {0, 0, 13, 14, 6, 7, 16, 17, 9, 10};
    const int parents[10] = {-1, 0, 0, 2, 0, 4, 0, 6, 0, 8};
    const float lower[10] = {0, -0.75f, -1.6f, -2.0943952f, 0.2f,
        -2.0943952f, -0.6f, -2.0943952f, -0.6f, -2.0943952f};
    const float upper[10] = {0, 0.75f, -0.2f, -0.087266468f, 1.6f,
        -0.087266468f, 0.6f, -0.087266468f, 0.6f, -0.087266468f};
    const float damping[10] = {0.18f, 0, 0, 0.075f, 0, 0.075f, 0.11f, 0.1f, 0.11f, 0.1f};
    for (int i = 0; i < 10; ++i) {
        auto &b = bones[i];
        b.start_bone = starts[i];
        b.end_bone = ends[i];
        b.body_index = i;
        b.parent_index = parents[i];
        b.center_fraction = 0.5f;
        b.start_trim = i == 0 ? (venom ? 0.35f : 0.2f)
            : i == 1 ? (venom ? 0.25f : 0.15f)
            : i == 7 || i == 9 ? 0.15f
            : i == 6 || i == 8 ? (venom ? 0.2f : 0.15f)
            : venom ? 0.2f : 0.1f;
        b.end_trim = i == 0 ? (venom ? 0.4f : 0.225f)
            : i == 1 || i == 6 || i == 7 || i == 8 || i == 9 ? 0.15f
            : venom ? 0.2f : 0.1f;
        b.sphere_count = i == 1 ? 1 : 2;
        b.mass = i == 0 ? 300.0f : 50.0f;
        b.radius = 0.3f;
        b.collision_scale = i == 1 || i == 7 || i == 9 ? 0.25f
            : i == 3 || i == 5 ? 0.5f : 1.0f;
        b.field_8C = i == 1 ? 50.0f : 0.0f;
        b.joint_type = i == 0 ? 0 : i == 3 || i == 5 || i == 7 || i == 9 ? 1 : 2;
        b.minimum_angle = lower[i];
        b.maximum_angle = upper[i];
        b.joint_damping = damping[i];
        b.joint_offset = vector3d(i == 0 ? -0.15f :
            i == 3 || i == 5 || i == 7 || i == 9 ? -0.05f : 0.0f, 0, 0);
        if (i == 1)
            b.end_offset.x = -0.35f;
        if (i == 7 || i == 9 || (!venom && (i == 3 || i == 5)))
            b.end_offset.x = -0.15f;
        if (venom && i == 0) {
            b.end_offset.z = 0.2f;
            b.offset_in_end_frame = true;
        }
        if (venom && (i == 3 || i == 5)) {
            b.end_offset.x = -0.5f;
            b.offset_in_end_frame = true;
        }
        if (i == 3 || i == 5 || i == 7 || i == 9) {
            b.axis1 = b.axis2 = vector3d(0, 1, 0);
            b.reference1 = b.reference2 = vector3d(1, 0, 0);
        } else if (i != 0) {
            b.axis1 = vector3d(-1, 0, 0);
            b.axis2 = vector3d(i == 6 || i == 8 ? 1.0f : -1.0f, 0, 0);
            b.reference1 = vector3d(0, 1, 0);
            b.reference2 = vector3d(0, i == 6 || i == 8 ? -1.0f : 1.0f, 0);
        }
        if (i == 1 || i == 2 || i == 4) {
            b.limit_axes[0] = vector3d(-1, 0, 0);
            b.limit_angles[0] = i == 1 ? 0.87266463f : 1.5707964f;
            b.limit_count = 1;
            if (i != 1) {
                b.limit_axes[1] = vector3d(0, 0, 1);
                b.limit_angles[1] = 2.1642084f;
                b.limit_count = 2;
            }
        }
        if (i == 6 || i == 8) {
            b.limit_axes[0] = vector3d(1, 0, 0);
            b.limit_angles[0] = 0.87266463f;
            b.limit_axes[1] = vector3d(0, i == 6 ? 1.0f : -1.0f, 0);
            b.limit_angles[1] = 1.6755161f;
            b.limit_count = 2;
        }
        b.calc_stuff(owner);
    }
}

void biped_system::initialize_storage()
{
    field_0.initialize_storage();
    for (int i = 0; i < 10; ++i) {
        spheres[i] = &sphere_slots[i];
        sphere_indices[i] = i;
    }
    sphere_storage = sphere_slots;
    sphere_count = 0;
    for (int i = 0; i < 7; ++i) {
        pairs[i] = &pair_slots[i];
        pair_indices[i] = i;
    }
    pair_storage = pair_slots;
    pair_count = 0;
    field_213C = false;
    field_213D = false;
}

biped_system *biped_system_pool::create_member()
{
    if (count >= 9)
        return nullptr;
    auto *biped = allocated[count++];
    biped->initialize_storage();
    return biped;
}

biped_system *create_biped_ragdoll(conglomerate *owner, int flags,
    physical_interface::biped_physics_body_types type)
{
    auto *biped = g_biped_system_pool->create_member();
    biped->create_bps(owner, flags, type);
    return biped;
}

rigid_body_sphere_list *biped_system::find_spheres(int index)
{
    for (int i = 0; i < sphere_count; ++i)
        if (spheres[i]->body_index == index)
            return spheres[i];
    return nullptr;
}

void biped_system::setup_physics(conglomerate *entity, physical_interface::biped_physics_body_types type)
{
    auto &animation = field_0.field_460;
    animation.owner = owner = entity;
    contact = false;
    animation.attach_physics_bones();
    po absolute[90], relative[90];
    animation.save_poses(absolute, relative);
    animation.prepare_physics_pose();
    const auto velocity = owner->physical_ifc()->get_velocity();
    bone_mass_info bones[10];
    setup_bone_mass_info(bones, owner, static_cast<int>(type));
    field_0.set_max_rb_index(10);
    const auto &previous = var<matrix4x4[10]>(0x00967920);
    const auto &current = var<matrix4x4[10]>(0x009676A0);
    const bool frame_delta = var<bool>(0x00965F32) && var<bool>(0x00965F31) && var<bool>(0x00965F30);
    for (const auto &bone : bones) {
        auto *body = field_0.add_rigid_body(bone.body_index);
        phys_vector3d linear, angular;
        linear.field_0[0] = velocity.x;
        linear.field_0[1] = velocity.y;
        linear.field_0[2] = velocity.z;
        angular.field_0[0] = angular.field_0[1] = angular.field_0[2] = 0.0f;
        phys_vector3d center;
        center.field_0[0] = bone.local_center.x;
        center.field_0[1] = bone.local_center.y;
        center.field_0[2] = bone.local_center.z;
        if (frame_delta)
            nuge::calc_velocities(previous[bone.body_index], current[bone.body_index], center,
                var<float>(0x00965F2C), linear, angular);
        po pose;
        pose.set_position(bone.bone->get_abs_po().slow_xform(bone.local_center));
        phys_vector3d inertia;
        inertia.field_0[0] = bone.inertia.x;
        inertia.field_0[1] = bone.inertia.y;
        inertia.field_0[2] = bone.inertia.z;
        body->set(bone.mass, inertia, pose.m, linear, angular, bone.collision_scale, 0);
        body->field_144 |= 2u;
        assert(field_0.bone_binding_count < 10);
        auto *binding = field_0.bone_bindings[field_0.bone_binding_count++];
        po::full_inv_multiply(*reinterpret_cast<po *>(binding->transform), pose,
            bone.bone->get_abs_po());
        binding->bone = bone.bone;
        binding->rigid_body_index = bone.body_index;
    }
    for (const auto &bone : bones) {
        if (bone.joint_type == 0)
            continue;
        auto *joint = field_0.add_joint(bone.parent_index, bone.body_index);
        const auto &first_pose = bone.parent->get_abs_po();
        const auto &second_pose = bone.bone->get_abs_po();
        const auto &first_body = body_pose(field_0.m_list_rigid_body.m_data[bone.parent_index]);
        const auto &second_body = body_pose(field_0.m_list_rigid_body.m_data[bone.body_index]);
        joint->set(first_body.inverse_xform(first_pose.slow_xform(bone.anchor1)),
            second_body.inverse_xform(second_pose.slow_xform(bone.anchor2)));
        if (bone.joint_damping > LARGE_EPSILON)
            joint->set_damp_k(bone.joint_damping);
        const auto first_axis = inverse_direction(first_body, direction(first_pose, bone.axis1));
        const auto second_axis = inverse_direction(second_body, direction(second_pose, bone.axis2));
        const auto first_reference = inverse_direction(first_body, direction(first_pose, bone.reference1));
        const auto second_reference = inverse_direction(second_body, direction(second_pose, bone.reference2));
        if (bone.joint_type == 1)
            joint->set_hinge(first_axis, second_axis, first_reference, second_reference,
                bone.minimum_angle, bone.maximum_angle);
        else
            joint->set_swivel(first_axis, second_axis, first_reference, second_reference,
                bone.minimum_angle, bone.maximum_angle);
        for (int i = 0; i < bone.limit_count; ++i)
            joint->add_joint_limit(inverse_direction(first_body, direction(first_pose, bone.limit_axes[i])),
                bone.limit_angles[i]);
    }
    for (const auto &bone : bones) {
        assert(sphere_count < 10);
        auto *list = spheres[sphere_count++];
        list->initialize();
        list->body_index = bone.body_index;
        auto *body = field_0.m_list_rigid_body.m_data[bone.body_index];
        const auto &pose = body_pose(body);
        list->sphere_count = bone.sphere_count <= 1 ? 1 : 2;
        for (int i = 0; i < list->sphere_count; ++i) {
            auto &sphere = list->sphere_data[i];
            sphere.contact = false;
            sphere.collision_entity = entity_base_vhandle{0};
            sphere.radius = i == 0 ? bone.start_trim : bone.end_trim;
            sphere.local_position = list->sphere_count == 1 ? ZEROVEC
                : pose.inverse_xform(i == 0 ? bone.start : bone.end);
        }
        list->set(body, 0.3f, pose.get_position());
        list->capsule.field_0 = pose.inverse_xform(bone.joint_world1);
        list->capsule.field_C = pose.inverse_xform(bone.joint_world2);
        list->capsule.m_radius = bone.joint_damping;
    }
    static const int pair_ids[7][2] = {{7,9},{7,8},{9,6},{3,0},{5,0},{3,2},{5,4}};
    for (const auto &ids : pair_ids) {
        auto *pair = pairs[pair_count++];
        pair->first = find_spheres(ids[0]);
        pair->second = find_spheres(ids[1]);
    }
    animation.restore_poses(absolute, relative);
    field_0.rdbi_calc_rb_mat_from_bone();
    for (int i = 0; i < 10; ++i)
        if (auto *list = find_spheres(i))
            list->previous_position = body_pose(field_0.m_list_rigid_body.m_data[i]).get_position();
    field_0.relax_joints();
    restored_position = animation.bones[0].bone->get_abs_position();
    collision_diameter = 0.1f;
}

void biped_system::prolog_frame_advance(Float)
{
    field_0.update_ballistic_target();
    for (int i = 0; i < 10; ++i)
        field_0.m_list_rigid_body.m_data[i]->field_134 = 3.0f;
    const float scale = owner->has_time_ifc()
        ? static_cast<float>(owner->time_ifc()->sub_4ADE50()) : g_world_ptr->time_manager.field_0;
    field_0.set_time_scale(scale);
    auto *physics = owner->physical_ifc();
    if (!_strcmpi(g_game_ptr->gamefile->field_340.m_hero_name.to_string(), "PLR_GUNGUY"))
        physics->field_9C = 0.1f;
    bool attached = false;
    for (auto *pendular : physics->field_110) {
        if (!pendular)
            continue;
        if (pendular->biped_physics_constraint) {
            auto *anchor = pendular->get_volatile_ptr();
            const bool hero = anchor && (anchor->is_hero() ||
                ((anchor->field_4 & 0x8000u) && anchor->get_conglom_owner()->is_hero()));
            pendular->biped_physics_constraint->field_3C =
                anchor && !hero ? var<float>(0x00922BA4) : 0.0f;
        }
        pendular->update_biped_constraint(physics);
        if (pendular->pivot_rigid_body)
            pendular->pivot_rigid_body->field_13C = scale;
        attached = true;
    }
    for (int i = 0; i < 10; ++i)
        if (auto *joint = field_0.constraints.m_data[i])
            joint->flags = attached ? joint->flags | 0x80u : joint->flags & ~0x80u;
}

namespace {
bool __fastcall root_entity_filter(const local_collision::entfilter_base *, void *, actor *actor,
    dynamic_conglomerate_clone *, const local_collision::query_args_t *args)
{
    if (actor->colgeom->get_type() == collision_geometry::CAPSULE || !actor->has_entity_collision())
        return false;
    if (actor->has_physical_ifc() && actor->physical_ifc()->field_174 != nullptr)
        return false;
    if (actor->field_4 & 4u) {
        const float radius = actor->get_colgeom_radius() + args->field_28;
        return (actor->get_colgeom_center() - args->field_10).length2() < radius * radius;
    }
    if (actor->colgeom->get_type() != collision_geometry::MESH)
        return false;
    const auto &obb = static_cast<cg_mesh *>(actor->colgeom)->data->field_10[0];
    const auto point = actor->get_abs_po().inverse_xform(args->field_10) - obb.field_0;
    const vector3d axes[3] = {
        vector3d(obb.field_10.x, obb.axis_y.x, obb.axis_z.x),
        vector3d(obb.field_10.y, obb.axis_y.y, obb.axis_z.y),
        vector3d(obb.field_10.z, obb.axis_y.z, obb.axis_z.z)};
    float distance = 0.0f;
    for (const auto &axis : axes) {
        const float extent = axis.length();
        const float excess = std::max(std::fabs(dot(axis, point) / extent) - extent, 0.0f);
        distance += excess * excess;
    }
    return distance <= args->field_28 * args->field_28;
}

const local_collision::entfilter_base::native_vtable root_filter_table{root_entity_filter};
const local_collision::entfilter_base root_filter{reinterpret_cast<std::intptr_t>(&root_filter_table)};

vector3d restore_root_position(const vector3d &previous, float radius, const vector3d &target)
{
    const vector3d delta = target - previous;
    const float length = delta.length();
    local_collision::query_args_t args{};
    auto *primitives = local_collision::query_sphere((previous + target) * 0.5f,
        length * 0.5f + radius + 0.1f, root_filter, *local_collision::obbfilter_sphere_test, args);
    vector3d center = target;
    if (length > LARGE_EPSILON) {
        const auto extended = target + delta * (radius * 0.25f / length);
        line_segment_t line(previous, extended);
        line.ent = nullptr;
        line.field_34 = false;
        if (local_collision::get_closest_line_intersection(primitives, &line, false, nullptr, nullptr, nullptr)) {
            const auto hit_delta = line.field_18 - previous;
            if (hit_delta.length2() >= 1.0000001e-6f && dot(line.field_24, hit_delta) < 0.0f)
                center += line.field_18 - extended;
        }
    }
    for (auto *entry = primitives; entry; entry = entry->field_0) {
        vector3d point, normal;
        bool collision;
        if (entry->is_ent) {
            auto *entity = entry->field_4.ent;
            po absolute = entity->get_abs_po();
            collision = collide_sphere_entity(center, radius, entity, &point, &normal, &absolute);
            if (collision) {
                normal = center - point;
                if (normal.length2() > 9.9999994e-11f)
                    normal *= 1.0f / normal.length();
            }
        } else {
            collision = entry->field_4.obb->sphere_intersection(center, radius, &point, &normal, nullptr);
        }
        if (collision)
            center = point + normal * radius;
    }
    local_collision::destroy_primitive_list(&primitives);
    return center;
}
}

void biped_system::epilog_frame_advance(Float elapsed)
{
    field_0.update_stability(elapsed);
    field_0.rdbi_calc_bone_mat_from_rb();
    auto &animation = field_0.field_460;
    if (field_2134 >= field_2138) {
        animation.copy_back_bones_recurse();
    } else {
        field_2134 += elapsed;
        animation.copy_back_tween_recurse(std::min(field_2134 / field_2138, 1.0f));
    }
    restored_position = restore_root_position(restored_position, collision_diameter,
        animation.bones[0].bone->get_abs_position());
    animation.update_owner_matrix(restored_position);
}

void biped_system::process_biped_physics(Float elapsed)
{
    struct saved_owner {
        physical_interface *physics;
        vector3d position;
    };
    saved_owner saved[25];
    int saved_count = 0;
    auto &inside = var<bool>(0x0095A6AB);
    inside = true;
    for (auto *physics : *physical_interface::all_phys_interfaces) {
        if ((physics->field_C & 0x80000u) && saved_count < 25)
            saved[saved_count++] = {physics, physics->field_4->get_abs_position()};
    }
    for (int i = 0; i < g_biped_system_pool->count; ++i)
        g_biped_system_pool->allocated[i]->prolog_frame_advance(elapsed);
    phys_sys::phys_frame_advance(elapsed);
    for (int i = 0; i < g_biped_system_pool->count; ++i)
        g_biped_system_pool->allocated[i]->epilog_frame_advance(elapsed);
    for (int i = 0; i < saved_count; ++i) {
        auto *owner = saved[i].physics->field_4;
        owner->set_frame_delta_trans(owner->get_abs_position() - saved[i].position, elapsed);
        moved_entities::add_moved(vhandle_type<entity>{owner->get_my_vhandle()});
    }
    inside = false;
}

namespace {

bool __fastcall ragdoll_accept(const local_collision::entfilter_base *, void *, actor *act,
    dynamic_conglomerate_clone *, const local_collision::query_args_t *args)
{
    if (act->colgeom->get_type() == collision_geometry::CAPSULE || !act->has_entity_collision())
        return false;
    if (act->has_physical_ifc() && act->physical_ifc()->field_174 != nullptr)
        return false;
    if (act->field_4 & 4) {
        const float radius = act->get_colgeom_radius() + args->field_28;
        return radius * radius > (act->get_colgeom_center() - args->field_10).length2();
    }
    if (act->colgeom->get_type() != collision_geometry::MESH)
        return false;
    const auto &box = static_cast<cg_mesh *>(act->colgeom)->data->field_10[0];
    const vector3d local = act->get_abs_po().inverse_xform(args->field_10) - box.field_0;
    const vector3d axes[3]{box.field_10, box.axis_y, box.axis_z};
    float distance_squared = 0.0f;
    for (const auto &axis : axes) {
        const float length = axis.length();
        const float outside = std::max(std::abs(dot(local, axis) / length) - length, 0.0f);
        distance_squared += outside * outside;
    }
    return distance_squared <= args->field_28 * args->field_28;
}

void update_collision_spheres(rigid_body_sphere_list &list)
{
    const po &pose = *reinterpret_cast<const po *>(&list.body->field_40);
    list.bounds.position = list.bounds.field_C = pose.slow_xform(list.bounds.local_position);
    list.bounds.contact = false;
    list.capsule.field_1C = pose.slow_xform(list.capsule.field_0);
    list.capsule.field_28 = pose.slow_xform(list.capsule.field_C);
    for (int i = 0; i < list.sphere_count; ++i) {
        auto &sphere = list.sphere_data[i];
        sphere.position = sphere.field_C = pose.slow_xform(sphere.local_position);
        sphere.contact = false;
    }
}



bool push_collision_sphere(local_collision::primitive_list_t *primitives,
    const vector3d &center, float radius, vector3d &result,
    entity_base_vhandle &collision_entity, subdivision_node *&collision_node)
{
    result = center;
    bool collided = false;
    for (auto *primitive = primitives; primitive; primitive = primitive->field_0) {
        vector3d point, normal;
        bool hit;
        if (primitive->is_entity()) {
            auto *ent = primitive->get_entity();
            po pose = ent->get_abs_po();
            hit = collide_sphere_entity(result, Float{radius}, ent, &point, &normal, &pose);
            if (hit) {
                normal = result - point;
                const float squared = normal.length2();
                if (squared > 1.0e-10f)
                    normal *= 1.0f / std::sqrt(squared);
            }
        } else {
            hit = primitive->field_4.obb->sphere_intersection(result, Float{radius}, &point, &normal, nullptr);
        }
        if (hit) {
            collided = true;
            result = point + normal * radius;
            if (primitive->is_entity())
                collision_entity = primitive->get_entity()->my_handle;
            else
                collision_node = primitive->field_4.obb;
        }
    }
    return collided;
}

void tunnel_collision_spheres(rigid_body_sphere_list &list, local_collision::primitive_list_t *primitives)
{
    auto &predicted = *reinterpret_cast<po *>(&list.body->field_40);
    const vector3d travel = predicted.get_position() - list.previous_position;
    const float travel_squared = travel.length2();
    const vector3d physical_delta = predicted.get_position() -
        reinterpret_cast<const po *>(&list.body->field_0)->get_position();
    if (4.0f * physical_delta.length2() >= travel_squared)
        list.tunnel_count = 0;
    else
        ++list.tunnel_count;
    list.field_EC = ZEROVEC;
    if (list.tunnel_count < 120 && travel_squared > list.smallest_radius * list.smallest_radius) {
        line_segment_t segment{};
        segment.ent = nullptr;
        segment.field_34 = false;
        segment.field_0 = list.previous_position;
        segment.field_C = predicted.get_position() +
            travel * (0.75f * list.smallest_radius / std::sqrt(travel_squared));
        if (local_collision::get_closest_line_intersection(primitives, &segment, false, nullptr, nullptr, nullptr)) {
            const vector3d hit_travel = segment.field_18 - list.previous_position;
            if (hit_travel.length2() >= 1.0000001111620804e-6f &&
                dot(hit_travel, segment.field_24) < 0.0f)
                list.field_EC = segment.field_18 - segment.field_C;
        }
    }
    predicted.set_position(predicted.get_position() + list.field_EC);
    list.previous_position = predicted.get_position();
}

void create_sphere_contacts(biped_system &biped, rigid_body_sphere_list &list)
{
    const float bounce = biped.owner && biped.owner->has_physical_ifc()
        ? biped.owner->physical_ifc()->field_9C : 0.5f;
    auto *environment = static_cast<rigid_body *>(phys_sys::get_environment_rigid_body());
    rigid_body_constraint_contact *constraint = nullptr;
    bool raised_event = false;
    for (int i = 0; i < list.sphere_count; ++i) {
        const auto &sphere = list.sphere_data[i];
        if (!sphere.contact)
            continue;
        const vector3d displacement = sphere.field_C - sphere.position;
        const float distance = displacement.length();
        if (distance <= 0.00001f)
            continue;
        const vector3d normal = displacement * (-1.0f / distance);
        const vector3d surface_offset = normal * sphere.radius;
        biped.contact = true;
        biped.contact_position = sphere.position;
        biped.contact_normal = -normal;
        if (!constraint)
            constraint = phys_sys::create_no_error_rbc_contact(list.body, environment);
        if (constraint) {
            const auto &predicted = *reinterpret_cast<const po *>(&list.body->field_40);
            const auto local = predicted.inverse_xform(sphere.position + surface_offset);
            const auto world = sphere.field_C + surface_offset;
            constraint->add_point(list.body, environment,
                *reinterpret_cast<const phys_vector3d *>(&local),
                *reinterpret_cast<const phys_vector3d *>(&world),
                *reinterpret_cast<const phys_vector3d *>(&normal),
                Float{list.body->field_148}, Float{bounce}, Float{100.0f}, true);
        }
        if (raised_event)
            continue;
        if (auto *other = sphere.collision_entity.get_volatile_ptr()) {
            collision_event own_event{other->my_handle, nullptr, sphere.field_C, normal};
            event_manager::raise_event(&own_event, biped.owner->my_handle);
            collision_event other_event{biped.owner->my_handle, nullptr, sphere.field_C, -normal};
            event_manager::raise_event(&other_event, other->my_handle);
            raised_event = true;
        } else if (sphere.collision_node) {
            collision_event own_event{INVALID_HANDLE, sphere.collision_node, sphere.field_C, normal};
            event_manager::raise_event(&own_event, biped.owner->my_handle);
            raised_event = true;
        }
    }
}
}

void biped_system::gather_collisions()
{

    vector3d center = reinterpret_cast<const po *>(&spheres[0]->body->field_40)->get_position();
    float radius = spheres[0]->bounds.radius;
    for (int i = 0; i < sphere_count; ++i) {
        const auto &list = *spheres[i];
        const auto position = reinterpret_cast<const po *>(&list.body->field_40)->get_position();
        merge_spheres(center, Float{radius}, position, Float{list.bounds.radius}, center, radius);
        merge_spheres(center, Float{radius}, list.previous_position, Float{list.bounds.radius}, center, radius);
    }
    radius = std::min(radius + 1.0f, 99.9000015258789f);
    static const local_collision::entfilter_base::native_vtable filter_table{ragdoll_accept};
    const local_collision::entfilter_base filter{reinterpret_cast<std::intptr_t>(&filter_table)};
    local_collision::query_args_t args{};
    auto *primitives = local_collision::query_sphere(center, Float{radius}, filter,
        *local_collision::obbfilter_sphere_test, args);
    for (int i = 0; i < sphere_count; ++i)
        tunnel_collision_spheres(*spheres[i], primitives);
    for (int i = 0; i < sphere_count; ++i)
        update_collision_spheres(*spheres[i]);
    for (int i = 0; i < pair_count; ++i) {
        rb_capsule_pair pair{pairs[i]->first, pairs[i]->second};
        pair.do_test();
    }
    for (int i = 0; i < sphere_count; ++i) {
        auto &list = *spheres[i];
        for (int j = 0; j < list.sphere_count; ++j) {
            auto &sphere = list.sphere_data[j];
            vector3d adjusted;
            entity_base_vhandle entity_handle = INVALID_HANDLE;
            subdivision_node *node = nullptr;
            if (push_collision_sphere(primitives, sphere.field_C, sphere.radius, adjusted, entity_handle, node)) {
                sphere.field_C = adjusted;
                sphere.contact = true;
                sphere.collision_entity = entity_handle;
                sphere.collision_node = node;
            }
        }
    }
    contact = false;
    for (int i = 0; i < sphere_count; ++i)
        create_sphere_contacts(*this, *spheres[i]);
    for (int i = 0; i < sphere_count; ++i) {
        auto &list = *spheres[i];
        auto &predicted = *reinterpret_cast<po *>(&list.body->field_40);
        predicted.set_position(predicted.get_position() - list.field_EC);
    }
    local_collision::destroy_primitive_list(&primitives);
}

void biped_system::collision_callback()
{
    for (int i = 0; i < g_biped_system_pool->count; ++i)
        g_biped_system_pool->allocated[i]->gather_collisions();
}
