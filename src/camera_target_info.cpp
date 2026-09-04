#include "camera_target_info.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_std_combat_target.h"
#include "ai_team.h"
#include "base_ai_core.h"
#include "collide.h"
#include "common.h"
#include "custom_math.h"
#include "entity.h"
#include "func_wrapper.h"
#include "generic_anim_controller.h"
#include "oldmath_po.h"
#include "oldmath_usefulmath.h"
#include "physical_interface.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(camera_target_info, 0x5C);

static const float flt_881AC0 = 0.5;
static const float flt_882098 = 2.5;
static const float flt_87EA34 = 0.75;
static const float flt_8820A0 = 0.66000003;
static const float flt_87EEDC = 0.69999999;

float &g_camera_min_dist = var<float>(0x00881AB4);

float &g_camera_max_dist = var<float>(0x00881AB8);

float &g_camera_supermax_dist = var<float>(0x00881ABC);

camera_target_info::camera_target_info(entity *_target, Float a3, const vector3d &_pos, const vector3d &_up)
{
    TRACE("camera_target_info::camera_target_info");

    assert(_target != nullptr);
    assert(_target->is_a_conglomerate());

    if constexpr (0) {
        this->field_54 = bit_cast<actor *>(_target);
        auto v69 = this->field_54->get_abs_po();
        this->field_C = this->field_54->get_visual_center();
        auto *v8 = this->field_54->anim_ctrl;
        if (v8 != nullptr) {
            this->field_C = v69.inverse_xform(this->field_C);

            v8->get_camera_root_abs_po(v69);
            this->field_C = v69.slow_xform(this->field_C);
        }

        this->pos = v69.get_position();

        this->facing = v69.get_z_facing();

        this->up = v69.get_y_facing();

        assert(facing.is_normal());

        assert(up.is_normal());

        int hero_type = [](auto *v19) -> int {
            auto v20 = v19->m_player_controller;
            return (v20 != nullptr ? v20->m_hero_type : 0);
        }(this->field_54);

        switch (hero_type) {
        case 1:
            this->radius = 0.75;
            break;
        case 2:
            this->radius = 1.15;
            break;
        case 3:
            this->radius = 0.75;
            break;
        default: {
            auto v24 = this->field_54->get_visual_radius() * flt_881AC0;
            if (v24 >= flt_87EA34) {
                if (v24 > flt_882098) {
                    v24 = flt_882098;
                }

                this->radius = v24;
            } else {
                this->radius = flt_87EA34;
            }
        }
        }

        this->min_look_dist = (this->radius * flt_8820A0 + flt_881AC0) * g_camera_min_dist;
        auto v63 = this->min_look_dist * 2;
        this->max_look_dist = std::min(v63, g_camera_supermax_dist);

        auto v66 = this->pos;

        if (this->get_loco_mode() == 8) {
            this->up = YVEC;
            auto *v33 = this->field_54;
            if (v33->m_player_controller != nullptr) {
                auto v64 = v33->m_player_controller->get_poleswing_anchor();
                if (v64.is_valid()) {
                    auto v61 = v64.get_origin();
                    auto v34 = v64.get_target();
                    closest_point_segment(this->pos, v34, v61, this->pos);
                }
            }
        } else {
            auto v35 = this->radius * 0.69999999;
            this->pos += this->up * v35;
        }

        this->field_C += this->pos - v66;
        this->field_24 = ZEROVEC;

        if (sqr(8.0) > (this->pos - _pos).length2()) {
            this->pos = lerp(this->pos, _pos, pronto_mix);
            if (dot(this->up, _up) > -0.99000001) {
                auto v51 = lerp(this->up, _up, slow_mix);
                this->up = v51.normalized();
            }

            if (a3 != 0.0f) {
                auto v63 = 1.f / a3;
                this->field_24 = (this->pos - _pos) * v63;
            }
        }

        if (this->field_54->get_ai_core() != nullptr) {
            auto v62 = ai::combat_target_inode::team_hash();
            auto *v56 = this->field_54;
            auto *v58 = v56->get_ai_core();
            auto v59 = v58->field_50.get_pb_hash(v62);
            this->field_58 = ai::team::manager::get_team_enum_by_hash(v59);
        } else {
            this->field_58 = 0;
        }
    } else {
        THISCALL(0x004B3DC0, this, _target, a3, &_pos, &_up);
    }
}

void *__fastcall camera_target_info_constructor(void *self, int, entity *target, Float a3, const vector3d &a4,
                                                const vector3d &a5)
{
    return new (self) camera_target_info{target, a3, a4, a5};
}

int camera_target_info::get_loco_mode() const
{
    TRACE("camera_target_info::get_loco_mode");

    auto *v1 = this->field_54->m_player_controller;
    if (v1 != nullptr) {
        return 1;
    }

    if (auto result = v1->get_spidey_loco_mode(); result >= 0) {
        return result;
    }

    return 1;
}

float camera_target_info::sub_4B42E0() const
{
    auto *v2 = this->field_54;
    auto radius = this->radius;
    if (v2->has_physical_ifc()) {
        auto *v4 = this->field_54->physical_ifc();
        radius = v4->get_floor_offset();
    }

    float v8 = 0.1f;
    float result = v8;
    if (radius >= 0.1f) {
        result = radius;
    }

    return result;
}

bool camera_target_info::sub_4B2980() const
{
    return this->field_24.length2() > 0.000099999997;
}

bool camera_target_info::sub_4B29C0() const
{
    auto *the_controller = this->field_54->m_player_controller;
    return the_controller != nullptr && the_controller->get_spidey_loco_mode() == 13;
}

eHeroLocoMode camera_target_info::get_prev_loco_mode() const
{
    auto *the_controller = this->field_54->m_player_controller;
    if (the_controller == nullptr) {
        return static_cast<eHeroLocoMode>(1);
    }

    int result = the_controller->get_prev_spidey_loco_mode();
    if (result < 0) {
        return static_cast<eHeroLocoMode>(1);
    }

    return static_cast<eHeroLocoMode>(result);
}

void camera_target_info_patch()
{
    REDIRECT(0x004B6415, &camera_target_info_constructor);
}
