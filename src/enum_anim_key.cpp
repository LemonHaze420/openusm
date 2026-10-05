#include "enum_anim_key.h"

#include "anim_record.h"
#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "utility.h"

VALIDATE_SIZE(anim_key, 0x4);
VALIDATE_SIZE(enum_anim_key, 0x8);

namespace {
void __fastcall key_destruct_mashed(anim_key *, void *)
{

}

void __fastcall key_unmash(anim_key *, void *, mash_info_struct *, void *)
{

}

void *__fastcall key_scalar_delete(anim_key *self, void *, unsigned int flags)
{
    self->~anim_key();
    if (flags & 1) {
        mash_virtual_base::operator delete(self, sizeof(*self));
    }
    return self;
}

void *__fastcall enum_key_scalar_delete(enum_anim_key *self, void *, unsigned int flags)
{
    self->~enum_anim_key();
    if (flags & 1) {
        mash_virtual_base::operator delete(self, sizeof(*self));
    }
    return self;
}

int __fastcall key_type(const anim_key *, void *) { return 144; }
int __fastcall enum_key_type(const enum_anim_key *, void *) { return 147; }
bool __fastcall key_is_subclass(const anim_key *, void *, mash::virtual_types_enum type) { return type == 573; }
bool __fastcall enum_key_is_subclass(const enum_anim_key *, void *, mash::virtual_types_enum type)
{
    return type == 144 || type == 573;
}
int __fastcall key_size(const anim_key *, void *) { return sizeof(anim_key); }
int __fastcall enum_key_size(const enum_anim_key *, void *) { return sizeof(enum_anim_key); }

int __fastcall key_compare(const anim_key *self, void *, const anim_key *other)
{

    auto other_type = other->get_virtual_type_enum();
    if (self->get_virtual_type_enum() < other_type) {
        return -1;
    }
    return self->get_virtual_type_enum() > other->get_virtual_type_enum();
}

int __fastcall enum_key_compare(const enum_anim_key *self, void *, const anim_key *other)
{

    if (self->get_virtual_type_enum() != other->get_virtual_type_enum()) {
        return key_compare(self, nullptr, other);
    }
    int value = static_cast<const enum_anim_key *>(other)->field_4.field_0;
    if (self->field_4.field_0 < value) {
        return -1;
    }
    return self->field_4.field_0 > value;
}
}

void *anim_key::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&key_destruct_mashed),
        reinterpret_cast<void *>(&key_unmash),
        reinterpret_cast<void *>(&key_scalar_delete),
        reinterpret_cast<void *>(&key_type),
        reinterpret_cast<void *>(&key_is_subclass),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        reinterpret_cast<void *>(&key_compare),
        reinterpret_cast<void *>(&key_size),
    };
    return table;
}

void *enum_anim_key::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&key_destruct_mashed),
        reinterpret_cast<void *>(&key_unmash),
        reinterpret_cast<void *>(&enum_key_scalar_delete),
        reinterpret_cast<void *>(&enum_key_type),
        reinterpret_cast<void *>(&enum_key_is_subclass),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        reinterpret_cast<void *>(&enum_key_compare),
        reinterpret_cast<void *>(&enum_key_size),
    };
    return table;
}

enum_anim_key::enum_anim_key()
{

    this->m_vtbl = reinterpret_cast<std::intptr_t>(
        STANDALONE_SYSTEM ? native_vtable() : reinterpret_cast<void *>(0x00873908));
}

enum_anim_key::enum_anim_key(enum_anim_key::key_enum a2)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x00873908;
#endif
    this->field_4 = a2;
}

anim_key::anim_key()
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x008738E8;
#endif
}

int anim_key::get_compare_value(const anim_key *a2)
{
    int(__fastcall * func)(anim_key *, void *, const anim_key *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this, nullptr, a2);
}

int anim_key::get_mash_sizeof()
{
    int(__fastcall * func)(anim_key *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    return func(this);
}

int anim_key::compare(anim_key *&a1, anim_record *&a2)
{
    return a1->get_compare_value(a2->my_key);
}
