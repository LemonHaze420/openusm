#include "patrol_manager.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(patrol_manager, 0x38u);

patrol_manager::patrol_manager()
{
    this->field_0 = 0;
    this->field_4.field_4 = nullptr;
    this->field_4.field_8 = 0;
    this->field_4.field_C = 0;
    this->field_4.field_14 = 0;
    this->field_4.field_18 = 0;
    this->field_4.field_1C = 0;
    this->field_4.field_20 = -1;
    this->field_4.field_24 = 0;
    this->field_4.field_25 = 0;
    this->field_4.field_26 = 0;
    this->field_4.field_27 = 0;
    this->field_34 = string_hash();
}

void patrol_manager::frame_advance(Float a2)
{
    THISCALL(0x005DD330, this, a2);
}
