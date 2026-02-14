#include "geometry_manager.h"

#include "aarect.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "matrix4x4.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "trace.h"
#include "utility.h"
#include "vector2d.h"
#include "vector3d.h"

#ifdef USE_GLM
#include "glm/ext/matrix_transform.hpp"
#endif

#include <cassert>
#include <cmath>
#include <cstring>

namespace geometry_manager {

Var<float> PROJ_FIELD_OF_VIEW = (0x0095FC5C);

Var<float> PROJ_ZOOM = (0x00921E24);

Var<float> PROJ_FAR_PLANE_D = (0x00921E2C);

Var<bool> scene_analyzer_enabled = (0x0095C1E8);

Var<bool> auto_rebuild_view_frame = (0x0095C1E9);

Var<bool> view_frame_dirty = (0x0095C1EA);

Var<matrix4x4[NUM_XFORMS]> xforms {0x0095FC60};

void set_xform(xform_t xformtype, const matrix4x4 &a2)
{
    assert(xformtype != XFORM_WORLD_TO_SCREEN
            && xformtype != XFORM_VIEW_TO_SCREEN
            && xformtype != XFORM_EFFECTIVE_WORLD_TO_VIEW
            && xformtype != XFORM_WORLD_TO_PROJECTION
            && xformtype != XFORM_VIEW_TO_WORLD);

    if constexpr (1)
    {
        if (xformtype == XFORM_WORLD_TO_VIEW)
        {
            xforms()[XFORM_EFFECTIVE_WORLD_TO_VIEW] = (scene_analyzer_enabled() ? xforms()[8] : a2);

            po v13{a2};
            auto *v3 = v13.inverse();
            xforms()[XFORM_VIEW_TO_WORLD] = v3->get_matrix();
        }

        if (xformtype == 8 && scene_analyzer_enabled()) {
            xforms()[XFORM_EFFECTIVE_WORLD_TO_VIEW] = a2;
        }

        xforms()[xformtype] = a2;
        if (xformtype == XFORM_WORLD_TO_VIEW
            || xformtype == XFORM_VIEW_TO_PROJECTION
            || xformtype == XFORM_PROJECTION_TO_SCREEN)
        {
            if (xformtype == XFORM_VIEW_TO_PROJECTION || xformtype == XFORM_PROJECTION_TO_SCREEN) {
                xforms()[XFORM_VIEW_TO_SCREEN] = xforms()[XFORM_VIEW_TO_PROJECTION] * xforms()[XFORM_PROJECTION_TO_SCREEN];
            }

            xforms()[XFORM_WORLD_TO_SCREEN] = xforms()[XFORM_EFFECTIVE_WORLD_TO_VIEW] * xforms()[XFORM_VIEW_TO_SCREEN];

            xforms()[XFORM_WORLD_TO_PROJECTION] = xforms()[XFORM_EFFECTIVE_WORLD_TO_VIEW] * xforms()[XFORM_VIEW_TO_PROJECTION];
            if (nglCurScene != nullptr) {
                math::MatClass<4, 3> v8 = xforms()[XFORM_EFFECTIVE_WORLD_TO_VIEW];
                nglSetWorldToViewMatrix(v8);
            }
        }

    }
    else
    {
        CDECL_CALL(0x00515750, xformtype, &a2);
    }
}

matrix4x4 &get_xform(xform_t a1) {
    return xforms()[a1];
}

} // namespace geometry_manager

Var<float> FAR_CLIP_PLANE = (0x00921E30);

void sub_515320() {
    for (uint8_t i = 0; i < 9; ++i) {
        geometry_manager::xforms()[i] = identity_matrix;
    }
}

void geometry_manager::set_aspect_ratio(Float a1) {
    if (not_equal(PROJ_ASPECT(), a1.value)) {
        PROJ_ASPECT() = a1;

        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }
    }
}

bool geometry_manager::is_scene_analyzer_enabled() {
    return scene_analyzer_enabled();
}

void geometry_manager::enable_scene_analyzer(bool a1)
{
    scene_analyzer_enabled() = a1;
}

void geometry_manager::reset() {
    scissor_rect().field_0[0][0] = -1.0;
    scissor_rect().field_0[0][1] = -1.0;
    scissor_rect().field_0[1][0] = 1.0;
    scissor_rect().field_0[1][1] = 1.0;

    viewport_rect().field_0[0][0] = -1.0;
    viewport_rect().field_0[0][1] = -1.0;
    viewport_rect().field_0[1][0] = 1.0;
    viewport_rect().field_0[1][1] = 1.0;

    PROJ_FAR_PLANE_D() = os_developer_options::instance->get_int(mString{"FAR_CLIP_PLANE"});
    FAR_CLIP_PLANE() = PROJ_FAR_PLANE_D();

    rebuild_view_frame();
}

void geometry_manager::compute_view_frustum_in_world_space() {
    po v11 = xforms()[1];

    world_space_frustum().field_0.m_size = 0;

    for (auto i = 0u; i < view_frustum().field_0.m_size; ++i) {
        auto &v1 = view_frustum().field_0.at(i).arr;

        float v7[4];
        v7[0] = v1[0];
        v7[1] = v1[1];
        v7[2] = v1[2];
        v7[3] = v1[3];

        auto v6 = v11.get_position();

        vector3d v9 = v11.non_affine_slow_xform({v7[0], v7[1], v7[2]});
        if (i == 2) {
            v6 += v11.get_z_facing() * PROJ_FAR_PLANE_D();
        }

        plane v5{v6, v9};
        world_space_frustum().add_face(v5);
    }
}

