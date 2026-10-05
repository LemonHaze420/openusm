#include "spline.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

#include <algorithm>
#include <cassert>

VALIDATE_OFFSET(spline, field_3C, 0x3C);
VALIDATE_SIZE(spline, 0x50u);

spline::spline()
{
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

void spline::set_control_pt(int32_t index, const vector3d &a3)
{
    assert(index >= 0);
    assert(index < (int)control_pts.size());

    if (index >= 0 && index < (int)control_pts.size()) {
        this->control_pts[index] = a3;
        this->need_rebuild = true;
        this->field_3E = true;
    }
}

vector3d &spline::get_control_pt(int index)
{
    assert(index >= 0 && index < (int)control_pts.size());

    return this->control_pts[index];
}

void spline::build(int a2, spline::eSplineType a3)
{
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

void spline::compute_spline_pos(Float percent, vector3d &position, bool reuse_controls, spline::eSplineType type)
{
    const int count = static_cast<int>(control_pts.size());
    if (field_3E || control_pts_pct.size() != control_pts.size()) {
        if (control_pts_pct.size() != control_pts.size()) {
            control_pts_pct.clear();
            control_pts_pct.reserve(count);
            for (int i = 0; i < count; ++i)
                control_pts_pct.push_back(0.0f);
        }
        control_pts_pct[0] = 0.0f;
        if (count == 1) {
            control_pts_pct[0] = 1.0f;
        } else if (count > 1) {
            float total = 0.0f;
            for (int i = 1; i < count; ++i) {
                total += (control_pts[i] - control_pts[i - 1]).length();
                control_pts_pct[i] = total;
            }
            const float inverse = 1.0f / total;
            for (int i = 1; i < count - 1; ++i) {
                const double cumulative = inverse * static_cast<double>(control_pts_pct[i]);
                control_pts_pct[i] = cumulative > 1.0 ? 1.0f
                    : cumulative < 0.0 ? 0.0f : static_cast<float>(cumulative);
            }
            control_pts_pct[count - 1] = 1.0f;
        }
        field_3E = false;
    }
    if (count <= 1) {
        if (count == 1)
            position = control_pts[0];
        return;
    }
    float amount = percent.value;
    if (amount > 1.0f)
        amount = 1.0f;
    else if (amount < 0.0f)
        amount = 0.0f;
    int index = 0;
    while (control_pts_pct[index + 1] < amount)
        ++index;
    const double interval = static_cast<double>(control_pts_pct[index + 1]) - control_pts_pct[index];
    const float local = interval <= 0.0 ? 0.0f
        : static_cast<float>((amount - control_pts_pct[index]) / interval);
    compute_spline_pos(index, Float{local}, position, reuse_controls, type);
}

vector3d spline::calc_point_at_percent(float percent)
{
    vector3d position;
    compute_spline_pos(Float{percent}, position, false, static_cast<eSplineType>(0));
    return position;
}

vector3d sub_5C2C30(float a3, float a4, float a5, const vector3d *a6)
{
    constexpr float v18 = 2.0f;
    constexpr float v19 = 3.0f;

    auto v12 = (a5 - a4) * a6[3];
    auto v11 = (-v19 * a5 + 4.0f * a4 + a3) * a6[2];
    auto v10 = (v19 * a5 - 5.0f * a4 + v18) * a6[1];
    auto v5 = (-a5 + v18 * a4 - a3) * a6[0];

    auto result = (v5 + v10 + v11 + v12) * 0.5f;
    return result;
}

vector3d sub_5C2B20(float a3, float a4, float a5, const vector3d *a6)
{
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

void spline::compute_spline_pos(int index, Float t, vector3d &a4, bool a5, spline::eSplineType a6)
{

    auto v6 = a6;
    if (a6 == 0) {
        v6 = this->field_38;
    }

    switch (v6) {
    case 1: {
        assert((uint32_t)index < (control_pts.size() - 1));

        assert(t >= 0.0f && t < 1.0001f);

        auto a3 = this->control_pts.at(index);
        auto v16 = this->control_pts.at(index + 1);

        auto v17 = v16 - a3;
        auto v18 = v17 * t;
        a4 = a3 + v18;

    } break;
    case 2:
    case 3: {
        assert((uint32_t)index < (control_pts.size() - 1));

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
            if ((unsigned int)index >= (this->control_pts.size() - 2)) {
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

vector3d sub_5C2A20(Float a2, Float a3, Float a4, const vector3d *a5)
{
    auto v11 = a4 * a5[3];
    auto v10 = ((-3.0f * a4) + (3.0f * a3)) * a5[2];
    auto v9 = (((3.0f * a4) - (6.0f * a3)) + (3.0f * a2)) * a5[1];
    auto v5 = ((((0.0 - a4) + (3.0f * a3)) - (3.0f * a2)) + 1.0f) * a5[0];
    auto v6 = v5 + v9;
    auto v7 = v6 + v10;

    auto a1 = v7 + v11;
    return a1;
}

void spline::compute_bezier_pos(int a2, Float a3, vector3d &a4)
{
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

Float spline::curve_length(Float a2)
{
    return (Float)THISCALL(0x005DCFB0, this, a2);
}

void spline::rebuild_helper()
{
    need_rebuild = false;
    curve_pts.clear();
    const int count = static_cast<int>(control_pts.size());
    const int effective_count = count + (field_3C ? 2 : 0);
    if (field_38 == 1 || effective_count < 4 || field_30 <= 1) {
        curve_pts = control_pts;
    } else {
        const int segments = field_38 == 4 ? (effective_count - 1) / 3 : effective_count - 3;
        curve_pts.reserve(field_30 * segments + 1);
        int index = field_3C ? 0 : 1;
        while (index + 1 < count && (field_3C ? index + 2 <= count : index + 2 < count)) {
            const vector3d points[4]{
                index == 0 ? control_pts[0] * 2.0f - control_pts[1] : control_pts[index - 1],
                control_pts[index], control_pts[index + 1],
                index + 2 >= count ? control_pts[index + 1] * 2.0f - control_pts[index] : control_pts[index + 2]};
            for (int sample = 0; sample < field_30; ++sample) {
                const float t = static_cast<float>(sample) / static_cast<float>(field_30 - 1);
                const float t2 = t * t;
                const float t3 = t2 * t;
                switch (field_38) {
                case 2: curve_pts.push_back(sub_5C2C30(t, t2, t3, points)); break;
                case 3: curve_pts.push_back(sub_5C2B20(t, t2, t3, points)); break;
                case 4: curve_pts.push_back(sub_5C2A20(t, t2, t3, points)); break;
                }
            }
            index += field_38 == 4 ? 3 : 1;
        }
    }
    if (count == 0) {
        field_44 = vector3d{0.0f, 0.0f, 0.0f};
        field_40 = 0.0f;
        return;
    }
    vector3d minimum{999999.0f, 999999.0f, 999999.0f};
    vector3d maximum{-999999.0f, -999999.0f, -999999.0f};
    for (const auto &point : curve_pts) {
        for (int axis = 0; axis != 3; ++axis) {
            minimum[axis] = std::min(minimum[axis], point[axis]);
            maximum[axis] = std::max(maximum[axis], point[axis]);
        }
    }
    field_44 = (maximum + minimum) * 0.5f;
    field_40 = (maximum - field_44).length();
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
        void (spline::*compute_spline_pos)(int index, Float a3, vector3d &a4, bool a5, spline::eSplineType a6) =
            &spline::compute_spline_pos;

        FUNC_ADDRESS(address, compute_spline_pos);
        SET_JUMP(0x005CE910, address);
    }
    {
        void (spline::*compute_spline_pos)(Float a3, vector3d &a4, bool a5, spline::eSplineType a6) =
            &spline::compute_spline_pos;

        FUNC_ADDRESS(address, compute_spline_pos);
        REDIRECT(0x005DB861, address);
    }
}
