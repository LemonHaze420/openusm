#include "ai_state_swing.h"

#include "actor.h"
#include "ai_find_best_swing_anchor.h"
#include "ai_player_controller.h"
#include "ai_state_jump.h"
#include "ai_state_run.h"
#include "ai_state_swing.h"
#include "ai_state_web_zip.h"
#include "ai_std_hero.h"
#include "ai_tentacle_web_curly.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_state.h"
#include "color32.h"
#include "common.h"
#include "conglom.h"
#include "controller_inode.h"
#include "custom_math.h"
#include "debug_render.h"
#include "debug_user_render.h"
#include "entity.h"
#include "entity_base_vhandle.h"
#include "event.h"
#include "event_manager.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "game_button.h"
#include "hit_react_state.h"
#include "info_node_desc_list.h"
#include "info_node_descriptor.h"
#include "interaction_inode.h"
#include "layer_state_machine.h"
#include "marker.h"
#include "memory.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "oldmath_usefulmath.h"
#include "os_developer_options.h"
#include "physical_interface.h"
#include "physics_inode.h"
#include "pick_up_state.h"
#include "polytube.h"
#include "polytubecustommaterial.h"
#include "quaternion.h"
#include "region.h"
#include "resource_manager.h"
#include "sampling_window.h"
#include "spiderman_camera.h"
#include "spidey_combat_inode.h"
#include "state_machine.h"
#include "sweet_cone.h"
#include "swinger.h"
#include "utility.h"
#include "variable.h"
#include "variables.h"
#include "vtbl.h"
#include "wds.h"
#include "web_polytube.h"
#include "web_sounds.h"

#include <cassert>
#include <cmath>

static int &swinger_lr_swing_accel = var<int>(0x0095804C);
static int &swinger_ud_swing_accel = var<int>(0x00958048);

static auto &fast_swing = var<bool>(0x0095833C);

static constexpr auto start_speed = 0.5f;

static constexpr auto minimum_web_length = 5.0f;

static constexpr float magic_velocity_cancelling_angle_in_degrees{120.0f};

static constexpr float min_swing_web_length_squared = 81.180107;
static constexpr float max_swing_web_length_squared = 5626.5005;
static constexpr float sweet_cone_angle_cos = std::cos((3.1415927 / 180.0) * 30.01);

static float &swing_collision_break_length = var<float>(0x009585BC);
static float &swing_collision_stick_length = var<float>(0x0091F468);

static auto &swingers = var<swinger_t[2]>(0x00958188);

static auto &no_swing_timer = var<float>(0x00958034);
static constexpr float no_swing_lockout_time{0.2f};

static auto &something_to_swing_to_data = var<something_to_swing_to_data_t>(0x00958098);

static const string_hash loco_allow_swing_id{static_cast<int>(to_hash("loco_allow_swing"))};

find_best_anchor_result_t::find_best_anchor_result_t()
{
    this->m_visual_point = ZEROVEC;
    this->m_normal = ZEROVEC;
    this->m_point = this->m_normal;
    this->m_entity = nullptr;
    this->m_best_distance_squared = -1.0;
    this->m_target_length = -1.0;
    this->valid_flags = 0;
}

float find_best_anchor_result_t::get_target_length()
{
    assert(valid_flags & TARGET_LENGTH_IS_VALID);

    return this->m_target_length;
}

void find_best_anchor_result_t::set_target_length(Float target_length)
{
    this->m_target_length = target_length;
    this->valid_flags |= TARGET_LENGTH_IS_VALID;
}

float find_best_anchor_result_t::get_best_distance_squared()
{
    assert(valid_flags & BEST_DISTANCE_SQUARED_IS_VALID);

    return this->m_best_distance_squared;
}

void find_best_anchor_result_t::set_best_distance_squared(Float best_distance_squared)
{
    this->m_best_distance_squared = best_distance_squared;
    this->valid_flags |= BEST_DISTANCE_SQUARED_IS_VALID;
}

vector3d *find_best_anchor_result_t::get_point()
{
    assert(valid_flags & POINT_IS_VALID);
    return &this->m_point;
}

void find_best_anchor_result_t::set_point(const vector3d &point)
{
    this->m_point = point;
    this->valid_flags |= POINT_IS_VALID;
}

vector3d &find_best_anchor_result_t::get_normal()
{
    assert(valid_flags & NORMAL_IS_VALID);
    return this->m_normal;
}

void find_best_anchor_result_t::set_normal(const vector3d &normal)
{
    this->m_normal = normal;
    this->valid_flags |= NORMAL_IS_VALID;
}

vector3d &find_best_anchor_result_t::get_visual_point()
{
    assert(valid_flags & VISUAL_POINT_IS_VALID);

    return this->m_visual_point;
}

void find_best_anchor_result_t::set_visual_point(const vector3d &a1)
{
    this->m_visual_point = a1;
    this->valid_flags |= VISUAL_POINT_IS_VALID;
}

entity_base *find_best_anchor_result_t::get_entity()
{
    assert(valid_flags & ENT_IS_VALID);

    return this->m_entity;
}

void find_best_anchor_result_t::set_entity(entity_base *a2)
{
    this->m_entity = a2;
    this->valid_flags |= ENT_IS_VALID;
}

static auto &s_initial_heading = var<vector3d>(0x00958154);

