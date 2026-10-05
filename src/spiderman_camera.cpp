#include "spiderman_camera.h"

#include "camera_frame.h"
#include "camera_mode.h"
#include "camera_target_info.h"
#include "common.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "trace.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"

#include <cmath>
#include <algorithm>
#include <array>
#include <cstdlib>

VALIDATE_SIZE(spiderman_camera, 0x204u);
VALIDATE_OFFSET(spiderman_camera, field_1A0, 0x1A0);

float g_yaw_mult = 2.0f;

float g_pitch_mult = 2.0f;


void set_filter_time(float dt)
{
    static const std::array<float, 5> filter_bases{
        static_cast<float>(std::pow(0.03999999910593033, 1.0)),
        static_cast<float>(std::pow(0.03999999910593033, 2.0)),
        static_cast<float>(std::pow(0.03999999910593033, 4.0)),
        static_cast<float>(std::pow(0.03999999910593033, 8.0)),
        static_cast<float>(std::pow(0.03999999910593033, 16.0)),
    };
    sluggish_mix = std::pow(filter_bases[0], dt);
    slow_mix = std::pow(filter_bases[1], dt);
    med_mix = std::pow(filter_bases[2], dt);
    fast_mix = std::pow(filter_bases[3], dt);
    pronto_mix = std::pow(filter_bases[4], dt);
}

static Var<vector3d> stru_959EBC{0x00959EBC};

void constrain_normal(vector3d &normal, const vector3d &basisA, float a4, float a5)
{
    assert(normal.is_normal());

    assert(basisA.is_normal());

    auto a3a = dot(normal, basisA);
    auto v8 = basisA * a3a;
    auto v18 = normal - v8;
    a3a = std::clamp(a3a, a4, a5);
    auto v9 = 1.0 - sqr(a3a);
    auto v17 = std::sqrt(v9);
    auto v16 = v18.length2();
    if (v16 > EPSILON) {
        auto v5 = (1.0f / std::sqrt(v16));
        auto v10 = v5 * v17;
        auto v11 = v18 * v10;
        auto v6 = basisA * a3a;
        normal = v6 + v11;
    }

    assert(normal.is_normal());
}

Var<spiderman_camera *> g_spiderman_camera_ptr{0x00959A70};

#if STANDALONE_SYSTEM
namespace {
void *__fastcall destroy_chase_camera(spiderman_camera *self, void *, unsigned int flags)
{
    self->~spiderman_camera();
    if ((flags & 1u) != 0) {
        spiderman_camera::operator delete(self);
    }
    return self;
}

int __fastcall chase_camera_flavor(spiderman_camera *, void *)
{
    return 21;
}

bool __fastcall chase_camera_is_spiderman(spiderman_camera *, void *)
{
    return true;
}

void __fastcall advance_chase_camera(spiderman_camera *self, void *, Float dt)
{
    self->_frame_advance(dt);
}

void __fastcall render_chase_camera(spiderman_camera *, void *, Float) {}

void __fastcall sync_chase_camera(spiderman_camera *self, void *, camera *source)
{
    self->_sync(*source);
}

void __fastcall adjust_chase_camera(spiderman_camera *self, void *, bool scene_analyzer)
{
    self->adjust_geometry_pipe(scene_analyzer);
}

void __fastcall set_chase_target(spiderman_camera *self, void *, entity *target)
{
    self->_set_target_entity(target);
}

void __fastcall recenter_chase_camera(spiderman_camera *self, void *, Float dt)
{
    self->_autocorrect(dt);
}
}  // namespace
#endif

