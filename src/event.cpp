#include "event.h"

#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"

VALIDATE_SIZE(event, 0xC);

event::event(string_hash a2) {
    THISCALL(0x0048ABA0, this, a2);
}

void event::_finalize(bool a2) {
    void (__fastcall *func)(void *, void *, bool) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, nullptr, a2);
}
