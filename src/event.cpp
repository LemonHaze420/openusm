#include "event.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "vtbl.h"
#include "wds.h"

VALIDATE_SIZE(event, 0xC);

namespace {
void __fastcall destruct_event(event *self, void *)
{
    self->field_4.destruct_mashed_class();
}
void __fastcall unmash_event(event *self, void *, mash_info_struct *info, void *)
{
    self->field_4.unmash(info, self);
}
int __fastcall event_size(event *, void *)
{
    return sizeof(event);
}
void __fastcall finalize_event(event *self, void *, bool release)
{
    if (release) {
        int(__fastcall * size)(event *, void *) = CAST(size, get_vfunc(self->m_vtbl, 0x1C));
        mem_dealloc(self, size(self, nullptr));
    }
}
int __fastcall event_type_id(event *, void *)
{
    return 539;
}
bool __fastcall event_is_subclass(event *, void *, int type)
{
    return type == 573;
}
bool __fastcall event_is_or_subclass(event *, void *, int type)
{
    return type == 539 || type == 573;
}
void __fastcall raise_event(event *self, void *)
{
    self->field_8 = g_world_ptr->time_manager.field_C;
}
std::intptr_t table[] = {reinterpret_cast<std::intptr_t>(&destruct_event),
                         reinterpret_cast<std::intptr_t>(&unmash_event),
                         reinterpret_cast<std::intptr_t>(&finalize_event),
                         reinterpret_cast<std::intptr_t>(&event_type_id),
                         reinterpret_cast<std::intptr_t>(&event_is_subclass),
                         reinterpret_cast<std::intptr_t>(&event_is_or_subclass),
                         reinterpret_cast<std::intptr_t>(&raise_event),
                         reinterpret_cast<std::intptr_t>(&event_size)};
}  // namespace

void *event::native_vtable()
{
    return table;
}

event::event(string_hash id)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = reinterpret_cast<std::intptr_t>(table);
        field_4 = id;
        field_8 = 0;
    } else {
        THISCALL(0x0048ABA0, this, id);
    }
}

void event::_finalize(bool release)
{
    void(__fastcall * func)(void *, void *, bool) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, nullptr, release);
}
