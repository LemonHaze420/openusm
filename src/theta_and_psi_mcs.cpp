#include "theta_and_psi_mcs.h"

#include "camera.h"
#include "common.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "memory.h"
#include "oldmath_po.h"

#include <cmath>

VALIDATE_SIZE(theta_and_psi_mcs, 0x1Cu);

theta_and_psi_mcs::theta_and_psi_mcs(entity *a2, Float a3, Float a4)
{
    this->m_vtbl = 0x00888EB4;
    this->m_theta = a3;
    this->m_psi = a4;
    this->d_theta_for_next_frame = 0.0;
    this->d_psi_for_next_frame = 0.0;
    this->m_ent = a2;
}

void *theta_and_psi_mcs::operator new(size_t size)
{
    return mem_alloc(size);
}

void theta_and_psi_mcs::reset_angles()
{
    if constexpr (1) {
        auto &z_facing = this->m_ent->get_abs_po().get_z_facing();

        auto cross = vector3d::cross(z_facing, YVEC);

        this->m_psi = std::atan2(z_facing[1], z_facing[2] / cross[0]);
        this->m_theta = std::atan2(cross[2], cross[0]);

        float v5;
        if (this->m_psi >= -half_PI) {
            if (this->m_psi <= half_PI) {
                return;
            }

            v5 = PI;
        } else {
            v5 = -PI;
        }

        this->m_psi = v5 - this->m_psi;
        this->m_theta += PI;
    } else {
        THISCALL(0x005196E0, this);
    }
}

void theta_and_psi_mcs::frame_advance(Float dt)
{
    THISCALL(0x0053B260, this, dt);
}
