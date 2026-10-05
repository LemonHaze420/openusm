#pragma once

#include "config.h"
#include "log.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

[[noreturn]] inline void fatal_error(const char *calling_convention, uintptr_t address)
{
    sp_log("STANDALONE_SYSTEM blocked %s original call to 0x%08lX\n",
           calling_convention,
           static_cast<unsigned long>(address));
    std::fprintf(stderr,
                 "STANDALONE_SYSTEM blocked %s original call to 0x%08lX\n",
                 calling_convention,
                 static_cast<unsigned long>(address));
    std::fflush(nullptr);
    assert(false && "STANDALONE_SYSTEM attempted an original executable call");
    std::abort();
}

typedef int(__cdecl *cdecl_call)(...);
typedef int(__stdcall *stdcall_call)(...);
typedef int(__fastcall *fastcall_call)(...);

template <typename... Args>
decltype(auto) THISCALL(int address, const void *obj, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("thiscall", address);
    if constexpr (sizeof...(Args) > 0) {
        using fastcall_fn = int(__fastcall *)(const void *, int, Args...);
        return (bit_cast<fastcall_fn>(address))(obj, 0, args...);
    } else {
        using fastcall_fn = int(__fastcall *)(const void *);
        return (bit_cast<fastcall_fn>(address))(obj, args...);
    }
}

template <typename... Args>
decltype(auto) STDCALL(int address, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("stdcall", address);
#ifdef TEST_CASE
    assert(0);
#endif

    using stdcall_fn = int(__stdcall *)(Args...);

    return (bit_cast<stdcall_fn>(address))(args...);
}

template <typename... Args>
decltype(auto) CDECL_CALL(int address, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("cdecl", address);
#ifdef TEST_CASE
    sp_log("0x%08X", address);
    assert(0);
#endif

    using cdecl_fn = int(__cdecl *)(Args...);

    return (bit_cast<cdecl_fn>(address))(args...);
}

inline void ESI_CALL(uintptr_t addr, int val)
{
    int a = val;
    __asm("mov esi, %[a]\n" ::[a] "m"(a));
    CDECL_CALL(addr);
}