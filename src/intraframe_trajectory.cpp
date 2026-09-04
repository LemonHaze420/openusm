#include "intraframe_trajectory.h"

#include "actor.h"
#include "collide_aux.h"
#include "collision_capsule.h"
#include "collision_geometry.h"
#include "common.h"
#include "dynamic_conglomerate_clone.h"
#include "func_wrapper.h"
#include "physical_interface.h"
#include "quaternion.h"
#include "sphere.h"

#include <cassert>
#include <cmath>

VALIDATE_SIZE(collision_free_state, 0x64u);

intraframe_trajectory_t::intraframe_trajectory_t() {}

intraframe_trajectory_t::intraframe_trajectory_t(actor *a2, Float a3, const po &a4, dynamic_conglomerate_clone *a5)
{
    this->init(a2, a3, a4, a5);
}

capsule xform3d_1_capsule(const matrix4x4 &a2, const capsule &a3)
{
    capsule cap{};
    cap.radius = a3.radius;

    cap.base = a2 * a3.base;
    cap.end = a2 * a3.end;

    return cap;
}

static constexpr auto MAX_TRAJECTORY_VELOCITY = 500.0f;

void intraframe_trajectory_t::init(entity *ent_arg, Float a3, const po &a4, dynamic_conglomerate_clone *a5)
{
    if constexpr (1) {
        assert(ent_arg->is_an_actor());

        this->field_15C = nullptr;
        this->ent = CAST(ent, ent_arg);
        this->field_13C = a5;
        this->field_14C = a3;
        this->final_relcap = nullptr;
        this->field_164 = false;
        this->field_165 = false;
        this->field_160 = 0;
        this->field_168 = nullptr;
        this->field_150 = (this->ent->has_physical_ifc() && this->ent->physical_ifc()->is_enabled()
                               ? this->ent->physical_ifc()->get_velocity()
                               : ZEROVEC);

        po my_abs_po{};
        if (auto *v8 = this->field_13C; v8 != nullptr) {
            my_abs_po = *v8->get_member_abs_po(this->ent);
        } else {
            my_abs_po = this->ent->get_abs_po();
        }

        this->final_po = my_abs_po;
        this->world_po1 = this->final_po;
        this->world_po0 = this->world_po1;

        assert(final_po.is_valid());

        assert(std::abs(final_po.get_matrix().w.length()) < 100000.0f);

        if (this->ent->field_A4 != 0 && !this->ent->get_allow_tunnelling_into_next_frame()) {
            this->world_po0 = this->ent->get_last_collision_free_state()->xform;

            assert(this->world_po0.is_valid());

            auto position = this->world_po0.get_position();
            auto v10 = this->world_po1.get_position();
            auto v1 = v10 - position;
            auto v32 = std::max(this->field_14C, 0.016666668f);
            vector3d assert_velocity = v1 / v32;

            assert(assert_velocity.length2() < MAX_TRAJECTORY_VELOCITY * MAX_TRAJECTORY_VELOCITY);
        }

        if (this->ent->are_collisions_active() && this->has_colgeom() && this->get_colgeom()->get_type() == 1) {
            auto v14 = this->world_po1.get_position() - this->world_po0.get_position();
            this->field_140 = v14 / this->field_14C;
            if (this->field_140.length2() > 22500.0f) {
                this->field_140.normalize();
                this->field_140 *= 150.0f;
                this->integrate(this->field_14C, &this->world_po1);
                this->final_po.set_position(this->world_po1.get_position());
            }

            assert(ent->get_colgeom() != nullptr);

            this->is_capsule = true;
            this->init_capsules();
        } else {
            if (this->has_colgeom() && this->get_colgeom()->get_type() == 1) {
                auto *cap = bit_cast<collision_capsule *>(this->get_colgeom());
                auto rel_cap = cap->rel_cap;

                this->relcap1 = rel_cap;
                this->relcap0 = rel_cap;

                this->my_abs_cap1 = xform3d_1_capsule(this->final_po.get_matrix(), this->relcap1);
                this->my_abs_cap0 = xform3d_1_capsule(this->final_po.get_matrix(), this->relcap0);
            } else {
                this->relcap0 = capsule{ZEROVEC, ZEROVEC, 0.0f};

                this->relcap1 = this->relcap0;

                this->my_abs_cap0 = this->relcap1;

                this->my_abs_cap1 = this->my_abs_cap0;
            }

            this->is_capsule = false;
        }

        this->field_140 = (this->world_po1.get_position() - this->world_po0.get_position()) / this->field_14C;
    } else {
        THISCALL(0x0053BC80, this, ent_arg, a3, &a4, a5);
    }
}

