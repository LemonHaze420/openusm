#include "vector3d.h"

#include "custom_math.h"
#include "euler_direction.h"
#include "func_wrapper.h"
#include "matrix4x4.h"
#include "oldmath_usefulmath.h"
#include "quaternion.h"
#include "trace.h"
#include "vector4d.h"

#ifdef USE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#else
#include <cmath>
#endif

#include <cstdio>

vector3d XVEC{1.0f, 0, 0};

vector3d YVEC{0, 1.0, 0};

vector3d ZVEC{0, 0, 1.f};

vector3d ZEROVEC{0, 0, 0};

bool lexicographical_compare(const vector3d &v1, const vector3d &v2) {
    if (bit_cast<int>(v1[0]) != bit_cast<int>(v2[0])) {
        //sp_log("0x%08X 0x%08X", bit_cast<int>(v1[0]), bit_cast<int>(v2[0]));
        return false;
    }

    if (bit_cast<int>(v1[1]) != bit_cast<int>(v2[1])) {
        //sp_log("0x%08X 0x%08X", bit_cast<int>(v1[1]), bit_cast<int>(v2[1]));

        return false;
    }

    if (bit_cast<int>(v1[2]) != bit_cast<int>(v2[2])) {
        //sp_log("0x%08X 0x%08X", bit_cast<int>(v1[2]), bit_cast<int>(v2[2]));
        return false;
    }

    return true;
}

bool vector3d::operator==(const vector3d &v) const {
    if constexpr (1) {
        return lexicographical_compare(*this, v);
    } else {
        return equal(x, v[0]) && equal(y, v[1]) && equal(z, v[2]);
    }
}

//0x00501AB0
bool vector3d::is_valid() const {
    return x > -1.0e10 && x < 1.0e10 &&
        y > -1.0e10 &&
        y < 1.0e10 &&
        z > -1.0e10 &&
        z < 1.0e10;
}

mString vector3d::to_string() const {
    auto str = mString{0, "vec3{%f, %f, %f}", x, y, z};

    //sprintf(str, "vector3d{%.2f, %.2f, %.2f}", arr[0], arr[1], arr[2]);

#if 0
    std::memset(str, 0, std::size(str));

    sprintf(str,
            "vector3d{0x%08X, 0x%08X, 0x%08X}",
            bit_cast<int>(arr[0]),
            bit_cast<int>(arr[1]),
            bit_cast<int>(arr[2]));
#endif

    return str;
}

bool vector3d::is_normal() const {
    return std::abs(this->length2() - 1.0) < LARGE_EPSILON;
}

float vector3d::abs(Float a1) {
    return std::abs(a1);
}

vector3d vector3d::cross(const vector3d &a2, const vector3d &a3) {
    vector3d v4{a2[1] * a3[2] - a2[2] * a3[1],
                a2[2] * a3[0] - a2[0] * a3[2],
                a2[0] * a3[1] - a2[1] * a3[0]};

    return v4;
}

float dot(const vector3d &a1, const vector3d &a2) {
#ifndef USE_GLM
    return a1[2] * a2[2] + a1[1] * a2[1] + a1[0] * a2[0];
#else
    return glm::dot(glm::vec3{a1[0], a1[1], a1[2]}, glm::vec3{a2[0], a2[1], a2[2]});
#endif
}

inline bool abs(const vector3d &a1, const vector3d &a2) {
    auto v1 = std::abs(dot(a1, a2));

    return std::isless(v1, LARGE_EPSILON);
}

float closest_point_infinite_line_point(const vector3d &a1, const vector3d &a2, const vector3d &a3) {
    auto result = 0.f;

    auto v3 = dot(a2, a2);
    if (std::abs(v3) >= EPSILON) {
        result = (dot(a3, a2) - dot(a1, a2)) / v3;
    }

    return result;
}

vector3d sub_48B5B0(const vector3d &a2, const vector3d &a3, Float a4) {
    vector3d result = (a3 - a2) * a4 + a2;

    return result;
}

vector3d::vector3d(const vector4d &v)
                    : x(v[0]), y(v[1]), z(v[2])
{}