void *spiderman_camera::native_vtable()
{
#if STANDALONE_SYSTEM
    static const auto table = [] {
        std::array<void *, 192> result;
        auto *base = static_cast<void **>(game_camera::native_vtable());
        std::copy_n(base, result.size(), result.begin());
        result[0x000 / 4] = reinterpret_cast<void *>(&destroy_chase_camera);
        result[0x054 / 4] = reinterpret_cast<void *>(&chase_camera_flavor);
        result[0x08C / 4] = reinterpret_cast<void *>(&chase_camera_is_spiderman);
        result[0x1A4 / 4] = reinterpret_cast<void *>(&advance_chase_camera);
        result[0x1AC / 4] = reinterpret_cast<void *>(&render_chase_camera);
        result[0x294 / 4] = reinterpret_cast<void *>(&sync_chase_camera);
        result[0x298 / 4] = reinterpret_cast<void *>(&adjust_chase_camera);
        result[0x2B4 / 4] = reinterpret_cast<void *>(&set_chase_target);
        result[0x2D0 / 4] = reinterpret_cast<void *>(&recenter_chase_camera);
        return result;
    }();
    return const_cast<void **>(table.data());
#else
    return reinterpret_cast<void *>(0x008820E0);
#endif
}

spiderman_camera::spiderman_camera(const string_hash &a2, entity *a3) : game_camera(a2, a3)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    this->field_1BC = false;
    this->field_1C0 = 0;
    this->field_1C4 = 0;
    this->field_1C8 = 0;
    this->field_1CC = false;
    this->field_1CD = true;
    this->field_1CE = true;
    this->field_1CF = false;
    game_camera::set_target_entity(a3);
    auto *target = this->get_target_entity();
    this->target_pos = target->get_abs_position();
    this->target_up = target->get_abs_po().get_y_facing();
    this->field_1A0 = create_native_camera_modes(this);
    this->field_1D0.set_id(input_mgr::instance->field_58);
    this->field_1D0.set_control(static_cast<game_control_t>(102));
#else
    THISCALL(0x004B78E0, this, &a2, a3);
#endif
}

spiderman_camera::~spiderman_camera()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#if STANDALONE_SYSTEM
    destroy_native_camera_modes(this->field_1A0);
#else
    if (this->field_1A0 != nullptr) {
        this->field_1A0->m_vtbl->finalize(this->field_1A0, nullptr, 1);
    }
#endif
}

void *spiderman_camera::operator new(size_t size)
{
#if STANDALONE_SYSTEM
    return _aligned_malloc(size, 4);
#else
    using aligned_malloc_t = void *(__cdecl *)(size_t, size_t);
    auto aligned_malloc = *bit_cast<aligned_malloc_t *>(0x0086F354);
    return aligned_malloc(size, 4);
#endif
}

void spiderman_camera::operator delete(void *ptr)
{
#if STANDALONE_SYSTEM
    _aligned_free(ptr);
#else
    using aligned_free_t = void(__cdecl *)(void *);
    auto aligned_free = *bit_cast<aligned_free_t *>(0x0086F328);
    aligned_free(ptr);
#endif
}

void spiderman_camera::sub_4B3260(bool a2)
{
    this->field_1BC = a2;
}

void spiderman_camera::set_fixedstatic(const vector3d &a2, const vector3d &a3)
{
    this->field_1A0->set_fixedstatic(a2, a3);
}

void spiderman_camera::sub_4B3220(const vector3d &a2)
{
    stru_959EBC() = a2;
    this->field_1CC = true;
}

void spiderman_camera::_sync(camera &a2)
{
    game_camera::_sync(a2);
    set_filter_time(0.0);
    auto *target = this->get_target_entity();
    auto y_facing = target->get_abs_po().get_y_facing();

    camera_target_info v11{target, 0.033333335, target->get_abs_position(), y_facing};

    this->target_pos = v11.pos;
    this->target_up = v11.up;

    this->field_1C4 = 0;
    this->field_1C0 = 0;

    this->field_1A0->deactivate();
    this->field_1A0->activate(v11);
}

void spiderman_camera::adjust_geometry_pipe(bool a1)
{
    if constexpr (1) {
        auto *abs_po = &this->get_abs_po();
        auto &pos = abs_po->get_position();

        auto &v2 = abs_po->get_z_facing();
        auto eye = v2 + pos;

        auto &up = abs_po->get_y_facing();
        if (a1) {
            assert(!is_externally_controlled());

            auto *v7 =
                entity_handle_manager::find_entity(string_hash{"SCENE_ANALYZER_CAM"}, entity_flavor_t::CAMERA, false);

            this->set_abs_po(v7->get_abs_po());

            auto &pos = abs_po->get_position();

            geometry_manager::set_view(pos, eye, up);
        } else if (g_game_ptr != nullptr && !g_game_ptr->m_user_camera_enabled) {
            auto &pos = abs_po->get_position();
            geometry_manager::set_view(pos, eye, up);
        }

    } else {
        THISCALL(0x004B6480, this, a1);
    }
}

