#include "motion_control_system.h"

#include "common.h"
#include "memory.h"

motion_control_system::motion_control_system()
{
#if STANDALONE_SYSTEM
    field_4 = true;
    field_5 = false;
    m_ent = nullptr;
#endif
}

void *motion_control_system::operator new(size_t size)
{
    return mem_alloc(size);
}

bool motion_control_system::is_active() const
{
    return field_4;
}

void motion_control_system::set_active(bool active)
{
    field_4 = active;
}

#if STANDALONE_SYSTEM
namespace {
bool __fastcall native_motion_is_active(motion_control_system *self, void *)
{
    return self->is_active();
}

void __fastcall native_motion_set_active(motion_control_system *self, void *, bool active)
{
    self->set_active(active);
}
}

void motion_control_system::initialize_native_vtable(std::intptr_t *table,
                                                    std::intptr_t destroy,
                                                    std::intptr_t advance)
{
    table[0] = destroy;
    table[1] = reinterpret_cast<std::intptr_t>(native_motion_is_active);
    table[2] = reinterpret_cast<std::intptr_t>(native_motion_set_active);
    table[3] = advance;
}
#endif
