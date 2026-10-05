#include "dolly_and_strafe_mcs.h"

#include "common.h"
#include "func_wrapper.h"
#include "mouselook_controller.h"
#include "entity.h"
#include "memory.h"
#include "oldmath_po.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(dolly_and_strafe_mcs, 0x18u);

#if STANDALONE_SYSTEM
namespace {
dolly_and_strafe_mcs *__fastcall native_dolly_destroy(dolly_and_strafe_mcs *self, void *,
                                                    unsigned char flags)
{
    self->~dolly_and_strafe_mcs();
    if (flags & 1)
        mem_dealloc(self, sizeof(dolly_and_strafe_mcs));
    return self;
}

void __fastcall native_dolly_advance(dolly_and_strafe_mcs *self, void *, Float dt)
{
    self->frame_advance(dt);
}

std::intptr_t *native_dolly_vtable()
{
    static std::intptr_t table[4];
    static const bool initialized = [] {
        motion_control_system::initialize_native_vtable(
            table, reinterpret_cast<std::intptr_t>(native_dolly_destroy),
            reinterpret_cast<std::intptr_t>(native_dolly_advance));
        return true;
    }();
    (void)initialized;
    return table;
}

void translate_camera(entity *camera, const vector3d &offset)
{
    const vector3d position = camera->get_abs_position() + offset;
    camera->get_rel_po().set_position(position);
    camera->dirty_family(false);
    if (camera->is_conglom_member() || camera->is_a_conglomerate())
        camera->dirty_model_po_family();
    auto changed = reinterpret_cast<void (__fastcall *)(entity *, void *)>(
        get_vfunc(camera->m_vtbl, 0x34));
    changed(camera, nullptr);
}
}
#endif

dolly_and_strafe_mcs::dolly_and_strafe_mcs(entity *a2)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_dolly_vtable());
#else
    this->m_vtbl = 0x00889104;
#endif
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

void dolly_and_strafe_mcs::do_dolly(Float a2)
{
#if STANDALONE_SYSTEM
    translate_camera(m_ent, m_ent->get_abs_po().get_z_facing() * a2);
#else
    THISCALL(0x00526940, this, a2);
#endif
}

void dolly_and_strafe_mcs::do_strafe(Float a2)
{
#if STANDALONE_SYSTEM
    translate_camera(m_ent, m_ent->get_abs_po().get_x_facing() * a2);
#else
    THISCALL(0x00526A30, this, a2);
#endif
}

void dolly_and_strafe_mcs::do_lift(Float a2)
{
#if STANDALONE_SYSTEM
    translate_camera(m_ent, YVEC * a2);
#else
    THISCALL(0x00526B20, this, a2);
#endif
}
