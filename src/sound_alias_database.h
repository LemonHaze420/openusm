#pragma once

#include "string_hash.h"
#include "mvector.h"

struct mash_info_struct;

struct sound_alias
{
    string_hash field_0;
    string_hash field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
};

struct sound_alias_database {

    mVector<sound_alias> field_0;
    sound_alias_database();

    void destruct_mashed_class();

    //0x005DD5D0
    void unmash(mash_info_struct *a1, void *a3);

    //0x005C9E50
    int *get_sound_alias(string_hash a2);
};

inline Var<sound_alias_database *> s_sound_alias_database {0x0095C854};

extern void sound_alias_database_patch();
