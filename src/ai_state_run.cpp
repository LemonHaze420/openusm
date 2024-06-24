#include "ai_state_run.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_full_target_inode.h"
#include "camera.h"
#include "common.h"
#include "controller_inode.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "game.h"
#include "game_settings.h"
#include "input.h"
#include "inputsettings.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "physics_inode.h"
#include "state_machine.h"
#include "utility.h"

#include <cmath>

namespace ai {

VALIDATE_SIZE(run_state, 0x5C);

run_state::run_state(from_mash_in_place_constructor *a2) {
    THISCALL(0x00449BD0, this, a2);
}

string_hash fence_hop_grind_angle_cosine_id{to_hash("fence_hop_grind_angle_cosine")};

bool run_state::check_for_fence_hop(Float a2, vector3d *a3)
{
    if constexpr (1)
    {
        auto *act = this->get_actor();

        vector3d abs_pos = act->get_abs_position();
        abs_pos[1] = abs_pos[1] - 0.1f;

        auto &v11 = act->get_abs_po();

        vector3d v20 = v11.get_z_facing();
        v20 *= 0.80000001f;
        v20 += abs_pos;

        line_info v21{abs_pos, v20};

        if (v21.check_collision(*local_collision::entfilter_entity_no_capsules,
                                *local_collision::obbfilter_lineseg_test,
                                nullptr)) {
            act->cancel_animated_movement(v21.hit_norm, 0.0f);
        }

        return false;

    }
    else
    {
        return (bool) THISCALL(0x004696B0, this, a2, a3);
    }
}

ai::state_trans_messages run_state::_frame_advance(Float a2)
{
    if constexpr (0)
    {
        vector3d v92;
        vector3d move_dir;
        vector3d norm_stick_dir;

        camera *cam_ptr;

        auto retVal = enhanced_state::frame_advance(a2);
        auto *v3 = this->field_50;
        auto *v4 = v3->field_20;
        auto *v5 = v3->field_24;
        auto *v6 = v3->field_30;

        auto *v83 = v4;

        vector3d v87;

        this->field_50->field_58[1] = v3->field_28->get_abs_position()[1];
        auto *v7 = this->get_actor();
        if (v7->is_frame_delta_valid()) {
            auto *v8 = this->get_actor();
            vector3d v89 = v8->get_frame_delta()->m[3];
            auto *v9 = this->get_actor();

            [[maybe_unused]] auto v87 = orthogonal_projection_onto_plane(v89, v9->get_abs_po().m[1]);
            auto lateral_distance = v89.length();
            assert(lateral_distance >= 0.0f);

            if (hero_inode::get_hero_type() == 2) {
                g_game_ptr->gamefile->update_miles_run_venom(lateral_distance);
            } else {
                g_game_ptr->gamefile->update_miles_run_spidey(lateral_distance);
            }
        }

        this->get_actor()->m_player_controller->set_spidey_loco_mode(static_cast<eHeroLocoMode>(1));

        if ( !v83->is_layer_interruptable(static_cast<als::layer_types>(0)) ) {
            return TRANS_TOTAL_MSGS;
        }

        auto v16 = v5->get_axis_2d(static_cast<controller_inode::eControllerAxis>(0));

        static string_hash loco_allow_walk_run_id{to_hash("loco_allow_walk_run")};

        auto tmp = v16.length();

        auto *v19 = this->get_core();

        auto opt_int = v19->field_50.get_optional_pb_int(loco_allow_walk_run_id, 0, nullptr);
        float v20 = ((opt_int && !v5->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0))) ? tmp : 0.0f);

        this->field_48 = v20 * 7.0f;

        als::param_list p_list{};

        p_list.add_param({0, this->field_48});

        static string_hash lock_on_target_id{to_hash("lock_on_target")};

        auto *v21 = this->get_core();

        auto v22 = v21->field_50.get_pb_int(lock_on_target_id);

        vhandle_type<actor> v23 = v6->quick_targeting();
        bool v81;

        if (v23.get_volatile_ptr() == nullptr || !v6->is_target_known() || (v81 = true, v22 == 0)) {
            v81 = false;
        }

        string_hash tmp_hash = v83->get_category_id(static_cast<als::layer_types>(0));
        if (tmp_hash != cat_id_idle_walk_run()) {
            v81 = false;
        }

        float v25 = (v81 ? 1.0f : 0.0f);

        p_list.add_param(als::param{52, v25});
        if (this->field_54 == 1) {
            auto *v26 = v83->get_als_layer(static_cast<als::layer_types>(0));

            string_hash v27 = v26->get_state_id();
            if (v27 == string_hash{}) {
                auto *v28 = this->get_actor();
                setup_hero_capsule(v28);
                this->field_54 = 0;
                auto *v29 = this->get_actor();

                auto *v31 = v29->physical_ifc();
                v31->set_allow_manage_standing(true);

                auto *v32 = this->get_actor();
                v32->set_terrain_collisions_active(true);
            }
        }

