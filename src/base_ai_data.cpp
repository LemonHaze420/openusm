#include "base_ai_data.h"

#include "actor.h"
#include "base_ai_core.h"
#include "common.h"
#include "core_ai_resource.h"
#include "memory.h"
#include "resource_manager.h"
#include "variables.h"

VALIDATE_SIZE(base_ai_data, 0x18);
VALIDATE_OFFSET(base_ai_data, field_14, 0x14);

base_ai_data::base_ai_data(from_mash_in_place_constructor *constructor)
    : field_0(constructor), field_8(constructor)
{
    if (field_0.m_hash != string_hash{}) {
        auto *resource = !g_is_the_packer
            ? reinterpret_cast<ai::core_ai_resource *>(
                resource_manager::get_resource(field_0, nullptr, nullptr))
            : nullptr;
        field_14 = resource != nullptr
            ? new (mem_alloc(sizeof(ai::ai_core))) ai::ai_core{
                resource, &field_8, global_transfer_variable_the_actor}
            : nullptr;
    }
}

base_ai_data::~base_ai_data()
{
    if (field_14 != nullptr) {
        field_14->~ai_core();
        mem_dealloc(field_14, sizeof(*field_14));
    }
}

void base_ai_data::post_entity_mash()
{
    this->field_14->post_entity_mash();
}

void base_ai_data::destruct_mashed_class()
{
    auto *v2 = this->field_14;
    if (v2 != nullptr) {
        (*this->field_14).~ai_core();

        mem_dealloc(v2, sizeof(*v2));
    }

    this->field_0.destruct_mashed_class();
    this->field_8.destruct_mashed_class();
}

void base_ai_data::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_8, this);
}
