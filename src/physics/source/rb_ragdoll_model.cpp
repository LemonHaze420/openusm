#include "rb_ragdoll_model.h"

#include "func_wrapper.h"
#include "nuge.h"
#include "phys_vector3d.h"
#include "rigid_body.h"
#include "variable.h"
#include "common.h"
#include "oldmath_po.h"
#include "rbc_ragdoll.h"
#include <algorithm>
#include <cmath>

#include <physics_system.h>

#include <cassert>

static Var<ragdoll_callbacks> g_ragdoll_callbacks{0x00984558};

rb_ragdoll_model::rb_ragdoll_model() {}

VALIDATE_OFFSET(rb_ragdoll_model, user_bodies, 0x90);
VALIDATE_OFFSET(rb_ragdoll_model, bone_bindings, 0x3C0);
VALIDATE_OFFSET(rb_ragdoll_model, bone_binding_count, 0x414);
VALIDATE_SIZE(rb_ragdoll_model, 0x1564);

void rb_ragdoll_model::rdbi_calc_bone_mat_from_rb()
{
    using callback = void(__cdecl *)(void *, rigid_body *, const po *);
    const auto calculate = reinterpret_cast<callback>(g_ragdoll_callbacks().m_calc_bone_mat_from_rb);
    for (int i = 0; i < bone_binding_count; ++i) {
        const auto *binding = bone_bindings[i];
        auto *body = m_list_rigid_body.m_data[binding->rigid_body_index];
        if (body != nullptr)
            calculate(binding->bone, body, reinterpret_cast<const po *>(binding->transform));
    }
}

void rb_ragdoll_model::destroy_bodies()
{
    for (int i = 0; i < user_bodies.m_alloc_count; ++i)
        if (user_bodies.m_data[i] != nullptr)
            phys_sys::destroy(user_bodies.m_data[i]);
    user_bodies.m_alloc_count = 0;
    field_C0.m_alloc_count = 0;
    for (int i = 0; i < 10; ++i)
        if (m_list_rigid_body.m_data[i] != nullptr)
            phys_sys::destroy(m_list_rigid_body.m_data[i]);
}

rigid_body *rb_ragdoll_model::add_rigid_body(int rb_id)
{
    assert(m_list_rigid_body[rb_id] == nullptr);

    auto *result = phys_sys::create_rigid_body();
    this->m_list_rigid_body[rb_id] = result;
    return result;
}

void rb_ragdoll_model::reset_state_variables()
{
    this->field_420 = 0;
    this->field_428 = 0;
    this->field_424 = 0.0;
    this->field_430 = 1000.0;
    this->field_434 = 0.0;
    this->field_44C = 0;
}

void rb_ragdoll_model::sub_4ADEF0(int i, const phys_vector3d &a3)
{
    assert(i >= 0 && i < this->m_list_rigid_body.m_alloc_count);

    auto v3 = 1.0f / this->m_list_rigid_body.m_data[i]->field_130;

    phys_vector3d a3a = v3 * a3;

    this->apply_pulse(i, a3a);
}

void rb_ragdoll_model::apply_pulse(int i, const phys_vector3d &a3)
{
    assert(i >= 0 && i < this->m_list_rigid_body.m_alloc_count);

    if constexpr (1) {
        auto v3 = &this->m_list_rigid_body.m_data[i];
        if (*v3 != nullptr) {
            this->field_420 = 0;
            this->field_428 = 0;
            this->field_424 = 0;
            this->field_430 = 1000.0;
            this->field_434 = 0.0;
            (*v3)->sub_5B2D50(a3);
        }

    } else {
        THISCALL(0x007A09A0, this, i, &a3);
    }
}

void rb_ragdoll_model::get_ballistic_info(phys_vector3d *a2, phys_vector3d *a3, float *a4)
{
    if constexpr (1) {
        math::VecClass<3, 1> v9;
        math::VecClass<3, 0> v11;

        nuge::get_ballistic_info(this->m_list_rigid_body.m_data, this->m_list_rigid_body.m_alloc_count, &v9, &v11, a4);
        a2->field_0[0] = v9[0];
        a2->field_0[1] = v9[1];
        a2->field_0[2] = v9[2];

        a3->field_0[0] = v11[0];
        a3->field_0[1] = v11[1];
        a3->field_0[2] = v11[2];

    } else {
        THISCALL(0x007A0300, this, a2, a3, a4);
    }
}

