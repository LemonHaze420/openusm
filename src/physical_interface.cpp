#include "physical_interface.h"

#include "actor.h"
#include "biped_system.h"
#include "common.h"
#include "func_wrapper.h"
#include "guidance_sys.h"
#include "log.h"
#include "oldmath_po.h"
#include "pendulum.h"
#include "phys_vector3d.h"
#include "rb_ragdoll_model.h"
#include "rigid_body.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "vtbl.h"
#include "wds.h"

#include <cassert>
#include <cmath>

VALIDATE_OFFSET(physical_interface, field_164, 0x164);
VALIDATE_SIZE(physical_interface, 0x1B0);

std::reference_wrapper<int[512]> physical_interface::rotators = var<int[512]>(0x0095AF78);

int & physical_interface::rotators_num = var<int>(0x0095A6B8);

static constexpr float MAX_ASSERT_PHYSICAL_VELOCITY = 500.f;

physical_interface::physical_interface(actor *a2) : field_188(), field_198()
{
    this->m_vtbl = 0x00883A44;
    this->field_4 = a2;
    this->dynamic = true;
    this->field_C = 0;
    this->field_84 = {};
    this->field_88 = {};
    this->field_A4 = 0;
    this->field_D8 = 0;
    this->field_124 = 0;
    this->field_128 = 0;
    this->field_12C = 0;
    this->field_130 = 0;
    this->field_134 = 0;
    this->field_138 = 0;
    this->field_13C = 0;
    this->field_140 = 0;
    this->field_144 = 0;
    this->field_148 = 0;
    this->field_14C = 0;
    this->field_150 = 0;
    this->field_154 = 0;
    this->field_158 = 0;
    this->field_17C = 0;
    this->field_17D = 0;
    this->field_184 = 0;
    this->field_185 = 0;
    this->field_186 = 0;

    if (!g_generating_vtables())
    {
        this->add_to_phys_ifc_list();
        this->field_174 = nullptr;
        this->m_bp_sys = nullptr;
        this->field_10 = 0.0;
        this->field_14 = 0.0;
        this->field_18 = 0;
        this->field_1C = 0;

        this->m_velocity = ZEROVEC;
        this->field_2C = ZEROVEC;
        this->field_38 = ZEROVEC;

        this->field_74 = -YVEC;

        this->field_44 = ZEROVEC;
        this->field_80 = 0;
        this->field_90 = 0.5;
        this->field_8C = 0.64999998;
        this->field_94 = 0.025;
        this->field_EC = 0.55000001;
        this->field_15C = 0.0;
        this->field_160 = 0.0;
        this->field_E8 = nullptr;
        this->field_164 = 1.0;
        this->field_168 = 1.0;
        this->field_16C = 1.0;
        this->m_gravity_multiplier = 1.0;
        this->field_17C = 1;
        this->field_F0 = -10.0;
        this->field_F4 = 0;
        this->field_FC = 0;
        this->field_F8 = 0;
        this->field_184 = 0;
        this->field_180 = g_world_ptr->field_158.field_8;
        this->field_100 = YVEC;
        this->field_10C = 0;
        this->field_C8 = 1.0;
        this->field_CC = -1.0;
        this->field_17D = 0;

        this->field_110[0] = nullptr;
        this->field_110[1] = nullptr;
        this->field_110[2] = nullptr;
        this->field_110[3] = nullptr;
        this->field_110[4] = nullptr;

        this->field_C |= 0x1000u;
        this->field_9C = 1.0;
        this->field_A0 = 1.0;
        this->field_A4 = 0;
        this->field_170 = 0;
        this->field_D8 = 0;
        this->field_D4 = 1.0;
        this->field_98 = 0.0;
        this->field_1AC = 1.0;

        this->field_188 = "";
        this->field_198 = "";
        this->field_DC = 0;
        this->field_E0 = 0;
        this->field_E4 = 10.0;
        this->field_1A8 = 0;
    }
}

pendulum *physical_interface::get_pendulum(int num)
{
    assert(num >= 0 && num < PHYS_IFC_MAX_PENDULUM_CONSTRAINTS);
    return this->field_110[num];
}

