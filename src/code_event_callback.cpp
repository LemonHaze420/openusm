#include "code_event_callback.h"

code_event_callback::code_event_callback(void (*a2)(event *, entity_base_vhandle, void *), void *a1, bool a4)
    : event_callback(a1, a4)
{
    this->m_vtbl = 0x00883090;
    this->m_callback = a2;
}
