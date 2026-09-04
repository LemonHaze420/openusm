#include "sound_interface.h"

#include "func_wrapper.h"
#include "common.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(sound_interface, 0x28);

sound_interface::sound_interface() {}

void sound_interface::frame_advance_all_sound_ifc(Float a3)
{
    TRACE("sound_interface::frame_advance_all_sound_ifc");

    CDECL_CALL(0x004D1910, a3);
}

void sound_interface_patch()
{
    REDIRECT(0x005584FA, sound_interface::frame_advance_all_sound_ifc);
}
