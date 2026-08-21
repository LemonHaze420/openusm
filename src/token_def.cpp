#include "token_def.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(token_def, 0x30u);

token_def::token_def(from_mash_in_place_constructor *a2) : field_0(a2)
{
    this->initialize(mash::FROM_MASH);
}

token_def::~token_def()
{
    this->finalize(mash::ALLOCATED);
}

void token_def::initialize(mash::allocation_scope a2)
{
    if (a2) {
        this->field_28 = nullptr;
    } else {
        this->clear();
    }
}

void token_def::finalize(mash::allocation_scope)
{
    this->show_dot(false);
    this->clear();
}

void token_def::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    this->field_0.destruct_mashed_class();
}

void token_def::clear()
{
    this->field_0 = {};
    this->field_10 = vector3d{0.0, 0.0, 0.0};
    this->field_20 = -1;
    this->type = -1;
    this->token_id = -1;
    this->field_28 = nullptr;
    this->map_dot = nullptr;
}

void token_def::show_dot(bool a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x005C9CE0, this, a2);
    }
}

void token_def::unmash(mash_info_struct *a2, void *)
{
    a2->unmash_class_in_place(this->field_0, this);
}
