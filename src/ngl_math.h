#pragma once

#include "matrix4x4.h"

#include <cmath>

namespace math {

template<uint32_t>
struct VecUnit {};

template<bool>
struct Rep_Std {};

template<int Int0, int Int1, typename T0 = void, typename T1 = void, typename T2 = Rep_Std<false>>
struct VecClass : vector4d {
    VecClass() = default;

    VecClass(const vector4d &v) : vector4d{v}
    {}

    VecClass(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    VecClass(float x, float y, float z, float w) : vector4d{x, y, z, w}
    {
    }

    friend VecClass operator+(const VecClass<Int0, Int1, T0, T1, T2> &a2,
                              const VecClass<Int0, Int1, T0, T1, T2> &a3) {
        VecClass<Int0, Int1, T0, T1, T2> v4;

        v4[0] = a2[0] + a3[0];
        v4[1] = a2[1] + a3[1];
        v4[2] = a2[2] + a3[2];
        v4[3] = a3[3] + a2[3];

        return v4;
    }

    friend VecClass operator*(float a2, const VecClass<Int0, Int1, T0, T1, T2> &a3)
    {
        VecClass<Int0, Int1, T0, T1, T2> v4;

        v4[0] = a2 * a3[0];
        v4[1] = a2 * a3[1];
        v4[2] = a2 * a3[2];
        v4[3] = a2 * a3[3];

        return v4;
    }

    void sub_413530(const VecClass<Int0, Int1, T0, T1, T2> &a2, const float &a3)
    {
        auto v3 = a3;
        this->x += v3 * a2[0];
        this->y += v3 * a2[1];
        this->z += v3 * a2[2];
        this->w += v3 * a2[3];
    }

    void sub_411A50(const math::VecClass<3, 0> &a2, const math::VecClass<3, 0> &a3)
    {
        this->x += a3[1] * a2[0];
        this->y += a2[1] * a3[1];
        this->z += a2[2] * a3[1];
        this->w += a2[3] * a3[1];
    }
};

inline const VecClass<3, 1> Float4_0001 {0, 0, 0, 1};

template<typename T, bool>
struct MatKind_Tran {};

template<typename T>
struct MatKind_Rot {};

struct MatKind_Rot_Full {};

template<int,
         int,
         typename T0 = MatKind_Tran<MatKind_Rot<MatKind_Rot_Full>, false>,
         typename T1 = Rep_Std<false>>
struct MatClass : matrix4x4 {
    MatClass() = default;

    MatClass(const matrix4x4 &p) : matrix4x4(p) {}

    MatClass(const vector3d &a2, const vector3d &a3, const vector3d &a4, const vector3d &a5) {
        arr[0] = {a2, 0.0f};
        arr[1] = {a3, 0.0f};
        arr[2] = {a4, 0.0f};
        arr[3] = {a5, 1.0f};
    }

    vector4d GetX() const {
        return this->arr[0];
    }

    vector4d GetY() const {
        return this->arr[1];
    }

    vector4d GetZ() const {
        return this->arr[2];
    }

    void sub_4134B0(
            VecClass<3, 0> &a2,
            VecClass<3, 0> &a3,
            VecClass<3, 0> &a4,
            VecClass<3, 0> &a5) const
    {
        a2 = {this->arr[0]};
        a3 = {this->arr[1]};
        a4 = {this->arr[2]};
        a5 = {this->arr[3]};
    }

};

template<uint32_t I>
struct _Float4BaseMasked {
    float field_0;

    _Float4BaseMasked(float f) : field_0(f) {}


    operator float() const {
        return field_0;
    }
};

} // namespace math

template<int Int0, int Int1, typename T0, typename T1, typename T2>
inline math::_Float4BaseMasked<1> Abs(math::VecClass<Int0, Int1, T0, T1, T2> &a1)
{
    return std::sqrt( (a1[0] * a1[0]) + (a1[1] * a1[1])
            + (a1[2] * a1[2]));
}
