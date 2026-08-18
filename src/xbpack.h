#pragma once

#include <cstdint>

namespace xbpack
{
#ifdef OPENUSM_XBPACK_V10
inline constexpr bool v10 = true;
inline constexpr unsigned type_count = 68;
inline constexpr unsigned counts_offset = 0x19C;
inline constexpr unsigned directory_size = 0x2B0;
#else
inline constexpr bool v10 = false;
inline constexpr unsigned type_count = 71;
inline constexpr unsigned counts_offset = 0x1A8;
inline constexpr unsigned directory_size = 0x2C4;
#endif

#ifdef OPENUSM_XBPACK_V10
uint32_t pc_state_type(uint32_t type);
#endif

inline constexpr unsigned starts_offset = 0x8C;

constexpr int pc_type(int type)
{
    if constexpr (v10) {
        if (type <= 4)
            return type;
        if (type <= 32)
            return type + 1;
        return type + 2;
    }

    if (type <= 54)
        return type;
    return type - 1;
}
}

bool install_xbpack_support();
