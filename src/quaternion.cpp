#include "quaternion.h"

#include "matrix4x4.h"

#include "custom_math.h"
#include "log.h"
#include "trace.h"
#include "variable.h"
#include "vector3d.h"

#include <cmath>

quaternion::quaternion(const matrix4x4 &matrix)
{
    uint32_t idx1, idx2, idx3;

    float v1 = matrix[0][0] + matrix[1][1] + matrix[2][2];
    if (v1 <= 0.0) {
        idx1 = 0;
        idx2 = 1;
        idx3 = 2;

        if (matrix[1][1] > matrix[0][0]) {
            idx1 = 1;
            idx2 = 2;
            idx3 = 0;
        }

        if (matrix[2][2] > matrix[0][5 * idx1]) {
            idx1 = 2;
            idx2 = 0;
            idx3 = 1;
        }

        float v2 = std::sqrt(matrix[0][5 * idx1] - matrix[0][5 * idx2] - matrix[0][5 * idx3] + 1.0f);

        float v3[3];

        v3[idx1] = 0.5f * v2;
        if (!equal(v2, 0.0f)) {
            v2 = 0.5f / v2;
        }

        float v10 = (matrix[0][idx2 + 4 * idx3] - matrix[0][idx3 + 4 * idx2]) * v2;
        v3[idx2] = (matrix[0][idx1 + 4 * idx2] + matrix[0][idx2 + 4 * idx1]) * v2;
        v3[idx3] = (matrix[0][idx1 + 4 * idx3] + matrix[0][idx3 + 4 * idx1]) * v2;

        this->arr[0] = v10;
        this->arr[1] = v3[0];
        this->arr[2] = v3[1];
        this->arr[3] = v3[2];

    } else {
        float v4 = std::sqrt(v1 + 1.0f);
        this->arr[0] = 0.5f * v4;
        float v5 = 0.5f / v4;
        this->arr[1] = (matrix[2][1] - matrix[1][2]) * v5;
        this->arr[2] = (matrix[0][2] - matrix[2][0]) * v5;
        this->arr[3] = (matrix[1][0] - matrix[0][1]) * v5;
    }
}

float quaternion::dot(const quaternion &quat0, const quaternion &quat1)
{
    float dot = quat0[0] * quat1[0] + quat0[1] * quat1[1] + quat0[2] * quat1[2] + quat0[3] * quat1[3];

    return dot;
}

bool quaternion::operator==(const quaternion &v) const
{
    return approx_equals(this->arr[0], v[0], LARGE_EPSILON) && approx_equals(this->arr[1], v[1], LARGE_EPSILON) &&
           approx_equals(this->arr[2], v[2], LARGE_EPSILON) && approx_equals(this->arr[3], v[3], LARGE_EPSILON);
}

mString quaternion::to_string() const
{
    return mString{0, "quat {%f, %f, %f, %f}", this->arr[0], this->arr[1], this->arr[2], this->arr[3]};
}

#ifndef USE_GLM
quaternion slerp(const quaternion &a2, const quaternion &a3, Float lambda)
{
    static quaternion qr{1, 0, 0, 0};
    static quaternion qp{1, 0, 0, 0};

    float cosTheta = quaternion::dot(a2, a3);
    if (cosTheta <= 0.99999899f) {
        if (cosTheta >= 0.0f) {
            qp = a3;
        } else {
            cosTheta = -cosTheta;
            if (cosTheta > 1.0) {
                cosTheta = 1.0;
            }

            qp = -a3;
        }

        float theta = bounded_acos(cosTheta);

        if (std::abs(theta) >= 0.0000001f) {
            float v10 = 1.0f / std::sin(theta);
            float v11 = std::sin((1.0f - lambda) * theta) * v10;
            float v12 = std::sin(theta * lambda) * v10;
            qr = a2 * v11 + qp * v12;
            return qr;
        }
    }

    return a2;
}
#else

#include <glm/gtc/quaternion.hpp>

quaternion slerp(const quaternion &x, const quaternion &y, Float lambda)
{
    auto quat = glm::slerp(glm::quat{x[0], x[1], x[2], x[3]}, glm::quat{y[0], y[1], y[2], y[3]}, lambda);

    return {quat[0], quat[1], quat[2], quat[3]};
}
#endif

void quaternion::to_matrix(matrix4x4 &mat) const
{
    float v1 = this->arr[1] + this->arr[1];
    float v2 = this->arr[2] + this->arr[2];
    float v3 = this->arr[3] + this->arr[3];

    mat[0][0] = 1.0f - (v3 * this->arr[3] + v2 * this->arr[2]);
    mat[0][1] = v2 * this->arr[1] - v3 * this->arr[0];
    mat[0][2] = v3 * this->arr[1] + v2 * this->arr[0];
    mat[0][3] = 0.0;

    mat[1][0] = v2 * this->arr[1] + v3 * this->arr[0];
    mat[1][1] = 1.0f - (v3 * this->arr[3] + v1 * this->arr[1]);
    mat[1][2] = v3 * this->arr[2] - v1 * this->arr[0];
    mat[1][3] = 0.0;

    mat[2][0] = v3 * this->arr[1] - v2 * this->arr[0];
    mat[2][1] = v3 * this->arr[2] + v1 * this->arr[0];
    mat[2][2] = 1.0f - (v2 * this->arr[2] + v1 * this->arr[1]);
    mat[2][3] = 0.0;

    mat[3][0] = 0.0;
    mat[3][1] = 0.0;
    mat[3][2] = 0.0;
    mat[3][3] = 1.0;
}
