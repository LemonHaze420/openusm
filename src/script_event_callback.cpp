#include "script_event_callback.h"
#include "common.h"
#include "entity_base_vhandle.h"
#include "memory.h"
#include "script_object.h"
#include "vm_executable.h"
#include "vm_thread.h"
#include <cstring>

VALIDATE_SIZE(script_event_callback, 0x18);
namespace {
void instance_callback(script_instance_callback_reason_t reason, script_instance *, vm_thread *thread, void *data)
{
    auto *callback = static_cast<script_event_callback *>(data);
    if (static_cast<int>(reason) == 0)
        callback->instance = nullptr;
    else if (static_cast<int>(reason) == 1 && thread->field_1DC != nullptr)
        static_cast<event_callback *>(thread->field_1DC)->field_C = false;
}
void __fastcall finalize(script_event_callback *self, void *, bool release)
{
    self->~script_event_callback();
    if (release)
        mem_dealloc(self, sizeof(script_event_callback));
}
void __fastcall spawn(script_event_callback *self, void *, event *, entity_base_vhandle)
{
    if (self->field_C || self->instance == nullptr)
        return;
    if (self->field_D)
        self->instance->add_thread(self->executable, static_cast<const char *>(self->field_4));
    else {
        self->field_C = true;
        self->instance->add_thread(self, self->executable, static_cast<const char *>(self->field_4));
    }
}
bool __fastcall is_code(script_event_callback *, void *) { return false; }
bool __fastcall is_script(script_event_callback *, void *) { return true; }
std::intptr_t table[] = {reinterpret_cast<std::intptr_t>(&finalize), reinterpret_cast<std::intptr_t>(&spawn),
    reinterpret_cast<std::intptr_t>(&is_code), reinterpret_cast<std::intptr_t>(&is_script)};
}
script_event_callback::script_event_callback(script_instance *owner, const vm_executable *function,
    const char *arguments, bool one_shot)
    : event_callback(nullptr, one_shot), instance(owner), executable(function)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
    instance->register_callback(instance_callback, this);
    const auto size = executable->get_parms_stacksize();
    if (size != 0) {
        field_4 = mem_alloc(size);
        std::memcpy(field_4, arguments, size);
    }
}
script_event_callback::~script_event_callback()
{
    if (instance != nullptr)
        instance->unregister_callback(this);
    if (field_4 != nullptr)
        mem_dealloc(field_4, executable->get_parms_stacksize());
}