void physical_interface::get_parent_terrain_type(string_hash *a2)
{
    THISCALL(0x004C93E0, this, a2);
}

bool physical_interface::set_ifc_num(const resource_key &a2, Float a3, bool a4) {
    return (bool) THISCALL(0x004C9500, this, &a2, a3, a4);
}

bool physical_interface::get_ifc_num(const resource_key &a2, float &a3, bool a4) {
    return (bool) THISCALL(0x004BD160, this, &a2, &a3, a4);
}

void physical_interface::add_to_phys_ifc_list() {
    THISCALL(0x004DA210, this);
}

bool physical_interface::is_biped_physics_running() const
{
    return 0x80000 & this->field_C;
}

biped_system *physical_interface::get_biped_system()
{
    if ( !this->is_biped_physics_running() )
    {
        return nullptr;
    }

    assert(m_bp_sys != nullptr);

    return this->m_bp_sys;
}

bool sub_5019B0(float *a1, float *a2, Float a3, Float a4, Float a5) {
    if (equal(float{a3}, 0.0f)) {
        return false;
    }

    auto v5 = a4 * a4 - a3 * a5 * 4.0f;
    if (v5 < 0.0f) {
        return false;
    }

    auto v6 = std::sqrt(v5);
    auto v7 = a3 + a3;
    *a1 = (-a4 - v6) / v7;
    *a2 = (v6 - a4) / v7;
    return true;
}

void physical_interface::un_mash(generic_mash_header *a2,
                                 void *a3,
                                 void *a4,
                                 generic_mash_data_ptrs *a5) {
    THISCALL(0x004DF4A0, this, a2, a3, a4, a5);
}

void physical_interface::frame_advance_all_phys_interfaces(Float a1) {
    TRACE("physical_interface::frame_advance_all_phys_interfaces");

    CDECL_CALL(0x004FB1D0, a1);
}

vector3d physical_interface::calculate_force_vector_2(const vector3d *a2,
                                                      const vector3d *a3,
                                                      Float a4,
                                                      Float a5) {
    static const Var<float> g_gravity = (0x00921E3C);

    vector3d v21 = (*a3) - (*a2);

    a5.value *= g_gravity();
    float v20 = v21[0] * v21[0] + v21[2] * v21[2];
    float v5 = v21[1] * v21[1] + v20;
    float v6 = v21[1] * a5 - a4 * a4;
    float v7 = a5 * a5 * 0.25f;

    float a2_1;
    float a3_1;
    if (sub_5019B0(&a2_1, &a3_1, v7, v6, v5)) {
        bool v8 = (a2_1 <= 0.0 || a3_1 <= 0.0) ? (a2_1 < a3_1) : (a3_1 < a2_1);

        float *v9 = &a3_1;
        if (!v8) {
            v9 = &a2_1;
        }

        auto v10 = std::sqrt(*v9);
        auto v11 = 1.0f / v10;

        v21[0] = v21[0] * v11;
        v21[1] = (v10 * v10 * a5 * 0.5f + v21[1]) * v11;
        v21[2] = v21[2] * v11;

    } else {
        auto v17 = std::sqrt(v20);
        if (v17 > std::abs(v21[1])) {
            v21[1] = v17;
        }
    }

    auto v15 = v21.length2();
    if (v15 > 9.9999994e-11) {
        auto v16 = 1.0f / std::sqrt(v15);
        v21 = v21 * v16;
    }

    vector3d result = v21 * a4;

    return result;
}

float physical_interface::cancel_all_velocity() {
    return (float) THISCALL(0x004CEDA0, this);
}

bool physical_interface::is_effectively_standing() {
    return this->field_184;
}

