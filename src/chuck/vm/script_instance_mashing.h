#pragma once

#include "mashable_vector.h"
#include "string_hash.h"

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct po;
struct script_executable;
struct script_instance_mash_record;
struct script_member_initializer;

extern bool initialize_game_init_instances(const script_executable *se, string_hash a2);

struct script_instance_info {
    mashable_vector<script_instance_mash_record> field_0;
    mashable_vector<script_member_initializer> field_8;
    char *field_10;
    int field_14;

    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);

    void un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *a5);

    bool initialize_single(const script_executable *a2, string_hash a3, const po &a4);

    bool initialize(const script_executable *a2, const po &a3);
};
