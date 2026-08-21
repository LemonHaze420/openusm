#pragma once

namespace mash {

enum allocation_scope {
    ALLOCATED = 0,
    FROM_MASH = 1,
};

inline constexpr auto CUSTOM_MASH_SENTRY = 0x15BADBAD;
}  // namespace mash
