#pragma once

#include "config.h"

#include <cassert>
#include <cstdio>

template <typename T>
struct Var {
    using value_type = T;
    using pointer_type = T *;

    Var() = delete;

    Var(const Var &) = delete;
    Var &operator=(const Var &) = delete;

    Var(Var &&) = delete;
    Var &operator=(Var &&) = delete;

    Var(ptrdiff_t &&address) : pointer(bit_cast<T *>(address)) {}

    T *pointer;

    inline T &operator()()
    {
        return (*pointer);
    }

    inline T &operator*()
    {
        return (*pointer);
    }

    inline const T &operator()() const
    {
        return (*pointer);
    }

    inline const T &operator*() const
    {
        return (*pointer);
    }
};


template <typename T>
inline auto &var(ptrdiff_t &&address)
{
    return *bit_cast<T *>(address);
}
