#include "generic_interaction.h"

#include "common.h"
#include "memory.h"
#include "resource_manager.h"
#include "variables.h"

#include <array>
#include <algorithm>

VALIDATE_SIZE(generic_interaction, 0x54u);

namespace {
void __fastcall generic_unmash(generic_interaction *self, int, mash_info_struct *info, void *context)
{
    self->generic_interaction::_unmash(info, context);
}

void __fastcall generic_destroy(generic_interaction *self)
{
    self->field_48.destruct_mashed_class();
    self->interaction::destruct_mashed_class();
}

void *__fastcall generic_delete(generic_interaction *self, int, unsigned flags)
{
    generic_destroy(self);
    if ((flags & 1) != 0) {
        mem_dealloc(self, sizeof(generic_interaction));
    }
    return self;
}

uint32_t __fastcall generic_type(const generic_interaction *)
{
    return 546;
}

bool __fastcall generic_subclass(const generic_interaction *, int, mash::virtual_types_enum type)
{
    return type == 547 || type == 573;
}

int __fastcall generic_size(const generic_interaction *)
{
    return sizeof(generic_interaction);
}

ai_interaction_data *__fastcall generic_resource(generic_interaction *self)
{
    return self->field_50;
}
}

void *generic_interaction::native_vtable()
{
    static auto table = [] {
        std::array<void *, 9> result {};
        std::copy_n(static_cast<void **>(interaction::native_vtable()), 8, result.data());
        result[0] = bit_cast<void *>(&generic_destroy);
        result[1] = bit_cast<void *>(&generic_unmash);
        result[2] = bit_cast<void *>(&generic_delete);
        result[3] = bit_cast<void *>(&generic_type);
        result[7] = bit_cast<void *>(&generic_size);
        result[4] = bit_cast<void *>(&generic_subclass);
        result[8] = bit_cast<void *>(&generic_resource);
        return result;
    }();
    return table.data();
}

generic_interaction::generic_interaction(const resource_key &resource, interaction_type_enum kind, string_hash id)
    : interaction(kind, id), field_48(resource), field_50(nullptr)
{
    assert(resource.get_type() == RESOURCE_KEY_TYPE_AI_INTERACTION);
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x0087E380;
    if (!g_is_the_packer) {
        field_50 = reinterpret_cast<ai_interaction_data *>(
            resource_manager::get_resource(field_48, nullptr, nullptr));
    }
}

generic_interaction::generic_interaction(from_mash_in_place_constructor *tag)
    : interaction(tag), field_48(tag)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x0087E380;
    field_50 = reinterpret_cast<ai_interaction_data *>(
        resource_manager::get_resource(field_48, nullptr, nullptr));
}

void generic_interaction::_unmash(mash_info_struct *info, void *context)
{
    interaction::_unmash(info, context);
    info->unmash_class_in_place(field_48, this);
}
