#pragma once

#include "float.hpp"

struct conglomerate;

namespace biped_physics {
void capture_frame(conglomerate *a1, int a2);

void set_capture_frame_delta(Float a1);

[[maybe_unused]] static bool is_frame_delta{false};
[[maybe_unused]] static Float curr_delta_t{0.0f};
}  // namespace biped_physics
