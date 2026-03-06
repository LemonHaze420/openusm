#pragma once

#include "string_hash.h"
#include "mvector.h"

struct from_mash_in_place_constructor;
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

    sound_alias(from_mash_in_place_constructor *a2);
};

struct sound_alias_database {

    mVector<sound_alias> field_0;

    sound_alias_database(from_mash_in_place_constructor *a2);

    void destruct_mashed_class();

    //0x005DD5D0
    void unmash(mash_info_struct *a1, void *a3);

    //0x005C9E50
    int *get_sound_alias(string_hash a2);
};

extern sound_alias_database *& s_sound_alias_database;

extern void sound_alias_database_patch();