vector3d sub_444A60(const vector3d &a2, const vector3d &a3)
{
    auto v1 = vector3d::cross(a2, a3);
    auto result = vector3d::cross(a3, v1);

    sp_log("a2 = %s, a3 = %s", a2.to_string().c_str(), a3.to_string().c_str());
    return result;
}


void reorient_vectors(vector3d a1,
                        vector3d a4,
                        vector3d a7,
                        vector3d a10,
                        vector3d &forward,
                        vector3d &up,
                        Float a15)
{
    bool v1 = false;
    if ( a7.is_normal()
         && a10.is_normal()
         && a1.is_normal()
         && a4.is_normal()
       )
    {
        if ( std::abs(dot(a7, a10)) < LARGE_EPSILON )
        {
            if ( std::abs(dot(a1, a4)) < LARGE_EPSILON ) {
                v1 = true;
            }
        }
    }

    if (v1)
    {
        a7.normalize();
        a10.normalize();
        a1.normalize();
        a4.normalize();

        matrix4x4 mat1 {vector3d::cross(a4, a1), a4, a1, vector3d {}};
        quaternion quat1 {mat1};

        matrix4x4 mat2 {vector3d::cross(a10, a7), a10, a7, vector3d {}};
        quaternion quat2 {mat2};

        quaternion a2 = slerp(quat1, quat2, a15);

        matrix4x4 mat3;
        a2.to_matrix(mat3);

        forward = mat3[2];
        up = mat3[1];
    } else {
        forward = a1;
        up = a4;
    }
}

float Abs(const vector3d &a1) {
    return a1.length();
}

vector3d make_vector3d(const euler_direction &a2) {
    auto v2 = a2.m_heading;
    auto v3 = a2.m_pitch;
    auto v5 = cos(v3);

    vector3d result;
    result[0] = cos(v2) * v5;
    result[1] = sin(v3);
    result[2] = sin(v2) * v5;
    return result;
}

float vector3d::xz_length2() const {
    return x * x + y * y;
}

float vector3d::xz_norm() const {
    return std::sqrt(this->x * this->x + this->z * this->z);
}

float vector3d::xy_norm() const {
    return std::sqrt(this->x * this->x + this->y * this->y);
}

Float vector3d::length2() const {
#ifndef USE_GLM
    return (this->x * this->x + this->y * this->y + this->z * this->z);
#else
    return glm::length2(glm::vec3{this->x, this->y, this->z});
#endif
}

float vector3d::length() const {
    return std::sqrt(this->length2());
}

vector4d vector3d::sub_48D010() const
{
    vector4d result;
    THISCALL(0x0048D010, this, &result);

    return result;
}

void vector3d::normalize() {
#ifndef USE_GLM
    float length2 = this->length2();

    if (length2 > 9.9999994e-11) {
        float v3 = 1.0f / sqrt(length2);
        this->x *= v3;
        this->y *= v3;
        this->z *= v3;
    }
#else
    glm::vec3 vec = glm::vec3{this->x, this->y, this->z};

    vec = glm::fastNormalize(vec);

    this->x = vec.x;
    this->y = vec.y;
    this->z = vec.z;
#endif
}

vector3d vector3d::normalized() const {
#ifndef USE_GLM
    vector3d result{this->x, this->y, this->z};
    const float length2 = this->length2();

    if (length2 > 9.9999994e-11) {
        const float v3 = 1.0f / sqrt(length2);
        result *= v3;
    }
#else
    glm::vec3 vec{this->x, this->y, this->z};
    vec = glm::fastNormalize(vec);

    vector3d result{vec[0], vec[1], vec[2]};
#endif
    return result;
}

vector3d vector3d::min(const vector3d &a2, const vector3d &a3)
{
    auto v6 = std::min(a2[2], a3[2]);
    auto v5 = std::min(a2[1], a3[1]);
    auto v3 = std::min(a2[0], a3[0]);
    return vector3d {v3, v5, v6};
}

