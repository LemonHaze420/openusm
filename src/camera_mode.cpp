#include "camera_mode.h"

#include "ai_player_controller.h"
#include "camera_frame.h"
#include "camera_target_info.h"
#include "collide_aux.h"
#include "common.h"
#include "custom_math.h"
#include "game.h"
#include "game_settings.h" 
#include "input_mgr.h"
#include "oldmath_usefulmath.h"
#include "spiderman_camera.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"
#include "settings.h"

#include <cmath>
#include "ai_std_combat_target.h"
#include "ai_team.h"
#include "base_ai_core.h"
#include "collision_geometry.h"
#include "damage_interface.h"
#include "line_info.h"
#include "local_collision.h"
#include "nal_anim_controller.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "subdivision_obb.h"
#include "os_developer_options.h"
#include <vector>

namespace {
camera_mode::vtable base_table, shake_table, lookaround_table, passive_table, fixedstatic_table, combat_table;
void native_shake(camera_mode_shake *, Float, camera_frame &, const camera_target_info &);
void native_combat(camera_mode_combat *, Float, camera_frame &, const camera_target_info &);
void initialize_native_tables();
}

VALIDATE_SIZE(camera_mode, 0xC);

VALIDATE_OFFSET(camera_mode_shake, frame_fwd, 0x24);
VALIDATE_OFFSET(camera_mode_shake, frame_eye, 0x30);

VALIDATE_SIZE(camera_mode_lookaround, 0x78);

VALIDATE_SIZE(camera_mode_fixedstatic, 0x28u);

static Var<int> dword_959E5C {0x00959E5C};

bool sub_4B2180(vector3d &a1, float a2)
{
    auto v2 = a1.length2();
    if ( v2 <= sqr(a2 + LARGE_EPSILON) ) {
        return false;
    }

    auto v4 = a2 / sqrt(v2);
    a1 *= v4;
    return true;
}

bool pull_sphere(vector3d &a1, float t, vector3d a2)
{
    auto a1a = a1 - a2;
    auto result = sub_4B2180(a1a, t);
    a1 = a1a + a2;
    return result;
}

void camera_mode_chase::pull_by_target(camera_frame &frame, const camera_target_info &target, Float a3)
{
    pull_sphere(frame.eye, a3, target.pos);
    auto v15 = target.pos - frame.eye;
    auto v12 = v15.normalized();
    if ( v12.length() < EPSILON ) {
        v12 = target.facing;
    }

    if (dword_959E5C()-- != 0) {
        frame.fwd = v12;
    } else {
        frame.fwd = lerp(v12, frame.fwd, slow_mix);
        frame.fwd.normalize();
    }
}

camera_mode::camera_mode(spiderman_camera *a2, camera_mode *a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        initialize_native_tables();
        this->m_vtbl = &base_table;
    } else {
        this->m_vtbl = CAST(m_vtbl, 0x00881E24);
    }
    this->slave = a2;
    this->field_8 = a3;

    assert(slave->get_target_entity());

    assert(slave->get_target_entity()->is_a_conglomerate());
}

void camera_mode::activate(camera_target_info &a2)
{
    this->m_vtbl->activate(this, nullptr, &a2);
}

void camera_mode::deactivate()
{
    this->m_vtbl->deactivate(this);
}

void camera_mode::request_recenter(Float a2, const camera_target_info &a3)
{
    {
        this->m_vtbl->request_recenter(this, nullptr, a2, &a3);
    }
}

void camera_mode::frame_advance(Float a2, camera_frame &frame, const camera_target_info &a4)
{
    this->m_vtbl->frame_advance(this, nullptr, a2, &frame, &a4);
}

void camera_mode::set_fixedstatic(const vector3d &a2, const vector3d &a3)
{
    this->m_vtbl->set_fixedstatic(this, nullptr, &a2, &a3);
}

void camera_mode::clear_fixedstatic()
{
    this->m_vtbl->clear_fixedstatic(this);
}

camera_mode_shake::camera_mode_shake(spiderman_camera *owner, camera_mode *child)
    : camera_mode(owner, child), field_C(ZEROVEC), field_18(ZEROVEC),
      frame_fwd(owner->get_abs_po().get_z_facing()), frame_eye(owner->get_abs_position())
{
    if constexpr (STANDALONE_SYSTEM) m_vtbl = &shake_table;
    else m_vtbl = CAST(m_vtbl, 0x00881E9C);
}

void camera_mode_shake::_frame_advance(Float a2, camera_frame &a3, const camera_target_info &a4)
{
    if constexpr (STANDALONE_SYSTEM) native_shake(this, a2, a3, a4);
    else THISCALL(0x004B6CE0, this, a2, &a3, &a4);
}

camera_mode_lookaround::camera_mode_lookaround(spiderman_camera *a2, camera_mode *a3) : camera_mode(a2, a3)
{
    if constexpr (STANDALONE_SYSTEM) this->m_vtbl = &lookaround_table;
    else this->m_vtbl = CAST(m_vtbl, 0x008820AC);

    this->field_70[0] = 0.0;
    this->field_70[1] = 0.0;

    this->field_C = true;

    this->field_10.set_id(input_mgr::instance->field_58);
    this->field_10.set_control(109);
    this->field_10.field_8 = 0.1;

    this->field_40.set_id(input_mgr::instance->field_58);
    this->field_40.field_4 = 108;
    this->field_40.field_8 = 0.34999999;
}

vector3d sub_4B22E0(const vector3d &a2, const vector3d &a3, float a4)
{
    auto v4 = std::cos(a4);
    auto v5 = (a2[0] * a3[0] + a2[1] * a3[1] + a3[2] * a2[2]) * (1.0f  - v4);

    vector3d v8;
    v8[0] = a3[1] * a2[2] - a2[1] * a3[2];
    v8[1] = a3[2] * a2[0] - a3[0] * a2[2];
    v8[2] = a2[1] * a3[0] - a3[1] * a2[0];

    auto v6 = std::sin(a4);
    v8 *= v6;

    auto v9 = v5 *  a3[0];
    auto v10 = v5 * a3[1];
    auto v13 = v4 * a2[2];
    auto v11 = v4 * a2[0] + v9;
    auto v12 = v4 * a2[1] + v10;
    auto v7 = v5 * a3[2] + v13;

    vector3d result;
    result[0] = v11 + v8[0];
    result[1] = v12 + v8[1];
    result[2] = v7 +  v8[2];
    return result;
}