void physical_interface::set_velocity(const vector3d &new_velocity, bool a3) {
    if constexpr (1) {
        assert(new_velocity.length2() <
               (MAX_ASSERT_PHYSICAL_VELOCITY * MAX_ASSERT_PHYSICAL_VELOCITY));
        assert(new_velocity.is_valid());

        if (this->field_C & 0x80000) {
            biped_system *bp_sys = this->m_bp_sys;

            assert(bp_sys != nullptr);

            if (new_velocity.length() >= EPSILON) {
                if (a3) {
                    auto *v28 = bp_sys->field_0.m_list_rigid_body.m_data[0];

                    vector3d v29 = v28->sub_503B80();

                    phys_vector3d v49;
                    v49[0] = new_velocity[0] - v28->field_130 * v29[0] + v28->field_D0[0];
                    v49[1] = new_velocity[1] - v28->field_130 * v29[1] + v28->field_D0[1];
                    v49[2] = new_velocity[2] - v28->field_130 * v29[2] + v28->field_D0[2];
                    bp_sys->field_0.sub_4ADEF0(0, v49);
                } else {
                    for (int i = 0; i < 10; ++i) {
                        assert(i >= 0 && i < bp_sys->field_0.m_list_rigid_body.m_alloc_count);

                        auto *v35 = bp_sys->field_0.m_list_rigid_body.m_data[i];
                        auto v36 = v35->field_130;

                        auto v42 = 1.0f / v35->field_130;

                        phys_vector3d a3a;
                        a3a.field_0[0] = (new_velocity[0] - v35->field_110[0] * v36 +
                                          v35->field_D0[0]) *
                            v42;
                        a3a.field_0[1] = (new_velocity[1] - v35->field_110[1] * v36 +
                                          v35->field_D0[1]) *
                            v42;
                        a3a.field_0[2] = (new_velocity[2] -
                                          (v35->field_110[2] * v36 + v35->field_D0[2])) *
                            v42;
                        bp_sys->field_0.apply_pulse(i, a3a);
                    }
                }
            } else if (a3) {
                auto *v4 = bp_sys->field_0.m_list_rigid_body.m_data[0];
                v4->sub_502600(ZEROVEC);
                v4->sub_502640(ZEROVEC);
            } else {
                for (int i = 0; i < 10; ++i) {
                    auto *v5 = bp_sys->field_0.m_list_rigid_body.m_data[i];

                    v5->sub_502600(ZEROVEC);
                    v5->sub_502640(ZEROVEC);
                }
            }
        } else {
            int **v43 = CAST(v43, this->field_174);
            if (v43 != nullptr) {
                bit_cast<rigid_body *>(v43[16])->sub_502600(new_velocity);

            } else {
                this->m_velocity = new_velocity;
            }
        }
    } else {
        THISCALL(0x004CA1C0, this, new_velocity, a3);
    }
}

vector3d physical_interface::get_velocity() const
{
    vector3d result;

    if (this->field_C & 0x80000)
    {
        auto &v3 = this->m_bp_sys->field_0.m_list_rigid_body.m_data[0]->field_D0;

        auto &v8 = v3;
        if (!v3.is_valid()) {
            v8 = ZEROVEC;
        }

        result = v8;
    } else {
        struct {
            char field_0[0x40];
            rigid_body *field_40;
        } *v10 = CAST(v10, this->field_174);

        if (v10 != nullptr) {
            auto &v11 = v10->field_40->field_D0;

            result = v11;
        } else {
            result = this->m_velocity;
        }
    }

    return result;
}

void physical_interface::start_biped_physics(physical_interface::biped_physics_body_types a2) {
    if constexpr (1) {
    } else {
        THISCALL(0x004F2460, this, a2);
    }
}

void physical_interface::set_control_parent(entity *a2) {
    if constexpr (1) {
        if (a2 != nullptr) {
            entity_set_abs_parent(this->field_4, a2);
            this->field_84.field_0.field_0 = a2->my_handle.field_0;
            return;
        }

        auto *v3 = &this->field_84;
        if (this->field_84.get_volatile_ptr() != nullptr) {
            auto *v4 = this->field_4;
            auto *v5 = v4->m_parent;
            if (v5 != nullptr) {
                if (v5 == this->field_84.get_volatile_ptr()) {
                    entity_set_abs_parent(this->field_4, nullptr);
                    v3->field_0.field_0 = 0;
                    return;
                }
            } else {
                auto &a2a = v4->get_abs_po();
                this->field_4->set_abs_po(a2a);
            }

            v3->field_0.field_0 = 0;
        }
    } else {
        THISCALL(0x004ECC10, this, a2);
    }
}

