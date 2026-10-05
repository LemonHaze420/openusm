#include "code_event_callback.h"
#include "common.h"
#include "memory.h"

namespace {
void __fastcall finalize_callback(code_event_callback *self, void *, bool release)
{
    if (release)
        mem_dealloc(self, sizeof(code_event_callback));
}
void __fastcall spawn_callback(code_event_callback *self, void *, event *payload, entity_base_vhandle recipient)
{
    self->m_callback(payload, recipient, self->field_4);
}
bool __fastcall is_code_callback(code_event_callback *, void *) { return true; }
bool __fastcall is_script_callback(code_event_callback *, void *) { return false; }
std::intptr_t table[] = {
    reinterpret_cast<std::intptr_t>(&finalize_callback), reinterpret_cast<std::intptr_t>(&spawn_callback),
    reinterpret_cast<std::intptr_t>(&is_code_callback), reinterpret_cast<std::intptr_t>(&is_script_callback)
};
}

code_event_callback::code_event_callback(void (*a2)(event *, entity_base_vhandle, void *), void *a1, bool a4)
    : event_callback(a1, a4)
{
    if constexpr (STANDALONE_SYSTEM)
        this->m_vtbl = reinterpret_cast<std::intptr_t>(table);
    else
        this->m_vtbl = 0x00883090;
    this->m_callback = a2;
}
