#pragma once

#include <cassert>
#include <cstdint>

#include "mcontainer.h"
#include "mcontainer_base.h"
#include "mash.h"
#include "mash_config.h"
#include "mash_info_struct.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

template<typename T>
struct mVector : mContainer_base {
    T **m_data;
    int field_C;
    bool field_10;

    struct iterator {
        T **_Ptr;

        bool operator==(const iterator &it) const
        {
            return _Ptr == it._Ptr;
        }

        bool operator!=(const iterator &it) const
        {
            return _Ptr != it._Ptr;
        }

        void operator++()
        {
            ++this->_Ptr;
        }

        auto &operator*()
        {
            return (*this->_Ptr);
        }

        auto &operator*() const
        {
            return (*this->_Ptr);
        }
    };

    mVector() = default;

    mVector(from_mash_in_place_constructor *) : mVector()
    {
        if (this->m_data == nullptr) {
            return;
        }

        if (this->m_size <= 0) {
            return;
        }

        for (auto i = 0; i < this->m_size; ++i)
        {
            T *v5 = nullptr;
            auto *&v4 = this->m_data[i];
            if (v4 != nullptr) {
                v5 = new (v4) T {nullptr};
            }

            v4 = v5;
        }
    }

    T *at(uint16_t index) {
        assert(index < this->m_size);
        assert(this->m_data != nullptr);

        return this->m_data[index];
    }

    const T *at(uint16_t index) const {
        assert(index < this->m_size);
        assert(this->m_data != nullptr);

        return this->m_data[index];
    }

    auto size() const {
        return m_size;
    }

    auto empty() const {
        return this->size() == 0;
    }

    auto begin()
    {
        if (this->m_data != nullptr)
        {
            return iterator {this->m_data};
        }

        return iterator {nullptr};
    }

    auto begin() const
    {
        if (this->m_data != nullptr)
        {
            return iterator {this->m_data};
        }

        return iterator {nullptr};
    }

    auto end()
    {
        if (this->m_data != nullptr)
        {
            return iterator {&this->m_data[this->m_size]};
        }

        return iterator {nullptr};
    }

    auto end() const
    {
        if (this->m_data != nullptr)
        {
            return iterator {&this->m_data[this->m_size]};
        }

        return iterator {nullptr};
    }

    void initialize(mash::allocation_scope );

    void finalize(mash::allocation_scope ) {
        this->clear();
    }

    void destroy_element(T **a2);

    void clear();

    void destruct_mashed_class();

    void unmash(mash_info_struct *a2, void *a3)
    {
#if OPENUSM_XBOX_MASH_FORMAT
        [](mash_info_struct *a1, mash::buffer_type a2, int &a3)
        {
            a3 = * bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
        }(a2, mash::SHARED_BUFFER, m_size);
#endif

        this->custom_unmash(a2, a3);
    }

    void custom_unmash(mash_info_struct *, void *);
};

template<typename T>
struct mVectorBasic : mContainer
{
    T *m_data;
    int field_C;

    int size() const {
        return this->m_size;
    }

    auto &at(int i) {
        return this->m_data[i];
    }

    void unmash(mash_info_struct *, void *);

    void custom_unmash(mash_info_struct *, void *);
};