void camera_mode_lookaround::_frame_advance(Float a2, camera_frame &a3, const camera_target_info &a4)
{

    if constexpr (STANDALONE_SYSTEM) {
        auto *v4 = &a4;
        auto *v6 = this->slave;
        auto *v7 = &a3;
        if (v6->field_1BC || !this->field_C) {
            v6->field_1CC = false;
        } else {
            eHeroLocoMode v9;
            auto *the_controller = a4.field_54->m_player_controller;
            if (the_controller != nullptr) {
                auto loco_mode = static_cast<eHeroLocoMode>(the_controller->get_spidey_loco_mode());
                v9 = static_cast<eHeroLocoMode>(1);
                if ( loco_mode >= 0 ) {
                    v9 = loco_mode;
                }
            } else {
                v9 = static_cast<eHeroLocoMode>(1);
            }

            this->field_10.update(a2);
            this->field_40.update(a2);

            if (this->field_40.field_2D && this->field_10.field_2D) {
                if (this->slave->field_1CC) {
                    this->field_70 = vector2d{0.0, 0.0};
                    if ((v9 == 3 || v9 == 5 || v9 == 6 || v4->sub_4B2980()) && !v4->sub_4B29C0()) {
                        this->slave->field_1CC = false;
                    }
                }
            } else {
                vector2d v82 {this->field_10.field_10, this->field_40.field_10};
                if (!Settings::MouseLook) {
                    auto v13 = v82.length2();
                    if (v13 > sqr(1.0f)) {
                        v82 /= std::sqrt(v13);
                    }

                    auto v15 = v82.length2();
                    vector2d v81 = v82 * v15;

                    v82 = lerp(v82, v81, 0.80000001f);

                    auto v16 = v82 - this->field_70;
                    auto v18 = v16.length2();
                    if (v18 > sqr(0.25f)) {
                        auto v19 = 0.25f / sqrt(v18);
                        v16 *= v19;
                    }

                    v82 = v16 + this->field_70;
                    auto v20 = v82.length2();
                    if (v20 > sqr(1.0f)) {
                        auto v21 = 1.0f / std::sqrt(v20);
                        v82 *= v21;
                    }
                }

                float v78 = v82[0] * a2;
                float v81 = v82[1] * a2;

                vector3d v86 = a2 * v4->field_24;
                v7->eye += v86;

                float v25 = 0.0f;
                if ( v9 == 1 ) {
                    v25 = v4->radius * 1.25f;
                }

                vector3d v84 = v4->up * v25 + v4->pos;
                auto v28 = (v84 - v7->eye).length();

                auto v29 = v7->fwd * v28;

                vector3d a3a = v29 + v7->eye;
                a3a = lerp(v84, a3a, slow_mix);
                auto *gamefile = g_game_ptr->gamefile;
                auto invert_camera_vert = gamefile->field_340.m_invert_camera_vert;
                v84 = v7->eye - a3a;

                auto invert_camera_horz = gamefile->field_340.m_invert_camera_horz;
                float v33 = ( invert_camera_horz ? -1.0f : 1.0f );

                auto v72 = -(v33 * v78 * 4.0f);
                v84 = sub_4B22E0(v84, v7->up, v72);

                auto v37 = ( invert_camera_vert ? -1.0f : 1.0f );

                auto v38 = v37 * v81 * 3.5;
                auto a2b = v38;
                auto v39 = v7->get_right();
                auto v40 = sub_4B22E0(v84, v39, -v38);
                eHeroLocoMode v46 = v9;
                auto v47 = v40.length();
                if (v47 < 4.0f) {
                    v40 *= 1.0f / v47;
                    if (v46 == 2 || v46 == 7) {
                        auto v50 = dot(v40, v4->up);
                        if ( v50 > 0.0f ) {
                            v47 = (1.0f - std::sqrt(1.0f - v50 * v50)) * 4.0f + v47;
                        }
                    } else if (a2b < 0.0f) {
                        v47 = v47 - a2b * 4.0f ;
                    }

                    v47 = std::min(v47, 4.0f);
                    v40 *= v47;
                }

                v7->eye = a3a + v40;
                if (v46 == 1 || v46 == 2 || v46 == 7) {
                    auto abs_pos = v4->field_54->get_abs_position();

                    auto a2c = v4->radius * 1.3f;
                    push_sphere(v7->eye, a2c, abs_pos);

                    auto v56 = v4->sub_4B42E0() - 0.25f;

                    auto v58 = std::max(v56, 0.0f);
                    auto v60 = v4->up * v58;

                    auto v84 = abs_pos - v60;

                    float t = 1.0f;
                    sub_5B8F40(a3a, v7->eye, v84, v4->up, &t);
                    if (t > 0.0f && t < 1.0f) {
                        auto v64 = v7->eye - a3a;
                        auto v81 = v64 * t;
                        v7->eye = v81 + a3a;
                        push_sphere(v7->eye, a2c, v84);
                    }
                }

                const float min_val = ( v46 == 1 ? -0.89999998f : -0.97000003f);

                v7->constrain_pos_relative_to_plane(a3a, YVEC, min_val, 0.97000003f);

                this->slave->sub_4B3220(a3a);
                this->slave->field_1C8 = 1;
                this->field_70 = v82;
            }
        }

        auto *v11 = this->slave;
        if (v11->field_1CC) {
            v11->field_1C4 = v11->field_1C0;
            v11->field_1C0 = 1;
        } else {
            auto *v71 = this->field_8;
            if ( v71 != nullptr ) {
                v71->frame_advance(a2, *v7, *v4);
            }

            this->field_70 = vector2d {0.0, 0.0};
        }
    } else {
        THISCALL(0x004B5480, this, a2, &a3, &a4);
    }
}

camera_mode_passive::camera_mode_passive(spiderman_camera *a2, camera_mode *a3) : camera_mode(a2, a3)
{
    if constexpr (STANDALONE_SYSTEM) this->m_vtbl = &passive_table;
    else this->m_vtbl = CAST(m_vtbl, 0x00882408);
    this->field_10 = g_camera_max_dist;
    this->field_C = g_camera_min_dist;
    this->field_14 = -0.5;
    this->field_18 = 0.69999999;
    this->field_1C = YVEC;
}

