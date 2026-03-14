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
struct mVector : mContainer_base {
    T **m_data;
    int m_max_size;
    bool field_10;

    using value_type = T;

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

    mVector() : mContainer_base() {
        this->initialize(mash::ALLOCATED);
    }

    mVector(from_mash_in_place_constructor *a2) : mContainer_base(a2) {
        this->initialize(mash::FROM_MASH);
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

    void initialize(mash::allocation_scope scope)
#if 1
    {
        if ( scope )
        {
            assert(scope == mash::FROM_MASH);

            if ( this->m_data != nullptr )
            {
                assert(m_size > 0);
                for ( int i = 0; i < this->m_size; ++i ) {
                    if constexpr (std::is_base_of_v<mash_virtual_base, value_type>) {
                        this->m_data[i] = bit_cast<value_type *>(mash_virtual_base::construct_class_helper(this->m_data[i]));
                    } else {
                        this->m_data[i] = new (this->m_data[i]) value_type {static_cast<from_mash_in_place_constructor *>(nullptr)};
                    }
                }
            }
        }
        else
        {
            this->m_data = nullptr;
            this->m_max_size = 0;
            this->field_10 = true;
        }
    }
#else
        ;
#endif

    void finalize(mash::allocation_scope ) {
        this->clear();
    }

    void destroy_element(T **a2);

    void clear();

    void destruct_mashed_class();

    void unmash(mash_info_struct *a2, void *a3)
    {
#ifdef TARGET_XBOX
        [](mash_info_struct *a1, mash::buffer_type a2, int &a3)
        {
            a3 = * bit_cast<int *>(a1->read_from_buffer(a2, 4, 4));
        }(a2, mash::SHARED_BUFFER, m_size);
#endif

        this->custom_unmash(a2, a3);
    }

    void custom_unmash(mash_info_struct *, void *);

    void reserve(int a2);

    void push_back()
    {
        assert(this->m_size <= this->m_max_size);
        if ( this->m_size == this->m_max_size
                || this->is_pointer_in_mash_image(this->m_data) )
        {
            this->reserve(8 * (this->m_size / 8) + 8);
        }

        this->m_data[this->m_size++] = nullptr;
    }

    void push_back(T *a2)
    {
        this->push_back();
        this->m_data[this->m_size - 1] = a2;
    }
};
