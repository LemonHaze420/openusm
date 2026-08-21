#include "decal_morphs.h"

#include "func_wrapper.h"

decal_morphs::decal_morphs() {}

void decal_morphs::frame_advance(Float a1)
{
    CDECL_CALL(0x004CE1E0, a1);
}
