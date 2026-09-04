#include "movie_manager.h"

#include "func_wrapper.h"

movie_manager::movie_manager() {}

bool movie_manager::load_and_play_movie(const char *a2, const char *str, bool a3)
{
    return (bool)CDECL_CALL(0x006299E0, a2, str, a3);
}
