#pragma once

#include "mash.h"

struct mash_info_struct;

struct ghetto_mash_file_header {
    char field_0[4];
    int field_4;

    ghetto_mash_file_header(int) {}

    ghetto_mash_file_header(mash::allocation_scope a2, const char *a3, int a4);

    //0x00420620
    void initialize(mash::allocation_scope a2, const char *a3, int a4);

    //0x00420650
    bool validate(const char *a2, int a3) const;

    //0x00420610
    void unmash(mash_info_struct *, void *);
};

extern void ghetto_mash_file_header_patch();
