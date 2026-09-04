#include "camera.h"

#include "collide.h"
#include "common.h"
#include "custom_math.h"
#include "entity.h"
#include "func_wrapper.h"
#include "geometry_manager.h"
#include "local_collision.h"
#include "memory.h"
#include "oldmath_po.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"
#include "vtbl.h"

#include <cmath>

VALIDATE_SIZE(camera, 0xCCu);

camera::camera([[maybe_unused]] entity *a2, const string_hash &a3) : actor(a3, 0)
#if 1
{
#else
{
    THISCALL(0x00577970, this, a2, &a3);

#endif
}

void *camera::operator new(size_t size)
{
    return mem_alloc(size);
}

void camera::sync(camera &a2)
{
    if constexpr (1) {
        void(__fastcall * func)(camera *, void *, camera *) = CAST(func, get_vfunc(m_vtbl, 0x294));

        func(this, nullptr, &a2);

    } else {
        THISCALL(0x0057EFD0, this, &a2);
    }
}

void camera::set_fov(Float fov)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x29C));
    func(this, nullptr, fov);
}

float camera::get_fov()
{
    float(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x2A0));
    return func(this);
}

float camera::get_far_plane_factor()
{
    float(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x2A8));
    return func(this);
}

void camera::adjust_geometry_pipe(bool scene_analyzer)
{
    TRACE("camera::adjust_geometry_pipe");

    auto &v10 = this->get_abs_position();

    auto &v3 = this->get_abs_po();
    auto &v4 = v3.get_matrix();

    vector3d center = v4[2] + v10;
    vector3d up = v4[1];
    if (scene_analyzer) {
        assert(!is_externally_controlled());

        auto &eye = this->get_abs_position();

        matrix4x4 view_mat;
        geometry_manager::set_look_at(&view_mat, eye, center, up);
        geometry_manager::set_xform(static_cast<geometry_manager::xform_t>(8), view_mat);
    } else {
        auto &v9 = this->get_abs_position();
        geometry_manager::set_view(v9, center, up);
    }
}

float camera::compute_xz_projected_fov()
{
    auto v6 = this->sub_57CB80();
    auto v1 = 1.f / geometry_manager::get_aspect_ratio();
    auto v5 = v1;
    auto v2 = std::cos(std::atan2(v1, 1.0) - std::abs(v6));
    if (v2 < v5) {
        v2 = v5;
    }

    auto v3 = std::atan2(v2 * std::sqrt(v5 * v5 + 1.f), 1.0);
    auto compensated_fov = v3 + v3 + EPSILON;

    assert(compensated_fov < PI);

    return compensated_fov;
}

bool camera::is_externally_controlled() const
{
    return this->is_flagged(0x400000);
}

void camera::get_look_and_up(vector3d *look, vector3d *up)
{
    if (look != nullptr) {
        *look = this->get_abs_po().m[2];
    }

    if (up != nullptr) {
        *up = this->get_abs_po().m[1];
    }
}

vector3d collide_with_world(camera *, const vector3d &a3, float a2, const vector3d &arg10, region *reg)
{
    auto a1 = arg10;
    auto *a7 = reg;
    if (a1.length2() > 0.0000099999997) {
        vector3d a5{};
        vector3d a6{};
        auto *v13 = local_collision::obbfilter_lineseg_test;
        auto *v12 = local_collision::entfilter_line_segment_camera_collision;
        auto v6 = a3 + a1;
        if (find_intersection(a3, v6, *v12, *v13, &a5, &a6, &a7, nullptr, nullptr, false)) {
            a1 = a5 - a3;
            auto v26 = a1.length();
            a1 *= ((v26 - 0.0099999998) / v26);
        }
    }

    vector3d v25{};
    int v24 = 0;
    bool v23 = false;
    do {
        auto v25 = a3;
        v25 += a1;
        vector3d impact_normal{};
        vector3d impact_pos{};
        if (find_sphere_intersection(v25,
                                     a2,
                                     *local_collision::entfilter_sphere_camera_collision,
                                     *local_collision::obbfilter_sphere_test,
                                     &impact_pos,
                                     &impact_normal,
                                     nullptr,
                                     nullptr)) {
            impact_normal = v25 - impact_pos;
            auto v19 = impact_normal.length2();
            if (v19 <= 0.0) {
                v24 = 5;
            } else {
                v19 = std::sqrt(v19);
                impact_normal *= 1.0 / v19;
                auto v9 = ((a2 + 0.000099999997) - v19) * impact_normal;
                a1 = a1 + v9;
                ++v24;
            }
        } else {
            v23 = true;
        }
    } while (!v23 && v24 < 5);

    vector3d result = (v23 ? v25 : a3);
    return result;
}

bool camera::_is_a_camera() const
{
    TRACE("camera::is_a_camera");

    return true;
}

void camera_patch()
{
    {
        FUNC_ADDRESS(address, &camera::_is_a_camera);
        set_vfunc(0x0088BDF0 + 0x6C, address);
        set_vfunc(0x008820E0 + 0x6C, address);
        set_vfunc(0x00881B50 + 0x6C, address);
    }

    {
        FUNC_ADDRESS(address, &camera::adjust_geometry_pipe);
        SET_JUMP(0x00577AF0, address);
    }
}
