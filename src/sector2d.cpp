#include "sector2d.h"

#include "custom_math.h"
#include "oldmath_po.h"
#include "vector3d.h"

#include <cmath>

sector2d::sector2d(const vector3d &a2, const vector3d &a3, Float a4)
{
    auto &v5 = a3;
    this->fov = a4;
    auto v6 = v5[0];
    auto v7 = v5[1];

    vector3d v23;
    v23[0] = v6;
    v23[1] = v7;
    v23[2] = v5[2];

    auto v8 = std::sqrt(v23[2] * v23[2] + v6 * v6);
    if (v8 >= EPSILON) {
        auto v9 = 1.f / v8;
        v23[0] = v23[0] * v9;
        v23[1] = 0.0f * v9;
        v23[2] = v23[2] * v9;
    } else {
        v23 = ZVEC;
    }

    auto a1 = a4 * 0.5f;

    po v26;
    v26.m[0][1] = 0.0;
    v26.m[0][3] = 0.0;
    v26.m[1][0] = 0.0;
    v26.m[1][2] = 0.0;
    v26.m[1][3] = 0.0;
    v26.m[2][1] = 0.0;
    v26.m[2][3] = 0.0;
    v26.m[3][0] = 0.0;
    v26.m[3][1] = 0.0;
    v26.m[3][2] = 0.0;

    float c, s;
    fast_sin_cos_approx(a1, &s, &c);

    v26.m[2][0] = -s;
    v26.m[0][0] = c;
    v26.m[0][2] = s;
    v26.m[2][2] = c;
    v26.m[1][1] = 1.0;
    v26.m[3][3] = 1.0;

    vector3d v24 = v26.slow_xform(v23);
    auto v25 = v26.inverse_xform(v23);

    auto v10 = -v24[0];
    this->field_10 = a2[0];
    this->field_14 = a2[2];
    auto v11 = v24[2];
    this->field_18 = v5[0];
    auto v12 = v5[2];
    this->field_24 = v10;
    this->field_20 = v11;
    this->field_1C = v12;
    if (this->field_24 * this->field_1C + this->field_20 * this->field_18 < 0.0f) {
        this->field_20 = -v24[2];
        this->field_24 = -v10;
    }

    auto v13 = this->field_24 * this->field_14;
    auto v14 = this->field_20 * this->field_10;
    this->field_2C = v25[2];
    this->field_28 = -(v13 + v14);
    auto v15 = -v25[0];
    this->field_30 = v15;

    if (this->field_30 * this->field_1C + this->field_2C * this->field_18 < 0.0f) {
        this->field_2C = -v25[2];
        this->field_30 = -v15;
    }

    auto v16 = v24[2];
    auto v17 = v25[0];
    auto v18 = this->field_30 * this->field_14 + this->field_2C * this->field_10;
    this->field_0 = v24[0];
    auto v19 = v25[2];
    this->field_4 = v16;
    auto v20 = v17;
    this->field_34 = -v18;

    this->field_8 = v20;
    this->field_C = v19;
}
