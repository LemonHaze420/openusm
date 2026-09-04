#include "dolly_and_strafe_mcs.h"

#include "common.h"
#include "func_wrapper.h"
#include "mouselook_controller.h"

VALIDATE_SIZE(dolly_and_strafe_mcs, 0x18u);

dolly_and_strafe_mcs::dolly_and_strafe_mcs(entity *a2)
{
    this->m_vtbl = 0x00889104;
    this->m_dolly = 0.0;
    this->m_strafe = 0.0;
    this->m_lift = 0.0;
    this->m_ent = a2;
}

void dolly_and_strafe_mcs::frame_advance([[maybe_unused]] Float a2)
{
    if constexpr (1) {
        if (g_mouselook_controller() != nullptr) {
            if (g_mouselook_controller()->field_4) {
                this->do_dolly(this->m_dolly);
                this->do_strafe(this->m_strafe);
                this->do_lift(this->m_lift);
                this->m_dolly = 0.0;
                this->m_strafe = 0.0;
                this->m_lift = 0.0;
            }
        }
    } else {
        THISCALL(0x0052E6B0, this, a2);
    }
}

int dolly_and_strafe_mcs::do_dolly(Float a2)
{
    return THISCALL(0x00526940, this, a2);
}

int dolly_and_strafe_mcs::do_strafe(Float a2)
{
    return THISCALL(0x00526A30, this, a2);
}

int dolly_and_strafe_mcs::do_lift(Float a2)
{
    return THISCALL(0x00526B20, this, a2);
}
