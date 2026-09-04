#include "rbc_def_distance.h"

#include "func_wrapper.h"
#include "phys_vector3d.h"
#include "common.h"
#include "outer_time.h"
#include "rigid_body.h"
#include "physics_system.h"

VALIDATE_SIZE(rigid_body_constraint_distance, 0x5C);

void rigid_body_constraint_distance::set(phys_vector3d const &a2, phys_vector3d const &a3, Float a4, Float a5)
{
    THISCALL(0x007A3390, this, &a2, &a3, a4, a5);
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
    THISCALL(0x007A3A40, this, a2, a3);
}

void rigid_body_constraint_distance::sub_502680(const vector3d &a2)
{
    THISCALL(0x502680, this, &a2);
}
