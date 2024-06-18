#include "ai_state_jump.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_state_swing.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "color32.h"
#include "common.h"
#include "controller_inode.h"
#include "custom_math.h"
#include "dvar.h"
#include "debug_user_render.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "info_node_desc_list.h"
#include "log.h"
#include "oldmath_po.h"
#include "param_list.h"
#include "physical_interface.h"
#include "physics_inode.h"
#include "state_machine.h"
#include "utility.h"
#include "vector2d.h"
#include "vtbl.h"

#include <cmath>

static auto & g_base_factor = var<float>(0x0091F6D8);

static auto & g_snow_balling = var<float>(0x0091F6DC);

static auto & g_jump_cap_vel = var<float>(0x0091F6E0);

namespace ai {

VALIDATE_SIZE(jump_param_t, 0x10);

VALIDATE_SIZE(jump_state, 0xA4);
VALIDATE_OFFSET(jump_state, field_30, 0x30);
VALIDATE_OFFSET(jump_state, field_4C, 0x4C);
VALIDATE_OFFSET(jump_state, field_81, 0x81);

static auto & jump_params = var<jump_param_t[21]>(0x00958CD0);

jump_state::jump_state(from_mash_in_place_constructor *a2) {
    THISCALL(0x00449D10, this, a2);
}

void jump_state::apply_jets(Float a1)
{
    THISCALL(0x00458890, this, a1);
}

vector3d jump_state::calculate_jump_vector(vector3d a3, vector3d a6, Float a9, Float a10) const
{
    auto *v10 = this->field_30->field_28;

    float v16 = 0.0;
    float a4a = 0.0;
    auto gravity_multiplier = v10->get_gravity_multiplier();
    physical_interface::calculate_force_vector(a9, a10, &v16, &a4a, gravity_multiplier);

    auto result = a3 * v16 + a6 * a4a;

    return result;
}

vector3d jump_state::compute_force(vector3d a3, vector3d a4) const
{
    if constexpr (1)
    {
        auto v5 = this->field_30->field_50;
        auto *v8 = this->get_core();
        auto a9 = v8->field_50.get_pb_float(jump_params[v5].m_height);

        auto *v12 = this->get_core();
        auto a10 = v12->field_50.get_pb_float(jump_params[v5].m_distance);

        vector3d v13 = ((a9 >= 0.0f) ? a4 : -a4);

        auto result = this->calculate_jump_vector(a3, v13, a9, a10);
        return result;
    }
    else
    {
        vector3d result;

        void (__fastcall *func)(const void *, void *, vector3d *, vector3d a3, vector3d a4) = CAST(func, 0x0044A640);
        func(this, nullptr, &result, a3, a4);

        return result;
    }
}

bool jump_state::process_flying(Float a2) {
    //sp_log("jump_state::process_flying(): %d %f", this->field_81, this->field_30->field_70);

    bool (__fastcall *func)(void *, void *, Float) = CAST(func, 0x00469EF0);
    return func(this, nullptr, a2);
}

bool jump_state::check_for_dive_fall()
{
    if constexpr (0)
    {
        const auto v9 = YVEC * 15.0f;
        auto *the_actor = this->get_actor();
        auto v12 = the_actor->get_abs_position() - v9;
        auto *v7 = this->get_actor();

        vector3d v13 {};
        vector3d v14 {};

        return !find_intersection(
                v7->get_abs_position(),
                v12,
                *local_collision::entfilter_entity_no_capsules,
                *local_collision::obbfilter_lineseg_test,
                &v13,
                &v14,
                nullptr,
                nullptr,
                nullptr,
                false);
    }
    else
    {
        bool (__fastcall *func)(void *) = CAST(func, 0x0044A150);
        return func(this);
    }
}

void jump_state::initiate()
{
    if constexpr (0)
    {
    }
    else
    {
        THISCALL(0x004599C0, this);
    }
}

void jump_state::initiate_from_ground()
{
    if constexpr (0)
    {
        auto *v2 = this->field_30;
        auto *v3 = v2->field_28;
        auto *v14 = v2->field_24;
        auto v16 = v3->get_z_facing();
        if ( this->field_30->field_50 != 12 )
        {
            auto *v7 = v14;
            auto v19 = v14->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            auto len2 = v19.length2();
            auto motion_force = v7->field_C->m_player_controller->get_motion_force();
            if ( len2 <= LARGE_EPSILON ) {
                motion_force = LARGE_EPSILON;
            } else {
                v16 = v19;
            }

            v16 *= motion_force;
        }

        auto v9 = this->field_30->field_50;
        if ( v9 == 18 || v9 == 17 )
        {
            auto v10 = dot(v16, YVEC);
            if ( v10 > 0.0f )
            {
                auto v11 = 1.0f - v10;
                auto v20 = v16 * v11;
                auto v19 = v10 * YVEC;
                v16 = v19 + v20;
            }
        }

        auto y_facing = v3->get_y_facing();
        this->field_4C = this->compute_force(v16, y_facing);
        this->field_30->field_64 = v3->get_z_facing();
    }
    else
    {
        THISCALL(0x0044AB30, this);
    }
}

void jump_state::initiate_super_jump()
{
    if (this->field_30->field_28->get_z_facing()[1] <= 0.98000002f) {
        this->initiate_from_wall();
    } else {
        this->initiate_from_ground();
    }
}

void jump_state::initiate_from_wall()
{
    if constexpr (0)
    {
        auto *v2 = this->field_30;
        auto *v3 = v2->field_20;
        auto *v4 = v2->field_24;
        auto &v5 = v2->field_28->get_abs_po();

        vector3d v27 = v5.get_y_facing();

        vector3d v31 = v5.get_z_facing();

        auto v36 = YVEC;

        vector3d a2;
        if (std::abs(dot(YVEC, v27)) < 0.80000001f)
        {
            a2 = v27;

            auto v27 = v4->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            if (v27.length2() > EPSILON) {
                a2 += vector3d{v27[0], 0.0f, v27[2]} * 5.f;

                a2.normalize();
            }

        } else {
            a2 = v5.get_z_facing();
        }

        a2[1] = 0.0;
        a2.normalize();

        als::param_list v29{};

        v29.add_param(0x1Bu, a2);
        v29.add_param(0x18u, YVEC);
        v29.add_param(0x2Du, v5.get_position());

        auto *v11 = v3->get_als_layer(static_cast<als::layer_types>(0));
        v11->set_desired_params(v29);
        auto *v12 = this->get_actor();

        po v37{};
        v37.set_po(a2, YVEC, v12->get_abs_position());

        auto *v14 = this->get_actor();
        entity_set_abs_po(v14, v37);
        auto v15 = jump_params[this->field_30->field_50].m_height;

        auto *v16 = this->get_core();

        auto v30 = v16->field_50.get_pb_float(v15);
        auto v17 = this->field_30->field_50;

        auto v18 = jump_params[v17].m_distance;

        auto *v19 = this->get_core();

        auto v28 = v19->field_50.get_pb_float(v18);
        auto v20 = v30;
        auto v21 = dot(v31, YVEC);

        if (v21 < 0.0f) {
            v20 = (v21 + 1.0f) * v30;
        }

        auto &v22 = this->field_4C;

        v22 = v36 * v20 + v28 * a2;

    } else {
        THISCALL(0x0044A770, this);
    }
}

void jump_state::initiate_from_swing()
{
    sp_log("jump_state::initiate_from_swing:");

    if constexpr (1)
    {
        auto *v2 = this->get_core();

        static const string_hash jump_from_swing_y_bias_id{"jump_from_swing_y_bias"};

        auto jump_from_swing_y_bias = v2->field_50.get_pb_float(jump_from_swing_y_bias_id);

        static const string_hash jump_from_swing_vel_mul_id{"jump_from_swing_vel_mul"};

        auto *v3 = this->get_core();

        auto jump_from_swing_vel_mul = v3->field_50.get_pb_float(jump_from_swing_vel_mul_id);

        static const string_hash web_swing_jump_nerf_threshhold_id{
            "web_swing_jump_nerf_threshhold"};

        auto *v4 = this->get_core();

        auto web_swing_jump_nerf_threshhold = v4->field_50.get_pb_float(
            web_swing_jump_nerf_threshhold_id);

        auto *hero_inode_ptr = this->field_30;
        auto *physics_inode_ptr = hero_inode_ptr->field_28;
        auto *swing_inode_ptr = hero_inode_ptr->field_40;
        auto *als_inode_ptr = hero_inode_ptr->field_20;

        auto vel = physics_inode_ptr->get_velocity();

        auto vel_length = vel.length();

        vector3d new_vel{vel[0], vel[1] * jump_from_swing_y_bias, vel[2]};

        if (new_vel[1] < 0.0f) {
            new_vel[1] = 0.0f;
        }

        new_vel.normalize();

        new_vel *= vel_length * jump_from_swing_vel_mul;

        vector2d local_vec2 = {new_vel[0], new_vel[2]};

        auto entry = local_vec2.length();
        if (entry > 35.0f) {
            new_vel *= 35.0f / entry;
        }

        if constexpr (1) { //SHOW_LOCOMOTION_INFO
            mString v25 = {0, "swing time %.2fs", swing_inode_ptr->m_swing_time};
            auto color = color32{255, 255, 255, 255};
            insertDebugString(1, v25, color);
        }

        if (swing_inode_ptr->m_swing_time < web_swing_jump_nerf_threshhold) {
            assert(not_equal(web_swing_jump_nerf_threshhold, 0.0f));

            auto v14 = swing_inode_ptr->m_swing_time / web_swing_jump_nerf_threshhold;

            new_vel *= v14 * v14;

            if constexpr (1) {
                mString v24 = {0, "un-nerfed %.2f", v14 * v14};

                auto v14 = color32{255, 255, 255, 255};
                insertDebugString(2, v24, v14);
            }
        }

        als::param_list list{};
        list.add_param({14, entry});
        list.add_param({10, RAD_TO_DEG(swing_inode_ptr->field_8C)});
        list.add_param({11, RAD_TO_DEG(swing_inode_ptr->field_90)});

        auto *v15 = als_inode_ptr->get_als_layer(static_cast<als::layer_types>(0));

        v15->set_desired_params(list);

        if constexpr (1)
        {
            mString v24 = {0, "entry %.2f", entry};

            auto v15 = color32{255, 255, 255, 255};
            insertDebugString(9, v24, v15);

            v24 = {0, "angle %.2f", RAD_TO_DEG(swing_inode_ptr->field_8C)};

            insertDebugString(10, v24, v15);
        }

        physics_inode_ptr->set_velocity(new_vel, false);
        auto &v16 = physics_inode_ptr->get_y_facing();
        auto &v17 = physics_inode_ptr->get_z_facing();

        this->field_4C = this->compute_force(v17, v16);
    }
    else
    {
        THISCALL(0x0044ADA0, this);
    }
}

void jump_state::initiate_from_pole_swing()
{
    auto *v2 = this->field_30->field_28;
    [[maybe_unused]] auto *v3 = this->field_30->field_48;
    v2->set_velocity(ZEROVEC, false);

    auto &dir = v2->get_z_facing();
    dir.y = 0.0;
    dir.normalize();

    static constexpr auto height = 10.0f;
    static constexpr auto distance = 15.0f;

    this->field_4C = this->calculate_jump_vector(dir, YVEC, height, distance);
}

void jump_state::initiate_from_air() {
    THISCALL(0x0044B220, this);
}

int jump_state::activate(ai_state_machine *a2,
                         const mashed_state *a3,
                         const mashed_state *a4,
                         const param_block *a5,
                         base_state::activate_flag_e a6) {
    return THISCALL(0x00469880, this, a2, a3, a4, a5, a6);
}

uint32_t jump_state::get_virtual_type_enum() {
    return 303;
}

bool jump_state::is_subclass_of(mash::virtual_types_enum a2) {
    return (bool) THISCALL(0x0043BAE0, this, a2);
}

void jump_state::deactivate(const ai::mashed_state *a1) {
    THISCALL(0x00449FA0, this, a1);
}

void jump_state::set_gravity_vector(const vector3d &a2, Float a3) {
    THISCALL(0x0044A230, this, &a2, a3);
}

int jump_state::frame_advance_jump_type_specifics(Float a2) {
    return THISCALL(0x00469AC0, this, a2);
}

int jump_state::frame_advance(Float a2) {
    return THISCALL(0x00473E70, this, a2);
}

int jump_state::get_info_node_list(info_node_desc_list &a1) {
    return THISCALL(0x0044A3B0, this, &a1);
}

int jump_state::get_mash_sizeof() {
    return 164;
}

} // namespace ai


void __fastcall set_velocity(ai::physics_inode *self, void *, const vector3d *a2, bool a3)
{
    self->field_1C->set_velocity(*a2, a3);

    {
        debug_variable_t v67 {"jump_cap_vel", g_jump_cap_vel};
        g_jump_cap_vel = v67;

        debug_variable_t v68 {"snow_balling", g_snow_balling};
        g_snow_balling = v68;

        debug_variable_t v88 {"base_factor", g_base_factor};
        g_base_factor = v88;
    }
}

void jump_state_patch()
{

    {
        REDIRECT(0x004593B2, set_velocity);
    }

    {
        FUNC_ADDRESS(address, &ai::jump_state::process_flying);
        REDIRECT(0x00473FD3, address);
    }

    {
        FUNC_ADDRESS(address, &ai::jump_state::compute_force);
        //REDIRECT(0x0044AD00, address);
    }

    {
        FUNC_ADDRESS(address, &ai::jump_state::initiate_from_swing);
        REDIRECT(0x0045A36E, address);
    }
}