void camera_mode_passive::_activate(camera_target_info &a2)
{
    auto *v3 = this->field_8;
    if ( v3 != nullptr ) {
        v3->activate(a2);
    }

    this->field_1C = YVEC;
}

void camera_mode_passive::_frame_advance([[maybe_unused]] Float a2, camera_frame &frame, camera_target_info &target)
{

    if constexpr (STANDALONE_SYSTEM) {
        [](spiderman_camera *self) -> void {
            self->field_1C4 = self->field_1C0;
            self->field_1C0 = 3;
        }(this->slave);

        int loco_mode = target.get_loco_mode();
        int prev_loco_mode = target.get_prev_loco_mode();

        bool is_crawling = prev_loco_mode == 2;
        bool is_running = loco_mode == 1;
        bool is_falling = loco_mode == 5;
        bool is_jumping = loco_mode == 6;
        auto is_swinging = loco_mode == 3;
        auto v47 = loco_mode == 9;
        auto v13 = (loco_mode == 2 || loco_mode == 7 || loco_mode == 14 || (is_crawling && loco_mode == 9));

        bool v12 = ( is_falling || is_jumping || is_swinging );

        assert(target.min_look_dist > target.radius);

        float v14, v15;
        if (v13) {
            v14 = 0.30000001f;
            v15 = 1.0f;
        } else {
            if ( is_running ) {
                v14 = 0.30000001f;
            } else {
                v14 = 0.0f;
            }

            v15 = 0.69999999f;
        }

        target.min_look_dist = lerp(target.min_look_dist, this->field_C, slow_mix);
        this->field_C = target.min_look_dist;

        target.max_look_dist = lerp(target.max_look_dist, this->field_10, slow_mix);
        this->field_10 = target.max_look_dist;

        this->field_14 = lerp(v14, this->field_14, slow_mix);
        this->field_18 = lerp(v15, this->field_18, slow_mix);

        vector3d v19 = YVEC;
        if (v12) {
            auto v50 = vector3d::cross(frame.up, frame.fwd);
            auto v23 = dot(target.field_24, v50);
            v23 = (2.0f / 30.0f) * v23;

            int a2a = sign(v23);

            auto v24 = sqr(v23) * a2a * 0.125f;

            v24 = std::clamp(v24, -0.5f, 0.5f);

            auto a2b = std::sqrt(1.0f - sqr(v24));
            v19 = a2b * frame.up + v50 * v24;
        }

        this->field_1C.sub_4B9FA0(v19, slow_mix);
        frame.up = this->field_1C;
        if (v47) {
            auto v32 = target.pos + target.facing * 5.0f;

            frame.rotate_to_include_target(v32, target.pos, target.up, 0.80000001);

            frame.include_target(v32, 0.0f, 0.80000001f);
        }

        frame.constrain_pos_relative_to_plane(target.pos, target.up, this->field_14, this->field_18);
        if ( !v13 || target.up[1] < -0.15000001f ) {
            frame.avoid_target(target, target.min_look_dist);
        }

        camera_mode_chase::pull_by_target(frame, target, target.max_look_dist);
        if (v12) {
            auto v35 = target.pos - frame.eye;
            auto v51 = vector3d::cross(v35, YVEC);
            auto v38 = v51.normalized();
            if ( v38.length() > EPSILON ) {
                constrain_normal(frame.fwd, v38, -0.1f, 0.1f);
            }
        }
    } else {
        void(__fastcall * func)(void *, void *, Float, camera_frame *, camera_target_info *) = CAST(func, 0x004B7400);
        func(this, nullptr, a2, &frame, &target);
    }
}

camera_mode_fixedstatic::camera_mode_fixedstatic(spiderman_camera *a2, camera_mode *a3) : camera_mode(a2, a3)
{
    if constexpr (STANDALONE_SYSTEM) this->m_vtbl = &fixedstatic_table;
    else this->m_vtbl = CAST(m_vtbl, 0x00881E74);
    this->field_C = ZEROVEC;
    this->field_18 = ZVEC;
    this->enabled = false;
}

void camera_mode_fixedstatic::_frame_advance(Float a2, camera_frame &a3, const camera_target_info &a4)
    {
    if (this->enabled) {
        a3.eye = this->field_C;
        a3.fwd = this->field_18;
        a3.up = YVEC;
        a3.include_target(a4.field_C, a4.radius, 0.80000001);
        auto *v5 = this->slave;

        {
            v5->field_1C4 = v5->field_1C0;
            v5->field_1C0 = 0;
        }
    } else {
        auto *v6 = this->field_8;
        if ( v6 != nullptr ) {
            v6->frame_advance(a2, a3, a4);
        }
    }
}

void camera_mode_fixedstatic::_set_fixedstatic(const vector3d &a2, const vector3d &a3)
{
    auto *v4 = this->field_8;
    if ( v4 != nullptr ) {
        v4->set_fixedstatic(this->field_C, a3);
    }

    this->enabled = true;
    this->field_C = a2;

    this->field_18 = (a3 - a2).normalized();
}

void camera_mode_fixedstatic::_clear_fixedstatic()
{
    auto *v2 = this->field_8;
    if ( v2 != nullptr ) {
        v2->clear_fixedstatic();
    }

    this->enabled = false;
}

void camera_mode_combat::_frame_advance(Float a2, camera_frame &a3, const camera_target_info &a4)
{
    if constexpr (STANDALONE_SYSTEM) native_combat(this, a2, a3, a4);
    else THISCALL(0x004B7F90, this, a2, &a3, &a4);
}

void camera_mode_patch()
{
    {
        FUNC_ADDRESS(address, &camera_mode_shake::_frame_advance);
        set_vfunc(0x00881EAC, address);
    }

    {
        FUNC_ADDRESS(address, &camera_mode_lookaround::_frame_advance);
        set_vfunc(0x008820BC, address);
    }

    {
        FUNC_ADDRESS(address, &camera_mode_passive::_frame_advance);
        set_vfunc(0x00882418, address);
    }

    {
        FUNC_ADDRESS(address, &camera_mode_combat::_frame_advance);
        set_vfunc(0x00882068, address);
    }
}

