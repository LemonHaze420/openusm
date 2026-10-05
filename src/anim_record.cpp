#include "anim_record.h"

#include "common.h"
#include "enum_anim_key.h"
#include "mash_info_struct.h"
#include "func_wrapper.h"
#include "utility.h"
#include "vtbl.h"
#include "mash_config.h"

VALIDATE_SIZE(anim_record, 0xC);
VALIDATE_SIZE(paired_anim_record, 0x10);
VALIDATE_OFFSET(paired_anim_record, field_C, 0xC);
VALIDATE_SIZE(attach_anim_record, 0x1C);
VALIDATE_OFFSET(attach_anim_record, field_10, 0x10);

anim_record::anim_record()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
        this->my_key = nullptr;
    } else {
        THISCALL(0x00695CE0, this);
    }
}

anim_record::anim_record(from_mash_in_place_constructor *constructor)
    : field_8(constructor)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(
        STANDALONE_SYSTEM ? native_vtable() : reinterpret_cast<void *>(0x00873928));
}

void *anim_record::native_vtable()
{
    static void *table[] = {
        func_address(&anim_record::_destruct_mashed_class),
        func_address(&anim_record::_unmash),
        func_address(&anim_record::_scalar_deleting_destructor),
        func_address(&anim_record::_get_virtual_type_enum),
        func_address(&anim_record::_is_subclass_of),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&anim_record::_get_interactor_hash),
        func_address(&anim_record::_get_mash_sizeof),
    };
    return table;
}

void anim_record::_destruct_mashed_class()
{
    this->field_8.destruct_mashed_class();
    if (this->my_key != nullptr) {
        using destroy_callback = void (__fastcall *)(anim_key *, void *);
        auto destroy = reinterpret_cast<destroy_callback>(get_vfunc(this->my_key->m_vtbl, 0));
        destroy(this->my_key, nullptr);
        this->my_key = nullptr;
    }
}

anim_record *anim_record::_scalar_deleting_destructor(uint32_t flags)
{
    this->~anim_record();
    if ((flags & 1) != 0) {
        mash_virtual_base::operator delete(this, sizeof(*this));
    }
    return this;
}

int anim_record::_get_virtual_type_enum() const
{
    return 145;
}

bool anim_record::_is_subclass_of(mash::virtual_types_enum type) const
{
    return type == static_cast<mash::virtual_types_enum>(573);
}

string_hash *anim_record::_get_interactor_hash(string_hash *out) const
{
    *out = this->field_8;
    return out;
}

int anim_record::_get_mash_sizeof() const
{
    return sizeof(*this);
}

paired_anim_record::paired_anim_record()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(
        STANDALONE_SYSTEM ? native_vtable() : reinterpret_cast<void *>(0x00873948));
}

paired_anim_record::paired_anim_record(from_mash_in_place_constructor *constructor)
    : anim_record(constructor), field_C(constructor)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(
        STANDALONE_SYSTEM ? native_vtable() : reinterpret_cast<void *>(0x00873948));
}

void *paired_anim_record::native_vtable()
{
    static void *table[] = {
        func_address(&paired_anim_record::_destruct_mashed_class),
        func_address(&paired_anim_record::_unmash),
        func_address(&anim_record::_scalar_deleting_destructor),
        func_address(&paired_anim_record::_get_virtual_type_enum),
        func_address(&paired_anim_record::_is_subclass_of),
        func_address(&mash_virtual_base::_is_or_is_subclass_of),
        func_address(&anim_record::_get_interactor_hash),
        func_address(&paired_anim_record::_get_mash_sizeof),
        func_address(&paired_anim_record::_get_target_hash),
    };
    return table;
}

void paired_anim_record::_destruct_mashed_class()
{
    this->field_C.destruct_mashed_class();
    anim_record::_destruct_mashed_class();
}

void paired_anim_record::_unmash(mash_info_struct *info, void *owner)
{
    anim_record::_unmash(info, owner);
    info->unmash_class_in_place(this->field_C, this);
}

int paired_anim_record::_get_virtual_type_enum() const
{
    return 146;
}

bool paired_anim_record::_is_subclass_of(mash::virtual_types_enum type) const
{
    return type == static_cast<mash::virtual_types_enum>(145) ||
           type == static_cast<mash::virtual_types_enum>(573);
}

int paired_anim_record::_get_mash_sizeof() const
{
    return sizeof(*this);
}

string_hash *paired_anim_record::_get_target_hash(string_hash *out) const
{
    *out = this->field_C;
    return out;
}

void anim_record::_unmash(mash_info_struct *a2, void *)
{
    a2->unmash_class_in_place(this->field_8, this);
    if (this->my_key != nullptr) {
        a2->unmash_class(this->my_key,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
                );
    }
}

int anim_record::get_mash_sizeof()
{
    int (__fastcall *func)(anim_record *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    return func(this);
}

void attach_anim_record::_unmash(mash_info_struct *a2, void *a3)
{
    anim_record::_unmash(a2, a3);
    a2->unmash_class_in_place(this->field_10, this);
}


void anim_record_patch()
{
    {
        FUNC_ADDRESS(address, &anim_record::_unmash);
        set_vfunc(0x0087392C, address);
    }

}
