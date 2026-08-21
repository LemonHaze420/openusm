#pragma once

#include "string_hash.h"

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct po;
struct script_executable;

extern bool initialize_game_init_instances(const script_executable *se, string_hash a2);

struct script_instance_info {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    char *field_10;
    int field_14;

    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);

    bool initialize_single(const script_executable *a2, string_hash a3, const po &a4);

    bool initialize(const script_executable *a2, const po &a3);
};
