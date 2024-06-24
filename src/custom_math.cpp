#include "custom_math.h"

#include "func_wrapper.h"
#include "log.h"
#include "matrix4x4.h"
#include "oldmath_po.h"
#include "trace.h"
#include "vector3d.h"

#ifdef USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#else
#include <algorithm>
#include <cmath>
#endif

#include <cassert>

void fast_sin_cos_approx(Float a1, float *s, float *c) {
    *c = std::cos(a1);
    *s = std::sin(a1);
}

float bounded_acos(float a1)
{
    TRACE("bounded_acos");

    a1 = std::clamp(a1, -1.f, 1.f);

    return std::acos(a1);
}

float sub_48A720(float a1, float a2)
{
    if ( !approx_equals(a2, 0.0, 0.001) ) {
        return atan2(a1, a2);
    }

    if ( approx_equals(a1, 0.0, 0.001) ) {
        return 0.0;
    }

    static constexpr auto flt_91F4D8 = 3.1415927 * 0.5f;

    if ( a1 <= 0.0f ) {
        return -flt_91F4D8;
    }

    return flt_91F4D8;
}

float sub_4ADC40(float a1)
{
    static constexpr auto flt_880AE4 = 0.33333331f;

    static constexpr auto flt_880AEC = -0.1079625f;

    static constexpr auto flt_880AE8 = 0.15000001f;

    static constexpr auto flt_880AF8 = 0.053981241f;

    static constexpr auto flt_880AF4 = 0.075000003f;

    static constexpr auto flt_880AF0 = 0.1666667f;

    float result;
    if (a1 >= 0.5f) {
        auto v3 = std::sqrt(std::abs((1.0f - a1) * 0.5f));
        result = v3 * v3 * v3 * (v3 * v3) * (v3 * v3) * flt_880AEC -
            v3 * v3 * v3 * (v3 * v3) * flt_880AE8 - v3 * v3 * v3 * flt_880AE4 - (v3 + v3) + half_PI;
    } else {
        auto v1 = a1 * a1;
        result = v1 * (a1 * v1 * v1) * flt_880AF8 + a1 * v1 * v1 * flt_880AF4 +
            a1 * v1 * flt_880AF0 + a1;
    }

    //sp_log("a1 = %f, result = %f", a1, result);

    return result;
}

float sub_5038D0(float a1, float a2) {
    auto v7 = std::abs(a2);
    auto v2 = std::abs(a1);
    if (equal(0.0f, v2 + v7)) {
        return 0.0f;
    }

    float result;

    auto v4 = 1.0f / sqrt(a1 * a1 + a2 * a2);
    if (v7 >= v2) {
        auto v6 = v4 * v2;
        result = sub_4ADC40(v6);
    } else {
        auto v5 = v4 * v7;
        result = half_PI - sub_4ADC40(v5);
    }

    if (a2 < 0.0f) {
        result = PI - result;
    }

    if (a1 < 0.0f) {
        result = -result;
    }

    return result;
}

float sub_48C0C0(
    const float &a1, const float &a2, const float &a3, const float &a4, const float &a5) {
    if (equal(a3, a2)) {
        return a4;
    }

    const auto v6 = (a1 - a2) * (a5 - a4) / (a3 - a2) + a4;

    float result;
    float v7;
    if (a4 <= a5) {
        result = a4;
        v7 = a5;
    } else {
        result = a5;
        v7 = a4;
    }

    if (v6 >= result) {
        if (v6 <= v7) {
            result = v6;
        } else {
            result = v7;
        }
    }

    return result;
}

double sub_48C0C0(float a1, float a2, float a3, float a4, float a5)
{
    if ( equal(a3, a2) ) { 
        return a4;
    }

    auto v6 = (((a1 - a2) * (a5 - a4)) / (a3 - a2)) + a4;
    if ( a4 <= a5 ) {
        return std::clamp(v6, a4, a5);
    } else {
        return std::clamp(v6, a5, a4);
    }
}

float calculate_xz_angle_relative_to_local_po(const vector3d &a1,
                                              [[maybe_unused]] const vector3d &a2,
                                              const vector3d &a3,
                                              const vector3d &a4,
                                              const vector3d &a5) {
    auto v5 = -dot(a4, a1);
    auto v6 = dot(a4, a3);
    auto v7 = -dot(a5, a1);
    auto v8 = dot(a5, a3);
    auto v10 = v8 * v6 + v7 * v5;
    auto v9 = v7 * v6 - v8 * v5;
    return sub_5038D0(v9, v10);
}

double calculate_xz_angle_relative_to_local_po(const po &a1,
                                               const vector3d &a4,
                                               const vector3d &a5) {
    const auto &z_facing = a1.get_z_facing();
    const auto &y_facing = a1.get_y_facing();
    const auto &x_facing = a1.get_x_facing();

    float result = calculate_xz_angle_relative_to_local_po(x_facing, y_facing, z_facing, a4, a5);

    if constexpr (0) {
        sp_log("x_facing = %s, z_facing = %s, a4 = %s, a5 = %s, result = 0x%08X",
               x_facing.to_string().c_str(),
               z_facing.to_string().c_str(),
               a4.to_string().c_str(),
               a5.to_string().c_str(),
               bit_cast<int>(result));
    }

    return result;
}

int triangular_number(int a1) {
    return (a1 + a1 * a1) / 2;
}

bool approx_equals(const vector4d &a1, const vector4d &a2, Float epsilon) {
    auto v3 = a1 - a2;
    return epsilon * epsilon >= v3.length2();
}

bool approx_equals(const matrix4x4 &a1, const matrix4x4 &a2, Float epsilon) {

    for (int i = 0; i < 4; ++i) {
        if ( !approx_equals(a1[i], a2[i], epsilon) ) {
            return false;
        }
    }

    return true;
}


bool approx_equals(const vector3d &a1, const vector3d &a2, Float epsilon) {
    auto v3 = a1 - a2;
    return epsilon * epsilon >= v3.length2();
}

bool approx_equals(Float a1, Float a2, Float epsilon) {
#ifndef USE_GLM
    return (std::abs(a1 - a2) < epsilon);
#else
    return glm::epsilonEqual(a1.value, a2.value, epsilon.value);
#endif
}

int sign(float a1)
{
    return (a1 >= 0.0 ? a1 > 0 : -1);
}

float sqr(float a1)
{
    return a1 * a1;
}
