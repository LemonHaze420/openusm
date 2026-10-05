#include "lookat_target_controller.h"

#include "common.h"
#include "entity_base.h"
#include "trace.h"
#include "utility.h"
#include "memory.h"

VALIDATE_SIZE(lookat_target_controller, 0x24u);

#if STANDALONE_SYSTEM
namespace {
lookat_target_controller *__fastcall native_lookat_destroy(lookat_target_controller *self, void *, unsigned char flags)
{
    self->~lookat_target_controller();
    if (flags & 1)
        mem_dealloc(self, sizeof(lookat_target_controller));
    return self;
}

void __fastcall native_lookat_advance(lookat_target_controller *self, void *, Float dt)
{
    self->_frame_advance(dt);
}

std::intptr_t *native_lookat_vtable()
{
    static std::intptr_t table[11];
    static const bool initialized = [] {
        controller::initialize_native_vtable(table,
                                             reinterpret_cast<std::intptr_t>(native_lookat_destroy),
                                             reinterpret_cast<std::intptr_t>(native_lookat_advance),
                                             false);
        return true;
    }();
    (void)initialized;
    return table;
}
}  // namespace
#endif

lookat_target_controller::lookat_target_controller(entity_base *a2)
{
    this->field_4 = true;
    this->field_5 = false;
    this->field_8 = a2;
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_lookat_vtable());
#else
    this->m_vtbl = 0x00889858;
#endif

    this->field_C = ZEROVEC;
    this->field_18 = 0.0;
    this->field_1C.set(0.2617994, 3.0);

    if (this->field_4) {
        this->kill();
    }
}

void lookat_target_controller::_frame_advance(Float)
{
    TRACE("lookat_target_controller::frame_advance");

    if (this->field_4) {
        auto v3 = this->field_18;
        auto v4 = this->field_1C.m_heading;

        euler_direction dir;
        dir.m_heading = v3;
        dir.m_pitch = v4;
        auto v9 = make_vector3d(dir);
        auto v5 = this->field_1C.m_pitch;
        auto v6 = this->field_8;
        dir.m_pitch = v9[1] * v5;
        auto v8 = v9[2] * v5;

        vector3d a2a;
        a2a[0] = v9[0] * v5 + this->field_C[0];
        a2a[1] = dir.m_pitch + this->field_C[1];
        a2a[2] = v8 + this->field_C[2];
        v6->set_abs_position(a2a);
        v6->look_at(this->field_C);
    }
}

bool lookat_target_controller::_is_mouselook_controller() const
{
    return false;
}

void lookat_target_controller_patch()
{
    {
        FUNC_ADDRESS(address, &lookat_target_controller::_frame_advance);
        set_vfunc(0x0088985C, address);
    }
}
