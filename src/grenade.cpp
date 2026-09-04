#include "grenade.h"

#include "func_wrapper.h"

grenade::grenade(const string_hash &a2, unsigned int a3) : actor(a2, a3)
{
    THISCALL(0x00536580, this, &a2, a3);
}

void grenade::frame_advance_all_grenades(Float a3)
{
    CDECL_CALL(0x0051E070, a3);
}

void grenade::sub_4D6B10(int a2)
{
    THISCALL(0x004D6B10, this, a2);
}
