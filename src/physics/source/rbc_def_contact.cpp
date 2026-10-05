#include "rbc_def_contact.h"

#include "common.h"
#include "func_wrapper.h"
#include "physics_system.h"
#include "physics_system_internal.h"

#include "utility.h"

#include <cassert>

VALIDATE_SIZE(contact_point_info, 0x6C);
VALIDATE_SIZE(rigid_body_constraint_contact, 0x30);

void rigid_body_constraint_contact::add_point(rigid_body *b1_, rigid_body *b2_, phys_vector3d const &a4,
                                              phys_vector3d const &a5, phys_vector3d const &a6, Float fric_coef,
                                              Float bounce_coef, Float a9, bool a10)
{
    if constexpr (STANDALONE_SYSTEM) {
        physics_add_contact(this, b1_, b2_, a4, a5, a6, fric_coef, bounce_coef, a9, a10);
        return;
    }

    assert((b1 == b1_ && b2 == b2_) || (b1 == b2_ && b2 == b1_));
    THISCALL(0x007A7360, this, b1_, b2_, &a4, &a5, &a6, fric_coef, bounce_coef, a9, a10);

}

void rigid_body_constraint_contact::setup_constraint(physics_system *a2, Float a3)
{
    if constexpr (STANDALONE_SYSTEM)
        physics_setup_contact(this, a2, a3);
    else
        THISCALL(0x007A7550, this, a2, a3);
}

void rbc_contact_patch()
{
    FUNC_ADDRESS(func, &rigid_body_constraint_contact::add_point);
    REDIRECT(0x00592C95, func);
    REDIRECT(0x0059C9B4, func);
    REDIRECT(0x0059D414, func);
}
