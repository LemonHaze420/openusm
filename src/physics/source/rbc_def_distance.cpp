#include "rbc_def_distance.h"

#include "func_wrapper.h"
#include "phys_vector3d.h"
#include "common.h"
#include "outer_time.h"
#include "rigid_body.h"
#include "physics_system.h"
#include "physics_system_internal.h"

VALIDATE_SIZE(rigid_body_constraint_distance, 0x5C);

void rigid_body_constraint_distance::set(phys_vector3d const &a2, phys_vector3d const &a3, Float a4, Float a5)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x007A3390, this, &a2, &a3, a4, a5);
        return;
    }
    field_C = a2;
    field_18 = 0;
    field_1C = a3;
    field_28 = 0;
    m_min_distance = a4;
    m_max_distance = field_34 = a5;
    field_38 = field_3C = 0;
    field_40 = 1;
}

void rigid_body_constraint_distance::outer_epilog_update(const outer_time &)
{
    this->m_max_distance = this->field_34;
}

void rigid_body_constraint_distance::outer_prolog_update(const outer_time &a2)
{
    auto *v2 = this->b1;
    if (this->b1 == nullptr || (v2->field_144 & 0x10) != 0) {
        v2 = this->b2;
    }

    this->field_38 = (this->field_34 - this->m_max_distance) / (v2->field_13C * a2.field_0);
}

void rigid_body_constraint_distance::setup_constraint(physics_system *a2, Float a3)
{
    if constexpr (STANDALONE_SYSTEM)
        physics_setup_distance(this, a2, a3);
    else
        THISCALL(0x007A3A40, this, a2, a3);
}

void rigid_body_constraint_distance::sub_502680(const vector3d &a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_1C.field_0[0] = a2.x;
        field_1C.field_0[1] = a2.y;
        field_1C.field_0[2] = a2.z;
        field_28 = 0;
    } else {
        THISCALL(0x502680, this, &a2);
    }
}
