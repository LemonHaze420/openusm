#include "weapon_inode.h"

#include "base_ai_core.h"
#include "common.h"
#include "func_wrapper.h"
#include "weapon_instance.h"

namespace ai {

VALIDATE_SIZE(weapon_inode, 0x38);

weapon_inode::weapon_inode() {}

void weapon_inode::activate(ai_core *a2) {
    this->field_8 = a2;
    this->field_C = a2->field_64;
    this->create_weapons();
}

void weapon_inode::create_weapons() {
    THISCALL(0x006CC150, this);
}

} // namespace ai
