#pragma once

#include "float.hpp"
#include "mstring.h"

struct matrix4x4;

struct quaternion {
    float arr[4];

    const float &operator[](int idx) const
    {
        return arr[idx];
    }

    auto &operator[](int idx)
    {
        return arr[idx];
    }

    quaternion operator+(const quaternion &a1) const
    {
        quaternion result;
        result[0] = a1[0] + this->arr[0];
        result[1] = a1[1] + this->arr[1];
        result[2] = a1[2] + this->arr[2];
        result[3] = a1[3] + this->arr[3];
        return result;
    }

    quaternion operator*(float a1) const
    {
        quaternion result;
        result[0] = a1 * this->arr[0];
        result[1] = a1 * this->arr[1];
        result[2] = a1 * this->arr[2];
        result[3] = a1 * this->arr[3];
        return result;
    }

    quaternion() = default;

    //0x00588410
    quaternion(const matrix4x4 &matrix);

    quaternion(float x, float y, float z, float w) : arr{x, y, z, w} {}

    const quaternion operator-() const
    {
        quaternion quat{-arr[0], -arr[1], -arr[2], -arr[3]};

        return quat;
    }

    bool operator==(const quaternion &v) const;

    static float dot(const quaternion &quat0, const quaternion &quat1);

    //0x00588550
    void to_matrix(matrix4x4 &mat) const;

    mString to_string() const;
};

//sub_588630
extern quaternion slerp(const quaternion &a2, const quaternion &a3, Float lambda);
