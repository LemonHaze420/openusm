#pragma once

#include "float.hpp"

inline constexpr auto PI = 3.1415927f;

inline constexpr auto half_PI = PI * 0.5f;

struct po;
struct vector3d;
struct vector4d;
struct matrix4x4;

inline constexpr float DEG_TO_RAD(float deg)
{
    return (PI / 180.0 * deg);
}

inline constexpr float RAD_TO_DEG(float rad)
{
    return (180.0 / PI) * rad;
}

//0x00588DB0
extern void fast_sin_cos_approx(Float a1, float *s, float *c);

//0x0048A630
extern float bounded_acos(float a1);

extern int triangular_number(int a1);

extern bool approx_equals(const matrix4x4 &a1, const matrix4x4 &a2, Float epsilon);

extern bool approx_equals(const vector4d &a1, const vector4d &a2, Float epsilon);

extern bool approx_equals(const vector3d &a1, const vector3d &a2, Float epsilon);

extern bool approx_equals(Float a1, Float a2, Float epsilon);

extern float sub_5038D0(float a1, float a2);

extern float clamp_and_acos(float a1);

extern float sub_48A720(float a1, float a2);

extern float sub_4ADC40(float a1);

extern float sub_48C0C0(const float &a1, const float &a2, const float &a3, const float &a4, const float &a5);

//0x004D0320
extern double calculate_xz_angle_relative_to_local_po(const po &a1, const vector3d &a4, const vector3d &a5);

//0x004CCA80
extern float calculate_xz_angle_relative_to_local_po(const vector3d &a1, const vector3d &a2, const vector3d &a3,
                                                     const vector3d &a4, const vector3d &a5);

extern int sign(float a1);

extern float sqr(float);