namespace {

struct camera_mode_stationary : camera_mode {
    using camera_mode::camera_mode;
};
struct camera_mode_include : camera_mode {
    using camera_mode::camera_mode;
};
struct camera_mode_precollide : camera_mode {
    using camera_mode::camera_mode;
};
struct camera_mode_filter : camera_mode {
    int reset_frames = 1;
    using camera_mode::camera_mode;
};
struct camera_mode_eye_filter : camera_mode_filter {
    using camera_mode_filter::camera_mode_filter;
};
struct camera_mode_transition : camera_mode {
    camera_frame previous;
    bool valid = false;
    float remaining = 0.0f;
    camera_mode_transition(spiderman_camera *owner, camera_mode *child)
        : camera_mode(owner, child), previous(owner->get_abs_po()) {
        previous.eye = ZEROVEC;
        previous.fwd = ZVEC;
        previous.up = YVEC;
    }
};
struct camera_mode_transition_eye : camera_mode_transition {
    using camera_mode_transition::camera_mode_transition;
};
struct camera_mode_transition_direction : camera_mode_transition {
    using camera_mode_transition::camera_mode_transition;
};
struct camera_mode_reorient : camera_mode {
    bool active = true;
    float remaining = 0.0f;
    unsigned last_target = 0;
    using camera_mode::camera_mode;
};
VALIDATE_SIZE(camera_mode_filter, 0x10);
VALIDATE_SIZE(camera_mode_transition, 0x38);
VALIDATE_SIZE(camera_mode_reorient, 0x18);
VALIDATE_SIZE(camera_mode_combat, 0x10);
VALIDATE_SIZE(camera_mode_shake, 0x3C);

camera_mode::vtable stationary_table, include_table, precollide_table, filter_table,
    eye_filter_table, transition_eye_table, transition_direction_table, reorient_table;
vector3d &lookaround_target = var<vector3d>(0x00959EBC);
bool &combat_extra_target = var<bool>(0x00959E58);
vector3d &combat_extra_position = var<vector3d>(0x00959F50);
float &combat_extra_radius = var<float>(0x00959E48);
float &combat_extra_constraint = var<float>(0x00959E3C);
float &falling_prediction = var<float>(0x00959E90);

void forward_frame(camera_mode *self, Float dt, camera_frame &frame, const camera_target_info &target) {
    if (self->field_8) self->field_8->frame_advance(dt, frame, target);
}
void __fastcall forward_activate(camera_mode *self, void *, camera_target_info *target) {
    if (self->field_8) self->field_8->activate(*target);
}
void __fastcall forward_deactivate(camera_mode *self) {
    if (self->field_8) self->field_8->deactivate();
}
void __fastcall forward_advance(camera_mode *self, void *, Float dt, camera_frame *frame,
                                 const camera_target_info *target) {
    forward_frame(self, dt, *frame, *target);
}
void __fastcall forward_recenter(camera_mode *self, void *, Float dt, const camera_target_info *target) {
    if (self->field_8) self->field_8->request_recenter(dt, *target);
}
void __fastcall forward_lookaround(camera_mode *self, void *, bool enabled) {
    if (self->field_8) self->field_8->m_vtbl->enable_lookaround(self->field_8, nullptr, enabled);
}
void __fastcall forward_fixed(camera_mode *self, void *, const vector3d *eye, const vector3d *at) {
    if (self->field_8) self->field_8->set_fixedstatic(*eye, *at);
}
void __fastcall forward_clear(camera_mode *self) {
    if (self->field_8) self->field_8->clear_fixedstatic();
}
void __fastcall forward_notify(camera_mode *self, void *) {
    if (self->field_8) self->field_8->m_vtbl->notify(self->field_8, nullptr);
}
void push_relative_plane(vector3d &eye, float radius, const vector3d &position, const vector3d &normal) {
    push_sphere(eye, radius, position - normal * dot(position - eye, normal));
}
template<class T>
void *__fastcall finalize_mode(camera_mode *self, void *, unsigned flags) {
    destroy_native_camera_modes(self->field_8);
    static_cast<T *>(self)->~T();
    if (flags & 1) ::operator delete(self);
    return self;
}
template<class T>
camera_mode *__fastcall clone_mode(camera_mode *self, void *) {
    auto *copy = new T(*static_cast<T *>(self));
    if (self->field_8) copy->field_8 = self->field_8->m_vtbl->clone(self->field_8, nullptr);
    return copy;
}
template<class T>
camera_mode::vtable mode_table() {
    return {finalize_mode<T>, clone_mode<T>, forward_activate, forward_deactivate,
        forward_advance, forward_recenter, forward_lookaround, forward_fixed, forward_clear,
        forward_notify, {nullptr}};
}

ai::base_full_target_inode *combat_inode(const camera_target_info &target) {
    return static_cast<ai::base_full_target_inode *>(
        target.field_54->get_ai_core()->get_info_node(ai::combat_target_inode::default_id, true));
}
vhandle_type<actor> current_combat_target(const camera_target_info &target) {
    return vhandle_type<actor>{combat_inode(target)->field_2C};
}
vector3d recenter_eye(const camera_target_info &target) {
    float distance = 0.5f * (target.min_look_dist + target.max_look_dist);
    auto *inode = combat_inode(target);
    auto *opponent = inode ? vhandle_type<actor>{inode->field_2C}.get_volatile_ptr() : nullptr;
    vector3d direction;
    if (opponent) {
        direction = (target.pos - opponent->get_abs_position()).normalized();
    } else {
        int loco = target.get_loco_mode();
        direction = -target.facing;
        vector3d normal = target.up;
        float low = 0.0f;
        if (loco == 2 || loco == 7 || loco == 1) low = 0.3f;
        else if (loco == 14) {
            direction = target.up;
            distance = target.max_look_dist;
            normal = YVEC;
            low = 0.7f;
        }
        constrain_normal(direction, normal, low, loco == 2 || loco == 7 ? 1.0f : 0.7f);
    }
    return target.pos + direction * distance;
}
void reorient_recenter(camera_mode_reorient *self, Float dt, const camera_target_info &target) {
    forward_recenter(self, nullptr, dt, &target);
    self->active = true;
    self->remaining = std::max(float(dt), 0.0f);
}
void reorient_frame(camera_mode_reorient *self, Float dt, camera_frame &frame,
                    const camera_target_info &target) {
    if (!self->active) {
        auto handle = current_combat_target(target);
        if (handle.field_0.get_goodies() != self->last_target) {
            if (handle.get_volatile_ptr()) reorient_recenter(self, 0.5f, target);
            self->last_target = handle.field_0.get_goodies();
        }
    }
    if (!self->active) {
        forward_frame(self, dt, frame, target);
        return;
    }
    self->slave->field_1C4 = self->slave->field_1C0;
    self->slave->field_1C0 = 1;
    vector3d eye = recenter_eye(target);
    vector3d forward = (target.pos - eye).normalized();
    if (std::fpclassify(self->remaining) == FP_ZERO) {
        self->active = false;
        frame.eye = eye;
        frame.fwd = forward.normalized();
    } else {
        frame.fwd = lerp(forward, frame.fwd, slow_mix).normalized();
        frame.eye = lerp(eye, frame.eye, slow_mix);
        self->remaining -= dt;
        if (self->remaining <= 0.0f) {
            self->remaining = 0.0f;
            self->active = false;
            lookaround_target = frame.eye + frame.fwd;
            self->slave->field_1CC = true;
        }
        self->slave->field_1C8 = 1;
    }
}
void __fastcall transition_eye_smooth(camera_mode *mode, void *, camera_frame *frame, Float mix) {
    auto *self = static_cast<camera_mode_transition *>(mode);
    float horizontal = std::sqrt(frame->eye.x * frame->eye.x + frame->eye.z * frame->eye.z);
    float previous = std::sqrt(self->previous.eye.x * self->previous.eye.x +
                                self->previous.eye.z * self->previous.eye.z);
    horizontal = lerp(horizontal, previous, mix);
    frame->eye = lerp(frame->eye, self->previous.eye, mix);
    vector3d planar{frame->eye.x, 0.0f, frame->eye.z};
    planar.normalize();
    frame->eye.x = planar.x * horizontal;
    frame->eye.z = planar.z * horizontal;
}
void __fastcall transition_direction_smooth(camera_mode *mode, void *, camera_frame *frame, Float mix) {
    frame->smooth_dir(static_cast<camera_mode_transition *>(mode)->previous, mix);
    frame->up = frame->fix_up_vector(frame->up);
}
void __fastcall reset_filter_state(camera_mode *mode, void *, const camera_target_info *) {
    static_cast<camera_mode_filter *>(mode)->reset_frames = 1;
}
void transition_frame(camera_mode_transition *self, Float dt, camera_frame &frame,
                       const camera_target_info &target) {
    forward_frame(self, dt, frame, target);
    if (self->slave->field_1C4 != self->slave->field_1C0)
        self->remaining = self->slave->field_1C0 == 2 ? 1.0f : 0.0f;
    if (!self->valid || self->slave->field_1C0 != 2) self->remaining = 0.0f;
    camera_frame relative = frame;
    relative.eye -= target.pos;
    if (self->remaining > 0.0f) {
        self->m_vtbl->extension.smooth_transition(self, nullptr, &relative, self->remaining);
        frame = relative;
        frame.eye += target.pos;
        self->remaining -= dt;
    }
    self->previous = relative;
    self->valid = true;
}
void filter_frame(camera_mode_filter *self, Float dt, camera_frame &frame,
                   const camera_target_info &target) {
    camera_frame previous = frame;
    forward_frame(self, dt, frame, target);
    if (self->reset_frames > 0 || self->slave->field_1C8 > 0) {
        self->reset_frames = std::max(self->reset_frames - 1, 0);
        if (self->m_vtbl == &filter_table)
            self->slave->field_1C8 = std::max(self->slave->field_1C8 - 1, 0);
    } else if (self->m_vtbl == &eye_filter_table) {
        frame.eye -= target.pos;
        previous.eye -= target.pos;
        frame.eye = lerp(frame.eye, previous.eye, slow_mix) + target.pos;
    } else {
        frame.smooth_dir(previous, med_mix);
        frame.up = frame.fix_up_vector(frame.up);
    }
}
void stationary_frame(camera_mode *self, Float dt, camera_frame &frame,
                       const camera_target_info &target) {
    vector3d eye = frame.eye, forward = frame.fwd;
    forward_frame(self, dt, frame, target);
    if (target.field_24.length2() < EPSILON) {
        if ((frame.eye - eye).length2() < EPSILON) frame.eye = eye;
        if ((frame.fwd - forward).length2() < EPSILON) frame.fwd = forward;
    }
}
void include_frame(camera_mode *self, Float dt, camera_frame &frame,
                     const camera_target_info &target) {
    forward_frame(self, dt, frame, target);
    float low = -0.97f;
    if (self->slave->field_1CC) {
        frame.fwd = (lookaround_target - frame.eye).normalized();
    } else {
        vector3d displacement = ZEROVEC;
        int loco = target.get_loco_mode();
        if (loco != 2 && loco != 7) {
            displacement = vector3d{2.0f * target.field_24.x, 0.0f, 2.0f * target.field_24.z};
            if (target.field_24.y < 0.0f) displacement += target.field_24 * falling_prediction;
            displacement *= dt;
        }
        frame.include_target(target.pos + displacement, 0.1f, 0.9f);
        if (combat_extra_target)
            frame.include_target(combat_extra_position, combat_extra_radius, combat_extra_constraint);
        frame.include_target(target.field_C, target.radius, 0.8f);
        if (loco == 1) low = -0.5f;
    }
    constrain_normal(frame.fwd, YVEC, low, 0.97f);
}

template<int Kind>
bool __fastcall camera_entity_filter(const local_collision::entfilter_base *, void *,
                                    actor *act, dynamic_conglomerate_clone *,
                                    const local_collision::query_args_t *) {
    if (act->colgeom->get_type() == collision_geometry::CAPSULE ||
        !act->has_entity_collision() || !act->has_camera_collision()) return false;
    if constexpr (Kind == 0) return true;
    bool solid = true;
    if (act->colgeom && (act->field_4 & 0x4000) != 0)
        solid = (act->colgeom->field_C & 0x10) == 0 || (act->colgeom->field_C & 0x100) != 0;
    return Kind == 1 ? solid : !solid;
}
template<int Kind>
bool __fastcall camera_obb_filter(const local_collision::obbfilter_base *, void *,
                                 subdivision_node_obb_base *node, const local_collision::query_args_t *args) {
    if constexpr (Kind == 3) return node->sphere_intersection(args->field_10, args->field_28);
    if constexpr (Kind != 0) {
        if ((node->flags & 0x120) != 0 || ((node->flags & 0x400) != 0) != (Kind == 1)) return false;
    }
    return node->line_segment_intersection(args->field_4, args->field_1C);
}
template<int Kind>
const local_collision::entfilter_base &entity_filter() {
    static local_collision::entfilter_base::native_vtable table{camera_entity_filter<Kind>};
    static local_collision::entfilter_base filter{reinterpret_cast<std::intptr_t>(&table)};
    return filter;
}
template<int Kind>
const local_collision::obbfilter_base &obb_filter() {
    static local_collision::obbfilter_base::native_vtable table{camera_obb_filter<Kind>};
    static local_collision::obbfilter_base filter{reinterpret_cast<std::intptr_t>(&table)};
    return filter;
}
template<int Kind>
bool collide_eye(vector3d &eye, vector3d destination) {
    vector3d start = eye;
    vector3d delta = start - destination;
    if (delta.length2() >= 10000.0f) start = destination + delta.normalized() * 99.0f;
    line_info line(start, destination);
    line.check_collision(entity_filter<Kind>(), obb_filter<Kind>(), nullptr);
    if (!line.collision) {
        eye = destination;
        return false;
    }
    eye = line.hit_pos;
    vector3d old = eye;
    eye -= (destination - eye).normalized() * 0.1f;
    float pushed = dot(line.hit_norm, eye - old);
    if (pushed > 0.0f && pushed < 0.25f) eye += line.hit_norm * (0.25f - pushed);
    return true;
}
void precollide_frame(camera_mode *self, Float dt, camera_frame &frame,
                       const camera_target_info &target) {
    vector3d previous = frame.eye;
    forward_frame(self, dt, frame, target);
    vector3d destination = frame.eye;
    frame.eye = previous;
    bool hit = collide_eye<1>(frame.eye, destination);
    if (self->slave->field_1CF)
        frame.eye.y = std::max(frame.eye.y, var<float>(0x00921BB4) + 1.0f);
    destination = frame.eye;
    frame.eye = target.pos;
    if (hit && (previous - target.pos).length2() > sqr(target.max_look_dist + 1.0f))
        collide_eye<0>(frame.eye, destination);
    else
        collide_eye<2>(frame.eye, destination);
    local_collision::query_args_t args{};
    auto *primitives = local_collision::query_sphere(frame.eye, 0.25f, entity_filter<0>(), obb_filter<3>(), args);
    vector3d point, normal;
    bool intersects = local_collision::get_closest_sphere_intersection(primitives, frame.eye, 0.25f,
                                                                       &point, &normal, nullptr);
    local_collision::destroy_primitive_list(&primitives);
    if (intersects) frame.eye = point + normal * 0.25f;
}
void add_shake(camera_frame &frame, const vector3d &shake, const vector3d &position) {
    if (shake.length2() >= 0.00000001f)
        frame.eye += shake * (1.5f / ((position - frame.eye).length() * 0.25f + 1.0f));
}
void native_shake(camera_mode_shake *self, Float dt, camera_frame &frame,
                    const camera_target_info &target) {
    frame.eye = self->frame_eye;
    frame.fwd = self->frame_fwd;
    forward_frame(self, dt, frame, target);
    self->frame_eye = frame.eye;
    self->frame_fwd = frame.fwd;
    auto *animation = target.field_54->anim_ctrl;
    add_shake(frame, animation ? animation->get_camera_shake() : ZEROVEC, target.pos);
    auto *cores = ai::ai_core::the_ai_core_list_high;
    if (cores) {
        for (auto *core : *cores) {
            auto *act = core->get_actor(0);
            if (!act->is_hero() && (act->get_abs_position() - frame.eye).length2() <= 10000.0f &&
                act->anim_ctrl)
                add_shake(frame, act->anim_ctrl->get_camera_shake(), act->get_abs_position());
        }
    }
}
}

