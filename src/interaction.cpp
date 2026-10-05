#include "interaction.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "generic_interaction.h"
#include "memory.h"
#include "vtbl.h"

#include <array>
#include <new>
#include <cstdlib>

VALIDATE_SIZE(interaction, 0x48);

namespace {
void __fastcall interaction_unmash(interaction *self, int, mash_info_struct *info, void *context)
{
    self->interaction::_unmash(info, context);
}

void __fastcall interaction_destroy(interaction *self)
{
    self->destruct_mashed_class();
}

void *__fastcall interaction_delete(interaction *self, int, unsigned flags)
{
    self->destruct_mashed_class();
    if ((flags & 1) != 0) {
        mem_dealloc(self, sizeof(interaction));
    }
    return self;
}

uint32_t __fastcall interaction_type(const interaction *)
{
    return 547;
}

int __fastcall interaction_size(const interaction *)
{
    return sizeof(interaction);
}

bool __fastcall interaction_subclass(const interaction *, int, mash::virtual_types_enum type)
{
    return type == 573;
}

bool __fastcall interaction_is_or_subclass(const interaction *self, int, mash::virtual_types_enum type)
{
    return self->mash_virtual_base::get_virtual_type_enum() == static_cast<uint32_t>(type) ||
        type == 573 || (self->mash_virtual_base::get_virtual_type_enum() == 546 && type == 547);
}

bool __fastcall interaction_contains(const interaction *self, int, const vector3d *position, actor *owner)
{
    return self->is_inside_trigger_region(position, owner);
}
}

void *interaction::native_vtable()
{
    static const std::array<void *, 8> table {
        bit_cast<void *>(&interaction_destroy),
        bit_cast<void *>(&interaction_unmash),
        bit_cast<void *>(&interaction_delete),
        bit_cast<void *>(&interaction_type),
        bit_cast<void *>(&interaction_subclass),
        bit_cast<void *>(&interaction_is_or_subclass),
        bit_cast<void *>(&interaction_contains),
        bit_cast<void *>(&interaction_size),
    };
    return const_cast<void **>(table.data());
}

interaction::interaction(from_mash_in_place_constructor *tag)
    : field_4(tag), field_18(tag), field_2C(tag)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x0087B8D8;
}

interaction::interaction(interaction_type_enum kind, string_hash id)
    : field_4(), field_18(), field_28(kind), field_2C(id)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x0087B8D8;
    field_18.m_data = nullptr;
    field_18.m_max_size = 0;
    field_30 = 0;
    field_34 = 0;
    field_38 = 0.0f;
    field_3C = 0;
    field_40 = 0;
    field_44 = true;
    field_45 = false;
    field_46 = false;
}

int interaction::get_approach()
{
    return field_40;
}

void interaction::set_enabled(bool enabled)
{
    field_44 = enabled;
    if (!enabled) {
        field_38 = 0;
    }
}

void interaction::_unmash(mash_info_struct *info, void *)
{
    info->unmash_class_in_place(field_4, this);
    info->unmash_class_in_place(field_18, this);
    info->unmash_class_in_place(field_2C, this);
}

int interaction::get_virtual_type_enum()
{
    return mash_virtual_base::get_virtual_type_enum();
}

int interaction::get_mash_sizeof() const
{
    auto fn = bit_cast<int(__fastcall *)(const interaction *)>(get_vfunc(m_vtbl, 0x1C));
    return fn(this);
}

bool interaction::is_inside_trigger_region(const vector3d *position, actor *owner) const
{
    for (auto *region : field_4) {
        if (region->is_inside_trigger_region(position, owner)) {
            return true;
        }
    }
    return false;
}

void interaction::destruct_mashed_class()
{
    if (field_4.field_10) {
        for (int i = 0; i < field_4.m_size; ++i) {
            auto *region = field_4.m_data[i];
            if (field_4.is_pointer_in_mash_image(region)) {
                region->destruct_mashed_class();
            } else if (region != nullptr) {
                region->delete_owned();
            }
            field_4.m_data[i] = nullptr;
        }
    }
    if (!field_4.is_pointer_in_mash_image(field_4.m_data)) {
        mem_dealloc(field_4.m_data, sizeof(trigger_region *) * field_4.m_max_size);
    }
    field_4.m_data = nullptr;
    field_4.m_max_size = 0;
    field_4.mContainer_base::clear();
    if (!field_18.is_pointer_in_mash_image(field_18.m_data)) {
        ::operator delete[](field_18.m_data);
    }
    field_18.m_data = nullptr;
    field_18.m_max_size = 0;
    field_18.clear();
    field_2C.destruct_mashed_class();
}

void *interaction::construct_native_in_place(uint32_t type, mash_virtual_base *storage, int storage_size)
{
    assert(storage != nullptr);
    auto *tag = static_cast<from_mash_in_place_constructor *>(nullptr);
    switch (type) {
    case 545:
        assert(storage_size >= static_cast<int>(sizeof(box_region)));
        return ::new (storage) box_region{tag};
    case 546:
        assert(storage_size >= static_cast<int>(sizeof(generic_interaction)));
        return ::new (storage) generic_interaction{tag};
    case 547:
        assert(storage_size >= static_cast<int>(sizeof(interaction)));
        return ::new (storage) interaction{tag};
    case 548:
        assert(storage_size >= static_cast<int>(sizeof(named_trigger_box_region)));
        return ::new (storage) named_trigger_box_region{tag};
    case 549:
        assert(storage_size >= static_cast<int>(sizeof(point_dist_region)));
        return ::new (storage) point_dist_region{tag};
    default:
        assert(false && "Not an interaction mash graph type");
        std::abort();
    }
}
