#pragma once

#include "mash_virtual_base.h"
#include "string_hash.h"

struct anim_key;

struct anim_record : mash_virtual_base {
    anim_key *my_key;
    string_hash field_8;

    anim_record();
    anim_record(from_mash_in_place_constructor *constructor);

    static void *native_vtable();
    void _destruct_mashed_class();
    anim_record *_scalar_deleting_destructor(uint32_t flags);
    int _get_virtual_type_enum() const;
    bool _is_subclass_of(mash::virtual_types_enum type) const;
    string_hash *_get_interactor_hash(string_hash *out) const;
    int _get_mash_sizeof() const;


    void _unmash(mash_info_struct *a2, void *a3);

    int get_mash_sizeof();
};

struct paired_anim_record : anim_record {
    string_hash field_C;

    paired_anim_record();
    paired_anim_record(from_mash_in_place_constructor *constructor);

    static void *native_vtable();
    void _destruct_mashed_class();
    void _unmash(mash_info_struct *info, void *owner);
    int _get_virtual_type_enum() const;
    bool _is_subclass_of(mash::virtual_types_enum type) const;
    int _get_mash_sizeof() const;
    string_hash *_get_target_hash(string_hash *out) const;
};

struct attach_anim_record : anim_record {
    int field_C;
    string_hash field_10;
    int field_14;
    int field_18;

    void _unmash(mash_info_struct *a2, void *a3);
};

extern void anim_record_patch();
