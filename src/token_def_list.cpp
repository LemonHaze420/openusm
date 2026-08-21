#include "token_def_list.h"

#include "common.h"
#include "func_wrapper.h"
#include "token_def.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(token_def_list, 0x38);

token_def_list::token_def_list(from_mash_in_place_constructor *a2) : field_0(a2)
{
    TRACE("token_def_list::token_def_list");

    if constexpr (1) {
        this->initialize(mash::FROM_MASH);
    } else {
        THISCALL(0x005DEDA0, this, a2);
    }
}

void token_def_list::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->clear();
    }
}

void token_def_list::clear()
{
    for (int i = 0; i < 9; ++i) {
        this->field_14[i] = 0;
    }

    this->field_0.clear();
}

void token_def_list::unmash(mash_info_struct *a2, void *a3)
{
    TRACE("token_def_list::unmash");

    if constexpr (1) {
        a2->unmash_class_in_place(this->field_0, this);
    } else {
        THISCALL(0x005DD200, this, a2, a3);
    }
}

void *__fastcall token_def_list_ctor(token_def_list *mem, void *, from_mash_in_place_constructor *a2)
{
    auto *self = new (mem) token_def_list{a2};
    return self;
}

void token_def_list_patch()
{
    {
        FUNC_ADDRESS(address, &token_def_list::unmash);
        REDIRECT(0x005587B3, address);
    }

    {
        REDIRECT(0x005587C7, token_def_list_ctor);
    }
}