namespace ai {

VALIDATE_SIZE(swing_state, 0x40);

swing_state::swing_state() : enhanced_state() {}

swing_state::swing_state(from_mash_in_place_constructor *a2) : enhanced_state(a2)
{
    this->m_vtbl = 0x008774B8;
}

swing_state::~swing_state()
{
    this->finalize(mash::ALLOCATED);
}

void swing_state::finalize(mash::allocation_scope)
{
    if (this->get_actor() != nullptr) {
        auto *actor = this->get_actor();
        if (actor->has_physical_ifc()) {
            if (this->field_C != nullptr) {
                auto *v4 = this->field_3C;
                auto *v5 = v4->field_40;
                auto *v6 = v4->field_3C;
                if (!v5->field_54) {
                    v5->play_fire_web_sound();
                }

                auto *v7 = this->get_actor();
                v6->add_swingback(swingers[0].field_90, nullptr, v7);

                auto *v8 = this->get_actor();
                auto *v9 = v8->physical_ifc();
                v9->field_C &= ~0x200u;

                swingers[0].field_0.m_active = false;
                swingers[0].field_90->set_visible(false, false);
                swingers[0].field_90->set_visible(false, false);
                swingers[1].field_90->set_visible(false, false);
                swingers[0].field_94->set_visible(swingers[0].field_90->is_visible(), false);

                swingers[1].field_94->set_visible(swingers[1].field_90->is_visible(), false);

                no_swing_timer = 0.0;
            }
        }
    }
}

uint32_t swing_state::get_virtual_type_enum()
{
    return 319;
}

void swing_state::_activate(ai_state_machine *a2, const mashed_state *arg4, const mashed_state *a4,
                            const param_block *a5, base_state::activate_flag_e a6)
{
    TRACE("ai::swing_state::activate");

    if constexpr (1) {
        enhanced_state::activate(a2, arg4, a4, a5, a6);

        auto *v7 = this->get_core();
        this->field_3C = bit_cast<hero_inode *>(v7->get_info_node(hero_inode::default_id, true));

        auto *v8 = this->get_actor()->m_player_controller;
        v8->set_spidey_loco_mode(static_cast<eHeroLocoMode>(3));

        auto &v10 = g_world_ptr->field_1B0;
        if (!v10.field_8.is_set()) {
            g_world_ptr->activate_web_splats();
        }

        auto &v11 = g_world_ptr->field_1F0;
        if (!v11.field_8.is_set()) {
            g_world_ptr->activate_corner_web_splats();
        }

        this->field_3C->field_2C->left_air();

        auto *v12 = this->field_3C;
        auto *v13 = v12->field_20;
        auto *v14 = v12->field_28;
        auto *v15 = v12->field_40;

        v15->field_24 = true;
        v15->field_54 = false;

        v15->m_swing_time = 0.0;
        auto *v35 = v14;
        auto *v32 = v13;

        static const string_hash Swing_Launch{to_hash("Swing_Launch")};

        v13->request_category_transition(Swing_Launch, static_cast<als::layer_types>(0), true, false, false);
        auto *v16 = this->get_actor();

        po &abs_po = v16->get_abs_po();

        static constexpr auto swing_up = 0.2f;
        static constexpr auto swing_front = 0.0f;

        s_initial_heading = abs_po.get_z_facing();

        vector3d v33 = abs_po.get_position() + abs_po.get_z_facing() * swing_front + abs_po.get_y_facing() * swing_up;

        auto *v17 = this->get_actor();
        entity_set_abs_position(v17, v33);

        vector3d v30 = something_to_swing_to_data.field_C - abs_po.get_position();
        v30.y = LARGE_EPSILON;
        v30.normalize();

        auto *v21 = v35;
        auto v22 = v35->get_velocity();

        v15->m_anchor_angle = RAD_TO_DEG(calculate_xz_angle_relative_to_local_po(abs_po, v30, v22));

        als::param_list v29{};

        v29.add_param({13, v15->m_anchor_angle});

        v32->set_desired_params(v29, static_cast<als::layer_types>(0));

        if (os_developer_options::instance->get_flag(mString{"SHOW_LOCOMOTION_INFO"})) {
            line_info v56;

            auto abs_pos = abs_po.get_position();
            v56.field_0 = abs_pos;
            v56.field_C = something_to_swing_to_data.field_C;
            v56.field_C.y = abs_pos.y;
            v56.render(20, false);

            v56.field_0 = abs_po.get_position();

            constexpr float v67 = 3.0;
            auto v52 = v35->get_velocity() * v67;
            v56.field_C = abs_po.get_position() + v52;
            v56.render(21, false);
            mString v40{0, "anchor ang %.2f", v15->m_anchor_angle};

            color32 v34{255, 255, 255, 255};
            insertDebugString(2, v40, v34);
        }

        v21->setup_for_swing();
        v15->setup_new_web();
    } else {
        THISCALL(0x0047DA60, this, a2, arg4, a4, a5, a6);
    }
}

void swing_state::_deactivate(const ai::mashed_state *a1)
{
    TRACE("ai::swing_state::deactivate");

    base_state::_deactivate(a1);
    if (this->get_actor() != nullptr) {
        auto *v3 = this->get_actor();
        if (v3->has_physical_ifc()) {
            this->field_3C->field_28->cleanup_from_swing();
        }
    }
}

ai::state_trans_messages swing_state::_frame_advance(Float a2)
{
    TRACE("ai::swing_state::frame_advance()");

    if constexpr (0) {
        auto *v3 = this->get_actor()->m_player_controller;
        v3->set_spidey_loco_mode(static_cast<eHeroLocoMode>(3));

        auto *hero_ptr = this->field_3C;
        auto *swing_inode_ptr = hero_ptr->field_40;
        auto *als_inode_ptr = hero_ptr->field_20;
        {
            auto &self = swingers[0].field_94;
            self->set_visible(swingers[0].field_90->is_visible(), false);

            self = swingers[1].field_94;
            self->set_visible(swingers[1].field_90->is_visible(), false);
        }

        if (swing_inode_ptr->field_24) {
            if (os_developer_options::instance->get_int(mString{"SWING_DEBUG_TRAILS"}) > 0) {
                color32 v15{128, 255, 255, 64};
                constexpr float radius = 0.2f;
                auto *v9 = this->get_actor();
                auto v13 = v9->get_abs_position();

                add_debug_sphere(v13, radius, v15);
            }

            swing_inode_ptr->field_24 = false;

        } else {
            if (!swing_inode_ptr->field_54 &&
                als_inode_ptr->get_eta_of_combat_signal(static_cast<als::layer_types>(0)) <= 0.0f) {
                swing_inode_ptr->fire_new_web(true);
            }

            swing_inode_ptr->update_mode_swinging(a2);
            if (swing_inode_ptr->m_constraint <= minimum_web_length) {
                if (this->field_3C->crawl_is_eligible(auto_crawl_attach_id, false)) {
                    return static_cast<state_trans_messages>(73);
                }

                if (this->field_3C->oldcrawl_is_eligible(auto_crawl_attach_id, false)) {
                    return TRANS_SUCCESS_MSG;
                }
            }
        }

        return TRANS_TOTAL_MSGS;
    } else {
        ai::state_trans_messages(__fastcall * func)(void *, void *edx, Float a2) = CAST(func, 0x0047DDD0);
        return func(this, nullptr, a2);
    }
}

void swing_state::get_info_node_list(info_node_desc_list &a1)
{
    return a1.add_entry(info_node_descriptor{swing_inode::default_id, 0});
}

int swing_state::get_mash_sizeof()
{
    return 64;
}

static const string_hash cat_id_swing{int(to_hash("Swing"))};

VALIDATE_OFFSET(swing_inode, field_1C, 0x1C);
VALIDATE_OFFSET(swing_inode, field_54, 0x54);
VALIDATE_OFFSET(swing_inode, field_7C, 0x7C);
VALIDATE_SIZE(swing_inode, 0x94);

swing_inode::swing_inode()
{
    this->field_1C = false;
    this->field_24 = false;
    this->field_44 = false;
    this->field_54 = false;
}

static const string_hash state_id_sticky_hang_left{int(to_hash("SwingStickyHangLeft"))};
static const string_hash state_id_sticky_hang_right{int(to_hash("SwingStickyHangRight"))};

static const string_hash cat_id_sticky_hang_right_push_off{int(to_hash("Swing_Sticky_Hang_Right_Push_Off"))};
static const string_hash cat_id_sticky_hang_left_push_off{int(to_hash("Swing_Sticky_Hang_Left_Push_Off"))};

void swing_inode::update_mode_swinging(Float a2)
{
    TRACE("swing_inode::update_mode_swinging");

    if constexpr (0) {
        auto *v174 = this->field_20->field_24;
        auto *v173 = this->field_20->field_20;
        auto *v172 = this->field_20->field_28;
        this->field_84 += a2;
        if (this->field_38 == 1) {
            static constexpr auto sticky_exit_threshhold = 0.0099999998f;

            vector3d v171 = v174->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            if (v171.length2() <= sticky_exit_threshhold) {
                v172->set_velocity(ZEROVEC, false);

                auto v76 = sub_444A60(YVEC, this->field_64);
                auto v15 = -this->field_64;

                po v170;
                v170.set_po(v15, v76, this->field_58);
                auto *v17 = this->get_actor();
                v17->set_abs_po(v170);
                return;
            }

            auto v83 = v173->get_state_id(static_cast<als::layer_types>(0));

            auto v84 = (v83 == state_id_sticky_hang_left || v83 == state_id_sticky_hang_right);

            if (v84) {
                string_hash hash_id =
                    (this->field_7C < 0.0 ? cat_id_sticky_hang_right_push_off : cat_id_sticky_hang_left_push_off);

                v173->request_category_transition(hash_id, static_cast<als::layer_types>(0), true, false, false);

                auto *v83 = this->get_actor();

                auto &v7 = v83->get_abs_po();
                auto v8 = v7.get_z_facing();

                auto *v9 = v83->physical_ifc();
                v9->set_velocity(v8 * 5.0f, false);
            } else {
                v173->request_category_transition(cat_id_swing, static_cast<als::layer_types>(0), true, false, false);

                auto *v83 = this->get_actor();

                auto &v13 = v83->get_abs_po();
                auto v76 = v13.get_z_facing();
                auto *v14 = v83->physical_ifc();
                v14->set_velocity(v76, false);
            }

            this->field_40 = 0;
        }

        this->check_for_collision();

        vector3d norm_move_dir;
        vector3d up;
        this->compute_forward_and_up_directions(norm_move_dir, up);
        assert(!is_colinear(norm_move_dir, up));

        als::param_list v167{};
        v167.add_param(27, norm_move_dir);
        v167.add_param(24, up);

        assert(std::abs(dot(norm_move_dir, up)) < LARGE_EPSILON);

        auto vel = v172->get_velocity();
        auto speed = vel.length();

        vel.y = 0.0;
        auto lateral = vel.length();
        if (os_developer_options::instance->get_flag(mString{"SHOW_LOCOMOTION_INFO"})) {
            insertDebugString(7, mString{0, "speed   %.2f", speed}, color32{255, 255, 255, 255});

            insertDebugString(8, mString{0, "lateral %.2f", lateral}, color32{255, 255, 255, 255});
        }

        this->clip_web(a2);
        this->update_pendulums(a2);

        if (os_developer_options::instance->get_int(mString{"SWING_DEBUG_TRAILS"}) > 1) {
            auto *v21 = this->get_actor();
            auto abs_pos = v21->get_abs_position();

            static constexpr auto v78 = 0.039999999f;
            color32 v77{0, 255, 255, 255};
            auto v23 = norm_move_dir * 0.5;
            auto v76 = abs_pos + v23;
            add_debug_line(abs_pos, v76, v77, v78);

            v77 = color32{255, 0, 255, 255};
            v76 = abs_pos + up * 0.5f;
            add_debug_line(abs_pos, v76, v77, v78);

            v77 = color32{255, 255, 0, 255};
            auto v25 = vector3d::cross(up, norm_move_dir);
            v76 = abs_pos + v25 * 0.5f;
            add_debug_line(abs_pos, v76, v77, v78);
        }

        v173->set_desired_params(v167, static_cast<als::layer_types>(0));

        for (int i = 0; i < 2; ++i) {
            if (swingers[i].field_0.m_active) {
                auto *v28 = this->get_actor();

                auto *v29 = swingers[i].field_90;
                v29->set_abs_po(v28->get_abs_po());

                auto &v77 = swingers[i].field_40;
                auto abs_pos = swingers[i].field_3C->get_abs_position();
                auto tmp = abs_pos + v77;
                v77 = tmp;

                auto *v32 = this->get_actor();
                auto &v33 = v32->get_abs_po();
                tmp = v33.slow_xform(v77);

                auto *v83 = this->get_actor();
                vector3d v36 = v83->get_render_scale();
                tmp[0] *= v36[0];

                vector3d v38 = v83->get_render_scale();
                tmp[1] *= v38[1];

                vector3d v40 = v83->get_render_scale();
                tmp[2] *= v40[2];
                auto &a1 = tmp;

                auto *v42 = this->get_actor();
                auto &v43 = v42->get_abs_po();
                tmp = v43.slow_xform(a1);

                a1 = tmp;

                auto pivot_abs_pos_0 = swingers[i].field_0.get_pivot_abs_pos();
                auto v160 = pivot_abs_pos_0 - a1;
                auto v159 = -v160;
                v159.normalize();
                float v158 = swingers[i].field_4C * 4.0;
                if (v158 > 0.0) {
                    v158 = 0.0;
                }

                a1 = tmp;

                auto *v46 = swingers[i].field_90;
                v46->set_abs_control_pt(0, a1);
                if (swingers[i].field_4C >= 1.0) {
                    vector3d v155;
                    auto v53 = v159 * 0.0;
                    v155 = tmp + v53;

                    auto *v55 = swingers[i].field_90;
                    v55->max_length = -1.0f;
                } else {
                    vector3d v157{};
                    vector3d v156{};

                    float v83 = (fast_swing ? 6.f : 1.5f);

                    swingers[i].field_4C += (a2 * v83);
                    auto v47 = v159 * v158;
                    v157 = tmp + v47;

                    auto v49 = v160 * swingers[i].field_4C;
                    v156 = tmp + v49;

                    float v78 = v160.length() * swingers[i].field_4C;
                    auto *v52 = swingers[i].field_90;
                    v52->max_length = v78;
                }

                auto v154 = swingers[i].field_0.get_pivot_abs_pos();
                auto v153 = tmp - v154;
                auto v152 = v153[1];

                auto v56 = YVEC * v152;
                auto v57 = v56 * 0.75f;
                v154 += v57;

                auto *v58 = swingers[i].field_90;
                auto idx = v58->get_num_control_pts() - 1;

                auto *v59 = swingers[i].field_90;
                v59->set_abs_control_pt(idx, swingers[i].m_visual_point);
                auto v151 = tmp;
                auto v150 = swingers[i].m_visual_point;

                auto v61 = v151 * 3.0f + v150 * 2.0f;
                auto v149 = v61 / 5.0f;

                auto v62 = swingers[i].field_0.get_pivot_abs_pos();
                auto v63 = v151 - v62;
                auto v64 = v63.length();
                auto v82 = (swingers[i].field_0.get_constraint() - v64 - 0.5f) / 2.0f;

                auto len = swingers[i].field_0.get_constraint();
                auto v148 = (len - 9.0) / 9.0 + v82;
                if (v148 < LARGE_EPSILON) {
                    v148 = 0.0;
                }

                auto v78 = YVEC * v148;

                auto v66 = v151 + v149;
                auto v67 = v66 / 2.f;
                v78 = v67 - v78;

                auto *v68 = swingers[i].field_90;
                v68->set_abs_control_pt(1, v78);
                v78 = YVEC * v148;

                auto v69 = v150 + v149;
                auto v70 = v69 / 2.0f;
                v78 = v70 - v78;

                auto *v71 = swingers[i].field_90;
                v71->set_abs_control_pt(2, v78);

                {
                    auto *v72 = swingers[i].field_90;

                    if (v72->the_spline.need_rebuild) {
                        v72->the_spline.rebuild_helper();
                    }
                }

                auto *pltb = swingers[i].field_90;
                pltb->unforce_regions();

                auto *v74 = this->get_actor();
                auto *v144 = (entity *)v74->get_my_vhandle().get_volatile_ptr();

                pltb = swingers[i].field_90;
                pltb->force_regions(v144);
            }
        }

        this->field_88 = this->field_8C;
    } else {
        THISCALL(0x004779B0, this, a2);
    }
}

double generic_compute_swing_angle(const vector3d &forward, const vector3d &up, const vector3d &basis)
{
    assert(forward.is_normal());
    assert(up.is_normal());
    assert(basis.is_normal());

    auto v8 = dot(forward, basis);

    auto ang = bounded_acos(v8);

    if (dot(up, basis) >= 0.0) {
        ang = -ang;
    }

    return wrap_angle(ang);
}

float sub_44BCD0(entity_base *a1)
{
    vector3d v3 = -YVEC;

    auto &v2 = a1->get_abs_po();

    return generic_compute_swing_angle(v2.get_z_facing(), v2.get_y_facing(), v3);
}

vector3d swing_inode::get_desired_up_facing() const
{
    vector3d v8 = swingers[0].m_visual_point - this->field_20->field_28->get_abs_position();

    if (v8.length2() >= LARGE_EPSILON) {
        v8.normalize();
        return v8;
    }

    return YVEC;
}

bool swing_inode::is_eligible(string_hash a2, Float a3)
{
    if constexpr (1) {
        auto &v4 = this->field_8->field_50;

        if (v4.get_pb_int(loco_allow_swing_id) == 0) {
            return false;
        }

        if (!hero_inode::is_a_crawl_state(a2, true)) {
            auto *hero_inode_ptr = this->field_20;
            auto *physics_inode_ptr = hero_inode_ptr->field_28;
            auto *cntrl_inode_ptr = hero_inode_ptr->field_24;
            no_swing_timer += a3;

            const vector3d abs_pos = physics_inode_ptr->get_abs_position();
            const vector3d dir = physics_inode_ptr->get_abs_po().get_z_facing();

            po v28{};
            v28.set_rotate_y(0.0);

            constexpr float swing_length_mod = 1.0f;
            const auto v24 = this->get_swing_cur_max_anchor_dist() * swing_length_mod * 0.5f;
            vector3d a4 = dir * v24;

            vector3d a3a = v28.slow_xform(a4);

            a3a += abs_pos;

            const float a6 = (a2 == run_state::default_id ? 15.f : 10.f);

            this->field_3C = 1;

            game_button v31 = cntrl_inode_ptr->get_button(static_cast<controller_inode::eControllerButton>(11));

            game_button v30;
            bool cond = v31.is_pressed() &&
                        (v30 = cntrl_inode_ptr->get_button(static_cast<controller_inode::eControllerButton>(4)),
                         !v30.is_pressed());

            if (cond) {
                float sweet_spot_ground_level;

                this->find_best_anchor_point(abs_pos, a3a, dir, nullptr, a6, &sweet_spot_ground_level);
                assert(not_equal(sweet_spot_ground_level, -1e10f));

                if (something_to_swing_to_data.field_64) {
                    this->update_something_to_swing_to(a3);

                    return true;
                }
            }

            if (a2 == jump_state::default_id && this->field_20->field_50 == 9) {
                return true;
            }

            return false;
        }

        auto *v6 = this->field_20->field_24;

        game_button v32 = v6->get_button(static_cast<controller_inode::eControllerButton>(11));

        game_button v33;

        bool cond = v32.is_pressed() &&
                    (v33 = v6->get_button(static_cast<controller_inode::eControllerButton>(4)), !v33.is_pressed());

        return cond;
    } else {
        bool(__fastcall * func)(void *, void *edx, string_hash a2, Float a3) = CAST(func, 0x004882C0);
        return func(this, nullptr, a2, a3);
    }
}

void swing_inode::play_fire_web_sound()
{
    if constexpr (0) {
        static const string_hash swing_hash{int(to_hash("SWING"))};

        if (this->field_C->has_sound_and_pfx_ifc()) {
            web_sounds_manager::add_web_sound(this->field_C, something_to_swing_to_data.field_C, swing_hash);
        }
    } else {
        THISCALL(0x0045C7B0, this);
    }
}

void swing_inode::frame_advance(Float a2)
{
    this->m_swing_time += a2;
    if (!this->field_1C) {
        this->field_1C = true;
        this->init_swingers();
    }
}

void swap_swing_attach_prop_callback(event *the_event, entity_base_vhandle a2, void *params)
{
    [[maybe_unused]] auto *ent = a2.get_volatile_ptr();

    assert(the_event != nullptr && params != nullptr);
}

void swing_inode::_activate(ai::ai_core *a2)
{
    info_node::_activate(a2);

    this->field_20 = bit_cast<hero_inode *>(this->get_core()->get_info_node(hero_inode::default_id, true));
    this->field_1C = false;
    this->m_constraint = 0.0;

    auto *v3 = this->get_actor();
    this->field_34 = event_manager::add_callback(
        event::SWAP_PROP_A, v3->get_my_handle(), swap_swing_attach_prop_callback, this, false);

    this->field_38 = event_manager::add_callback(
        event::SWAP_PROP_B, v3->get_my_handle(), swap_swing_attach_prop_callback, this, false);
}

void swing_inode::_deactivate()
{
    //TRACE("ai::swing_inode::deactivate");

    this->cleanup_swingers();

    auto v5 = this->get_actor()->get_my_handle();
    event_manager::remove_callback(this->field_34, event::SWAP_PROP_A, v5);

    v5 = this->get_actor()->get_my_handle();
    event_manager::remove_callback(this->field_38, event::SWAP_PROP_B, v5);
}

float swing_inode::get_swing_cur_max_anchor_dist() const
{
    static constexpr float swing_sweet_upper = 25.f;
    static auto &swing_sweet_lower = var<float>(0x009591C0);
    static constexpr float swing_sweet_min_dist = 30.f;
    static constexpr float swing_max_anchor_dist = 72.f;

    auto v3 = this->get_actor()->get_abs_position()[1];
    if (v3 >= swing_sweet_upper) {
        return swing_max_anchor_dist;
    }

    if (v3 >= swing_sweet_lower) {
        return (v3 - swing_sweet_lower) / (swing_sweet_upper - swing_sweet_lower) *
                   (swing_max_anchor_dist - swing_sweet_min_dist) +
               swing_sweet_min_dist;
    }

    return swing_sweet_min_dist;
}

void swing_inode::init_swingers()
{
    if constexpr (0) {
        if (webline_texture == nullptr) {
            auto *slot = this->get_actor()->get_resource_context();
            auto *__old_context = resource_manager::push_resource_context(slot);

            auto *mem = mem_alloc(sizeof(PolytubeCustomMaterial));

            tlFixedString a1{"spideywebstring"};
            auto *Tex = nglGetTexture(a1);
            webline_texture = new (mem) PolytubeCustomMaterial{Tex, static_cast<nglBlendModeType>(2), 72};

            resource_manager::pop_resource_context();
            assert(resource_manager::get_resource_context() == __old_context);
        }

        for (int i = 0; i < 2; ++i) {
            auto *mem = mem_alloc(sizeof(web_polytube));

            auto entity_id = make_unique_entity_id();
            auto *v5 = new (mem) web_polytube{&swingers[i], entity_id, 0};

            g_world_ptr->ent_mgr.add_dynamic_instanced_entity(v5);

            auto *v7 = v5;
            v7->set_render_color(color32{255, 255, 254, 255});
            if (v7->tube_radius != 0.1f) {
                v7->tube_radius = 0.1;
                v7->field_78 = false;
            }

            v7->set_tiles_per_meter(1.5f);

            if (v7->num_sides != 2) {
                v7->num_sides = 2;
                v7->field_78 = false;
            }

            v7->field_104 = 30.0f * 1.5f;

            if (webline_texture != nullptr) {
                v7->set_material(webline_texture);
                v7->field_D0->m_blend_mode = static_cast<nglBlendModeType>(2);
            }

            v7->force_regions(this->get_actor());

            v7->set_force_start(true);

            v7->reserve_control_pts(4);

            for (int j = 0; j < 4; ++j) {
                v7->add_control_pt(ZEROVEC);
            }

            v7->build(10, static_cast<spline::eSplineType>(3));

            v7->set_visible(false, false);

            if (this->get_actor()->is_a_conglomerate()) {
                static string_hash bip01_r_prop{int(to_hash("BIP01_R_PROP_HAND"))};

                auto *conglom = bit_cast<conglomerate *>(this->get_actor());

                swingers[i].field_3C = bit_cast<entity *>(conglom->get_bone(bip01_r_prop, true));
            } else {
                swingers[i].field_3C = nullptr;
            }

            auto *v13 = mem_alloc(sizeof(polytube));

            auto v15 = make_unique_entity_id();
            swingers[i].field_94 = new (v13) polytube{v15, 0};

            g_world_ptr->ent_mgr.add_dynamic_instanced_entity(swingers[i].field_94);

            swingers[i].field_94->set_render_color(color32{254, 255, 255, 255});

            auto *v17 = swingers[i].field_94;
            if (v17->tube_radius != 0.1f) {
                v17->tube_radius = 0.1;
                v17->field_78 = false;
            }

            v17->set_tiles_per_meter(1.5f);

            if (v17->num_sides != 2) {
                v17->num_sides = 2;
                v17->field_78 = 0;
            }

            if (webline_texture != nullptr) {
                v17->set_material(webline_texture);
                v17->field_D0->m_blend_mode = static_cast<nglBlendModeType>(2);
            }

            auto *v18 = swingers[i].field_94;

            v18->set_force_start(true);

            v18->reserve_control_pts(10);

            for (int k = 0; k < 10; ++k) {
                v18->add_control_pt(ZEROVEC);
            }

            auto *v22 = v18;
            v22->build(5, static_cast<spline::eSplineType>(3));

            v22->set_visible(false, false);

            v18->set_parent(swingers[i].field_3C);
            v18->create_tentacle_info();

            [](ai_tentacle_info *self, bool a2) -> void {
                self->field_A8 = (a2 ? (self->field_A8 | 4u) : (self->field_A8 & (~4u)));
            }(v18->field_130, false);

            auto *v23 = new ai_tentacle_web_curly{v18->field_130};
            swingers[i].field_98 = v23;

            auto v26 = this->get_actor()->get_my_vhandle();
            auto v46 = swingers[i].field_3C;
            v23->setup({v26}, v46);

            swingers[i].field_94->field_130->push_engine(swingers[i].field_98);

            swingers[i].field_94->field_130->set_code_blend(1.0, 0.0);

            swingers[i].field_40 = ZEROVEC;
            swingers[i].field_4C = 0.0f;

            auto *v35 = mem_alloc(sizeof(marker));
            auto v36 = make_unique_entity_id();
            auto *v37 = new (v35) marker{v36, 0};

            swingers[i].field_0.sub_48AFB0(v37);

            auto *ent = bit_cast<entity *>(swingers[i].field_0.get_volatile_ptr());
            g_world_ptr->ent_mgr.add_dynamic_instanced_entity(ent);

            auto *v45 = this->get_actor()->physical_ifc();
            v45->set_pendulum(i, &swingers[i].field_0);
        }

        swinger_lr_swing_accel = 0;
        swinger_ud_swing_accel = 0;
    } else {
        THISCALL(0x004875F0, this);
    }
}

void swing_inode::cleanup_swingers()
{
    if constexpr (0) {
        if (this->field_1C) {
            for (auto i = 0u; i < 2; ++i) {
                if (swingers[i].field_94 != nullptr) {
                    g_world_ptr->ent_mgr.destroy_entity(swingers[i].field_94);
                }

                if (swingers[i].field_90 != nullptr) {
                    g_world_ptr->ent_mgr.destroy_entity(swingers[i].field_90);
                }

                auto *v3 = (entity *)swingers[i].field_0.get_volatile_ptr();
                if (v3 != nullptr) {
                    g_world_ptr->ent_mgr.destroy_entity(v3);
                }
            }

            this->field_1C = false;
        }
    } else {
        THISCALL(0x0044C320, this);
    }
}

void swing_inode::clip_web(Float a2)
{
    TRACE("ai::swing_inode::clip_web");

    if constexpr (0) {
        auto *v3 = this->field_20->field_28;

        for (swinger_t &v4 : swingers) {
            if (v4.field_0.m_active) {
                line_info v19{};

                v19.field_0 = v3->get_abs_position();
                auto v5 = v4.field_0.get_pivot_abs_pos();
                v19.field_C = v5;

                v19.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
                if (v19.collision) {
                    if (v19.hit_entity.get_volatile_ptr() != nullptr) {
                        event_manager::raise_event(event::COLLIDED_WITH_SWING_WEB, v19.hit_entity.field_0);
                    }

                    auto v7 = v3->get_abs_position();

                    auto v18 = v7 - v19.hit_pos;
                    auto v12 = v18.length2();
                    if (v12 >= swing_collision_break_length * swing_collision_break_length) {
                        if (v12 < swing_collision_stick_length * swing_collision_stick_length) {
                            auto *v13 = v4.field_0.get_volatile_ptr();
                            v13->clear_parent(true);

                            auto *v14 = static_cast<entity_base *>(v4.field_0.get_volatile_ptr());
                            v14->set_abs_position(v19.hit_pos);

                            v4.field_0.set_constraint(v18.length());
                            v4.m_visual_point = v4.field_0.get_pivot_abs_pos();
                        }
                    } else {
                        this->field_44 = true;
                    }
                }
            }
        }

    } else {
        THISCALL(0x0045AF30, this, a2);
    }
}

void swing_inode::fire_new_web(bool is_play_fire_web_sound)
{
    TRACE("swing_inode::fire_new_web:");

    if constexpr (1) {
        this->field_54 = true;

        vector3d new_velocity = swingers[0].field_40 + swingers[0].field_3C->get_abs_position();

        auto *v12 = swingers[0].field_90;

        v12->set_abs_control_pt(0, new_velocity);

        auto &pivot_abs_pos = swingers[0].field_0.get_pivot_abs_pos();

        auto idx = v12->get_num_control_pts() - 1;

        v12->set_abs_control_pt(idx, pivot_abs_pos);

        v12->max_length = 0.0;

        v12->build(10, static_cast<spline::eSplineType>(3));

        v12->set_visible(true, false);

        swingers[0].field_98->reset_curl();
        if (is_play_fire_web_sound) {
            this->play_fire_web_sound();
        }

        swingers[0].field_50 = -1.0;
        if (std::abs(this->m_anchor_angle) > magic_velocity_cancelling_angle_in_degrees) {
            auto *act = this->get_actor();

            auto *v15 = act->physical_ifc();
            v15->cancel_all_velocity();
            v15->set_velocity(-YVEC, false);
        }

        do_web_splat(something_to_swing_to_data.m_visual_point,
                     something_to_swing_to_data.field_18,
                     *local_collision::entfilter_reject_all);

    } else {
        THISCALL(0x00478820, this, is_play_fire_web_sound);
    }
}

[[nodiscard]] vector3d average_direction(Float a2, const vector3d &a3, const vector3d &a4, Float a5,
                                         direction_sampling_window &a7)
{
    TRACE("average_direction");

    vector3d a3a = a7.average(a5);
    a3a.normalize();

    static constexpr float flt_9591B8 = std::cos(DEG_TO_RAD(60.0)) * (-1.f);

    auto v17 = a3;
    auto v15 = dot(v17, a3a);
    if (flt_9591B8 > v15) {
        auto v10 = dot(a3a, YVEC);
        if (std::abs(v10) <= 0.99000001) {
            v17 = vector3d::cross(YVEC, a3a);
        } else {
            v17 = vector3d::cross(ZVEC, a3a);
        }
    }

    auto v14 = dot(v17, a4);
    if (v14 < -0.30000001) {
        v17 = a4;
    }

    a7.field_0[0].push_sample(a2, a3.x);
    a7.field_0[1].push_sample(a2, a3.y);
    a7.field_0[2].push_sample(a2, a3.z);

    v17 = a7.average(a5);
    v17.normalize();

    return v17;
}

vector3d *average_direction_original(vector3d *out, Float a2, const vector3d &a3, const vector3d &a4, Float a5,
                                     direction_sampling_window &a7)
{
    *out = average_direction(a2, a3, a4, a5, a7);
    return out;
}

static constexpr float triggered_reel_in_factor = 11.0f;

static constexpr float reel_in_factor = 10.f;

void swing_inode::update_pendulums(Float a2)
{
    TRACE("swing_inode::update_pendulums");

    if constexpr (1) {
        auto *the_hero_inode = this->field_20;
        auto *controller_ptr = the_hero_inode->field_24;
        auto *physics_inode_ptr = the_hero_inode->field_28;
        auto *als_inode_ptr = the_hero_inode->field_20;
        auto *v124 = physics_inode_ptr;

        vector3d v121 = physics_inode_ptr->get_abs_po().get_z_facing();

        vector3d v117 = controller_ptr->get_axis(static_cast<controller_inode::eControllerAxis>(2));

        vector3d v113 = this->get_desired_up_facing();
        v113.normalize();

        vector3d v10 = physics_inode_ptr->get_y_facing();

        v113 = average_direction(a2, v113, v10, 0.30000001, up_sampling_window());

        for (int i = 0; i < 2; ++i) {
            if (0 || i == 0) {
                auto *v11 = v124;
                auto v12 = v124->get_abs_position();
                vector3d v13 = swingers[0].field_0.get_pivot_abs_pos();

                auto v16 = v13 - v12;

                auto len = v16.length();
                this->m_constraint = len;

                auto div_result = v16 / len;

                auto abs_result = std::abs(dot(div_result, YVEC));

                vector3d v131 = -div_result;

                auto v18 = v11->get_velocity();
                auto v113 = sub_444A60(v18, v131);
                v113.normalize();

                if (os_developer_options::instance->get_int(mString{"SWING_DEBUG_TRAILS"}) > 0) {
                    auto v76 = v16 * (this->field_2C / this->m_constraint);
                    auto v9 = swingers[i].field_0.get_pivot_abs_pos();
                    auto v75 = v9 - v76;
                    add_debug_sphere(v75, 0.1f, color32{0, 0, 255, 128});

                    v75 = physics_inode_ptr->get_abs_position();
                    add_debug_sphere(v75, 0.2f, color32{128, 255, 255, 64});

                    auto length = swingers[i].field_0.get_constraint();
                    v76 = v16 * (length / this->m_constraint);
                    auto v11 = swingers[i].field_0.get_pivot_abs_pos();
                    v75 = v11 - v76;
                    add_debug_sphere(v75, 0.1f, color32{255, 0, 0, 255});
                }

                if (swingers[0].field_50 < 0.f) {
                    swingers[0].field_50 = this->m_constraint;
                }

                float v31;

                if (this->field_3C == 1) {
                    if (this->field_2C + LARGE_EPSILON > swingers[0].field_50) {
                        this->field_3C = 2;
                    }
                }

                if (this->field_3C == 1 && (swingers[0].field_50 > this->field_2C + LARGE_EPSILON)) {
                    static string_hash swing_ground_avoidance_height_multiplier_id{
                        int(to_hash("swing_ground_avoidance_height_multiplier"))};

                    auto *v21 = &this->field_8->field_50;

                    auto v22 = v21->get_pb_float(swing_ground_avoidance_height_multiplier_id);
                    auto *v23 = this->field_C;

                    auto v114 = v22 * this->field_2C;

                    vector3d v128 = YVEC * 15.0f;

                    vector3d v25 = v23->get_abs_position();

                    vector3d a2a = v25 - v128;

                    auto *v26 = this->field_C;

                    float v30;

                    vector3d point;
                    vector3d normal;
                    if (find_intersection(v26->get_abs_position(),
                                          a2a,
                                          *local_collision::entfilter_entity_no_capsules,
                                          *local_collision::obbfilter_lineseg_test,
                                          &point,
                                          &normal,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          false)) {
                        auto *v29 = this->get_actor();

                        vector3d abs_pos = v29->get_abs_position();

                        v30 = std::max(abs_pos.y - point.y, 0.0f);
                    } else {
                        v30 = v114 + EPSILON;
                    }

                    if (v30 <= 0.0f || v114 <= v30) {
                        if (v30 <= 0.0) {
                            v31 = 1.0;
                        }
                    } else {
                        v31 = 1.0f - v30 / v114;
                    }

                    auto v116 = swingers[0].field_50 - this->field_2C;

                    auto v32 = std::max(0.1f, v116);
                    auto v33 = std::min(v32, 20.0f);

                    auto v34 = (v33 * 4.5f) * v31 * a2;
                    swingers[0].field_0.set_constraint(swingers[0].field_0.get_constraint() - v34);
                    this->m_constraint = this->m_constraint - v34;
                    if (swingers[0].field_50 > v34) {
                        swingers[0].field_50 = swingers[0].field_50 - v34;
                    }
                } else if (swingers[0].field_50 <= 0.0f) {
                    auto tmp = swingers[0].m_visual_point * (a2 * 0.80000001f);
                    auto *v41 = static_cast<entity_base *>(swingers[0].field_0.get_volatile_ptr());

                    auto v43 = 1.f - (a2 * 0.80000001f);
                    vector3d abs_pos = v41->get_abs_position();

                    vector3d v136 = abs_pos * v43 + tmp;
                    auto *v45 = static_cast<entity_base *>(swingers[0].field_0.get_volatile_ptr());
                    entity_set_abs_position(v45, v136);
                } else if (abs_result > 0.85000002f && swingers[0].field_50 * 1.2f > this->m_constraint) {
                    swingers[0].field_50 = 0.0;
                }

                if (swingers[0].field_50 > len + 0.1f) {
                    swingers[0].field_50 = (swingers[0].field_50 + len) * 0.5f;
                    swingers[0].field_0.set_constraint(swingers[0].field_50);
                }

                static const string_hash swing_entry_smoothing_id{int(to_hash("swing_entry_smoothing"))};

                auto &v35 = this->field_8->field_50;
                auto swing_entry_smoothing = v35.get_pb_float(swing_entry_smoothing_id);
                auto v111 = (1.0f - abs_result) * swing_entry_smoothing * 50.0f;
                swingers[0].field_0.set_constraint_lenience(v111);

                auto v110 = 0.0f;

                static const string_hash reel_buttons_delay_id{int(to_hash("reel_buttons_delay"))};
                auto &v37 = this->field_8->field_50;
                auto reel_buttons_delay = v37.get_optional_pb_float(reel_buttons_delay_id, v110, nullptr);

                game_button v38 = controller_ptr->get_button(static_cast<controller_inode::eControllerButton>(14));

                bool v39 = v38.is_pressed();
                if (v39) {
                    game_button v46 = controller_ptr->get_button(static_cast<controller_inode::eControllerButton>(14));
                    auto v47 = (v46.is_flagged(0x20) ? 0.0f : v46.field_1C);
                    auto v48 = v47 < reel_buttons_delay;

                    auto v49 = v48 ? triggered_reel_in_factor : reel_in_factor;
                    auto v50 = v49 * a2 * 1.1f;
                    swingers[0].field_0.m_constraint += v50;
                    if (swingers[0].field_50 > 0.0f) {
                        swingers[0].field_50 += v50;
                    }
                }

                game_button v51 = controller_ptr->get_button(static_cast<controller_inode::eControllerButton>(10));

                static string_hash cat_id_swing_reel_up{int(to_hash("Swing_Reel_Up"))};

                bool v53 = false;
                if (v51.is_pressed()) {
                    if (swingers[0].field_0.get_constraint() > minimum_web_length) {
                        v53 = true;
                    }
                }

                if (v53) {
                    auto v54 = this->field_20->field_34->is_in_master_mode();

                    game_button v55 = controller_ptr->get_button(static_cast<controller_inode::eControllerButton>(10));

                    float v56 = (v55.is_flagged(0x20) ? 0.0f : v55.field_1C);

                    auto v112 = v56 < reel_buttons_delay;
                    if (v112) {
                        auto v57 = triggered_reel_in_factor * a2;
                        swingers[0].field_0.m_constraint = swingers[0].field_0.m_constraint - v57;
                        if (swingers[0].field_50 > 0.0f) {
                            swingers[0].field_50 = swingers[0].field_50 - v57;
                        }

                        if (!v54) {
                            als_inode_ptr->request_category_transition(
                                cat_id_swing_reel_up, static_cast<als::layer_types>(0), true, false, false);
                            this->field_40 = 3;
                        }

                    } else {
                        auto v58 = 10.0f * a2;
                        swingers[0].field_0.m_constraint = swingers[0].field_0.m_constraint - v58;
                        if (swingers[0].field_50 > 0.0f) {
                            swingers[0].field_50 = swingers[0].field_50 - v58;
                        }

                        if (!v54) {
                            als_inode_ptr->request_category_transition(
                                cat_id_swing_reel_up, static_cast<als::layer_types>(0), true, false, false);
                            this->field_40 = 3;
                        }
                    }
                } else if (this->field_40 == 3) {
                    als_inode_ptr->request_category_transition(
                        cat_id_swing, static_cast<als::layer_types>(0), true, false, false);
                }

                if (minimum_web_length > this->m_constraint) {
                    this->m_constraint = minimum_web_length;
                    swingers[0].field_0.set_constraint(this->m_constraint);
                }
            }
        }

        auto *v60 = v124;
        auto v110 = 1.0f;
        auto v126 = v124->get_velocity();
        this->field_8C = sub_44BCD0(this->field_C);
        if (this->field_88 < 0.0f) {
            this->field_88 = this->field_8C;
        }

        auto v123 = v126.length();

        float v62;
        if (v123 < 15.0) {
            auto v171 = std::abs(270.0f - (this->field_8C * (180.0 / 3.1415927)));
            if (v171 >= 5.0) {
                v62 = (v171 <= 45.0f ? 1.0f - (v171 / 45.0f) : 1.0f);
            } else {
                v62 = 1.0;
            }

            v62 *= v62;
            v62 = physics_inode_ptr->get_abs_po().get_y_facing().y;
            if (v62 < EPSILON) {
                v62 = 0.0f;
            }
        }

        static constexpr auto swing_horiz_thrust = 20.0f;

        auto v114 = v62 * swing_horiz_thrust;
        if (v117.length2() <= LARGE_EPSILON) {
            if (this->field_3C != 1) {
                float v110 = 0.0;
                float v111 = 0.0;

                constexpr auto initial_boost_time = 0.2f;
                constexpr auto initial_boost_factor = 0.60000002f;
                v117 = v121 * sub_48C0C0(this->m_swing_time, v111, initial_boost_time, initial_boost_factor, v110);
            }
        } else {
            v117.y = 0.0;
            v117.normalize();
        }

        if (v117.length2() > LARGE_EPSILON) {
            auto v113 = physics_inode_ptr->get_abs_po().get_y_facing();
            auto v68 = v113 * swingers[0].field_0.get_constraint();
            v68.y = 0.0f;

            if (v68.length2() >= EPSILON) {
                v68.normalize();
            } else {
                v68 = v113;
            }

            float v110;
            auto v75 = dot(v117, v68);

            static constexpr auto mag_of_thrust_b = 0.5f;
            static constexpr auto mag_of_thrust_f = 1.5f;
            static constexpr auto mag_of_thrust_lr = 1.0f;

            float v108;
            if (v75 <= 0.0f) {
                v110 = -1.0;
                v108 = mag_of_thrust_b;
            } else {
                v110 = 1.0;
                v108 = mag_of_thrust_f;
            }

            auto v109 = sub_48C0C0(v75, 0.0f, v110, mag_of_thrust_lr, v108);

            sp_log("angle: %.1f, thrust: %.1f", v75, v114 * v109);

            auto v76 = sub_48C0C0(this->field_2C, 15.0f, 60.0f, 1.0f, 3.5f);
            v109 *= v76;
            vector3d v121 = (((v117 * a2) * v114) * v109);

            auto &v77 = v60->get_abs_po();
            v121 = sub_444A60(v121, v77.get_y_facing());

            physics_inode_ptr->apply_force_increment(
                v121, static_cast<physical_interface::force_type>(1), IGNORE_LOC, 0);
        }

        auto v122 = this->field_C->get_velocity();
        auto &v81 = physics_inode_ptr->get_abs_po();
        auto v82 = average_direction(a2, v122, v81.get_z_facing(), 0.30000001f, heading_sampling_window());
        v122 = v82;
        auto v84 = v122;
        v84.y = 0.0;
        static constexpr auto initial_swing_time = 0.5f;
        if (this->m_swing_time <= initial_swing_time) {
            s_initial_heading = v84;
        }

        v110 = dot(v122, s_initial_heading);
        heading_change_sampling_window().push_sample(a2, v110);
        als::param_list v120{};

        als::param v108;
        v108.field_4 = this->m_swing_time;
        v108.field_0 = 17;
        v120.add_param(v108);

        v108.field_4 = this->field_8C * (180.0 / 3.1415927);
        v108.field_0 = 10;
        v120.add_param(v108);

        v108.field_4 = heading_change_sampling_window().average(0.30000001f);
        v108.field_0 = 12;
        v120.add_param(v108);

        v108.field_4 = v123;
        v108.field_0 = 0;
        v120.add_param(v108);

        als_inode_ptr->set_desired_params(v120, static_cast<als::layer_types>(0));
        if (v122.length2() < EPSILON) {
            v122 = physics_inode_ptr->get_abs_po().get_z_facing();
        }

        auto v87 = swingers[0].field_0.get_pivot_abs_pos();
        auto v88 = physics_inode_ptr->get_abs_position();
        auto v89 = v88 - v87;
        v89.normalize();

        auto &v93 = this->field_8->field_50;

        static string_hash boost_force_id{int(to_hash("boost_force"))};
        auto boost_force = v93.get_pb_float(boost_force_id);

        static string_hash boost_cooldown_id{int(to_hash("boost_cooldown"))};
        auto boost_cooldown = v93.get_pb_float(boost_cooldown_id);

        static string_hash boost_min_angle_id{int(to_hash("boost_min_angle"))};
        auto boost_min_angle = v93.get_pb_float(boost_min_angle_id);

        static string_hash boost_max_angle_id{int(to_hash("boost_max_angle"))};
        auto boost_max_angle = v93.get_pb_float(boost_max_angle_id);
        boost_min_angle = boost_min_angle * (3.1415927 / 180.0);
        v114 = boost_max_angle * (3.1415927 / 180.0);

        auto v98 = dot(v89, -YVEC);
        if (v98 > 1.f) {
            v98 = 1.0f;
        }

        if (v98 < -1.f) {
            v98 = -1.f;
        }

        v98 = bounded_acos(v98);

        float &boost_cooldown_timer = var<float>(0x00958100);
        auto v102 = boost_cooldown_timer - a2;
        boost_cooldown_timer = v102;
        if (v102 < 0.0f) {
            boost_cooldown_timer = 0.0;
        }

        if (v98 > v114) {
            boost_cooldown_timer = 0.0;
        } else {
            auto v103 = controller_ptr->get_button(static_cast<controller_inode::eControllerButton>(9));
            bool v104 = false;
            if (v103.is_triggered() && boost_cooldown_timer <= 0.0f) {
                v104 = true;
            }

            if (v104) {
                this->field_C->has_sound_and_pfx_ifc();
                auto v149 = boost_force;
                if (v98 > boost_min_angle && v114 > boost_min_angle) {
                    auto v105 = (v98 - boost_min_angle) / (v114 - boost_min_angle);
                    v105 *= v105;
                    v105 = 1.0f - v105;
                    v149 = v105 * boost_force;
                }

                auto v128 = v122 * v149;
                v124->apply_force_increment(v128, static_cast<physical_interface::force_type>(1), IGNORE_LOC, 0);
                boost_cooldown_timer = boost_cooldown;
            }
        }
    } else {
        THISCALL(0x0045B110, this, a2);
    }
}

void swing_inode::check_for_collision()
{
    TRACE("swing_inode::check_for_collision");

    if constexpr (1) {
        auto *v2 = this->field_20;
        auto *v3 = v2->field_20;
        auto *v4 = v2->field_28;
        if (v4->get_collided_last_frame() && v3->get_category_id(static_cast<als::layer_types>(0)) == cat_id_swing) {
            this->field_40 = 0;
            float hit_angle = -1.0f;
            this->field_84 = 0.0;

            auto last_collision_normal = v4->get_last_collision_normal();
            auto &abs_po = v4->get_abs_po();
            hit_angle = dot(last_collision_normal, abs_po.get_z_facing());

            auto &v6 = v4->get_abs_po();
            auto v38 = dot(v6.get_x_facing(), last_collision_normal);
            hit_angle = sub_48A720(v38, hit_angle);
            hit_angle = wrap_angle(hit_angle);
            hit_angle = (180.0 / 3.1415927) * hit_angle;

            assert(hit_angle >= -0.1f);
            assert(hit_angle <= 360.1f);

            sp_log("hit_angle: %.1f", hit_angle);

            static constexpr auto hard_coded_sticky_hang_distance = 0.57999998f;

            auto *v9 = this->field_C;
            auto v11 = v9->physical_ifc()->field_68;
            this->field_58 = v11 + last_collision_normal * hard_coded_sticky_hang_distance;
            this->field_70 = last_collision_normal;
            this->field_64 =
                (is_colinear(last_collision_normal, YVEC, 0.0099999998) ? v4->get_abs_po().get_z_facing()
                                                                        : vector3d::cross(last_collision_normal, YVEC));

            auto &v21 = v4->get_abs_po();
            this->field_7C = dot(this->field_64, v21.get_z_facing());
            if (this->field_7C >= 0.0f) {
                this->field_64 = -this->field_64;
            }

            auto abs_position = v4->get_abs_position();
            vector2d v27{abs_position[0], abs_position[2]};

            auto pivot_abs_pos = swingers[0].field_0.get_pivot_abs_pos();
            vector2d v29{pivot_abs_pos[0], pivot_abs_pos[2]};

            auto v39 = (v29 - v27).length();
            auto velocity = v4->get_velocity();
            auto v32 = velocity.length();
            if (v39 >= 1.2f || v32 >= 10.0f) {
                if (v32 > 10.0f) {
                    static const string_hash cat_id_swing_bounce_right{int(to_hash("Swing_Bounce_Right"))};
                    static const string_hash cat_id_swing_bounce_left{int(to_hash("Swing_Bounce_Left"))};

                    if (hit_angle < 225.0f || hit_angle > 315.0f) {
                        if (hit_angle >= 45.0f || hit_angle <= 135.0f) {
                            v3->request_category_transition(
                                cat_id_swing_bounce_right, static_cast<als::layer_types>(0), true, false, false);
                            this->field_40 = 2;
                        }
                    } else {
                        v3->request_category_transition(
                            cat_id_swing_bounce_left, static_cast<als::layer_types>(0), true, false, false);
                        this->field_40 = 2;
                    }
                }
            } else {
                static const string_hash cat_id_swing_sticky_hang_left{int(to_hash("Swing_Sticky_Hang_Left"))};
                static const string_hash cat_id_swing_sticky_hang_right{int(to_hash("Swing_Sticky_Hang_Right"))};

                string_hash v40 =
                    (this->field_7C < 0.0f ? cat_id_swing_sticky_hang_right : cat_id_swing_sticky_hang_left);

                v3->request_category_transition(v40, static_cast<als::layer_types>(0), true, false, false);

                this->field_40 = 1;
            }
        }
    } else {
        THISCALL(0x0045C3C0, this);
    }
}

void sub_44C3B0(line_info *a1)
{
    if constexpr (1) {
        auto v1 = a1->hit_norm * 0.1f;

        auto &v3 = g_world_ptr->field_1B0;

        vector3d arg4 = v1 + a1->hit_pos;

        auto *v4 = a1->hit_entity.get_volatile_ptr();
        v3.spawn(false, arg4, a1->hit_norm, nullptr, nullptr, v4, ZEROVEC, false, true);

    } else {
        CDECL_CALL(0x0044C3B0, a1);
    }
}

void sub_44C430(line_info *a1)
{
    if constexpr (1) {
        vector3d v5 = a1->hit_norm * 0.1f + a1->hit_pos;
        auto *v4 = a1->hit_entity.get_volatile_ptr();

        auto *v3 = &g_world_ptr->field_1F0;
        v3->spawn(false, v5, a1->hit_norm, nullptr, nullptr, v4, ZEROVEC, false, true);

    } else {
        CDECL_CALL(0x0044C430, a1);
    }
}

void swing_inode::do_web_splat(vector3d target_point, vector3d target_normal,
                               const local_collision::entfilter_base &entfilter_arg)
{
    TRACE("swing_inode::do_web_splat");

    if constexpr (1) {
        vector3d array_vectors[6];

        array_vectors[0] = target_point + target_normal;
        array_vectors[1] = target_point - target_normal;

        static constexpr vector3d halfUp{0.0, 0.5, 0.0};

        array_vectors[2] = array_vectors[0] + halfUp;
        array_vectors[3] = array_vectors[1] - halfUp;
        array_vectors[4] = array_vectors[0] - halfUp;
        array_vectors[5] = array_vectors[1] + halfUp;

        line_info line{};

        static bool &byte_958044 = var<bool>(0x00958044);

        for (int i = 0; i < 6; i += 2) {
            {
                line.clear();
                line.field_0 = array_vectors[i];
                line.field_C = array_vectors[i + 1];
            }

            if (byte_958044) {
                line.render(i / 2, false);
            }

            if (line.check_collision(entfilter_arg, *local_collision::obbfilter_lineseg_test, nullptr)) {
                vector3d axis;

                if (line.hit_norm == YVEC) {
                    axis = XVEC;
                } else {
                    axis = YVEC;
                }

                vector3d local_vec3 = vector3d::cross(line.hit_norm, YVEC);

                auto float_from_world = g_world_ptr->field_1B0.field_2C * 0.5f;

                auto hit_pos = line.hit_pos;

                auto local_vec3_1 = local_vec3 * float_from_world;
                const auto axis1 = axis * float_from_world;

                auto local_vec3_4 = (hit_pos + axis1) - local_vec3_1;

                auto a2 = (hit_pos - axis1) - local_vec3_1;

                auto v87 = (hit_pos + axis1) + local_vec3_1;

                auto v86 = (hit_pos - axis1) + local_vec3_1;

                const auto local_normal = target_normal * 0.2;

                line_info array_lines[4];
                array_lines[3] = line_info{local_vec3_4 + local_normal, local_vec3_4 - local_normal};
                array_lines[2] = line_info{a2 + local_normal, a2 - local_normal};
                array_lines[1] = line_info{v87 + local_normal, v87 - local_normal};
                array_lines[0] = line_info{v86 + local_normal, v86 - local_normal};

                if (byte_958044) {
                    array_lines[3].render(3, false);
                    array_lines[2].render(4, false);
                    array_lines[1].render(5, false);
                    array_lines[0].render(6, false);
                }

                bool cond[4]{};

                cond[3] =
                    array_lines[3].check_collision(entfilter_arg, *local_collision::obbfilter_lineseg_test, nullptr);
                cond[2] =
                    array_lines[2].check_collision(entfilter_arg, *local_collision::obbfilter_lineseg_test, nullptr);
                cond[1] =
                    array_lines[1].check_collision(entfilter_arg, *local_collision::obbfilter_lineseg_test, nullptr);
                cond[0] =
                    array_lines[0].check_collision(entfilter_arg, *local_collision::obbfilter_lineseg_test, nullptr);

                static constexpr auto flt_87EEE4 = 0.050000001f;

                if (!cond[1] || !cond[0] || cond[3] || cond[2]) {
                    if (cond[1] || cond[0] || !cond[3] || !cond[2]) {
                        if (cond[1] && cond[0] && cond[3] && cond[2]) {
                            sub_44C3B0(&line);
                        }
                    } else {
                        auto tmp = local_vec3 * flt_87EEE4;
                        line.hit_pos = line.hit_pos - tmp;

                        tmp = line.hit_norm * flt_87EEE4;
                        line.hit_pos = line.hit_pos + tmp;

                        if (line.hit_norm != YVEC) {
                            line.hit_norm = vector3d::cross(line.hit_norm, YVEC);
                            sub_44C430(&line);
                        }
                    }

                } else {
                    auto tmp = local_vec3 * flt_87EEE4;
                    line.hit_pos = line.hit_pos - tmp;

                    tmp = line.hit_norm * flt_87EEE4;
                    line.hit_pos = line.hit_pos - tmp;

                    sub_44C430(&line);
                }

                break;
            }
        }

    } else {
        CDECL_CALL(0x0046B180, target_point, target_normal, &entfilter_arg);
    }
}

void swing_inode::compute_forward_and_up_directions(vector3d &forward, vector3d &up)
{
    TRACE("ai::swing_inode::compute_forward_and_up_directions");

    if constexpr (1) {
        physical_interface *v4 = this->get_actor()->physical_ifc();

        forward = v4->get_velocity();

        up = this->get_desired_up_facing();

        this->field_48 = up;

        forward = sub_444A60(forward, up);

        if (forward.length2() < EPSILON) {
            auto *v13 = this->get_actor();

            forward = v13->get_abs_po().get_z_facing();

            forward = sub_444A60(forward, up);
        }

        forward.normalize();

        assert(up.is_normal());
        assert(forward.is_normal());

        assert(std::abs(dot(forward, up)) < LARGE_EPSILON);

        float slerp;
        if (this->m_swing_time >= 1.0f) {
            slerp = 0.5f;
        } else {
            float v16 = this->m_swing_time / 1.0f;
            slerp = v16 * v16;
        }

        po &v19 = this->get_actor()->get_abs_po();

        vector3d v23 = v19.get_y_facing();
        vector3d v24 = v19.get_z_facing();

        vector3d a14;
        reorient_vectors(v24, v23, forward, up, forward, a14, slerp);

        forward = sub_444A60(forward, up);
        forward.normalize();

        if (os_developer_options::instance->get_flag(mString{"SHOW_LOCOMOTION_INFO"})) {
            auto v46 = forward * 2.0f;
            auto *v35 = this->get_actor();
            v46 += v35->get_abs_position();

            auto v39 = v35->get_abs_position();
            line_info v75{v39, v46};
            v75.render(25, false);
        }

        if (os_developer_options::instance->get_flag(mString{"SHOW_LOCOMOTION_INFO"})) {
            mString v47{0, "slerp %.2f", slerp};

            color32 v40{255, 255, 255, 255};
            insertDebugString(9, v47, v40);
        }
    } else {
        THISCALL(0x0044BEB0, this, &forward, &up);
    }

    assert(!is_colinear(forward, up));
}

float swing_inode::compute_ground_level_at_sweet_spot_position(const vector3d &a1, Float a2, const region *reg)
{
    assert(reg != nullptr);

    if constexpr (1) {
        vector3d v1 = a1 - YVEC * a2;

        vector3d a5;
        vector3d a6;

        float result;
        if (find_intersection(a1,
                              v1,
                              *local_collision::entfilter_entity_no_capsules,
                              *local_collision::obbfilter_lineseg_test,
                              &a5,
                              &a6,
                              nullptr,
                              nullptr,
                              nullptr,
                              false)) {
            result = a5[1];
        } else {
            result = reg->get_ground_level();
        }
        return result;

    } else {
        float(__fastcall * func)(void *, void *, const vector3d *a1, Float a2, const region *reg) =
            CAST(func, 0x0044C280);
        return func(this, nullptr, &a1, a2, reg);
    }
}

static constexpr float min_dynamic_sweet_angle = (3.1415927 / 180.0) * 40.009998;

static constexpr float max_dynamic_sweet_angle = (3.1415927 / 180.0) * 50.009998;

static constexpr float min_dynamic_sweet_length = 35.0;

static constexpr float max_dynamic_sweet_length = 55.0;

void swing_inode::compute_dynamic_sweet_spot_params(float *result_angle, float *result_length,
                                                    float *result_ground_level, vector3d *result_direction)
{
    assert(result_angle != nullptr && result_length != nullptr && result_ground_level != nullptr);

    if constexpr (1) {
        auto *cntrl_inode_ptr = this->field_20->field_24;
        auto *cam_ptr = g_world_ptr->get_chase_cam_ptr(0);

        vector3d a2;
        vector3d a3;
        cam_ptr->get_look_and_up(&a2, &a3);

        a2.normalize();

        vector3d dir = cntrl_inode_ptr->get_axis(static_cast<controller_inode::eControllerAxis>(0));
        if (dir.length() < EPSILON) {
            dir = this->get_actor()->get_abs_po().get_z_facing();

        } else {
            dir = cntrl_inode_ptr->get_axis(static_cast<controller_inode::eControllerAxis>(0));
        }

        auto a1 = dir;
        bool v19 = false;
        auto v20 = dir.length();
        if (v20 <= LARGE_EPSILON) {
            v19 = true;
        } else {
            a1 = a1 / v20;
        }

        auto *v22 = this->get_actor()->physical_ifc();
        auto vel = v22->get_velocity();
        assert(vel.is_valid());

        auto xz_velocity = vel.xz_norm();
        assert(vector3d(0.0f, 0.0f, xz_velocity).is_valid());

        float cos_alpha = dot(a1, a2);
        assert(std::abs(cos_alpha) < 1.0f + EPSILON);

        cos_alpha = std::clamp(cos_alpha, -1.0f, 1.0f);

        float alpha = bounded_acos(cos_alpha);
        if (alpha > half_PI) {
            alpha = PI - alpha;
        }

        alpha = alpha / half_PI;

        assert(alpha > -EPSILON && alpha < 1.0f + EPSILON);

        alpha = std::clamp(alpha, 0.0f, 1.0f);

        auto v30 = 1.f - (1.f - alpha) * (1.f - alpha);

        static constexpr auto sweet_spot_overdrive_velocity = 20.01f;
        assert(sweet_spot_overdrive_velocity > LARGE_EPSILON);

        float v31 = xz_velocity / sweet_spot_overdrive_velocity;
        if (v31 > 1.f) {
            v31 = 1.f;
        }

        auto v52 = 1.f - v30;
        float current_length =
            ((1.f - v31) * max_dynamic_sweet_length + 55.f * v31) * v52 + min_dynamic_sweet_length * v30;
        auto angle = v52 * min_dynamic_sweet_angle + max_dynamic_sweet_angle * v30;

        assert(vector3d(0.0f, 0.0f, current_length).is_valid());

        auto saturate_and_asin = [](float a1) -> float {
            if (a1 < -1.f) {
                a1 = -1.f;
            } else if (a1 > 1.f) {
                a1 = 1.f;
            }

            return asin(a1);
        };

        if (v19) {
            constexpr float sweet_spot_distance = 30.01f;

            current_length = sweet_spot_distance;

            assert(vector3d(0.0f, 0.0f, current_length).is_valid());

            static const float sweet_spot_incline_angle_sin = std::sin((3.1415927 / 180.0) * 40.009998);

            assert(sweet_spot_incline_angle_sin >= 0.0f && sweet_spot_incline_angle_sin <= 1.0f);

            angle = saturate_and_asin(sweet_spot_incline_angle_sin);
            if (angle < 0.f) {
                angle = 0.0;
            }
        }

        auto *v33 = this->get_actor();

        auto v56 = v33->get_abs_position().y + 2.f;
        for (int i = 0; i < 2; ++i) {
            assert(vector3d(0.0f, 0.0f, current_length).is_valid());

            float s, c;
            fast_sin_cos_approx(angle, &s, &c);

            swing_anchor_finder anchor_finder{
                c, s, current_length, min_swing_web_length_squared, max_swing_web_length_squared, sweet_cone_angle_cos};

            auto v61 = s * current_length + v56 - current_length;

            auto *v35 = this->get_actor();

            vector3d sweet_spot{};
            anchor_finder.find_sweet_spot(v35, dir, ZEROVEC, &sweet_spot);

            sweet_spot.y = v56;

            if (v35->get_primary_region() != nullptr) {
                auto *reg = v35->get_primary_region();
                this->compute_ground_level_at_sweet_spot_position(a1, current_length, reg);
            }

            *result_ground_level = 0.0;
            if (v61 >= 1.5f) {
                break;
            }

            if (i != 0) {
                assert(i == 1);

                assert(current_length > LARGE_EPSILON);

                float v42 = (current_length + 1.5f - v56) / current_length;

                v42 = std::clamp(v42, -1.0f, 1.0f);

                angle = saturate_and_asin(v42);

                angle = std::clamp(angle, min_dynamic_sweet_angle, max_dynamic_sweet_angle);

                break;
            }

            current_length = (1.5f - v56) / (s - 1.f);
            current_length *= 0.69999999f;
            current_length = std::clamp(current_length, min_dynamic_sweet_length, max_dynamic_sweet_length);
        }

        *result_angle = angle;
        *result_length = current_length;
        *result_direction = dir;
    } else {
        THISCALL(0x0046AB90, this, result_angle, result_length, result_ground_level, result_direction);
    }
}

void swing_inode::setup_new_web()
{
    TRACE("ai::swing_inode::setup_new_web");

    if constexpr (0) {
        static constexpr float max_swing_length_mod = 3.0;

        this->field_54 = false;
        this->field_44 = false;
        this->field_84 = 100.0;
        constexpr float swing_length_mod = 1.0;
        swinger_lr_swing_accel = 0;
        swinger_ud_swing_accel = 0;
        this->field_8C = -1.0;
        this->field_88 = -1.0;
        fast_swing = false;
        this->field_40 = 0;
        constexpr bool v6 = (max_swing_length_mod * 0.5 + 0.5 <= swing_length_mod);

        this->field_2C = something_to_swing_to_data.m_target_length;
        physical_interface *v8 = this->get_actor()->physical_ifc();
        v8->set_gravity(true);

        auto *ent = swingers[0].field_0.get_volatile_ptr();
        ent->clear_parent(true);

        if (ent->is_rel_po_dirty()) {
            ent->compute_rel_po_from_model();
        }

        ent->my_rel_po->set_position(something_to_swing_to_data.field_C);

        ent->dirty_family(false);

        if (ent->is_conglom_member() || ent->is_a_conglomerate()) {
            ent->dirty_model_po_family();
        }

        ent->po_changed();

        swingers[0].m_visual_point = something_to_swing_to_data.m_visual_point;
        swingers[0].field_60 = something_to_swing_to_data.field_C;

        if (something_to_swing_to_data.field_8.get_volatile_ptr() &&
            something_to_swing_to_data.field_8.get_volatile_ptr()) {
            entity_base *v12 = something_to_swing_to_data.field_8.get_volatile_ptr();
            entity_base *v10 = swingers[0].field_0.get_volatile_ptr();
            entity_set_abs_parent(v10, v12);
        }

        vector3d pivot_abs_pos = swingers[0].field_0.get_pivot_abs_pos();

        const float distance = (something_to_swing_to_data.field_28 - pivot_abs_pos).length();

        this->m_constraint = distance;

        if (this->m_constraint > this->field_2C) {
            float v45 = 0.75 * this->m_constraint + this->field_2C * 0.25;
            float v22 = this->m_constraint - start_speed;

            this->m_constraint = std::max(v22, v45);
        }

        swingers[0].field_0.m_constraint = this->m_constraint;
        if (swingers[0].field_0.m_constraint >= 3.0) {
            swingers[0].field_0.m_active = true;
            something_to_swing_to_data.goal_constraint = swingers[0].field_0.get_constraint();
            swingers[0].field_38 = swingers[0].field_0.get_constraint();
        }

        fast_swing = v6;

        swingers[0].field_4C = 0;
        entity_base *v25 = this->get_actor();

        auto *_polytube = swingers[0].field_90;

        //sp_log("Address of vtbl = 0x%08X", get_vtbl(_polytube));

        entity_set_abs_po(_polytube, v25->get_abs_po());
        entity_base *v26 = swingers[0].field_3C;

        vector3d a2a = swingers[0].field_40 + v26->get_abs_position();

        auto *v30 = swingers[0].field_90;

        v30->set_abs_control_pt(0, a2a);

        vector3d v48 = swingers[0].field_0.get_pivot_abs_pos();

        auto v36 = v30->get_num_control_pts() - 1;

        v30->set_abs_control_pt(v36, v48);

        v30->set_max_length(0.0f);

        swingers[0].field_50 = -1.0f;

        this->get_actor()->physical_ifc()->m_gravity_multiplier = 5.0f;

        vector3d norm_move_dir;
        vector3d up;
        this->compute_forward_and_up_directions(norm_move_dir, up);
        ai::als_inode *v40 = this->field_20->field_20;

        assert(norm_move_dir.is_normal());
        assert(norm_move_dir.is_valid());
        assert(up.is_normal());
        assert(up.is_valid());
        assert(!is_colinear(norm_move_dir, up));

        als::param_list v44;
        v44.add_param(27u, norm_move_dir);
        v44.add_param(24u, up);

        //assert(std::abs(dot(norm_move_dir, up)) < LARGE_EPSILON);

        auto *v41 = v40->get_als_layer(static_cast<als::layer_types>(0));

        v41->set_desired_params(v44);
    } else {
        THISCALL(0x004783B0, this);
    }
}

void swing_inode::propose_something_to_swing_to(vector3d a1, entity_base *a4, vector3d a5, vector3d normal, Float a11,
                                                Float a12, vector3d visual_point)
{
    if constexpr (1) {
        if (something_to_swing_to_data.field_0 <= 0.0f) {
            something_to_swing_to_data.field_28 = a1;

            uint32_t a4a = (a4 != nullptr ? a4->get_my_handle().field_0 : 0);

            something_to_swing_to_data.field_38 = a5;
            something_to_swing_to_data.field_34 = a4a;
            something_to_swing_to_data.m_normal = normal;
            something_to_swing_to_data.m_visual_point = visual_point;
            something_to_swing_to_data.m_target_length = a12;
            something_to_swing_to_data.m_best_distance = a11;
            something_to_swing_to_data.field_64 = (no_swing_timer > no_swing_lockout_time);
        }

    } else {
        THISCALL(0x0044C100, this, a1, a4, a5, normal, a11, a12, visual_point);
    }
}

static string_hash web_swing_timer_id{static_cast<int>(to_hash("web_swing_timer"))};

bool swing_inode::can_go_to(string_hash a2) const
{
    if constexpr (0) {
        auto v2 = a2.source_hash_code;
        auto *v4 = this->field_20;
        auto *v5 = v4->field_20;
        auto *v6 = v4->field_24;

        if (hero_inode::is_a_crawl_state(a2, 1)) {
            return this->field_84 < 0.2f;
        }

        if (v2 != run_state::default_id.source_hash_code && v2 != pick_up_state::default_id.source_hash_code) {
            static string_hash loco_allow_aerial_hit_react_id{to_hash("loco_allow_aerial_hit_react")};

            auto &v8 = this->field_8->field_50;

            auto v9 = v8.get_optional_pb_int(loco_allow_aerial_hit_react_id, 0, 0);
            if (v2 == ai::hit_react_state::default_id.source_hash_code && !v9) {
                return false;
            }

            auto &v10 = this->field_8->field_50;

            if (v10.get_pb_float(web_swing_timer_id) > this->m_swing_time) {
                return false;
            }

            auto &v11 = this->field_8->field_50;

            auto v12 = v11.get_pb_int(loco_allow_swing_id);
            if (!this->field_44) {
                if (v12 != 0) {
                    auto v13 = v6->get_button(static_cast<controller_inode::eControllerButton>(11));
                    auto v14 = v13.is_pressed();
                    if (v14) {
                        return false;
                    }

                    auto *v15 = v5->get_als_layer(static_cast<als::layer_types>(0));
                    return v15->is_interruptable();
                }
            }
        }

        auto *v16 = v5->get_als_layer(static_cast<als::layer_types>(0));
        return v16->is_interruptable();

    } else {
        bool(__fastcall * func)(const void *, void *edx, string_hash a2) = CAST(func, 0x0045C850);
        return func(this, nullptr, a2);
    }
}

void swing_inode::update_something_to_swing_to(Float a2)
{
    TRACE("ai::swing_inode::update_something_to_swing_to");

    if constexpr (1) {
        auto v2 = something_to_swing_to_data.field_0 - a2;
        something_to_swing_to_data.field_0 = v2;
        if (v2 >= 0.0f) {
            if (something_to_swing_to_data.field_0 > 0.0f) {
                return;
            }
        } else {
            something_to_swing_to_data.field_0 = 0.0;
        }

        auto v4 = something_to_swing_to_data.field_4 - a2;
        something_to_swing_to_data.field_4 = v4;
        if (v4 >= 0.0f) {
            if (something_to_swing_to_data.field_4 > 0.0f) {
                return;
            }
        } else {
            something_to_swing_to_data.field_4 = 0.0;
        }

        if (something_to_swing_to_data.field_64) {
            static float &dword_958050 = var<float>(0x00958050);
            something_to_swing_to_data.field_4 = dword_958050;
            if (static_cast<int>(something_to_swing_to_data.field_8.field_0) != something_to_swing_to_data.field_34)
                something_to_swing_to_data.field_8.field_0 = something_to_swing_to_data.field_34;

            something_to_swing_to_data.field_C = something_to_swing_to_data.field_38;
            auto v5 = something_to_swing_to_data.field_38.y - -1.0e10;
            something_to_swing_to_data.goal_constraint = v5;
            something_to_swing_to_data.field_18 = something_to_swing_to_data.m_normal;
            if (v5 < 5.0f) {
                if (this->get_actor()->is_in_limbo() || this->get_actor()->get_primary_region() == nullptr) {
                    something_to_swing_to_data.goal_constraint = something_to_swing_to_data.field_C.y;
                } else {
                    auto *reg = this->get_actor()->get_primary_region();
                    something_to_swing_to_data.goal_constraint =
                        something_to_swing_to_data.field_C.y - reg->get_ground_level();
                }

                if (something_to_swing_to_data.goal_constraint < 2.0f) {
                    auto v8 = something_to_swing_to_data.field_C - this->get_actor()->get_abs_position();
                    something_to_swing_to_data.goal_constraint = v8.length();
                }
            }

            something_to_swing_to_data.goal_constraint *= 0.89999998f;
            assert(something_to_swing_to_data.goal_constraint > 0.0f);
        }
    } else {
        THISCALL(0x0045AD80, this, a2);
    }
}

void swing_inode::find_best_anchor_point(const vector3d &a1, [[maybe_unused]] const vector3d &a2,
                                         [[maybe_unused]] const vector3d &a3, [[maybe_unused]] const entity_base *a4,
                                         Float length, float *sweet_spot_ground_level)
{
    //TRACE("swing_inode::find_best_anchor_point:");

    if constexpr (1) {
        static constexpr float safety_34730 = 3.0;

        entity *v8 = this->get_actor();

        if (v8->get_primary_region()) {
            region *v9 = v8->get_primary_region();
            v9->get_ground_level();
        }

        vector3d direction;
        float ground_level = 0.0;
        float angle = 0.0;
        float sweet_spot_length = 0.0;
        this->compute_dynamic_sweet_spot_params(&angle, &sweet_spot_length, &ground_level, &direction);

        float s, c;
        fast_sin_cos_approx(angle, &s, &c);

        swing_anchor_finder finder{
            c, s, sweet_spot_length, min_swing_web_length_squared, max_swing_web_length_squared, sweet_cone_angle_cos};

        finder.set_max_pull_length(length);

        find_best_anchor_result_t anchor_result;
        if (finder.find_best_anchor(this->get_actor(), direction, &anchor_result)) {
            auto v10 = anchor_result.m_point - a1;

            auto target_length = anchor_result.get_target_length();

            float v13 = v10.length();
            sweet_spot_length = target_length - safety_34730;

            target_length = std::min(sweet_spot_length, v13, std::less_equal<float>{});

            auto best_distance = sqrt(anchor_result.m_best_distance_squared);

            this->propose_something_to_swing_to(a1,
                                                anchor_result.m_entity,
                                                anchor_result.m_point,
                                                anchor_result.m_normal,
                                                best_distance,
                                                target_length,
                                                anchor_result.m_visual_point);
            //*(float *) v33 = v19;
        } else {
            if (something_to_swing_to_data.field_0 <= 0.0) {
                something_to_swing_to_data.field_34 = 0;
                something_to_swing_to_data.field_8.field_0 = 0;
                something_to_swing_to_data.field_64 = false;
            }
        }

        *sweet_spot_ground_level = ground_level;
    } else {
        THISCALL(0x00488060, this, &a1, &a2, &a3, a4, length, sweet_spot_ground_level);
    }
}

}  // namespace ai

