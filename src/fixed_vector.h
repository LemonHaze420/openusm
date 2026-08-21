#pragma once

#include "float.hpp"

#include <cassert>
#include <cstdint>

struct vector3d;
struct line_marker_base;
struct entity_base;

template <typename T, uint32_t Num>
struct fixed_vector {
    static constexpr auto N = Num;

    T m_data[Num];
    uint32_t m_size{0};

    struct iterator {
        T *m_ptr;

        T &operator*()
        {
            return (*m_ptr);
        }

        bool operator==(const iterator &it) const
        {
            return (this->m_ptr == it.m_ptr);
        }

        bool operator!=(const iterator &it) const
        {
            return (this->m_ptr != it.m_ptr);
        }

        void operator++()
        {
            ++this->m_ptr;
        }
    };

    auto begin()
    {
        return iterator{&m_data[0]};
    }

    auto begin() const
    {
        return iterator{&m_data[0]};
    }

    auto end()
    {
        return iterator{&m_data[this->m_size]};
    }

    auto end() const
    {
        return iterator{&m_data[this->m_size]};
    }

    auto size()
    {
        return m_size;
    }

    auto &at(uint32_t index)
    {
        assert(index < m_size);

        return this->m_data[index];
    }

    void sub_CBF970(int index)
    {
        assert(index >= 0 && index < static_cast<int>(this->m_size));

        --this->m_size;
        for (uint32_t i = index; i < this->m_size; ++i) {
            this->m_data[i] = this->m_data[i + 1];
        }
    }

    void push_back(const T &a1)
    {
        assert(this->m_size < Num);

        this->m_data[this->m_size++] = a1;
    }

    void sort();

    void resize(int size, const T &a3);
};