bool physical_interface::allow_manage_standing() {
    return (this->field_C >> 12) & 1;
}

void physical_interface::set_allow_manage_standing(bool a2) {
    uint32_t v3;

    auto v2 = this->field_C;
    if (a2) {
        v3 = v2 | 0x1000;
    } else {
        v3 = v2 & 0xFFFFEFFF;
    }

    this->field_C = v3;
}

void physical_interface::clear_static_lists()
{
    memset(physical_interface::rotators, 0, sizeof(physical_interface::rotators));
    physical_interface::rotators_num = 0;
}

void physical_interface::set_current_gravity_vector(const vector3d &a2) {
    this->field_74 = a2;
}

float physical_interface::get_floor_offset()
{
    TRACE("physical_interface::get_floor_offset");

    if constexpr (1) {
        if (this->field_CC <= 0.0f) {
            if ( this->is_biped_physics_running() || this->is_prop_physics_running() ) {
                this->field_10C = 0.25f;
            } else {
                this->field_10C = this->field_4->get_floor_offset();
            }

        } else {
            this->field_10C = this->field_CC;
        }

        if (this->field_17D) {
            this->field_10C *= this->field_4->get_render_scale()[1];
        }

        return this->field_10C;

    } else {
        return (float) THISCALL(0x004BCC50, this);
    }
}

void physical_interface::suspend(bool a2) {
    if (a2) {
        this->field_C |= 2;
    } else {
        this->field_C &= 0xFFFFFFFD;
    }
}

vector3d physical_interface::calculate_perfect_force_vector(const vector3d &start,
                                                            const vector3d &target,
                                                            Float max_y,
                                                            Float gravity_multiplier) {
    assert(start != target);

    auto v5 = g_gravity * gravity_multiplier * 0.5f;

    assert(max_y > start.y);

    auto v0_y_sq = (4.0 * v5) * (max_y - start.y);
    assert(v0_y_sq > 0.0f);

    vector3d result;
    result.y = std::sqrt(v0_y_sq);

    assert(max_y > target.y);
    auto determinant = (max_y - target.y) / v5;

    assert(determinant > 0.0f);

    auto total_time = (result.y / (2.0 * v5)) + std::sqrt(determinant);
    assert(total_time > 0.0f);

    result.x = (target.x - start.x) / total_time;
    result.z = (target.z - start.z) / total_time;
    return result;
}

void physical_interface::calculate_force_vector(Float a1, Float a2, float *a3, float *a4, Float a5) {
    *a4 = 0.0;
    *a3 = 0.0;
    auto v5 = g_gravity * a5;
    if (a1 < LARGE_EPSILON) {
        a1 = LARGE_EPSILON;
    }

    if (v5 > 0.0) {
        auto v6 = std::sqrt(a1 * v5 + a1 * v5);
        *a4 = v6;
        auto v7 = (v6 + v6) / v5;
        if (std::abs(v7) > EPSILON) {
            *a3 = a2 / v7;
        }
    }
}

bool physical_interface::is_enabled() const
{
    return this->is_flag(1);
}

int physical_interface::get_num_active_pendulums() const
{
    int v3 = 0;
    for ( auto &p : this->field_110 )
    {
        if ( p != nullptr )
        {
            if ( p->m_active ) {
                ++v3;
            }
        }
    }

    return v3;
}

void physical_interface::enable(bool a2) {
    if (a2) {
        this->field_C |= 1;
    } else {
        this->field_C &= 0xFFFFFFFE;
    }
}


void physical_interface::set_pendulum(int num, pendulum *a3)
{
    assert(num >= 0 && num < PHYS_IFC_MAX_PENDULUM_CONSTRAINTS);

    if constexpr (0)
    {
        auto *v4 = this->field_110[num];
        if ( v4 != nullptr && v4 != a3 )
        {
            v4->sub_4BD990(this);
        }

        if ( a3 != nullptr && this->field_110[num] != a3 )
        {
            a3->biped_physics_constraint = nullptr;
            a3->pivot_rigid_body = nullptr;
        }

        this->field_110[num] = a3;
    }
    else
    {
        THISCALL(0x004D19E0, this, num, a3);
    }    
}

