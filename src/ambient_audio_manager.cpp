#include "ambient_audio_manager.h"

#include "func_wrapper.h"
#include "trace.h"

void ambient_audio_manager::create_inst() {
    CDECL_CALL(0x0053EC10);
}

void ambient_audio_manager::delete_inst()
{
    TRACE("ambient_audio_manager");

    CDECL_CALL(0x00552800);
}

void ambient_audio_manager::frame_advance(Float a1) {
    CDECL_CALL(0x00559380, a1);
}

void ambient_audio_manager::reset() {
    CDECL_CALL(0x0054DF90);
}