void swing_state_patch()
{
    {
        REDIRECT(0x0045B206, ai::average_direction_original);
        REDIRECT(0x0045BEEB, ai::average_direction_original);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_state::_activate);
        set_vfunc(0x008774D0, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_state::_deactivate);
        set_vfunc(0x008774D4, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_state::_frame_advance);
        set_vfunc(0x008774D8, address);
    }

    return;

    if constexpr (0) {
        {
            {
                FUNC_ADDRESS(address, &ai::enhanced_state::activate);
                REDIRECT(0x0047DAAB, address);
            }

            {
                FUNC_ADDRESS(address, &world_dynamics_system::activate_web_splats);
                REDIRECT(0x0047DB2B, address);
            }

            {
                FUNC_ADDRESS(address, &ai::als_inode::request_category_transition);
                REDIRECT(0x0047DBAC, address);
            }

            REDIRECT(0x0047DC97, entity_set_abs_position);

            {
                void (als::param_list::*func)(als::param a2) = &als::param_list::add_param;

                FUNC_ADDRESS(address, func);
                REDIRECT(0x0047DD6F, address);
            }

            {
                FUNC_ADDRESS(address, &ai::physics_inode::setup_for_swing);
                REDIRECT(0x0047DD8D, address);
            }

            {
                double (*func)(const po &, const vector3d &, const vector3d &) =
                    calculate_xz_angle_relative_to_local_po;
                REDIRECT(0x0047DD2F, func);
            }
        }
    }
}

