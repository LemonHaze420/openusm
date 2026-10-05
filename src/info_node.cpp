#include "info_node.h"

#include "base_ai_core.h"
#include "common.h"
#include "entity_base.h"
#include "func_wrapper.h"
#include "vtbl.h"

namespace ai {

VALIDATE_SIZE(info_node, 0x1C);

namespace {
void __fastcall native_destruct(info_node *self, void *)
{
    self->_destruct_mashed_class();
}

void __fastcall native_unmash(info_node *self, void *, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}

void *__fastcall native_delete(info_node *self, void *, unsigned flags)
{
    self->~info_node();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(info_node));
    return self;
}

unsigned __fastcall native_type(info_node *, void *)
{
    return 537;
}
bool __fastcall native_subclass(info_node *, void *, unsigned type)
{
    return type == 573;
}
bool __fastcall native_is_or_subclass(info_node *self, void *, unsigned type)
{
    return self->_is_or_is_subclass_of(static_cast<mash::virtual_types_enum>(type));
}

bool __fastcall native_needs_advance(info_node *, void *)
{
    return false;
}
void __fastcall native_advance(info_node *, void *, Float) {}
void __fastcall native_activate(info_node *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall native_deactivate(info_node *, void *) {}
void __fastcall native_reset(info_node *self, void *)
{
    self->_reset();
}
int __fastcall native_size(info_node *, void *)
{
    return sizeof(info_node);
}
}  // namespace

void *info_node::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_destruct),
        reinterpret_cast<void *>(&native_unmash),
        reinterpret_cast<void *>(&native_delete),
        reinterpret_cast<void *>(&native_type),
        reinterpret_cast<void *>(&native_subclass),
        reinterpret_cast<void *>(&native_is_or_subclass),
        reinterpret_cast<void *>(&native_needs_advance),
        reinterpret_cast<void *>(&native_advance),
        reinterpret_cast<void *>(&native_activate),
        reinterpret_cast<void *>(&native_deactivate),
        reinterpret_cast<void *>(&native_reset),
        reinterpret_cast<void *>(&native_size),
    };
    return table;
}

info_node::info_node()
{
    this->initialize(mash::ALLOCATED);
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[537]);
}

info_node::info_node(from_mash_in_place_constructor *constructor) : field_4(constructor), my_param_block(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[537]);
}

void info_node::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->field_8 = nullptr;
    }
}

void info_node::_unmash(mash_info_struct *a1, void *)
{
    TRACE("info_node::unmash");

    a1->unmash_class_in_place(this->field_4, this);
    a1->unmash_class_in_place(this->my_param_block, this);
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

void info_node::_reset()
{
    deactivate();
    activate(field_8);
}

void info_node::_destruct_mashed_class()
{
    field_4.destruct_mashed_class();
    my_param_block.destruct_mashed_class();
}

void info_node::deactivate()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x24));
    func(this);
}

void info_node::reset()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x28));
    func(this);
}

int info_node::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x2C));
    return func(this);
}

}  // namespace ai

void info_node_patch()
{
    {
        FUNC_ADDRESS(address, &ai::info_node::_unmash);
        set_vfunc(0x0087BB40, address);
    }
}