void rb_ragdoll_model::update_stability(Float a2)
{
    if constexpr (1) {
        auto begin = this->m_list_rigid_body.m_data;
        int rbodies_count = 0;
        auto v4 = begin;
        auto end = &begin[this->m_list_rigid_body.m_alloc_count];

        if (end != begin) {
            do {
                if (*v4 != nullptr) {
                    ++rbodies_count;
                }

                ++v4;
            } while (v4 != end);
        }

        assert(rbodies_count > 0);

        auto v6 = this->m_list_rigid_body.m_data;
        this->field_430 = 0.0;
        this->field_434 = 0.0;
        if (end != begin) {
            do {
                auto *v7 = *v6;
                if (v7 != nullptr) {
                    this->field_430 += v7->field_168;

                    float v8 = (v7->field_15C <= 0 ? 0.0f : 1.0f);

                    this->field_434 += v8;
                }

                ++v6;
            } while (v6 != end);
        }

        auto v10 = begin;
        auto v11 = 1.f;
        this->field_42C = 0;
        auto v12 = v11 / rbodies_count;
        this->field_430 = v12 * this->field_430;
        for (this->field_434 = v12 * this->field_434; v10 != end; ++v10) {
            if (*v10 != nullptr && ((*v10)->field_144 & 4) != 0) {
                ++this->field_42C;
            }
        }

        this->field_438 = this->field_42C * v12;
        this->field_428 = this->field_42C == rbodies_count;
        if (this->field_42C >= rbodies_count * 0.80000001f) {
            if (!this->field_420) {
                this->field_424 += a2;
                if (this->field_424 >= 0.25f || this->field_428) {
                    this->field_420 = true;
                }
            }

        } else {
            this->field_424 = 0.0;
            this->field_420 = false;
        }

    } else {
        THISCALL(0x007A0810, this, a2);
    }
}

void rb_ragdoll_model::set_ragdoll_callbacks(const ragdoll_callbacks &a1)
{
    g_ragdoll_callbacks().m_calc_bone_mat_from_rb = a1.m_calc_bone_mat_from_rb;
    g_ragdoll_callbacks().m_calc_rb_mat_from_bone = a1.m_calc_rb_mat_from_bone;
}

void rb_ragdoll_model::set_max_rb_index(int count)
{
    for (int i = 0; i < count; ++i) {
        assert(m_list_rigid_body.m_alloc_count < 10);
        m_list_rigid_body.m_data[m_list_rigid_body.m_alloc_count++] = nullptr;
        assert(constraints.m_alloc_count < 10);
        constraints.m_data[constraints.m_alloc_count++] = nullptr;
        assert(contact_constraints.m_alloc_count < 10);
        contact_constraints.m_data[contact_constraints.m_alloc_count++] = nullptr;
    }
}

void rb_ragdoll_model::initialize_storage()
{
    m_list_rigid_body.m_data = reinterpret_cast<rigid_body **>(&m_list_rigid_body);
    m_list_rigid_body.m_alloc_count = 0;
    constraints.m_data = reinterpret_cast<rigid_body_constraint_ragdoll **>(&constraints);
    constraints.m_alloc_count = 0;
    contact_constraints.m_data = reinterpret_cast<void **>(&contact_constraints);
    contact_constraints.m_alloc_count = 0;
    user_bodies.m_data = reinterpret_cast<user_rigid_body **>(&user_bodies);
    user_bodies.m_alloc_count = 0;
    field_C0.m_data = reinterpret_cast<void **>(&field_C0);
    field_C0.m_alloc_count = 0;
    for (int i = 0; i < 10; ++i) {
        bone_bindings[i] = reinterpret_cast<ragdoll_bone_binding *>(field_F0 + 72 * i);
        reinterpret_cast<int *>(bone_bindings)[10 + i] = i;
    }
    bone_bindings[20] = reinterpret_cast<ragdoll_bone_binding *>(field_F0);
    bone_binding_count = 0;
    field_450 = 0;
    field_460.bones = reinterpret_cast<phys_anim_bone_entry *>(field_460.field_0);
    field_460.bone_count = 0;
    field_460.saved_poses = reinterpret_cast<phys_anim_saved_pose *>(field_460.field_5A8 + 8);
    field_460.field_10F4 = 0;
}

