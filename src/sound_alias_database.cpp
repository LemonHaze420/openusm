#include "sound_alias_database.h"

#include "func_wrapper.h"
#include "utility.h"
#include "common.h"
#include "mash_info_struct.h"

#include <cassert>

VALIDATE_SIZE(sound_alias, 0x20);
VALIDATE_SIZE(sound_alias_database, 0x14);

sound_alias_database::sound_alias_database() {}

void sound_alias_database::destruct_mashed_class()
{
    this->field_0.destruct_mashed_class();
}

void sound_alias_database::unmash(mash_info_struct *a1, void *a3)
{
    a1->unmash_class_in_place(this->field_0, a3);
}

int *sound_alias_database::get_sound_alias(string_hash a2) {
    sp_log("sound_alias_database::get_sound_alias:");

    assert(this);

    return (int *) THISCALL(0x005C9E50, this, a2);
}

void sound_alias_database_patch() {
    FUNC_ADDRESS(address, &sound_alias_database::get_sound_alias);
    REDIRECT(0x005204D3, address);
}
