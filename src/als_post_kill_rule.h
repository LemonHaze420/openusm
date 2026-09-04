#pragma once

struct mash_info_struct;

struct from_mash_in_place_constructor;

namespace als {
struct post_kill_rule {
    int field_0;
    int field_4;

    post_kill_rule(from_mash_in_place_constructor *) {}

    void unmash(mash_info_struct *, void *);
};
}  // namespace als
