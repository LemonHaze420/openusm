#include "rumble_struct.h"

#include "func_wrapper.h"

#include "common.h"

VALIDATE_SIZE(rumble_struct, 0x24);

rumble_struct::rumble_struct()
{
#if STANDALONE_SYSTEM
    this->field_20 = 0;
    this->field_0 = -1.0;
    this->field_4 = -1.0;
    this->field_8 = 0;
    this->field_C = 0;
    this->field_10 = -1;
    this->field_14 = -1.0;
    this->field_18 = 0;
    this->field_1C = 0;
    this->field_1D = 1;
    this->field_20 = 0;
#else
    void(__fastcall * func)(rumble_struct *) = CAST(func, 0x005BA400);
    func(this);
#endif
}
