#pragma once

#include <cassert>
#include <cstdint>

#include "mcontainer.h"
#include "mcontainer_base.h"
#include "mash.h"
#include "mash_info_struct.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

template<typename T>
struct mVectorBasic : mContainer_base
{
    T *m_data;
    int m_max_size;

    using value_type = T;

    mVectorBasic() : mContainer_base() {}

    mVectorBasic(from_mash_in_place_constructor *a2) : mContainer_base(a2) {
    }

    int size() const {
        return this->m_size;
    }

    auto &at(int i) {
        return this->m_data[i];
    }

    void unmash(mash_info_struct *, void *);

    void custom_unmash(mash_info_struct *, void *);

    void reserve(int a2);

    void push_back()
    {
        assert(m_size <= m_max_size);

        if ( this->m_size == this->m_max_size || this->is_pointer_in_mash_image(this->m_data) ) {
            this->reserve(8 * (this->m_size / 8) + 8);
        }

        ++this->m_size;
    }

    void push_back(const T &a2)
    {
        this->push_back();
        this->m_data[this->m_size - 1] = a2;
    }
};
