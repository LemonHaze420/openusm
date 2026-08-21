#include "pendulum.h"

#include "biped_system.h"
#include "common.h"
#include "entity_base.h"
#include "entity_base_vhandle.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "physics_system.h"
#include "phys_vector3d.h"
#include "rbc_def_distance.h"
#include "vector3d.h"

#include <cassert>

VALIDATE_SIZE(pendulum, 0x38);

pendulum::pendulum()
{
    this->field_0 = 0;
    this->field_4 = vector3d{0, 0, 0};
    this->field_10 = vector3d{0, 0, 0};
    this->m_constraint = 0.0;
    this->field_20 = 0.0;
    this->biped_physics_constraint = nullptr;
    this->pivot_rigid_body = nullptr;
    this->field_2C = 0;
    this->field_30 = 0;
    this->m_active = false;
}

bool pendulum::has_a_moving_anchor() const
{
    auto *ent = this->get_volatile_ptr();
    if (ent == nullptr) {
        return false;
    }

    while (ent != nullptr) {
        if (ent->is_flagged_in_the_moved_list()) {
            return true;
        }

        if (ent->is_conglom_member()) {
            ent = ent->get_conglom_owner();
        } else {
            if (ent->m_parent != nullptr) {
                ent = ent->get_parent();
            } else {
                ent = nullptr;
            }
        }
    }

    return false;
}

void pendulum::set_constraint(Float constraint)
{
    assert(constraint > 0.001f);

    this->m_constraint = constraint;
}

void pendulum::set_constraint_lenience(Float a2)
{
    this->field_20 = a2;
}

namespace biped_bone_array {
static constexpr auto num_rb_phys_bones = 10;
}

void pendulum::set_attach_limb(int l)
{
    assert(l >= 0 && l < biped_bone_array::num_rb_phys_bones);

    if (l >= 0 && l < biped_bone_array::num_rb_phys_bones) {
        this->field_30 = l;
    }
}

const vector3d &pendulum::get_pivot_abs_pos()
{
    auto *ent = this->get_volatile_ptr();
    if (ent != nullptr) {
        auto &local_po = ent->get_abs_po();

        this->field_10 = local_po.slow_xform(this->field_4);
    }

    return this->field_10;
}

void pendulum::create_biped_constraint(physical_interface *a2)
{
    if (a2 != nullptr && this->biped_physics_constraint == nullptr && a2->is_biped_physics_running()) {
        assert(pivot_rigid_body == nullptr);
        auto *v3 = this->get_volatile_ptr();
        if (v3 != nullptr) {
            auto *v4 = phys_sys::create_user_rigid_body();
            this->pivot_rigid_body = v4;
            v4->set((const math::MatClass<4, 3> *)v3->my_abs_po);
        }

        auto *v5 = a2->get_biped_system();

        auto *v6 = v5->field_0.m_list_rigid_body.m_data[this->field_30];
        rigid_body *v7 = this->pivot_rigid_body;
        if (v7 == nullptr) {
            v7 = phys_sys::get_environment_rigid_body();
        }

        this->biped_physics_constraint = phys_sys::create_rbc_dist(v6, v7);

        float length = this->get_constraint();
        phys_vector3d v11 = (v3 != nullptr ? this->field_4 : this->get_pivot_abs_pos());

        phys_vector3d a2a{};
        this->biped_physics_constraint->set(a2a, v11, 0.0, length);
        this->field_2C = this->field_30;

        assert(biped_physics_constraint != nullptr);
    }
}

void pendulum::sub_4BD990(physical_interface *a2)
{
    if (a2 != nullptr && a2->is_biped_physics_running()) {
        if (this->biped_physics_constraint != nullptr) {
            phys_sys::destroy(this->biped_physics_constraint);
            this->biped_physics_constraint = nullptr;
        }

        if (this->pivot_rigid_body != nullptr) {
            phys_sys::destroy((user_rigid_body *)this->pivot_rigid_body);
            this->pivot_rigid_body = nullptr;
        }
    }

    assert(biped_physics_constraint == nullptr);
    assert(pivot_rigid_body == nullptr);
}

void pendulum::update_biped_constraint(physical_interface *a2)
{
    if (a2 != nullptr && a2->is_biped_physics_running()) {
        if (this->m_active) {
            if (this->biped_physics_constraint != nullptr) {
                auto *ent = this->get_volatile_ptr();
                if (ent != nullptr) {
                    auto *v4 = (user_rigid_body *)this->pivot_rigid_body;
                    if (v4 == nullptr || v4->m_dictator != (const math::MatClass<4, 3> *)ent->my_abs_po) {
                        this->sub_4BD990(a2);
                    }
                } else if (this->pivot_rigid_body != nullptr) {
                    this->sub_4BD990(a2);
                }


                if (this->biped_physics_constraint && this->field_2C != this->field_30) {
                    this->sub_4BD990(a2);
                }
            }

            auto *v6 = this->biped_physics_constraint;
            if (v6 != nullptr) {
                if (this->pivot_rigid_body != nullptr) {
                    v6->sub_502680(this->field_4);
                } else {
                    auto v7 = this->get_pivot_abs_pos();
                    this->biped_physics_constraint->sub_502680(v7);
                }

                this->biped_physics_constraint->field_34 = this->m_constraint;
            } else {
                this->create_biped_constraint(a2);
            }
        } else {
            this->sub_4BD990(a2);
        }
    }
}

vector3d pendulum::sub_48AFB0(entity_base *a2)
{
    *bit_cast<entity_base_vhandle *>(this) = (a2 != nullptr ? a2->get_my_handle() : entity_base_vhandle{});

    this->field_10 = ZEROVEC;
    this->field_4 = ZEROVEC;
    return this->get_pivot_abs_pos();
}
