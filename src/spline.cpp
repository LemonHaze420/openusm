#include "spline.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

#include <cassert>

VALIDATE_OFFSET(spline, field_3C, 0x3C);
VALIDATE_SIZE(spline, 0x50u);

spline::spline() {
    this->curve_pts = {};

    this->field_3C = false;
    this->need_rebuild = false;
    this->field_3E = false;
    this->field_30 = 5;
    this->field_38 = static_cast<eSplineType>(3);
    this->field_3C = true;
    this->need_rebuild = true;
    this->field_3E = true;
}

void spline::set_control_pt(int32_t index, const vector3d &a3) {
    assert(index >= 0);
    assert(index < (int) control_pts.size());

    if (index >= 0 && index < (int) control_pts.size()) {
        this->control_pts[index] = a3;
        this->need_rebuild = true;
        this->field_3E = true;
    }
}

vector3d &spline::get_control_pt(int index) {
    assert(index >= 0 && index < (int) control_pts.size());

    return this->control_pts[index];
}

void spline::build(int a2, spline::eSplineType a3) {
    this->field_30 = a2;

    if (a3 != 0) {
        this->field_38 = a3;
    }

    if (this->field_38 == 4) {
        this->field_34 = (this->get_num_control_pts() - 1) / 3;
    } else {
        this->field_34 = 1;
    }

    this->need_rebuild = true;
    this->rebuild_helper();
}

void spline::compute_spline_pos(Float a3, vector3d &a4, bool a5, spline::eSplineType a6) {
    sp_log("compute_spline_pos: ");

    if constexpr (0) {
    } else {
        THISCALL(0x005DB4A0, this, a3, &a4, a5, a6);
    }
}

vector3d sub_5C2C30(float a3, float a4, float a5, const vector3d *a6) {
    constexpr float v18 = 2.0f;
    constexpr float v19 = 3.0f;

    auto v12 = (a5 - a4) * a6[3];
    auto v11 = (-v19 * a5 + 4.0f * a4 + a3) * a6[2];
    auto v10 = (v19 * a5 - 5.0f * a4 + v18) * a6[1];
    auto v5 = (-a5 + v18 * a4 - a3) * a6[0];

    auto result = (v5 + v10 + v11 + v12) * 0.5f;
    return result;
}

vector3d sub_5C2B20(float a3, float a4, float a5, const vector3d *a6) {
    constexpr float v20 = 0.16666667f;
    constexpr float v19 = 0.5f;
    constexpr float v18 = 0.66666669f;

    auto v11 = (v20 * a5) * a6[3];
    auto v10 = (-v19 * a5 + v19 * a4 + v19 * a3 + v20) * a6[2];
    auto v9 = (v19 * a5 - a4 + v18) * a6[1];
    auto v5 = (-v20 * a5 + v19 * a4 - v19 * a3 + v20) * a6[0];

    auto result = v5 + v9 + v10 + v11;
    return result;
}

void spline::compute_spline_pos(int index, Float t, vector3d &a4, bool a5, spline::eSplineType a6) {
    sp_log("compute_spline_pos: ");

    auto v6 = a6;
    if (a6 == 0) {
        v6 = this->field_38;
    }

    switch (v6) {
    case 1: {
        assert((uint32_t) index < (control_pts.size() - 1));

        assert(t >= 0.0f && t < 1.0001f);

        auto a3 = this->control_pts.at(index);
        auto v16 = this->control_pts.at(index + 1);

        auto v17 = v16 - a3;
        auto v18 = v17 * t;
        a4 = a3 + v18;

    } break;
    case 2:
    case 3: {
        assert((uint32_t) index < (control_pts.size() - 1));

        assert(t >= 0.0f && t < 1.0001f);

        static vector3d stru_96A50C[4]{};

        if (!a5) {
            stru_96A50C[1] = this->control_pts.at(index);

            stru_96A50C[2] = this->control_pts.at(index + 1);

            if (index >= 1) {
                stru_96A50C[0] = this->control_pts.at(index - 1);

            } else {
                auto v8 = stru_96A50C[1] - stru_96A50C[2];
                stru_96A50C[0] = stru_96A50C[1] + v8;
            }

            vector3d v11;
            if ((unsigned int) index >= (this->control_pts.size() - 2)) {
                auto v12 = stru_96A50C[2] - stru_96A50C[1];
                v11 = stru_96A50C[2] + v12;
            } else {
                v11 = this->control_pts.at(index + 2);
            }

            stru_96A50C[3] = v11;
        }

        auto v15 = t * t;

        auto a5a = v15 * t;

        vector3d v16;
        if (v6 == 2) {
            v16 = sub_5C2C30(t, v15, a5a, stru_96A50C);
        } else if (v6 == 3) {
            v16 = sub_5C2B20(t, v15, a5a, stru_96A50C);
        } else {
            assert(0);
        }

        a4 = v16;
    } break;
    case 4: {
        this->compute_bezier_pos(index, t, a4);
        break;
    }
    default:

        assert(0);
        break;
    }
}

vector3d sub_5C2A20(Float a2, Float a3, Float a4, const vector3d *a5) {
    auto v11 = a4 * a5[3];
    auto v10 = ((-3.0f * a4) + (3.0f * a3)) * a5[2];
    auto v9 = (((3.0f * a4) - (6.0f * a3)) + (3.0f * a2)) * a5[1];
    auto v5 = ((((0.0 - a4) + (3.0f * a3)) - (3.0f * a2)) + 1.0f) * a5[0];
    auto v6 = v5 + v9;
    auto v7 = v6 + v10;

    auto a1 = v7 + v11;
    return a1;
}

void spline::compute_bezier_pos(int a2, Float a3, vector3d &a4) {
    if constexpr (1) {
        auto v4 = a3 - a2;

        auto v5 = v4 * v4;
        auto a4a = v5;
        auto a5 = v5 * v4;
        a4 = sub_5C2A20(v4, a4a, a5, &this->control_pts.m_first[3 * a2]);

    } else {
        THISCALL(0x005CE8B0, this, a2, a3, a4);
    }
}

Float spline::curve_length(Float a2) {
    return (Float) THISCALL(0x005DCFB0, this, a2);
}

void spline::rebuild_helper() {
    THISCALL(0x005DC830, this);
}

void spline::add_control_pt(const vector3d &a1)
{
    this->control_pts.push_back(a1);
    this->need_rebuild = true;
    this->field_3E = true;
}

void spline::clear()
{
    this->control_pts.clear();
    this->curve_pts.clear();
    this->need_rebuild = true;
}

void spline::reserve_control_pts(int a2)
{
    this->clear();
    this->control_pts.reserve(a2);
}

void spline::set_force_start(bool a2)
{
    auto v2 = this->field_3C != a2;
    this->need_rebuild = v2;
    this->field_3C = a2;
}

void spline_patch()
{
    {
        void (spline::*compute_spline_pos)(int index,
                                           Float a3,
                                           vector3d &a4,
                                           bool a5,
                                           spline::eSplineType a6) = &spline::compute_spline_pos;

        FUNC_ADDRESS(address, compute_spline_pos);
        SET_JUMP(0x005CE910, address);
    }
    {
        void (spline::*compute_spline_pos)(Float a3,
                                           vector3d & a4,
                                           bool a5,
                                           spline::eSplineType a6) = &spline::compute_spline_pos;

        FUNC_ADDRESS(address, compute_spline_pos);
        REDIRECT(0x005DB861, address);
    }
}
