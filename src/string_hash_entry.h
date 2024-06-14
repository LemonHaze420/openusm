#pragma once

#include "mash.h"
#include "mstring.h"
#include "string_hash.h"

struct mash_info_struct;

struct string_hash_entry {
    string_hash field_0;
    mString field_4;

    string_hash_entry();

    string_hash_entry(const char *a2, const string_hash &a3);

    void initialize(mash::allocation_scope, const char *a2, const string_hash *a3);

    mString generate_text(const char *a3) const;

    void unmash(mash_info_struct *a1, void *a2);

    void custom_unmash(mash_info_struct *a2, void *a3);
};