void swing_inode_patch()
{
    {
        FUNC_ADDRESS(address, &ai::swing_inode::update_pendulums);
        REDIRECT(0x00477BCB, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::compute_forward_and_up_directions);
        REDIRECT(0x00477B64, address);
        REDIRECT(0x004787A6, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::update_something_to_swing_to);
        REDIRECT(0x0048863F, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::check_for_collision);
        REDIRECT(0x00477B4D, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::update_mode_swinging);
        REDIRECT(0x0047DE7E, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::setup_new_web);
        REDIRECT(0x0047DD94, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::fire_new_web);
        REDIRECT(0x0047DE72, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::_deactivate);
        set_vfunc(0x0087DB58, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::find_best_anchor_point);
        REDIRECT(0x00488627, address);
    }

    return;

    {
        FUNC_ADDRESS(address, &ai::swing_inode::clip_web);
        REDIRECT(0x00477BC3, address);
    }

#if 0
    {
        FUNC_ADDRESS(address, &ai::swing_inode::frame_advance);
        set_vfunc(0x0087DB50, address);
    }

    

    {
        FUNC_ADDRESS(address, &ai::swing_inode::is_eligible);
        REDIRECT(0x00488BE6, address);
    }

    {
        FUNC_ADDRESS(address, &conglomerate::get_bone);
        REDIRECT(0x004878E5, address);
    }

    {
        FUNC_ADDRESS(address, &swing_anchor_finder::find_best_offset_anchor);
        REDIRECT(0x00488148, address);
    }

    {
        FUNC_ADDRESS(address, &ai::swing_inode::propose_something_to_swing_to);
        //REDIRECT(0x0048824A, address);
    }
#endif
}
