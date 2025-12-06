#pragma once

#include "float.hpp"
#include "mstring.h"

#include <cstdio>

struct vector3d;
struct matrix4x4;

struct vector4d {
    float x;
    float y;
    float z;
    float w;

    constexpr vector4d() : x(0), y(0), z(0), w(0) {}

    constexpr explicit vector4d(float a1) : x(a1), y(a1), z(a1), w(a1)  {}

    constexpr vector4d(float a1, float a2, float a3, float a4) : x(a1), y(a2), z(a3), w(a4) {}

    vector4d(const vector3d &v);

    vector4d(const vector3d &v, float a2);

    bool is_valid() const;

    float length2() const;

    float length() const;

    const auto &operator[](int idx) const {
        return (&x)[idx];
    }

    auto &operator[](int idx) {
        return (&x)[idx];
    }

    static vector4d min(const vector4d &a2, const vector4d &a3);

    //0x004012F0
    static vector4d max(const vector4d &a2, const vector4d &a3);

    //0x0048C460
    static vector4d floor(const vector4d &a2);

    //0x0048C4E0
    static vector4d ceil(const vector4d &a2);

    float dot() const;

    static float dot(const vector4d &a1, const vector4d &a2);

    mString to_string() const {
        mString str{0, "vec4{%.2f, %.2f, %.2f, %.2f}", x, y, z, w};

        return str;
    }

    vector4d sub_74B1C0(Float a3);

    void sub_411A50(const vector4d &a2, const vector4d &a3);

    [[nodiscard]] vector4d sub_41CF30() const;

    void sub_413530(const vector4d &a2, const vector4d &a3);

    [[nodiscard]] static vector4d sub_413E90(const vector4d &arg4,
                                             const vector4d &arg8,
                                             const vector4d &a2,
                                             const vector4d &a3,
                                             const vector4d &a6,
                                             const vector4d &a7,
                                             const vector4d &a8);

    [[nodiscard]] vector4d operator+(const vector4d &a3) const {
        vector4d out;
        out[0] = a3[0] + (*this)[0];
        out[1] = a3[1] + (*this)[1];
        out[2] = a3[2] + (*this)[2];
        out[3] = a3[3] + (*this)[3];
        return out;
    }

    void operator+=(const vector4d &a3);

    [[nodiscard]] vector4d operator+(const float a3) const {
        vector4d out;
        out[0] = (*this)[0] + a3;
        out[1] = (*this)[1] + a3;
        out[2] = (*this)[2] + a3;
        out[3] = (*this)[3] + a3;
        return out;
    }

    void operator*=(const vector4d &a3);

    //0x00776EC0
    friend vector4d operator-(const vector4d &a2, const vector4d &a3);

    //0x00412870
    friend vector4d operator*(const vector4d &a2, const vector4d &a3);

    //0x00410DF0
    friend vector4d operator*(const vector4d &a2, float a3);

    void operator*=(float a3);

    //0x004010A0
    friend vector4d operator-(const vector4d &a2);

    bool operator==(const vector4d &a2) const;
};


[[nodiscard]] extern vector4d sub_4126E0(const vector4d &a2,
                                             const vector4d &a3,
                                             const vector4d &a4,
                                             const vector4d &a5,
                                             const vector4d &a6,
                                             const vector4d &a7);

extern bool sub_55F1D0(const vector4d &a1, const vector4d &a2, float a3);

extern vector4d sub_55DA40(const float *a2, const vector4d *a3);

extern vector4d sub_5FD0C0(Float a2, const vector4d &a3, const vector4d &a4);

extern vector4d sub_5FC6D0(
    const vector4d &a2,
    const vector4d &a3,
    const vector4d &a4,
    const vector4d &a5,
    const vector4d &a6,
    const vector4d &a7,
    const vector4d &a8);

extern vector4d sub_5FC770(const vector4d &a2, const vector4d &a3, const vector4d &a4, const vector4d &a5);

extern float AbsSquared(const vector4d &a1);
