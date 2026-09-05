#pragma once

#include "config.h"
#include "log.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

[[noreturn]] inline void fatal_error(
    const char *calling_convention, uintptr_t address)
{
    sp_log("STANDALONE_SYSTEM blocked %s original call to 0x%08lX\n",
           calling_convention, static_cast<unsigned long>(address));
    std::fprintf(stderr, "STANDALONE_SYSTEM blocked %s original call to 0x%08lX\n",
                 calling_convention, static_cast<unsigned long>(address));
    std::fflush(nullptr);
    assert(false && "STANDALONE_SYSTEM attempted an original executable call");
    std::abort();
}

typedef int(__cdecl *cdecl_call)(...);
typedef int(__stdcall *stdcall_call)(...);
typedef int(__fastcall *fastcall_call)(...);

template<typename... Args>
decltype(auto) THISCALL(int address, const void *obj, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("thiscall", address);
    if constexpr (sizeof...(Args) > 0) {
        return (bit_cast<fastcall_call>(address))(obj, 0, args...);
    } else {
        return (bit_cast<fastcall_call>(address))(obj);
    }
}

template<typename... Args>
decltype(auto) STDCALL(int address, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("stdcall", address);
#ifdef TEST_CASE
    assert(0);
#endif

    return (bit_cast<stdcall_call>(address))(args...);
}

template<typename... Args>
decltype(auto) CDECL_CALL(int address, Args... args)
{
    if constexpr (STANDALONE_SYSTEM)
        fatal_error("cdecl", address);
#ifdef TEST_CASE
    sp_log("0x%08X", address);
    assert(0);
#endif

    return (bit_cast<cdecl_call>(address))(args...);
}


inline void ESI_CALL(uintptr_t addr, int val)
{
    int a = val;
    __asm("mov esi, %[a]\n" ::[a] "m"(a));
    CDECL_CALL(addr);
}