void spiderman_camera::autocorrect(Float a2)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x2D0));
    func(this, nullptr, a2);
}

void spiderman_camera::_autocorrect(Float a2)
{
    TRACE("spiderman_camera::autocorrect");

    if constexpr (1) {
        auto *target = this->get_target_entity();
        camera_target_info v13{target, 0.033333335f, this->target_pos, this->target_up};

        this->field_1A0->request_recenter(a2, v13);
        if (equal(a2.value, 0.0f)) {
            this->target_pos = v13.pos;
            this->target_up = v13.up;
        }

    } else {
        THISCALL(0x004B63F0, this, a2);
    }
}

void spiderman_camera::_set_target_entity(entity *e)
{
    TRACE("spiderman_camera::set_target_entity");

    game_camera::set_target_entity(e);
}

void spiderman_camera::_frame_advance(Float a2)
{
    TRACE("spiderman_camera::frame_advance");

    if constexpr (STANDALONE_SYSTEM) {
        if (g_game_ptr->level_is_loaded() && !g_game_ptr->is_paused() &&
            !os_developer_options::instance->get_flag(mString{"SHOW_PROFILE_INFO"})) {
            static int &old_devopt_fov = var<int>(0x00959E54);
            this->field_1D0.update(a2);
            auto CAMERA_FOV = os_developer_options::instance->get_int(mString{"CAMERA_FOV"});
            if (CAMERA_FOV != old_devopt_fov) {
                old_devopt_fov = CAMERA_FOV;
                auto fov = CAMERA_FOV * 0.017453292f;
                this->set_fov(fov);
            }

            if (!this->field_12C) {
                this->autocorrect(0.0);
                this->field_12C = true;
            }

            set_filter_time(a2);
            if (this->get_target_entity() == nullptr) {
                this->set_target_entity(g_world_ptr->get_hero_ptr(0));
            }

            auto *target_entity = this->get_target_entity();
            camera_target_info v18{target_entity, a2, this->target_pos, this->target_up};

            vector3d v17 = this->get_abs_position() - v18.pos;
            v18.field_48 = v17;
            auto len2 = (v18.pos - this->target_pos).length2();
            if (len2 > sqr(16.0)) {
                this->autocorrect(0.0);
            }

            auto *the_controller = v18.field_54->m_player_controller;
            if (the_controller != nullptr) {
                the_controller->force_always_camera_relative(false);
            }

            camera_frame v19{this->get_abs_po()};
            v19.fwd.normalize();

            this->field_1A0->frame_advance(a2, v19, v18);
            auto a2a = v19.get_po();

            this->set_abs_po(a2a);

            this->set_frame_delta(a2a, a2);

            this->target_pos = v18.pos;
            this->target_up = v18.up;
        }
    } else {
        THISCALL(0x004B60B0, this, a2);
    }
}

void constrain_relative_to_plane(vector3d &a1, const vector3d &a2, const vector3d &norm, float a4, float a5)
{
    assert(norm.is_normal());

    auto v8 = a1 - a2;
    auto len = v8.length();
    if (len > EPSILON) {
        auto v9 = 1.0f / len;
        vector3d a1a = v8 * v9;
        constrain_normal(a1a, norm, a4, a5);

        a1 = a1a * len + a2;
    }
}

void spiderman_camera_patch()
{
    {
        REDIRECT(0x004B6159, set_filter_time);
    }

    {
        FUNC_ADDRESS(address, &spiderman_camera::_autocorrect);
        set_vfunc(0x008823B0, address);
    }

    {
        FUNC_ADDRESS(address, &spiderman_camera::_set_target_entity);
        set_vfunc(0x00882394, address);
    }

    {
        FUNC_ADDRESS(address, &spiderman_camera::_frame_advance);
        set_vfunc(0x00882284, address);
    }
}
