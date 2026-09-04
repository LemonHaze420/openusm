#include "force_generator.h"

#include "vtbl.h"

force_generator::force_generator() {}

bool force_generator::is_active()
{
    bool(__fastcall * func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x4));
    return func(this);
}

void force_generator::frame_advance(Float a4)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(this->m_vtbl, 0xC));
    func(this, nullptr, a4);
}
