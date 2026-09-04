#include "event_callback.h"

#include "common.h"
#include "event_manager.h"
#include "memory.h"
#include "vtbl.h"

VALIDATE_SIZE(event_callback, 0x10);

event_callback::event_callback(void *a2, bool a3)
{
    this->m_vtbl = 0x00882CD0;
    this->field_4 = a2;
    this->field_D = a3;
    this->field_C = false;
    auto v4 = (event_callback::id_counter++ == -1);
    this->id = event_callback::id_counter;
    if (v4) {
        this->id = ++event_callback::id_counter;
    }

    assert(!event_manager::callback_exists(id));
}

void event_callback::_finalize(bool a2)
{
    void(__fastcall * func)(void *, void *edx, bool) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this, nullptr, a2);
}

void *event_callback::operator new(size_t size)
{
    return mem_alloc(size);
}

void event_callback::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}