namespace {
struct combat_candidate {
    actor *act;
    vector3d direction;
    float distance;
    float weight;
};
bool combat_candidate_valid(actor *act, const camera_target_info &target) {
    if (!act || !act->get_ai_core() || !act->has_damage_ifc() || act == target.field_54) return false;
    auto *damage = act->damage_ifc();
    if (!damage->is_alive() && damage->is_subdued()) return false;
    auto hash = act->get_ai_core()->get_param_block()->get_pb_hash(ai::combat_target_inode::team_hash());
    return ai::team::manager::is_enemy(ai::team::manager::get_team_enum_by_hash(hash),
                                      static_cast<ai::team::team_enum>(target.field_58));
}
void native_combat(camera_mode_combat *self, Float dt, camera_frame &frame,
                     const camera_target_info &const_target) {
    auto &target = const_cast<camera_target_info &>(const_target);
    auto *controller = target.field_54->m_player_controller;
    int loco = target.get_loco_mode();
    bool crawling = loco == 2 || loco == 7;
    bool airborne = loco == 5 || loco == 6 || loco == 3;
    auto *inode = combat_inode(target);
    combat_extra_target = false;
    if (!self->slave->field_1BC && self->slave->field_1CE &&
        !self->slave->field_1D0.is_flagged(0x20) && self->slave->field_1D0.is_flagged(2))
        self->slave->field_1CD = !self->slave->field_1CD;
    auto *physical = target.field_54->physical_ifc();
    if ((physical && (physical->field_C & 0x80000) != 0 && target.field_54->has_physical_ifc() &&
         !physical->is_effectively_standing()) || crawling || loco == 9 || loco == 3 || target.sub_4B28E0())
        self->disabled = true;
    else if (self->disabled && !airborne)
        self->disabled = false;
    if (!self->slave->field_1CD || !self->slave->field_1CE || self->disabled ||
        !controller || static_cast<int>(controller->m_hero_type) == 0 ||
        os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(140))) {
        forward_frame(self, dt, frame, target);
        return;
    }
    actor *scripted = vhandle_type<actor>{inode->field_30}.get_volatile_ptr();
    std::vector<combat_candidate> candidates;
    vector3d center;
    if (scripted) {
        center = scripted->get_abs_position();
        if ((center - target.pos).length2() > 3600.0f) {
            forward_frame(self, dt, frame, target);
            return;
        }
    } else {
        auto &actors = ai::combat_target_inode::combat_list();
        for (int i = 0; i < actors.size(); ++i) {
            auto *act = actors.at(i).get_volatile_ptr();
            if (!combat_candidate_valid(act, target)) continue;
            vector3d direction = act->get_abs_position() - target.pos;
            float distance = direction.length();
            if (distance < 30.0f) candidates.push_back({act, direction, distance, 0.0f});
        }
        if (candidates.empty()) {
            forward_frame(self, dt, frame, target);
            return;
        }
    }
    self->slave->field_1C4 = self->slave->field_1C0;
    self->slave->field_1C0 = 2;
    if (scripted) {
        controller->force_always_camera_relative(true);
        frame.eye += target.field_24 * (float(dt) * 0.75f);
        self->slave->field_1C8 = 1;
        camera_frame previous = frame;
        frame.constrain_pos_relative_to_plane(target.pos, target.up, 0.35f, 0.5f);
        float distance = (center - target.pos).length();
        target.max_look_dist *= 0.8f;
        auto visual_radius = reinterpret_cast<float (__fastcall *)(entity_base *, void *)>(
            get_vfunc(scripted->m_vtbl, 0x28));
        float radius = std::clamp(visual_radius(scripted, nullptr) * 0.5f, 0.75f, 2.5f);
        vector3d horizontal = center - target.pos;
        horizontal.y = 0.0f;
        float constraint = std::clamp((horizontal.length() /
            std::clamp(radius + target.radius, 3.0f, 5.0f) - 1.0f) * 1.7f - 1.0f, -1.0f, 0.7f);
        frame.rotate_to_include_target(center, target.pos, target.up, constraint);
        frame.eye = lerp(frame.eye, previous.eye, fast_mix);
        float gap = std::max(distance - (radius + target.radius), 0.0f) * 0.5f + 1.0f;
        float adjustment = (std::max(target.radius, radius) - target.radius) / sqr(gap) *
            (0.66f * g_camera_min_dist);
        target.min_look_dist += adjustment;
        target.max_look_dist += adjustment;
        pull_sphere(frame.eye, target.max_look_dist, target.pos);
        push_relative_plane(frame.eye, target.min_look_dist, target.pos, frame.up);
        push_relative_plane(frame.eye, target.min_look_dist, center, frame.up);
        center = center * 2.0f - target.pos;
        combat_extra_position = center;
        combat_extra_target = true;
        combat_extra_radius = radius;
        combat_extra_constraint = 0.88f;
        frame.smooth_with(previous, med_mix, slow_mix);
    } else {
        center = ZEROVEC;
        for (auto &candidate : candidates) {
            candidate.weight = std::clamp(1.0f - (candidate.distance - 10.0f) * 0.05f, 0.0f, 1.0f);
            candidate.direction *= candidate.weight;
            center += candidate.direction;
        }
        center /= float(candidates.size());
        float radius = 0.0f;
        for (const auto &candidate : candidates)
            radius = std::max(radius, (candidate.direction - center).length() * candidate.weight);
        target.max_look_dist *= std::clamp(radius * 0.065f + 1.1f, 1.1f, 1.75f);
        target.min_look_dist = target.max_look_dist * 0.75f;
        camera_mode_chase::pull_by_target(frame, target, target.max_look_dist);
        push_relative_plane(frame.eye, target.min_look_dist, target.pos, frame.up);
        frame.constrain_pos_relative_to_plane(target.pos, target.up, 0.3f, 0.35f);
        float distance = center.length();
        center += target.pos;
        if (distance > 3.0f) {
            camera_frame previous = frame;
            frame.rotate_to_include_target(center, target.pos, target.up, 0.6f);
            float weight = std::clamp(std::abs(distance - 16.5f) * (2.0f / 27.0f), 0.0f, 1.0f);
            frame.smooth_with(previous, weight, weight);
        }
        frame.include_target(center, radius, 0.8f);
    }
}
template<class T, void (*Function)(T *, Float, camera_frame &, const camera_target_info &)>
void __fastcall mode_advance(camera_mode *self, void *, Float dt, camera_frame *frame,
                             const camera_target_info *target) {
    Function(static_cast<T *>(self), dt, *frame, *target);
}
void shake_advance(camera_mode_shake *self, Float dt, camera_frame &frame, const camera_target_info &target) {
    self->_frame_advance(dt, frame, target);
}
void lookaround_advance(camera_mode_lookaround *self, Float dt, camera_frame &frame, const camera_target_info &target) {
    self->_frame_advance(dt, frame, target);
}
void passive_advance(camera_mode_passive *self, Float dt, camera_frame &frame, const camera_target_info &target) {
    self->_frame_advance(dt, frame, const_cast<camera_target_info &>(target));
}
void fixedstatic_advance(camera_mode_fixedstatic *self, Float dt, camera_frame &frame, const camera_target_info &target) {
    self->_frame_advance(dt, frame, target);
}
void __fastcall shake_activate(camera_mode *self, void *, camera_target_info *target) {
    forward_activate(self, nullptr, target);
    auto *shake = static_cast<camera_mode_shake *>(self);
    shake->frame_fwd = self->slave->get_abs_po().get_z_facing();
    shake->frame_eye = self->slave->get_abs_position();
}
void __fastcall passive_activate(camera_mode *self, void *, camera_target_info *target) {
    static_cast<camera_mode_passive *>(self)->_activate(*target);
}
void __fastcall passive_clear(camera_mode *self) {
    forward_clear(self);
    static_cast<camera_mode_passive *>(self)->field_1C = YVEC;
}
void __fastcall reset_recenter(camera_mode *self, void *, Float dt, const camera_target_info *target) {
    if (std::fpclassify(float(dt)) == FP_ZERO) ++dword_959E5C();
    forward_recenter(self, nullptr, dt, target);
}
void __fastcall combat_activate(camera_mode *self, void *, camera_target_info *target) {
    forward_activate(self, nullptr, target);
    static_cast<camera_mode_combat *>(self)->disabled = false;
}
void __fastcall filter_activate(camera_mode *self, void *, camera_target_info *target) {
    forward_activate(self, nullptr, target);
    static_cast<camera_mode_filter *>(self)->reset_frames = 1;
}
void __fastcall filter_recenter(camera_mode *self, void *, Float dt, const camera_target_info *target) {
    if (std::fpclassify(float(dt)) == FP_ZERO) static_cast<camera_mode_filter *>(self)->reset_frames = 1;
    forward_recenter(self, nullptr, dt, target);
}
void __fastcall transition_activate(camera_mode *self, void *, camera_target_info *target) {
    forward_activate(self, nullptr, target);
    static_cast<camera_mode_transition *>(self)->valid = false;
}
void __fastcall reorient_activate(camera_mode *self, void *, camera_target_info *target) {
    forward_activate(self, nullptr, target);
    auto *mode = static_cast<camera_mode_reorient *>(self);
    mode->last_target = 0;
    reorient_recenter(mode, 0.0f, *target);
}
void __fastcall reorient_request(camera_mode *self, void *, Float dt, const camera_target_info *target) {
    reorient_recenter(static_cast<camera_mode_reorient *>(self), dt, *target);
}
void __fastcall lookaround_request(camera_mode *self, void *, Float dt, const camera_target_info *target) {
    forward_recenter(self, nullptr, dt, target);
    self->slave->field_1CC = false;
}
void __fastcall lookaround_enable(camera_mode *self, void *, bool enabled) {
    forward_lookaround(self, nullptr, enabled);
    static_cast<camera_mode_lookaround *>(self)->field_C = enabled;
    if (!enabled) self->slave->field_1CC = false;
}
void __fastcall fixedstatic_set(camera_mode *self, void *, const vector3d *eye, const vector3d *at) {
    static_cast<camera_mode_fixedstatic *>(self)->_set_fixedstatic(*eye, *at);
}
void __fastcall fixedstatic_clear(camera_mode *self) {
    static_cast<camera_mode_fixedstatic *>(self)->_clear_fixedstatic();
}
void initialize_native_tables() {
    static const bool initialized = [] {
        base_table = mode_table<camera_mode>();
        shake_table = mode_table<camera_mode_shake>();
        shake_table.activate = shake_activate;
        shake_table.frame_advance = mode_advance<camera_mode_shake, shake_advance>;
        stationary_table = mode_table<camera_mode_stationary>();
        stationary_table.frame_advance = mode_advance<camera_mode, stationary_frame>;
        fixedstatic_table = mode_table<camera_mode_fixedstatic>();
        fixedstatic_table.frame_advance = mode_advance<camera_mode_fixedstatic, fixedstatic_advance>;
        fixedstatic_table.set_fixedstatic = fixedstatic_set;
        fixedstatic_table.clear_fixedstatic = fixedstatic_clear;
        filter_table = mode_table<camera_mode_filter>();
        filter_table.activate = filter_activate;
        filter_table.request_recenter = filter_recenter;
        filter_table.frame_advance = mode_advance<camera_mode_filter, filter_frame>;
        filter_table.extension.reset_state = reset_filter_state;
        eye_filter_table = mode_table<camera_mode_eye_filter>();
        eye_filter_table.activate = filter_activate;
        eye_filter_table.request_recenter = filter_recenter;
        eye_filter_table.frame_advance = mode_advance<camera_mode_filter, filter_frame>;
        eye_filter_table.extension.reset_state = reset_filter_state;
        transition_eye_table = mode_table<camera_mode_transition_eye>();
        transition_eye_table.activate = transition_activate;
        transition_eye_table.frame_advance = mode_advance<camera_mode_transition, transition_frame>;
        transition_eye_table.extension.smooth_transition = transition_eye_smooth;
        transition_direction_table = mode_table<camera_mode_transition_direction>();
        transition_direction_table.activate = transition_activate;
        transition_direction_table.frame_advance = mode_advance<camera_mode_transition, transition_frame>;
        transition_direction_table.extension.smooth_transition = transition_direction_smooth;
        include_table = mode_table<camera_mode_include>();
        include_table.frame_advance = mode_advance<camera_mode, include_frame>;
        precollide_table = mode_table<camera_mode_precollide>();
        precollide_table.frame_advance = mode_advance<camera_mode, precollide_frame>;
        reorient_table = mode_table<camera_mode_reorient>();
        reorient_table.activate = reorient_activate;
        reorient_table.request_recenter = reorient_request;
        reorient_table.frame_advance = mode_advance<camera_mode_reorient, reorient_frame>;
        lookaround_table = mode_table<camera_mode_lookaround>();
        lookaround_table.frame_advance = mode_advance<camera_mode_lookaround, lookaround_advance>;
        lookaround_table.request_recenter = lookaround_request;
        lookaround_table.enable_lookaround = lookaround_enable;
        combat_table = mode_table<camera_mode_combat>();
        combat_table.activate = combat_activate;
        combat_table.frame_advance = mode_advance<camera_mode_combat, native_combat>;
        combat_table.request_recenter = reset_recenter;
        passive_table = mode_table<camera_mode_passive>();
        passive_table.activate = passive_activate;
        passive_table.frame_advance = mode_advance<camera_mode_passive, passive_advance>;
        passive_table.request_recenter = reset_recenter;
        passive_table.clear_fixedstatic = passive_clear;
        return true;
    }();
    (void)initialized;
}
template<class T>
camera_mode *wrap_mode(spiderman_camera *slave, camera_mode *child, camera_mode::vtable &table) {
    auto *mode = new T(slave, child);
    mode->m_vtbl = &table;
    return mode;
}
}

