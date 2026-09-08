#pragma once

#include "config.h"
#include "log.h"

#include <cassert>

template <typename... Args>
decltype(auto) THISCALL(int address, const void *obj, Args... args)
{
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
#ifdef TEST_CASE
    assert(0);
#endif

    using stdcall_fn = int(__stdcall *)(Args...);

    return (bit_cast<stdcall_fn>(address))(args...);
}

template <typename... Args>
decltype(auto) CDECL_CALL(int address, Args... args)
{
#ifdef TEST_CASE
    sp_log("0x%08X", address);
    assert(0);
#endif

    using cdecl_fn = int(__cdecl *)(Args...);

    return (bit_cast<cdecl_fn>(address))(args...);
}