rigid_body_constraint_ragdoll *rb_ragdoll_model::add_joint(int first, int second)
{
    auto &pool = g_physics_system->field_254;
    assert(pool.m_alloc_count < pool.m_slot_array_size);
    auto *joint = reinterpret_cast<rigid_body_constraint_ragdoll *>(pool.m_alloc_list[pool.m_alloc_count++]);
    joint->reset();
    joint->b1 = m_list_rigid_body.m_data[first];
    joint->b2 = m_list_rigid_body.m_data[second];
    constraints.m_data[second] = joint;
    return joint;
}

void rb_ragdoll_model::rdbi_calc_rb_mat_from_bone()
{
    using callback = void(__cdecl *)(void *, rigid_body *, const po *);
    const auto calculate = reinterpret_cast<callback>(g_ragdoll_callbacks().m_calc_rb_mat_from_bone);
    for (int i = 0; i < bone_binding_count; ++i) {
        const auto *binding = bone_bindings[i];
        auto *body = m_list_rigid_body.m_data[binding->rigid_body_index];
        if (body)
            calculate(binding->bone, body, reinterpret_cast<const po *>(binding->transform));
    }
}

void rb_ragdoll_model::relax_joints()
{
    float error;
    do {
        error = 0.0f;
        for (int i = 0; i < constraints.m_alloc_count; ++i)
            if (auto *joint = constraints.m_data[i])
                error = std::max(error, joint->relax());
    } while (error > 0.00001f);
}

void rb_ragdoll_model::set_time_scale(float value)
{
    for (int i = 0; i < m_list_rigid_body.m_alloc_count; ++i)
        if (auto *body = m_list_rigid_body.m_data[i])
            body->field_13C = value;
}

void rb_ragdoll_model::update_ballistic_target()
{
    if (field_44C == 0)
        return;
    --field_44C;
    phys_vector3d center, momentum;
    float mass;
    get_ballistic_info(&center, &momentum, &mass);
    const vector3d velocity(momentum[0] / mass, momentum[1] / mass, momentum[2] / mass);
    const auto *target = reinterpret_cast<const float *>(&field_43C);
    const vector3d delta(target[0] - center[0], target[1] - center[1], target[2] - center[2]);
    if (delta.length2() < 2.25f)
        field_44C = 0;
    const float horizontal = std::sqrt(delta.x * delta.x + delta.z * delta.z);
    if (horizontal < 0.001f)
        return;
    const vector3d axis(delta.x / horizontal, 0.0f, delta.z / horizontal);
    const float gravity_distance = -4.9f * horizontal;
    const float slope = delta.y / horizontal;
    const float along = dot(axis, velocity);
    float speed = 1.0f;
    for (int i = 0; i <= 10; ++i) {
        const float vertical = slope * speed - gravity_distance / speed;
        const float derivative = gravity_distance / (speed * speed) + slope;
        const float difference = vertical - velocity.y;
        const float next = speed - (difference * derivative + speed - along) /
                                       (difference * (-2.0f * gravity_distance / (speed * speed * speed)) +
                                        derivative * derivative + 1.0f);
        const bool converged = std::fabs(next - speed) < 0.001f;
        speed = next;
        if (converged)
            break;
    }
    if (std::fabs(speed) < 1.0f && std::fabs(speed) > 0.001f)
        speed = speed < 0.0f ? -1.0f : 1.0f;
    const vector3d desired = axis * speed + vector3d(0.0f, slope * speed - gravity_distance / speed, 0.0f);
    float squared_masses = 0.0f;
    for (int i = 0; i < m_list_rigid_body.m_alloc_count; ++i)
        if (auto *body = m_list_rigid_body.m_data[i])
            squared_masses += 1.0f / (body->field_130 * body->field_130);
    const vector3d impulse = (desired - velocity) * (mass / squared_masses);
    for (int i = 0; i < m_list_rigid_body.m_alloc_count; ++i) {
        auto *body = m_list_rigid_body.m_data[i];
        if (!body)
            continue;
        const vector3d weighted = impulse / (body->field_130 * body->field_130);
        phys_vector3d pulse;
        pulse[0] = weighted.x;
        pulse[1] = weighted.y;
        pulse[2] = weighted.z;
        body->sub_5B2D50(pulse);
    }
}
