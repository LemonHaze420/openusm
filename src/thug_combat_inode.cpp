#include "thug_combat_inode.h"

#include "base_ai_core.h"
#include "common.h"

namespace ai {

VALIDATE_SIZE(thug_combat_inode::blocking_info_t, 0x14u);
VALIDATE_SIZE(thug_combat_inode, 0x33Cu);

void thug_combat_inode::blocking_info_t::sub_4458A0(ai_core *a2, const string_hash &a3, const string_hash &a4,
                                                    const string_hash &a5)
{
    this->field_0 = 0.0;
    this->field_4 = 0.0;

    float v15 = 0.0f;
    this->field_8 = a2->field_50.get_optional_pb_float(a3, v15, nullptr) / 30.0f;

    v15 = 1.0f;
    this->field_C = a2->field_50.get_optional_pb_float(a4, v15, nullptr) / 30.0f;

    v15 = 0.0f;
    this->field_10 = a2->field_50.get_optional_pb_float(a5, v15, nullptr) / 30.0f;
}

void thug_combat_inode::_activate(ai_core *a2)
{
    combat_inode::_activate(a2);
    this->field_328.sub_4458A0(a2,
                               thug_combat_inode::punch_to_block_timer_hash,
                               thug_combat_inode::punch_to_block_limit_hash,
                               thug_combat_inode::punch_blocking_timer_hash);
}

}  // namespace ai