void geometry_manager::compute_view_frustum_verts_in_world_space() {
    auto a1 = PROJ_FIELD_OF_VIEW() * 0.5f;

    float a2, a3;
    fast_sin_cos_approx(a1, &a2, &a3);
    a3 = std::abs(a3);

    a2 = std::abs(a2);

    auto v6 = a2 / PROJ_ASPECT();

    frustum_verts() = {};

    frustum_verts().push_back(ZEROVEC);

    frustum_verts().push_back(vector3d{-a2, v6, a3});

    frustum_verts().push_back(vector3d{a2, v6, a3});

    frustum_verts().push_back(vector3d{a2, -v6, a3});

    frustum_verts().push_back(vector3d{-a2, -v6, a3});

    po v9 = xforms()[1];

    for (auto i = 0u; i < frustum_verts().size(); ++i) {
        if (i > 0) {
            auto &v = frustum_verts().at(i);
            v.normalize();
        }

        auto &v = frustum_verts().at(i);

        v = v9.slow_xform(v);
    }
}

void geometry_manager::set_viewport(const aarect<float, vector2d> &a1)
{
    TRACE("geometry_manager::set_viewport");

    if constexpr (1) {
        viewport_rect() = a1;

        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }

    } else {
        CDECL_CALL(0x00540130, &a1);
    }
}

void geometry_manager::set_look_at(matrix4x4 *a1,
                                   const vector3d &eye,
                                   const vector3d &center,
                                   const vector3d &up) {
#ifndef USE_GLM
    if constexpr (1) {
        vector3d f = center - eye;
        auto fLength = f.length();

        static constexpr auto flt_87E6F8 = 1e-6f;

        if (fLength <= flt_87E6F8) {
            fLength = 1.0f;
            f = ZVEC;
        }

        f /= fLength;

        auto u = up - f * dot(up, f);

        fLength = u.length();
        if (fLength < flt_87E6F8) {
            u = YVEC - f * f[1];

            fLength = u.length();
            if (fLength < flt_87E6F8) {
                u = ZVEC - f * f[2];

                fLength = u.length();

                assert(fLength >= 1e-6f);
            }
        }

        u /= fLength;

        vector3d s = vector3d::cross(u, f);

        matrix4x4 v38 = identity_matrix;

        v38[0][0] = s[0];
        v38[1][0] = s[1];
        v38[2][0] = s[2];

        v38[0][1] = u[0];
        v38[1][1] = u[1];
        v38[2][1] = u[2];

        v38[0][2] = f[0];
        v38[1][2] = f[1];
        v38[2][2] = f[2];

        v38[3][0] = -dot(eye, s);
        v38[3][1] = -dot(eye, u);
        v38[3][2] = -dot(eye, f);

        *a1 = v38;
    } else {
        CDECL_CALL(0x005153E0, a1, &eye, &center, &up);
    }
#else

    glm::vec3 a = *bit_cast<glm::vec3 *>(&eye), b = *bit_cast<glm::vec3 *>(&center),
              c = *bit_cast<glm::vec3 *>(&up);
    auto result = glm::lookAtLH(a, b, c);

    *a1 = *bit_cast<matrix4x4 *>(&result);

#endif
}

bool geometry_manager::get_auto_rebuild_view_frame()
{
    return auto_rebuild_view_frame();
}

void geometry_manager::set_auto_rebuild_view_frame(bool a1)
{
    auto_rebuild_view_frame() = a1;
}

void geometry_manager::set_view(const vector3d &a1, const vector3d &a2, const vector3d &a3)
{
    if constexpr (1)
    {
        matrix4x4 v1;
        set_look_at(&v1, a1, a2, a3);
        set_xform(XFORM_WORLD_TO_VIEW, v1);
    }
    else
    {
        CDECL_CALL(0x00524930, &a1, &a2, &a3);
    }
}

void geometry_manager::set_scissor(const aarect<float, vector2d> &a1)
{
    TRACE("geometry_manager::set_scissor");

    if constexpr (1)
    {
        scissor_rect() = a1;
        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }

    }
    else
    {
        CDECL_CALL(0x00540170, &a1);
    }
}

void geometry_manager::create_inst() {
    scene_analyzer_enabled() = false;
    auto_rebuild_view_frame() = true;
    view_frame_dirty() = true;
    sub_515320();
    reset();
}

float geometry_manager::get_aspect_ratio() {
    return PROJ_ASPECT();
}

float geometry_manager::get_field_of_view() {
    return PROJ_FIELD_OF_VIEW();
}

void geometry_manager::set_far_plane(Float far_plane) {
    if (not_equal(PROJ_FAR_PLANE_D(), float{far_plane})) {
        PROJ_FAR_PLANE_D() = far_plane;

        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }
    }
}

void geometry_manager::set_field_of_view(Float fov) {
    if (not_equal(PROJ_FIELD_OF_VIEW(), float{fov})) {
        PROJ_FIELD_OF_VIEW() = fov;

        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }
    }
}

void geometry_manager::set_zoom(Float zoom) {
    if (not_equal(PROJ_ZOOM(), float{zoom})) {
        PROJ_ZOOM() = zoom;

        if (auto_rebuild_view_frame()) {
            rebuild_view_frame();
        } else {
            view_frame_dirty() = true;
        }
    }
}

float geometry_manager::get_zoom() {
    return PROJ_ZOOM();
}

void geometry_manager::rebuild_view_frame() {
    CDECL_CALL(0x0053A930);
}

void geometry_manager_patch()
{
    SET_JUMP(0x00540170, geometry_manager::set_scissor);

    SET_JUMP(0x00540130, geometry_manager::set_viewport);

    SET_JUMP(0x005153E0, geometry_manager::set_look_at);

    REDIRECT(0x00577BC1, geometry_manager::set_view);
}
