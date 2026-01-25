#include "base_ai_data.h"

#include "base_ai_core.h"
#include "common.h"
#include "memory.h"

VALIDATE_SIZE(base_ai_data, 0x18);
VALIDATE_OFFSET(base_ai_data, field_14, 0x14);

base_ai_data::base_ai_data(from_mash_in_place_constructor *a2)
{
    void (__fastcall *func)(void *, void *edx, from_mash_in_place_constructor *) = CAST(func, 0x006D9AF0);
    func(this, nullptr, a2);
}

void base_ai_data::post_entity_mash() {
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
