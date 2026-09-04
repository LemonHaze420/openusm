#pragma once

#include "float.hpp"
#include "variable.h"

namespace audio_box_manager {
//0x005100E0
extern void create_inst();

//0x00552610
extern void frame_advance(Float a1);

extern void delete_inst();
}  // namespace audio_box_manager

extern Var<int> s_holds;
