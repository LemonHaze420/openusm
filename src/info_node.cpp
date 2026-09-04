#include "info_node.h"

#include "base_ai_core.h"
#include "common.h"
#include "entity_base.h"
#include "func_wrapper.h"
#include "vtbl.h"

namespace ai {

VALIDATE_SIZE(info_node, 0x1C);

info_node::info_node()
{
    this->initialize(mash::ALLOCATED);
}

info_node::info_node(from_mash_in_place_constructor *a2)
{
    THISCALL(0x006D9930, this, a2);
}

void info_node::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->field_8 = nullptr;
    }
}

bool info_node::does_need_advance() const
{
    bool(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

void info_node::frame_advance(Float a2)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this, nullptr, a2);
}

void info_node::activate(ai_core *a2)
{
    void(__fastcall * func)(void *, void *, ai_core *) = CAST(func, get_vfunc(m_vtbl, 0x20));
    func(this, nullptr, a2);
}

void info_node::_activate(ai_core *a2)
{
    this->field_8 = a2;
    this->field_C = a2->field_64;
}

void info_node::reset()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x28));
    func(this);
}

void info_node::deactivate()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x24));
    func(this);
}

}  // namespace ai
