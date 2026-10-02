#include "base_full_target_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {
VALIDATE_SIZE(base_full_target_inode, 0x84);

base_full_target_inode::base_full_target_inode()
{
    if constexpr (STANDALONE_SYSTEM) {
        this->field_1C = nullptr;
        this->field_20 = 3;
        this->field_24 = 0;
        this->field_28 = nullptr;
        this->field_2C = 0;
        this->field_30 = 0;
        this->field_34 = {0};
        this->field_38 = 7;
        for (auto &field : this->field_3C) {
            field = 0;
        }
        this->field_3C[3] = static_cast<int>(0x7F7FFFFFu);
        this->field_3C[4] = 1;
    } else {
        THISCALL(0x006A1910, this);
    }
}

base_full_target_inode::base_full_target_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x006A1990, this, a2);
}

vhandle_type<actor> base_full_target_inode::quick_targeting()
{
    return this->field_34;
}

bool base_full_target_inode::is_target_known()
{
    return this->field_38 <= 5;
}

}  // namespace ai
