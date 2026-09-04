#include "rb_ragdoll_model.h"

#include "func_wrapper.h"
#include "nuge.h"
#include "phys_vector3d.h"
#include "rigid_body.h"
#include "variable.h"

#include <physics_system.h>

#include <cassert>

static Var<ragdoll_callbacks> g_ragdoll_callbacks{0x00984558};

rb_ragdoll_model::rb_ragdoll_model() {}

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

        this->field_438 = this->field_42C / rbodies_count;
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

void rb_ragdoll_model::set_max_rb_index(int a2)
{
    THISCALL(0x007A0460, this, a2);
}