camera_mode *create_native_camera_modes(spiderman_camera *slave) {
    initialize_native_tables();
    camera_mode *root = new camera_mode_passive(slave, nullptr);
    root = wrap_mode<camera_mode_combat>(slave, root, combat_table);
    static_cast<camera_mode_combat *>(root)->disabled = false;
    root = new camera_mode_lookaround(slave, root);
    root = wrap_mode<camera_mode_reorient>(slave, root, reorient_table);
    root = wrap_mode<camera_mode_transition_eye>(slave, root, transition_eye_table);
    root = wrap_mode<camera_mode_precollide>(slave, root, precollide_table);
    root = wrap_mode<camera_mode_eye_filter>(slave, root, eye_filter_table);
    root = wrap_mode<camera_mode_include>(slave, root, include_table);
    root = wrap_mode<camera_mode_transition_direction>(slave, root, transition_direction_table);
    root = wrap_mode<camera_mode_filter>(slave, root, filter_table);
    root = new camera_mode_fixedstatic(slave, root);
    root = wrap_mode<camera_mode_stationary>(slave, root, stationary_table);
    return new camera_mode_shake(slave, root);
}
void destroy_native_camera_modes(camera_mode *root) {
    if (root) root->m_vtbl->finalize(root, nullptr, 1);
}