void physical_interface::set_gravity(bool a2) {
    unsigned int v2 = this->field_C;
    if (((v2 >> 2) & 1) != a2) {
        if (a2) {
            this->field_C |= 4;
        } else {
            this->field_C &= 0xFFFFFFFB;
        }

        this->field_A4 = 0;
    }
}

vector3d physical_interface::apply_positional_constraints(
        Float a3,
        const vector3d &a4,
        bool a5)
{
    vector3d result;
    THISCALL(0x004ECCD0, this, &result, a3, &a4, a5);
    return result;
}

void physical_interface::apply_force_increment_in_biped_physics_mode(
    const vector3d &a2, physical_interface::force_type a3, const vector3d &a4, int a5)
{
    if constexpr (1)
    {
        biped_system *v5 = nullptr;
        if ((this->field_C & 0x80000) != 0) {
            v5 = this->m_bp_sys;
        }

        if (a3 == 1) {
            phys_vector3d a2a;
            phys_vector3d a3a;
            float a4a;
            v5->field_0.get_ballistic_info(&a2a, &a3a, &a4a);

            auto v6 = a2[0];
            auto v7 = a2[1];
            auto v15 = a2[2];
            auto v8 = v5->field_0.m_list_rigid_body.m_data;
            auto v13 = v6;
            auto v9 = *v8;
            auto v10 = a4a * v9->field_130;
            auto v14 = v10 * v7;
            v15 = v15 * v10;
            auto v11 = 1.f / v9->field_130;
            auto v19 = v11;

            phys_vector3d v16;
            v16[0] = v11 * (v13 * v10);
            v16[1] = v19 * v14;
            v16[2] = v19 * v15;

            v5->field_0.apply_pulse(0, v16);
        }

    } else {
        THISCALL(0x004C9430, this, &a2, a3, &a4, a5);
    }
}

void physical_interface::apply_force_increment(const vector3d &a2,
                                               physical_interface::force_type arg4,
                                               const vector3d &a4,
                                               int a5) {
    THISCALL(0x004ECFF0, this, &a2, arg4, &a4, a5);
}

void physical_interface::manage_standing(bool a2)
{
    TRACE("physical_interface::manage_standing");

    THISCALL(0x004F8580, this, a2);
}

void physical_interface::stop_biped_physics(bool a2) {
    THISCALL(0x004F2700, this, a2);
}

void physical_interface::stop_prop_physics(bool a2)
{
    THISCALL(0x004F10F0, this, a2);
}

void physical_interface::remove_from_phys_ifc_list()
{
    for ( auto it = all_phys_interfaces->begin(); it != all_phys_interfaces->end(); ++it )
    {
        if ((*it) == this) {
            all_phys_interfaces->erase(it);
        }
    }

    if ( all_phys_interfaces->empty() )
    {
        delete all_phys_interfaces;
        all_phys_interfaces = nullptr;
    }
}

void physical_interface::release_ifc()
{
    auto *v2 = this->field_E8;
    if ( v2 != nullptr )
    {
        if ( LOBYTE(v2->field_1C) ) {
            void (__fastcall *finalize)(void *, void *edx, bool) = CAST(finalize, get_vfunc(v2->m_vtbl, 0x0));
            finalize(v2, nullptr, true);
        }

        this->field_E8 = nullptr;
    }

    if ( (this->field_C & 0x80000) != 0 ) {
        this->stop_biped_physics(true);
    }

    if ( this->field_174 ) {
        this->stop_prop_physics(true);
    }

    this->remove_from_phys_ifc_list();
}

void physical_interface_patch() {
    {
        FUNC_ADDRESS(address, &physical_interface::start_biped_physics);

        //SET_JUMP(0x004F2460, address);
    }

    {
        REDIRECT(0x005584B2, physical_interface::frame_advance_all_phys_interfaces);
    }
}
