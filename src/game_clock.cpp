#include "game_clock.h"

#include "func_wrapper.h"

void game_clock::frame_advance(Float a1)
{
#if 0
    ++game_clock::frames;
    game_clock::delta = a1;
    LODWORD(game_clock::ticks) += (unsigned __int64)(a1 * flt_871B3C);
#else
    CDECL_CALL(0x0058E2F0, a1);
#endif
}
