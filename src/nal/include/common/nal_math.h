#pragma once

#include "float.hpp"
#include "vector4d.h"

namespace math {

inline const vector4d _Float4_SinCoefs{-0.1666667f, 8.333334e-3f, -1.917616e-4f, 0.0f};


//0x005FD0C0
extern vector4d Slerp(Float a2, const vector4d &a3, const vector4d &a4);
}  // namespace math
