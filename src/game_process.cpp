#include "game_process.h"

#include "common.h"

#include <cassert>

VALIDATE_SIZE(game_process, 0x18);

game_process::game_process(const char *a2, int *a3, int a4) : field_0(a2), field_4(a3), num_states(a4)
{
    this->index = 0;
    this->field_10 = 0;
    this->field_14 = false;
}

void game_process::reset_index()
{
    index = 0;
}

void game_process::go_next_state()
{
    ++this->index;
    assert(index < num_states);
}
