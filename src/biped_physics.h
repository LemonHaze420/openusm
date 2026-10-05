#pragma once

#include "float.hpp"
#include "matrix4x4.h"
#include "variable.h"

struct conglomerate;

namespace biped_physics {
void capture_frame(conglomerate *a1, int a2);

void set_capture_frame_delta(Float a1);

inline auto &is_frame_delta = var<bool>(0x00965F32);
inline auto &is_frame_0 = var<bool>(0x00965F30);
inline auto &is_frame_1 = var<bool>(0x00965F31);
inline auto &curr_delta_t = var<float>(0x00965F2C);
inline auto &frame_0_mats = var<matrix4x4[10]>(0x00967920);
inline auto &frame_1_mats = var<matrix4x4[10]>(0x009676A0);
}  // namespace biped_physics
