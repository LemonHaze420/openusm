#include "pole_swing_state.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "controller_inode.h"
#include "custom_math.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "log.h"
#include "oldmath_po.h"
#include "oldmath_usefulmath.h"
#include "param_list.h"
#include "physics_inode.h"
#include "pole_swing_inode.h"
#include "state_machine.h"
#include "utility.h"
#include "variables.h"

#include <cmath>

namespace ai {

VALIDATE_SIZE(pole_swing_state, 0x70);

string_hash &cat_id_pole_swing = var<string_hash>(0x0096BFB4);

string_hash &cat_id_pole_idle = var<string_hash>(0x0096C10C);

string_hash &cat_id_pole_crawl = var<string_hash>(0x0096C350);

string_hash &pole_swing_ang_vel_dampen_id = var<string_hash>(0x0095882C);

string_hash &pole_swing_min_ang_vel_id = var<string_hash>(0x009583B8);

string_hash &pole_swing_max_ang_vel_id = var<string_hash>(0x00958524);

string_hash &pole_swing_ang_acc_id = var<string_hash>(0x009585C0);

string_hash &pole_swing_grav_mul_id = var<string_hash>(0x009587E0);

static constexpr float flt_95847C{3.1415927f};

pole_swing_state::pole_swing_state(from_mash_in_place_constructor *a2)
{
    THISCALL(0x0044C4B0, this, a2);
}

void pole_swing_state::_activate(ai_state_machine *a2, const mashed_state *a3, const mashed_state *a4,
                                 const param_block *a5, base_state::activate_flag_e a6)
{
    sp_log("pole_swing_state::activate");

    if constexpr (0) {
        enhanced_state::activate(a2, a3, a4, a5, a6);

        auto core = this->get_core();
        auto *info_node = (physics_inode *)core->get_info_node(physics_inode::default_id, true);

        auto *v8 = this->get_core();
        auto *v51 = (als_inode *)v8->get_info_node(als_inode::default_id, true);

        auto *v9 = this->get_core();
        auto *a4a = (pole_swing_inode *)v9->get_info_node(pole_swing_inode::default_id, true);

        info_node->setup_for_pole_swing();
        v51->request_category_transition(cat_id_pole_swing, static_cast<als::layer_types>(0), true, false, false);
        this->field_30 = a4a->field_1C.get_target();
        this->field_3C = a4a->field_1C.get_origin();

        this->field_48 = this->field_3C - this->field_30;
        this->field_48.normalize();
        auto v15 = this->field_30[1] - this->field_3C[1];
        auto v16 = this->field_30[0] - this->field_3C[0];
        auto v17 = this->field_30[2] - this->field_3C[2];
        auto v18 = std::sqrt(v16 * v16 + v15 * v15 + v17 * v17);
        this->field_68 = v18;
        this->field_64 = this->field_68 * 0.5f;
        auto front = info_node->get_abs_po().get_z_facing();
        front[1] = 0.0;
        front = sub_444A60(front, this->field_48);
        front.normalize();

        auto v48 = info_node->get_abs_po().get_x_facing();
        if (front.length2() < LARGE_EPSILON) {
            front = v48;
            front = sub_444A60(front, this->field_48);
            front[1] = 0.0f;
            front.normalize();

            assert(front.length2() > LARGE_EPSILON);
        }

        if (dot(this->field_48, v48) < 0.0f) {
            std::swap(this->field_30, this->field_3C);
            this->field_48 = -this->field_48;
        }

        this->field_54 = vector3d::cross(front, this->field_48);
        this->field_54.normalize();

        a4a->field_28 = flt_95847C;
        a4a->field_24 = flt_95847C;
        this->field_60 = 3.0;
        this->field_6C = 0;
        auto *act = this->get_actor();
        entity_set_abs_position(act, this->field_30);

        als::param_list params{};
        params.add_param(0x27u, this->field_30);
        auto *als_layer = v51->get_als_layer(static_cast<als::layer_types>(0));
        als_layer->set_desired_params(params);
    } else {
        THISCALL(0x0046BD90, this, a2, a3, a4, a5, a6);
    }
}

state_trans_messages pole_swing_state::_frame_advance(Float a1)
{
    TRACE("pole_swing_state::frame_advance");

    if constexpr (0) {
        auto *core = this->get_core();
        auto *physics_inode = (ai::physics_inode *)core->get_info_node(ai::physics_inode::default_id, true);

        auto *v4 = this->get_core();
        auto *v5 = (ai::pole_swing_inode *)v4->get_info_node(ai::pole_swing_inode::default_id, true);

        auto *v6 = this->get_core();
        auto *v42 = (ai::als_inode *)v6->get_info_node(ai::als_inode::default_id, true);

        auto *v7 = this->get_core();
        auto *v8 = (ai::controller_inode *)v7->get_info_node(ai::controller_inode::default_id, true);
        auto *player_controller = this->get_actor()->m_player_controller;
        player_controller->set_spidey_loco_mode(eHeroLocoMode::POLE_SWING);

        auto v43 = v8->get_axis(static_cast<controller_inode::eControllerAxis>(0));
        auto v46 = ZEROVEC;
        po v55{};
        float v44 = 0.0;
        v55.set_rot(this->field_48, v5->field_28);
        auto cur_up = v55.non_affine_slow_xform(this->field_54);
        v5->field_28 = wrap_angle(v5->field_28);

        auto *v14 = this->get_core();
        auto pole_swing_min_ang_vel = v14->field_50.get_pb_float(pole_swing_min_ang_vel_id);

        auto *v15 = this->get_core();
        auto pole_swing_max_ang_vel = v15->field_50.get_pb_float(pole_swing_max_ang_vel_id);

        auto *v16 = this->get_core();
        auto pole_swing_ang_acc = v16->field_50.get_pb_float(pole_swing_ang_acc_id);
        if (v42->get_category_id(static_cast<als::layer_types>(0)) != ai::cat_id_pole_swing) {
            this->field_6C = 0;
        }

        if (v42->get_category_id(static_cast<als::layer_types>(0)) == ai::cat_id_pole_swing) {
            auto *v17 = this->get_core();
            auto pole_swing_ang_vel_dampen = v17->field_50.get_pb_float(ai::pole_swing_ang_vel_dampen_id);

            auto *v18 = this->get_core();
            auto pole_swing_grav_mul = v18->field_50.get_pb_float(ai::pole_swing_grav_mul_id);
            auto v52 = this->field_54 * -1.0f;
            auto abs_po = physics_inode->get_abs_po();
            this->field_60 = dot(abs_po.get_z_facing(), v52) * pole_swing_grav_mul * a1 + this->field_60;
            this->field_60 *= pole_swing_ang_vel_dampen;

            this->field_60 = std::clamp(this->field_60, pole_swing_min_ang_vel, pole_swing_max_ang_vel);

            v5->field_28 = a1 * this->field_60 + v5->field_28;
            if ((v5->field_24 > v5->field_28 && approx_equals(flt_95847C, 0.0f, EPSILON)) ||
                (v5->field_24 < flt_95847C && v5->field_28 >= flt_95847C)) {
                ++this->field_6C;
                auto vhandle = this->get_actor()->my_handle;
                event_manager::raise_event(event::TRICK_POLE_SWING_REVOLUTION, vhandle);
            }

            if (a1 * this->field_60 * s_camera_target_radius_factor <= std::abs(3.1415927f - v5->field_28) ||
                pole_swing_min_ang_vel * 1.2f <= std::abs(this->field_60)) {
                v44 = 1.0;
            } else {
                v5->field_28 = 3.1415927;
            }
        } else {
            auto *v24 = v42;
            if (v42->get_category_id(static_cast<als::layer_types>(0)) != ai::cat_id_pole_idle &&
                v24->get_category_id(static_cast<als::layer_types>(0)) == ai::cat_id_pole_crawl) {
                auto v1 = 2.0f * a1;
                auto v2 = dot(v43, this->field_48);
                this->field_64 = v2 * v1 + this->field_64;
                this->field_64 = std::clamp(this->field_64, 0.0f, this->field_68);
            }
        }

        if (v43.length2() > 0.30000001 * 0.30000001) {
            v46 = v43;
            auto v27 = dot(v46, this->field_48);
            if (std::abs(v27) < 0.2f) {
                v44 = 1.0;
                if (std::abs(3.1415927f - v5->field_28) < 0.5f) {
                    this->field_60 = pole_swing_ang_acc * a1 + this->field_60;
                    this->field_60 = std::clamp(this->field_60, 1.5f, 8.0f);
                }
            }
        }

        auto v30 = this->field_48 * this->field_64;
        auto v54 = this->field_30 + v30;

        als::param_list v40{};
        auto new_dir = vector3d::cross(this->field_48, cur_up);

        assert(new_dir.is_normal());
        assert(new_dir.is_valid());

        assert(cur_up.is_normal());
        assert(cur_up.is_valid());

        assert(!is_colinear(new_dir, cur_up));

        v40.add_param(27u, new_dir);
        v40.add_param(24u, cur_up);
        v40.add_param(39u, v54);
        v40.add_param(30u, v46);

        v40.add_param(als::param{0, v44});
        auto *als_layer = v42->get_als_layer(static_cast<als::layer_types>(0));
        als_layer->set_desired_params(v40);
        v5->field_24 = v5->field_28;
        return static_cast<ai::state_trans_messages>(75);
    } else {
        state_trans_messages(__fastcall * func)(void *, void *edx, Float) = CAST(func, 0x0045CA30);
        return func(this, nullptr, a1);
    }
}

}  // namespace ai

void pole_swing_state_patch()
{
    {
        FUNC_ADDRESS(address, &ai::pole_swing_state::_activate);
        set_vfunc(0x008771B8, address);
    }
}
