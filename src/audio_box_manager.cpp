#include "audio_box_manager.h"

#include "func_wrapper.h"

Var<int> s_holds{0x0095C858};

void audio_box_manager::create_inst()
{
    s_holds() = 0;
}

void audio_box_manager::frame_advance(Float a1)
{
    CDECL_CALL(0x00552610, a1);
}

void audio_box_manager::delete_inst()
{
    s_holds() = 0;
}
