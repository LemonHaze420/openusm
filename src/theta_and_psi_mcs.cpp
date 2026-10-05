#include "theta_and_psi_mcs.h"

#include "camera.h"
#include "common.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "memory.h"
#include "oldmath_po.h"
#include "mouselook_controller.h"
#include "variables.h"

#include <algorithm>
#include <cmath>

VALIDATE_SIZE(theta_and_psi_mcs, 0x1Cu);

#if STANDALONE_SYSTEM
namespace {
theta_and_psi_mcs *__fastcall native_theta_destroy(theta_and_psi_mcs *self, void *, unsigned char flags)
{
    self->~theta_and_psi_mcs();
    if (flags & 1)
        mem_dealloc(self, sizeof(theta_and_psi_mcs));
    return self;
}

void __fastcall native_theta_advance(theta_and_psi_mcs *self, void *, Float dt)
{
    self->frame_advance(dt);
}

std::intptr_t *native_theta_vtable()
{
    static std::intptr_t table[4];
    static const bool initialized = [] {
        motion_control_system::initialize_native_vtable(table,
                                                        reinterpret_cast<std::intptr_t>(native_theta_destroy),
                                                        reinterpret_cast<std::intptr_t>(native_theta_advance));
        return true;
    }();
    (void)initialized;
    return table;
}
}  // namespace
#endif

theta_and_psi_mcs::theta_and_psi_mcs(entity *a2, Float a3, Float a4)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_theta_vtable());
#else
    this->m_vtbl = 0x00888EB4;
#endif
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
#if STANDALONE_SYSTEM
    if (g_mouselook_controller() != nullptr && g_mouselook_controller()->field_4 && !cam_target_locked) {
        m_theta += d_theta_for_next_frame;
        m_psi = std::clamp(m_psi + d_psi_for_next_frame, -half_PI, half_PI);

        po pitch;
        po yaw;
        po transform;
        pitch.set_rot(XVEC, m_psi);
        yaw.set_rot(YVEC, m_theta);
        po::compose(transform, yaw, pitch);
        transform.set_position(m_ent->get_rel_position());
        m_ent->set_abs_po(transform);
        d_theta_for_next_frame = 0.0f;
        d_psi_for_next_frame = 0.0f;
    }
    (void)dt;
#else
    THISCALL(0x0053B260, this, dt);
#endif
}
