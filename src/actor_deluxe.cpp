#include "actor_deluxe.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

VALIDATE_SIZE(actor_deluxe, 0xC0);

actor_deluxe::actor_deluxe() : actor(0) {}

void actor_deluxe::render(Float a2)
{
    THISCALL(0x004E35D0, this, a2);
}
