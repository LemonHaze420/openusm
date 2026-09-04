#pragma once

#include "float.hpp"

namespace ambient_audio_manager {

//0x0053EC10
extern void create_inst();

extern void delete_inst();

//0x00559380
extern void frame_advance(Float a1);

//0x0054DF90
extern void reset();
}  // namespace ambient_audio_manager