vector3d vector3d::max(const vector3d &a2, const vector3d &a3)
{
    auto v6 = std::max(a2[2], a3[2]);
    auto v5 = std::max(a2[1], a3[1]);
    auto v3 = std::max(a2[0], a3[0]);
    return vector3d {v3, v5, v6};
}

void vector3d::sub_4B9FA0(vector3d a2, Float a5)
{
    auto v3 = lerp(a2, *this, a5);

    (*this) = v3.normalized();
}

void vector3d::sub_48A850(Float a2)
{
    auto v2 = this->length2();
    if ( v2 > (0.0000099999997 * 0.0000099999997) )
    {
        auto v4 = a2 / std::sqrt(v2);
        (*this) *= v4;
    }
}

#ifndef USE_GLM
bool is_colinear(vector3d a1, vector3d a4, Float epsilon) {
#if 0       
    long double v3;  // st7
    long double v4;  // st6
    long double v5;  // st5
    long double v6;  // st4
    long double v7;  // st3
    long double v8;  // st4
    long double v9;  // st3
    long double v10; // st2
    long double v11; // st1
    long double v12; // st0
    
    v3 = a1[0];
    v4 = a1[1];
    v5 = a1[2];
    v6 = v5 * v5 + v4 * v4 + v3 * v3;
    if (v6 > 9.9999994e-11) {
        v7 = 1.0 / sqrt(v6);
        v3 = v3 * v7;
        v4 = v4 * v7;
        v5 = v5 * v7;
    }

    v8 = a4[0];
    v9 = a4[1];
    v10 = a4[2];
    v11 = v10 * v10 + v9 * v9 + v8 * v8;
    if (v11 > 9.9999994e-11) {
        v12 = 1.0 / sqrt(v11);
        v8 = v8 * v12;
        v9 = v9 * v12;
        v10 = v10 * v12;
    }

    return 1.0 - epsilon < std::abs(v10 * v5 + v9 * v4 + v8 * v3);
#else
    a1.normalize();
    a4.normalize();

    return 1.0 - epsilon < std::abs(dot(a1, a4));
#endif
}
#else
bool is_colinear(vector3d a1, vector3d a4, Float epsilon) {
    return glm::areCollinear(glm::vec3{a1[0], a1[1], a1[2]},
                             glm::vec3{a4[0], a4[1], a4[2]},
                             epsilon.value);
}

#endif

vector3d orthogonal_projection_onto_plane(const vector3d &a2, const vector3d &a3) {
    if constexpr (1) {
        auto v3 = dot(a2, a3);

        vector3d v6 = a2 - v3 * a3;

        return v6;
    } else {
        vector3d out;
        CDECL_CALL(0x0058FFE0, &out, &a2, &a3);

        return out;
    }
}

bool collide_line_with_plane_infinite(
    const vector3d &a1, const vector3d &a2, Float a3, Float a4, Float a5, Float a6, vector3d &a7)
{
    auto v7 = a2[0] - a1[0];
    auto v13 = a2[1] - a1[1];
    auto v8 = a2[2] - a1[2];

    auto v14 = v8;
    auto v9 = std::sqrt(v8 * v14 + v13 * v13 + v7 * v7);
    if (v9 > 0.0f) {
        auto v10 = 1.0f / v9;
        v7 = v7 * v10;
        v13 = v13 * v10;
        v14 = v10 * v14;
    }

    auto v11 = a3 * v7 + v13 * a4 + v14 * a5;
    if (equal(0.0f, v11)) {
        return false;
    }

    auto a3a = (a6 - a1[0] * a3 - a1[1] * a4 - a1[2] * a5) / v11;

    vector3d v16;
    v16[0] = v7 * a3a;
    v16[1] = v13 * a3a;
    v16[2] = v14 * a3a;

    a7 = v16;
    a7 += a1;

    return true;
}

float compute_angle_between_vectors(const vector3d &a1, const vector3d &a2) {
    auto v5 = dot(a1, a2);
    auto v3 = a1.length();
    auto v4 = a2.length() * v3;
    if (equal(v4, 0.0f)) {
        return 0.0;
    }

    if ((v5 / v4) <= 1.0f) {
        return bounded_acos(v5 / v4);
    }

    return 0.0;
}
