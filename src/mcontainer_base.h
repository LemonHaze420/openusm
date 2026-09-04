#pragma once

#include "mash.h"

struct from_mash_in_place_constructor;

struct mContainer_base {
    int field_0;
    int m_size;

    mContainer_base();

    mContainer_base(from_mash_in_place_constructor *);

    void initialize(mash::allocation_scope a2);

    void finalize(mash::allocation_scope) {}

    void clear();

    auto size() const
    {
        return this->m_size;
    }

    bool is_pointer_in_mash_image(void *a3) const;

    void destruct_mashed_class();
};
