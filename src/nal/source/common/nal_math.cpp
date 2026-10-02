#include "nal_math.h"

#include <cmath>

namespace math {

vector4d Slerp(Float a2, const vector4d &a3, const vector4d &a4)
{
    TRACE("Slerp");

    vector4d result{};

    if constexpr (1) {
        auto cosTheta = vector4d::dot(a3, a4);

        vector4d v17;
        if (cosTheta >= 0.0f) {
            v17 = vector4d{1.0f - a2, a2, 1.0f, 0.0f};
        } else {
            v17 = vector4d{1.0f - a2, -a2, 1.0f, 0.0f};
            cosTheta = -cosTheta;
        }

        if (cosTheta < 0.99999899) {
            double v9;
            if (cosTheta >= 0.5f) {
                auto v10 = std::sqrt((1.0f - cosTheta) * 0.5f);
                v9 = v10 * v10 * v10 * (v10 * v10) * (v10 * v10) * 0.1079625f +
                     v10 * v10 * v10 * (v10 * v10) * 0.15000001f + v10 * v10 * v10 * 0.33333331f + v10 + v10;
            } else {
                auto v5 = cosTheta;
                v9 = v5 * v5 * v5 * (v5 * v5) * (v5 * v5) * -0.053981241f - v5 * v5 * v5 * (v5 * v5) * 0.075000003f -
                     v5 * v5 * v5 * 0.1666667f - v5 + 1.570796f;
            }

            auto v22 = v17 * v9;

            v17 = v22;
            auto v11 = v22[0] * v22[0];
            auto v12 = v22[1] * v22[1];

            vector4d v23;
            v23[2] = v22[2] * v22[2];
            v23[3] = v22[3] * v22[3];

            vector4d v18{};
            v18[0] = v22[0] * v11;
            v18[1] = v22[1] * v12;
            v18[2] = v22[2] * v23[2];
            v18[3] = v22[3] * v23[3];

            v22[0] = v18[0] * v11;
            v22[1] = v18[1] * v12;
            v22[2] = v18[2] * v23[2];
            v22[3] = v18[3] * v23[3];

            vector4d v24;
            v24[0] = v22[0] * v11;
            v24[1] = v22[1] * v12;
            v24[2] = v22[2] * v23[2];
            v24[3] = v22[3] * v23[3];

            v23 = _Float4_SinCoefs;

            v17 = sub_5FC6D0(v24, v23, v22, v23, v18, v23, v17);
            const auto v14 = 1.0f / v17[2];
            v17 *= v14;
        }

        auto blended = sub_5FC770(a3, v17, a4, v17);
        result[0] = blended[0];
        result[1] = blended[1];
        result[2] = blended[2];
        result[3] = blended[3];
    } else {
        void (*func)(vector4d *, Float a2, const vector4d *, const vector4d *) = CAST(func, 0x005FD0C0);
        func(&result, a2, &a3, &a4);
    }

    return result;
}

}  // namespace math