        auto *v33 = this->get_actor();
        if (v33->has_physical_ifc()) {
            auto *v34 = this->get_actor();
            v34->physical_ifc();
            if (!this->field_58) {
                auto *v35 = this->get_actor();
                if (v35->physical_ifc()->field_100[1] <= 0.73242188f) {
                    auto *v36 = this->get_actor();
                    shrink_capsule_for_slanted_surfaces(v36);
                    this->field_58 = true;
                    goto LABEL_34;
                }

                if (!this->field_58) {
                    goto LABEL_34;
                }
            }

            auto *v37 = this->get_actor();
            if (v37->physical_ifc()->field_100[1] > 0.73242188f) {
                auto *v38 = this->get_actor();
                setup_hero_capsule(v38);
                this->field_58 = false;
            }
        }

    LABEL_34:

        vector3d *v51;

        if (opt_int == 0 || v5->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0))) {
            auto *v52 = this->get_actor();

            auto &abs_po = v52->get_abs_po();

            vector3d curr_player_dir = abs_po.get_z_facing();
            if (v81) {
                auto v87 = v3->field_28->get_abs_position();

                vhandle_type<actor> v53 = v6->quick_targeting();
                auto *v54 = v53.get_volatile_ptr();

                auto *v56 = (float *) &v54->get_abs_po();
                auto v57 = v87[2] - this->field_30[2];
                auto &v58 = this->field_30;
                v56 += 12;
                auto v59 = v87[1] - this->field_30[1];
                auto v60 = v56[0];
                auto v61 = v56[1];
                auto v62 = v87[0] - this->field_30[0];

                vector3d v89;
                v89[2] = v56[2];
                v89[0] = v60;
                v89[1] = v61;
                if (sqrt(v62 * v62 + v59 * v59 + v57 * v57) < 0.30000001f) {
                    auto v64 = v89[0] - this->field_3C[0];
                    auto v63 = v89[1] - this->field_3C[1];
                    auto v65 = v89[2] - this->field_3C[2];
                    if (sqrt(v63 * v63 + v65 * v65 + v64 * v64) < 0.30000001f) {
                        v87[0] = this->field_30[0];
                        v87[1] = this->field_30[1];
                        v87[2] = this->field_30[2];

                        v89[0] = this->field_3C[0];
                        v89[1] = this->field_3C[1];
                        v89[2] = this->field_3C[2];
                    }
                }

                vector3d v93 = v87 - v89;

                auto v69 = v93.normalized();

                vector3d a3;
                a3[0] = -v69[0];
                curr_player_dir[0] = a3[0];
                a3[1] = -v69[1];

                v58[0] = v87[0];
                a3[2] = -v69[2];

                curr_player_dir[1] = a3[1];

                this->field_3C[0] = v60;
                this->field_30[1] = v87[1];

                curr_player_dir[2] = a3[2];

                this->field_3C[1] = v61;
                this->field_30[2] = v87[2];
                this->field_3C[2] = v89[2];
            }

            p_list.add_param(27u, curr_player_dir);

        } else {
            norm_stick_dir = v5->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            norm_stick_dir.normalize();
            move_dir = norm_stick_dir;
            if (v81) {
                vhandle_type<actor> v39 = v6->quick_targeting();
                auto *v40 = v39.get_volatile_ptr();
                auto v41 = v40->get_abs_position();
                auto v42 = v3->field_28->get_abs_position();

                vector3d v93 = v42 - v41;

                auto v45 = v93.normalized();

                move_dir = -v45;
            }

            this->check_for_fence_hop(a2, &norm_stick_dir);
            assert(move_dir.is_normal());
            assert(move_dir.is_valid());

            assert(norm_stick_dir.is_normal());
            assert(norm_stick_dir.is_valid());

            cam_ptr = g_game_ptr->get_current_view_camera(0);

            v92 = cam_ptr->get_abs_po().m[2];

            if (Input::instance()->field_129D8[0]->field_18.get_state(InputAction::Forward) >
                    0.80000001f &&
                Input::instance()->field_129D8[0]->field_18.get_state(InputAction::Backward) <
                    0.80000001f &&
                Input::instance()->field_129D8[0]->field_18.get_state(InputAction::TurnRight) <
                    0.80000001f) {
                p_list.add_param(27u, v92);
                v51 = &v92;

            } else {
                p_list.add_param(27u, move_dir);
                v51 = &norm_stick_dir;
            }
        }

        p_list.add_param(30u, *v51);
        p_list.add_param(24u, YVEC);
        auto *v77 = v83->get_als_layer(static_cast<als::layer_types>(0));
        v77->set_desired_params(p_list);
        this->field_50->cleanup_collision_lists();

        return retVal;
    } else {
        return static_cast<state_trans_messages>(THISCALL(0x00473650, this, a2));
    }
}

} // namespace ai

void run_state_patch()
{
    FUNC_ADDRESS(address, &ai::run_state::_frame_advance);
    set_vfunc(0x00877498, address);
}
