#include "oldmath_usefulmath.h"

#include <cmath>

static constexpr float _2PI = 3.1415927 * 2.0;

float wrap_angle(float ang)
{
    assert(ang < _2PI * 16 && ang > -_2PI * 16);

    float anga = ang / _2PI;
    auto angb = anga - std::floor(anga);
    return (angb * _2PI);
}