bool build_quat_that_aligns_two_vectors(const vector3d &a1, const vector3d &a2, quaternion &a3)
{
    auto a2a = a2;
    a2a.normalize();

    auto a3a = a1;
    a3a.normalize();

    auto v9 = a2a + a3a;
    v9.normalize();

    auto v8 = dot(a2a, v9);
    vector3d v7 = vector3d::cross(a2a, v9);
    auto v6 = v7.length();
    if (v6 < 0.000099999997) {
        return false;
    }

    v7 *= 1.0 / v6;
    auto v5 = v6;
    auto v4 = vector3d::cross(a2, v7);
    if (dot(a1, v4) > 0.0) {
        v5 = 0.0 - v5;
    }

    a3[1] = v5 * v7[0];
    a3[2] = v5 * v7[1];
    a3[3] = v5 * v7[2];
    a3[0] = v8;
    return true;
}

void extract_axis_aligned_capsule_and_world_xform(const capsule &a3, capsule *aa_cap, po *world_po)
{
    assert(aa_cap != nullptr && world_po != nullptr);

    quaternion v10{};
    auto v3 = a3.end - a3.base;
    build_quat_that_aligns_two_vectors(YVEC, v3, v10);
    world_po->set_po(a3.base, v10, 1.0f);

    aa_cap->radius = a3.radius;
    aa_cap->base = ZEROVEC;

    auto v4 = a3.end - a3.base;
    aa_cap->end = YVEC * v4.length();
}

bool capsules_almost_equal(const capsule &a1, const capsule &a2)
{
    auto v2 = a1.base - a2.base;
    if (v2.length() < 0.0099999998) {
        auto v3 = a1.end - a2.end;
        if (v3.length() < 0.0099999998) {
            return true;
        }
    }

    return false;
}

void intraframe_trajectory_t::init_capsules()
{
    assert(is_capsule);

    auto *cap = bit_cast<collision_capsule *>(this->get_colgeom());
    this->relcap1 = cap->rel_cap;
    this->relcap0 = this->relcap1;

    if (this->ent->field_A4 != 0 && !this->ent->get_allow_tunnelling_into_next_frame()) {
        this->relcap0 = this->ent->get_last_collision_free_state()->rel_cap;
    }

    assert(relcap1.radius >= MIN_CAPSULE_RADIUS && relcap0.radius >= MIN_CAPSULE_RADIUS);

    po world_po{};

    po save_world_po0 = this->world_po0;
    po save_world_po1 = this->world_po1;

    extract_axis_aligned_capsule_and_world_xform(this->relcap0, &this->my_abs_cap0, &world_po);
    this->world_po0 = world_po.sub_4BAB00(this->world_po0);

    extract_axis_aligned_capsule_and_world_xform(this->relcap1, &this->my_abs_cap1, &world_po);
    this->world_po1 = world_po.sub_4BAB00(this->world_po1);

    assert(capsules_almost_equal(get_abs_cap1(), xform3d_1_capsule(save_world_po1.get_matrix(), relcap1)));

    assert(capsules_almost_equal(get_abs_cap0(), xform3d_1_capsule(save_world_po0.get_matrix(), relcap0)));
}

void intraframe_trajectory_t::integrate(Float a2, po *integrated_xform)
{
    assert(integrated_xform != nullptr);

    *integrated_xform = this->world_po0;
    auto v4 = this->field_140 * a2;
    auto v3 = this->world_po0.get_position() + v4;
    integrated_xform->set_position(v3);
}

capsule intraframe_trajectory_t::get_abs_cap0()
{
    assert(is_capsule);

    auto &v2 = this->world_po0.get_matrix();
    auto result = xform3d_1_capsule(v2, this->my_abs_cap0);
    return result;
}

capsule intraframe_trajectory_t::get_abs_cap1()
{
    assert(is_capsule);

    auto &v2 = this->world_po1.get_matrix();
    auto result = xform3d_1_capsule(v2, this->my_abs_cap1);
    return result;
}

collision_geometry *intraframe_trajectory_t::get_colgeom()
{
    assert(ent->get_colgeom());

    return this->ent->get_colgeom();
}

collision_capsule &intraframe_trajectory_t::get_capsule()
{
    assert(ent->get_colgeom());

    assert(ent->get_colgeom()->get_type() == collision_geometry::CAPSULE);

    return *bit_cast<collision_capsule *>(this->get_colgeom());
}

sphere intraframe_trajectory_t::get_bounding_sphere()
{
    sphere a3{};

    if (this->is_capsule) {
        capsule cap1 = this->get_abs_cap1();

        capsule cap0 = this->get_abs_cap0();

        compute_bounding_sphere_for_two_capsules(cap0, cap1, &a3);
    } else {
        assert(world_po0.is_valid());
        assert(world_po1.is_valid());

        auto colgeom_radius = this->ent->get_colgeom_radius();

        assert(ent->get_colgeom() != nullptr);

        vector3d v9 = this->ent->get_colgeom()->get_local_space_bounding_sphere_center();

        vector3d v6 = this->world_po1.get_matrix() * v9;

        vector3d v4 = this->world_po0.get_matrix() * v9;

        merge_spheres(v4, colgeom_radius, v6, colgeom_radius, a3.center, a3.radius);
    }

    return a3;
}
