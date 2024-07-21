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

    ~string_hash_entry();

    void *operator new(size_t sz);

    void operator delete(void *ptr, size_t);

    void initialize(mash::allocation_scope, const char *a2, const string_hash *a3);

    void finalize(mash::allocation_scope ) {}

    void destruct_mashed_class();

    mString generate_text(const char *a3) const;

    void unmash(mash_info_struct *a1, void *a2);

    void custom_unmash(mash_info_struct *a2, void *a3);

    bool operator>(const string_hash_entry &a2) const {
        return this->field_0 > a2.field_0;
    }

    bool operator<(const string_hash_entry &a2) const {
        return this->field_0 < a2.field_0;
    }
};
