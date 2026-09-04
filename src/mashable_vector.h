#pragma once

#include "log.h"

#include <cassert>
#include <cstdint>

struct generic_mash_header;
struct generic_mash_data_ptrs;

template <typename T>
struct mashable_vector {
    using value_t = T;

    T *m_data;
    uint16_t m_size;
    bool m_shared;
    bool field_7;

    inline bool from_mash()
    {
        return field_7;
    }

    inline bool is_shared()
    {
        return m_shared;
    }

    inline decltype(auto) size() const
    {
        return m_size;
    }

    inline decltype(auto) data()
    {
        return m_data;
    }

    inline decltype(auto) data() const
    {
        return m_data;
    }

    inline bool empty()
    {
        return (m_size == 0);
    }

    inline auto &at(int index)
    {
        assert(index >= 0 && index < size());
        return m_data[index];
    }

    inline auto &at(int index) const
    {
        assert(index >= 0 && index < size());
        return m_data[index];
    }

    auto &operator[](int index)
    {
        return m_data[index];
    }

    auto &operator[](int index) const
    {
        return m_data[index];
    }

    struct iterator {
        T *m_ptr;

        bool operator==(const iterator &it)
        {
            return (this->m_ptr == it.m_ptr);
        }

        bool operator!=(const iterator &it)
        {
            return (this->m_ptr != it.m_ptr);
        }

        auto &operator*()
        {
            return (*m_ptr);
        }

        auto &operator++()
        {
            return (++m_ptr);
        }
    };

    iterator begin()
    {
        return iterator{&m_data[0]};
    }

    iterator end()
    {
        if (this->m_data != nullptr) {
            return iterator{&this->m_data[this->m_size]};
        }

        return {};
    }

    iterator begin() const
    {
        return iterator{&m_data[0]};
    }

    iterator end() const
    {
        if (this->m_data != nullptr) {
            return iterator{&this->m_data[this->m_size]};
        }

        return {};
    }


    void custom_un_mash(generic_mash_header *header, [[maybe_unused]] void *a3, generic_mash_data_ptrs *a4,
                        [[maybe_unused]] void *a5);

    void un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3, void *a4)
    {
        assert(from_mash());

        this->custom_un_mash(a1, a2, a3, a4);
    }
};

extern void mashable_vector_patch();
