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
#include "entity_mash.h"
#include "mic.h"
#include "time_interface.h"
#include "nal_system.h"
#include "nal_anim_controller.h"
#include <algorithm>
#include <array>

#include <cmath>

VALIDATE_SIZE(camera, 0xCCu);

#if STANDALONE_SYSTEM
namespace {
void __fastcall camera_destroy(camera *self, void *, bool release)
{
    self->~camera();
    if (release)
        mem_dealloc(self, sizeof(camera));
}
int __fastcall camera_size(camera *, void *)
{
    return sizeof(camera);
}
int __fastcall camera_flavor(camera *, void *)
{
    return 2;
}
bool __fastcall camera_identity(camera *, void *)
{
    return true;
}
void __fastcall camera_advance(camera *self, void *, Float dt)
{
    self->_frame_advance(dt);
}
void __fastcall camera_sync(camera *self, void *, camera *source)
{
    self->_sync(*source);
}
void __fastcall camera_geometry(camera *self, void *, bool analyzer)
{
    self->adjust_geometry_pipe(analyzer);
}
void __fastcall camera_fov(camera *self, void *, Float value)
{
    self->set_fov(value);
}
float __fastcall camera_get_fov(camera *self, void *)
{
    return self->get_fov();
}
float __fastcall camera_aspect(camera *, void *)
{
    return 1.0f;
}
float __fastcall camera_far(camera *self, void *)
{
    return self->get_far_plane_factor();
}
void __fastcall camera_set_far(camera *self, void *, Float value)
{
    self->set_far_plane_factor(value);
}
nal_anim_controller *__fastcall camera_animation(camera *self, void *, unsigned flags, nalBaseSkeleton *)
{
    return self->select_and_new_anim_controller(nalGetSkeleton(tlFixedString{"camera"}), flags);
}
}  // namespace
#endif

void *camera::native_vtable()
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    static auto table = [] {
        std::array<void *, 192> result;
        std::copy_n(reinterpret_cast<void **>(ent_v_table_lookup[3]), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(camera_destroy);
        result[1] = reinterpret_cast<void *>(camera_size);
        result[0x54 / 4] = reinterpret_cast<void *>(camera_flavor);
        result[0x6C / 4] = reinterpret_cast<void *>(camera_identity);
        result[0x1A4 / 4] = reinterpret_cast<void *>(camera_advance);
        result[0x214 / 4] = reinterpret_cast<void *>(camera_animation);
        result[0x294 / 4] = reinterpret_cast<void *>(camera_sync);
        result[0x298 / 4] = reinterpret_cast<void *>(camera_geometry);
        result[0x29C / 4] = reinterpret_cast<void *>(camera_fov);
        result[0x2A0 / 4] = reinterpret_cast<void *>(camera_get_fov);
        result[0x2A4 / 4] = reinterpret_cast<void *>(camera_aspect);
        result[0x2A8 / 4] = reinterpret_cast<void *>(camera_far);
        result[0x2AC / 4] = reinterpret_cast<void *>(camera_set_far);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x0088BDF0);
#endif
}

camera::camera(entity *parent, const string_hash &id) : actor(id, 0)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<int>(native_vtable());
    this->field_C4 = 1.5707964f;
    this->field_C8 = 1.0f;
    this->field_8 &= ~0x400000u;
    if (parent != nullptr) {
        this->set_parent(parent);
    }
    this->field_C0 = new mic(this, make_unique_entity_id());
    this->create_time_ifc();
    this->field_58->field_2C = 1;
    this->field_58->field_C = 1.0f;
#else
    THISCALL(0x00577970, this, parent, &id);
#endif
}

camera::~camera()
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<int>(native_vtable());
    if (field_C0) {
        auto destroy = reinterpret_cast<void(__fastcall *)(mic *, void *, bool)>(get_vfunc(field_C0->m_vtbl, 0));
        destroy(field_C0, nullptr, true);
    }
#endif
}

void camera::_sync(camera &source)
{
#if STANDALONE_SYSTEM
    if (!is_externally_controlled()) {
        if (m_parent) {
            po relative;
            po::full_inv_multiply(relative, m_parent->get_abs_po(), source.get_abs_po());
            set_abs_po(relative);
        } else {
            set_abs_po(source.get_abs_po());
        }
    }
#else
    THISCALL(0x0057EFD0, this, &source);
#endif
}

void camera::frame_advance(Float dt)
{
    auto advance = reinterpret_cast<void(__fastcall *)(camera *, void *, Float)>(get_vfunc(m_vtbl, 0x1A4));
    advance(this, nullptr, dt);
}

void camera::_frame_advance(Float dt)
{
    auto advance = reinterpret_cast<void(__fastcall *)(mic *, void *, Float)>(get_vfunc(field_C0->m_vtbl, 0x1A4));
    advance(field_C0, nullptr, dt);
}

void camera::set_far_plane_factor(Float factor)
{
    field_C8 = std::clamp(static_cast<float>(factor), 0.0001f, 1.0f);
}

void *camera::operator new(size_t size)
{
    return mem_alloc(size);
}

void camera::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
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
#if STANDALONE_SYSTEM
    this->field_C4 = fov;
#else
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x29C));
    func(this, nullptr, fov);
#endif
}

float camera::get_fov()
{
#if STANDALONE_SYSTEM
    return this->field_C4;
#else
    float(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x2A0));
    return func(this);
#endif
}

float camera::get_far_plane_factor()
{
#if STANDALONE_SYSTEM
    if (anim_ctrl) {
        auto animated_factor =
            reinterpret_cast<float(__fastcall *)(nal_anim_controller *, void *)>(get_vfunc(anim_ctrl->m_vtbl, 0x80));
        return std::clamp(animated_factor(anim_ctrl, nullptr) * 0.0001f, 0.0001f, 1.0f);
    }
    return this->field_C8;
#else
    float(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x2A8));
    return func(this);
#endif
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
#if STANDALONE_SYSTEM

    auto displacement = a3;
    auto *hit_region = reg;
    if (displacement.length2() > 0.00001f) {
        vector3d hit, normal;
        const auto end = arg10 + displacement;
        if (find_intersection(arg10,
                              end,
                              *local_collision::entfilter_line_segment_camera_collision,
                              *local_collision::obbfilter_lineseg_test,
                              &hit,
                              &normal,
                              &hit_region,
                              nullptr,
                              nullptr,
                              false)) {
            displacement = hit - arg10;
            const float distance = displacement.length();
            displacement *= (distance - LARGE_EPSILON) / distance;
        }
    }
    for (int attempt = 0; attempt < 5; ++attempt) {
        const auto position = arg10 + displacement;
        vector3d impact, normal;
        if (!find_sphere_intersection(position,
                                      a2,
                                      *local_collision::entfilter_sphere_camera_collision,
                                      *local_collision::obbfilter_sphere_test,
                                      &impact,
                                      &normal,
                                      nullptr,
                                      nullptr))
            return position;
        normal = position - impact;
        const float square = normal.length2();
        if (square <= 0.0f)
            break;
        const float distance = std::sqrt(square);
        displacement += normal * ((a2 + EPSILON - distance) / distance);
    }
    return arg10;
#else
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
#endif
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
