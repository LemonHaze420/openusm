#include "camera_frame.h"

#include "algebra.h"
#include "camera_target_info.h"
#include "custom_math.h"
#include "oldmath_po.h"
#include "oldmath_usefulmath.h"
#include "spiderman_camera.h"
#include "variables.h"

#include <cmath>

camera_frame::camera_frame(const po &a2)
    : eye(a2.get_position()),
      fwd(a2.get_z_facing()),
      up(YVEC)
{
    assert(is_valid());
}

bool camera_frame::is_valid() const
{
    return this->eye.is_valid()
      && this->fwd.is_valid()
      && this->fwd.is_normal();
}

vector3d camera_frame::fix_up_vector(vector3d upn) const
{
    assert(upn.is_normal());

    assert(fwd.is_valid());

    assert(fwd.is_normal());

    auto v3 =  sub_87D3A0(this->fwd, YVEC);
    auto v4 = v3.normalized();

    auto v5 = sub_87D3A0(upn, v4);
    upn = v5.normalized();

    assert(upn.is_valid());

    return upn;
}

po make_look_at(const vector3d &a1, const vector3d &a2, const vector3d &a3)
{
    po v19 {};

    auto v12 = a2 - a1;
    auto v17 = v12.normalized();

    auto v18 = sub_87D3A0(a3, v17);

    v19.set_po(v17, v18, a1);
    return v19;
}

po camera_frame::get_po() const
{
    auto v8 = this->fwd + this->eye;
    auto upn = this->fix_up_vector(this->up);
    auto a2 = make_look_at(this->eye, v8, upn);
    return a2;
}

vector3d camera_frame::get_right() const
{
    auto r = vector3d::cross(this->up, this->fwd);
    assert(r.length2() > 0);

    r.normalize();
    return r;
}

vector3d projected_to_plane_rel(vector3d a2, vector3d normal, vector3d a4)
{
    auto v4 = a2 - a4;

    auto v1 = [=]() -> vector3d {
        assert(normal.is_normal());

        auto v6 = dot(v4, normal);
        return v4 - normal * v6;
    }();
    
    vector3d result = v1 + a4;
    return result;
}

bool sub_4B21E0(vector3d &a1, float a2)
{
    auto v4 = a1.length2();
    if ( v4 >= sqr(a2 - LARGE_EPSILON) ) {
        return false;
    }

    if ( v4 > 0.0f )
    {
        auto v2 = a2 / std::sqrt(v4);
        a1 *= v2;
    }

    return true;
}

void push_sphere(vector3d &a1, float a2, vector3d a3)
{
    vector3d v7 = a1 - a3;
    sub_4B21E0(v7, a2);

    a1 = v7 + a3;
}


void sub_4B26F0(vector3d &a1, float a2, vector3d a3, vector3d a6)
{
    auto v5 = projected_to_plane_rel(a3, a6, a1);
    push_sphere(a1, a2, v5);
}

void camera_frame::avoid_target(const camera_target_info &a1, Float a2)
{
    sub_4B26F0(this->eye, a2, a1.pos, YVEC);
}

void camera_frame::include_target(vector3d a2, Float a5, Float a6)
{
    auto v21 = a2 - this->eye;
    auto v6 = v21.length2();
    if ( v6 > EPSILON )
    {
        auto v7 = sqr(a6) * v6;
        auto v20 = std::sqrt(v6);
        auto v8 = std::sqrt(v6 - v7) - a5;

        v8 = std::max(0.0f, v8);

        a6 = std::sqrt(v7 / (v7 + sqr(v8)));

        a6 = std::min<float>(a6, 0.98000002f);
        auto v14 = 1.0f / v20;
        v21 *= v14;
        constrain_normal(this->fwd, v21, a6, 1.0);
        if ( v21[1] < 0.99900001f )
        {
            auto v15 = vector3d::cross(YVEC, v21);
            auto v17 = v15.normalized();
            auto v22 = vector3d::cross(v21, v17);
            a6 = std::sqrt(1.0f - sqr(a6));
            auto v19 = a6 * -0.5f;
            constrain_normal(this->fwd, v22, v19, a6);
        }
    }
}

void camera_frame::rotate_to_include_target(
        vector3d a2,
        vector3d target_pos,
        [[maybe_unused]] vector3d a4,
        Float a11)
{
    auto v8 = this->eye;
    auto v9 = target_pos - a2;
    auto v10 = v9.length2();
    if ( v10 > EPSILON )
    {
        auto v11 = 1.0f / std::sqrt(v10);

        v9 *= v11;
        this->constrain_pos_relative_to_plane(target_pos, v9, a11, 1.0f);

        this->eye = lerp(this->eye, v8, pronto_mix);
    }
}

void camera_frame::constrain_pos_relative_to_plane(const vector3d &a3, const vector3d &a1, Float a4, Float a5)
{
    constrain_relative_to_plane(this->eye, a3, a1, a4, a5);
}

void camera_frame::smooth_with(const camera_frame &a2, Float a3, Float a4)
{
    this->eye = lerp(this->eye, a2.eye, a3);

    this->smooth_dir(a2, a4);
}

void camera_frame::smooth_dir(const camera_frame &a2, Float a3)
{
    auto v2 = lerp(this->fwd, a2.fwd, a3);
    this->fwd = v2.normalized();

    auto v5 = lerp(this->up, a2.up, a3);
    auto v12 = v5.normalized();

    this->up = this->fix_up_vector(v12);
}
