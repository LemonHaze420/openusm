#include "region_lookup_entry.h"

#include "common.h"

VALIDATE_SIZE(region_lookup_entry, 8u);

region_lookup_entry::region_lookup_entry() {}

region_lookup_entry::region_lookup_entry(string_hash a2, int a3) : field_0(a2), reg_idx(a3) {}
