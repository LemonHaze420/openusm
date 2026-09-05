#pragma once

#include "config.h"

#include <cassert>
#include <cstdio>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

#if STANDALONE_SYSTEM
inline constexpr std::size_t standalone_address_space_size = 0x01000000;

inline std::byte *standalone_address_space()
{
    static auto *storage = new std::byte[standalone_address_space_size]{};
    return storage;
}

inline std::uint8_t *standalone_address_initialization()
{
    static auto *initialized = new std::uint8_t[standalone_address_space_size]{};
    return initialized;
}

template <typename T>
T &standalone_var(ptrdiff_t address)
{
    assert(address >= 0);
    const auto offset = static_cast<std::size_t>(address);
    assert(offset <= standalone_address_space_size);
    assert(sizeof(T) <= standalone_address_space_size - offset);

    auto *const bytes = standalone_address_space() + offset;
    auto &initialized = standalone_address_initialization()[offset];
    if (!initialized) {
        if constexpr (std::is_default_constructible_v<T>) {
            ::new (static_cast<void *>(bytes)) T{};
        } else {
            static_assert(std::is_trivially_destructible_v<T>,
                "Non-default standalone globals must be trivially destructible");
            std::memset(bytes, 0, sizeof(T));
        }
        initialized = true;
    }

    return *reinterpret_cast<T *>(bytes);
}
#endif
template <typename T>
struct Var {
    using value_type = T;
    using pointer_type = T *;

    Var() = delete;

    Var(const Var &) = delete;
    Var &operator=(const Var &) = delete;

    Var(Var &&) = delete;
    Var &operator=(Var &&) = delete;

    Var([[maybe_unused]] ptrdiff_t &&address)
#if STANDALONE_SYSTEM
        : address(address)
#else
        : pointer(bit_cast<T *>(address))
#endif
    {
    }

#if STANDALONE_SYSTEM
    ptrdiff_t address;
#else
    T *pointer;
#endif

    inline T &operator()()
    {
#if STANDALONE_SYSTEM
        return standalone_var<T>(address);
#else
        return *pointer;
#endif
    }

    inline T &operator*()
    {
        return operator()();
    }

    inline const T &operator()() const
    {
#if STANDALONE_SYSTEM
        return standalone_var<T>(address);
#else
        return *pointer;
#endif
    }

    inline const T &operator*() const
    {
        return operator()();
    }
};


template <typename T>
inline auto &var(ptrdiff_t &&address)
{
#if STANDALONE_SYSTEM
    return standalone_var<T>(address);
#else
    return *bit_cast<T *>(address);
#endif
}
