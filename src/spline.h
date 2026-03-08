#pragma once

#include "vector3d.h"

#include "config.h"

#include <vector.hpp>

struct spline {
    enum eSplineType {
    };

    _std::vector<vector3d> control_pts;
    _std::vector<float> control_pts_pct;
    _std::vector<vector3d> curve_pts;
    int field_30;
    int field_34;
    eSplineType field_38;
    bool field_3C;
    bool need_rebuild;
    bool field_3E;
    float field_40;
    vector3d field_44;

    //0x00508410
    spline();

    //0x0048F030
    void set_control_pt(int32_t index, const vector3d &a3);

    vector3d &get_control_pt(int index);

    //0x0048DA10
    void build(int a2, spline::eSplineType a3);

    //0x005DB4A0
    void compute_spline_pos(Float a3, vector3d &a4, bool a5, spline::eSplineType a6);

    //0x005CE910
    void compute_spline_pos(int index, Float a3, vector3d &a4, bool a5, spline::eSplineType a6);

    void compute_bezier_pos(int a2, Float a3, vector3d &a4);

    //0x005DCFB0
    Float curve_length(Float a2);

    //0x005DC830
    void rebuild_helper();

    void add_control_pt(const vector3d &a1);

    void clear();

    //0x0048FCE0
    void reserve_control_pts(int a2);

    void set_force_start(bool a2);

    inline uint32_t get_num_control_pts() {
        return control_pts.size();
    }
};

extern void spline_patch();
