#pragma once

#include <cstddef>

#include <algorithm>
#include <cstring>
#if STANDALONE_SYSTEM
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#endif

#define PROGRESS_LOG

#ifndef __has_feature
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif

// Any compiler claiming C++11 supports, Visual C++ 2015 and Clang version supporting constexpr
#if ((__cplusplus >= 201103L) || (_MSC_VER >= 1900) || (__has_feature(cxx_constexpr)))  // C++ 11 implementation
#define _STDEX_NATIVE_CPP11_SUPPORT
#define _STDEX_NATIVE_CPP11_TYPES_SUPPORT
#endif

#if !defined(_STDEX_NATIVE_CPP11_TYPES_SUPPORT)
#if ((__cplusplus > 199711L) || defined(__CODEGEARC__))
#define _STDEX_NATIVE_CPP11_TYPES_SUPPORT
#endif
#endif

#if ((!defined(_MSC_VER) || _MSC_VER < 1600) && !defined(_STDEX_NATIVE_CPP11_SUPPORT))
#define _STDEX_IMPLEMENTS_nullptr_SUPPORT
#else
#define _STDEX_NATIVE_nullptr_SUPPORT
#endif

#if (_MSC_VER >= 1600)
#ifndef _STDEX_NATIVE_CPP11_TYPES_SUPPORT
#define _STDEX_NATIVE_CPP11_TYPES_SUPPORT
#endif
#endif

#if _MSC_VER // Visual C++ fallback
#define _STDEX_NATIVE_MICROSOFT_COMPILER_EXTENSIONS_SUPPORT
#define _STDEX_CDECL __cdecl

#if (__cplusplus >= 199711L)
#define _STDEX_NATIVE_CPP_98_SUPPORT
#endif
#endif

// C++ 98 check:
#if ((__cplusplus >= 199711L) &&                          \
     ((defined(__INTEL_COMPILER) || defined(__clang__) || \
       (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4))))))
#ifndef _STDEX_NATIVE_CPP_98_SUPPORT
#define _STDEX_NATIVE_CPP_98_SUPPORT
#endif
#endif

#ifdef _STDEX_NATIVE_CPP11_SUPPORT
#include <cstdint>
#else
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int size_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef int ptrdiff_t;
#endif

using ushort = uint16_t;
using uint = uint32_t;

#ifndef _STDEX_NATIVE_CPP11_SUPPORT
#define nullptr 0
#endif

#ifdef _STDEX_NATIVE_CPP11_SUPPORT
#include <type_traits>
#endif

#ifdef _STDEX_NATIVE_CPP11_SUPPORT
#include <functional>
#endif

using rational_t = float;

template<typename T>
bool equal(T a1, T a2)
{
    std::equal_to<T> q{};
    return q(a1, a2);
}

template<typename T>
bool not_equal(T a1, T a2)
{
    std::not_equal_to<T> q{};
    return q(a1, a2);
}

template<class To, class From>
constexpr typename std::enable_if_t<
    sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>, To>
    // constexpr support needs compiler magic
bit_cast(const From &src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
        "This implementation additionally requires destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template <typename T, typename = void>
struct is_complete : std::false_type {};

template <typename T>
struct is_complete<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

#if STANDALONE_SYSTEM
template <typename Pointer>
Pointer standalone_original_vtable(std::uintptr_t address)
{
    using value_type = std::remove_cv_t<std::remove_pointer_t<Pointer>>;
    static std::map<std::uintptr_t, std::unique_ptr<value_type>> vtables;
    auto &vtable = vtables[address];
    if (!vtable)
        vtable = std::make_unique<value_type>();
    return vtable.get();
}
#endif

template <typename T0, typename T1>
T0 cast_to(T1 address)
{
#if STANDALONE_SYSTEM
    if constexpr (std::is_pointer_v<T0> &&
                  std::is_function_v<std::remove_pointer_t<T0>> &&
                  std::is_integral_v<T1>) {
        const auto target = static_cast<std::uintptr_t>(address);
        if (target >= 0x00400000u && target < 0x00900000u) {
            std::fprintf(stderr,
                "STANDALONE_SYSTEM blocked raw original function cast to 0x%08lX\n",
                static_cast<unsigned long>(target));
            std::fflush(nullptr);
            std::abort();
        }
    }
    if constexpr (std::is_pointer_v<T0> &&
                  !std::is_function_v<std::remove_pointer_t<T0>> &&
                  !std::is_void_v<std::remove_cv_t<std::remove_pointer_t<T0>>> &&
                  is_complete<std::remove_cv_t<std::remove_pointer_t<T0>>>::value &&
                  std::is_integral_v<T1>) {
        const auto target = static_cast<std::uintptr_t>(address);
        if (target >= 0x00870000u && target < 0x00900000u)
            return standalone_original_vtable<T0>(target);
    }
#endif
    return bit_cast<T0>(address);
}

#define CAST(var, address) \
    cast_to<std::remove_reference_t<decltype(var)>>(address)
