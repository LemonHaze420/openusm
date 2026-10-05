#include <cstddef>

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

void patrol_manager::frame_advance(Float)
{
    auto &sequence = field_4;
    const auto *first = reinterpret_cast<const char *>(sequence.field_0);
    const auto *last = reinterpret_cast<const char *>(sequence.field_4);
    const int count = first != nullptr && last != nullptr ? static_cast<int>((last - first) / 8) : 0;

    if (sequence.field_20 >= count) {
        if (sequence.field_0 != 0) {
            operator delete(reinterpret_cast<void *>(sequence.field_0));
        }
        sequence.field_0 = 0;
        sequence.field_4 = nullptr;
        sequence.field_8 = 0;
        sequence.field_20 = -1;
        sequence.field_24 = 0;
        sequence.field_25 = 0;
        sequence.field_26 = 1;
        sequence.field_27 = 1;
    }
    if (sequence.field_26) {
        field_0 = 0;
    }
}
