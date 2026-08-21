#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"
#include "enum_anim_key.h"
#include "mash.h"
#include "mvector.h"
#include "mVectorBasic.h"

struct ai_adv_strength_test_data;
struct mash_info_struct;
struct anim_record;
struct from_mash_in_place_constructor;
struct interact_sound_entry;
struct attach_interact_data;
struct resource_pack_slot;

struct ai_interaction_data {
    vector3d field_0;
    float field_C[4];
    mVector<anim_record> field_1C;
    bool field_30;
    bool field_31;
    int field_34;
    float field_38;
    float field_3C;
    float field_40;
    string_hash field_44;
    string_hash field_48;
    string_hash field_4C;
    string_hash field_50;
    mVector<interact_sound_entry> field_54;
    attach_interact_data *field_68;
    string_hash field_6C;
    vector3d field_70;
    resource_pack_slot *field_7C;
    mVectorBasic<vhandle_type<actor>> field_80;
    bool field_90;
    mVector<ai_adv_strength_test_data> my_adv_str_test_list;

    ai_interaction_data(from_mash_in_place_constructor *);

    void initialize(mash::allocation_scope scope);

    //0x0069AA50
    void unregister_interactor(vhandle_type<actor> a2);

    anim_record *does_anim_exist(const anim_key *a2, bool a3);

    //0x0069D600
    anim_record *does_anim_exist(enum_anim_key::key_enum a2, bool a3);

    //0x0069F2F0
    void register_interactor(vhandle_type<actor> a3);

    void destruct_mashed_class();

    void unmash(mash_info_struct *a1, void *);

    //0x0068DDF0
    string_hash get_anim_hash_name(const anim_record *a2, bool a3);

    //0x0069D6A0
    void *get_anim_ptr(const anim_key *the_anim_key, bool a3);
};

extern void ai_interaction_data_patch();
