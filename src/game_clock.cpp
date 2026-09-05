#include "game_clock.h"

#include "func_wrapper.h"

void game_clock::frame_advance(Float a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        ++frames;
        delta = a1;
        ticks += static_cast<std::uint64_t>(a1 * 10000.0f);
    } else {
        CDECL_CALL(0x0058E2F0, a1);
    }
}
