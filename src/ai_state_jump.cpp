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
#include "als_animation_logic_system.h"
#include "colgeom_alter_sys.h"
#include "combat_inode.h"
#include "combat_state.h"
#include "event_manager.h"
#include "event.h"
#include "game.h"
#include "game_settings.h"
#include "glass_house_inode.h"
#include "glass_house_manager.h"
#include "interaction_inode.h"
#include "mashed_state.h"
#include "memory.h"
#include "ai_state_web_zip.h"
#include <algorithm>
#include <array>

static auto &g_base_factor = var<float>(0x0091F6D8);

static auto &g_snow_balling = var<float>(0x0091F6DC);

static auto &g_jump_cap_vel = var<float>(0x0091F6E0);

namespace ai {

VALIDATE_SIZE(jump_param_t, 0x10);

VALIDATE_SIZE(jump_state, 0xA4);
VALIDATE_OFFSET(jump_state, field_30, 0x30);
VALIDATE_OFFSET(jump_state, field_4C, 0x4C);
VALIDATE_OFFSET(jump_state, field_81, 0x81);
VALIDATE_OFFSET(jump_state, field_58, 0x58);
VALIDATE_OFFSET(jump_state, field_84, 0x84);
VALIDATE_OFFSET(jump_state, field_94, 0x94);

namespace {
constexpr auto layer = static_cast<als::layer_types>(0);
const string_hash jump_in_air_cat{static_cast<int>(to_hash("Jump_In_Air"))};
const string_hash jump_launch_cat{static_cast<int>(to_hash("Jump_Launch"))};
const string_hash jump_double_cat{static_cast<int>(to_hash("Jump_Double"))};
const string_hash jump_off_wall_cat{to_hash("Jump_Off_Wall")};
const string_hash jump_off_wall_fly_cat{to_hash("Jump_Off_Wall_Fly")};
const string_hash jump_off_ceiling_fly_cat{static_cast<int>(to_hash("Jump_Off_Ceiling_Fly"))};
const string_hash jump_release_cat{static_cast<int>(to_hash("Jump_Release"))};
const string_hash super_jump_launch_cat{to_hash("Super_Jump_Launch")};
const string_hash super_wall_jump_launch_cat{static_cast<int>(to_hash("Super_Wall_Jump_Launch"))};
const string_hash instant_super_wall_jump_launch_cat{to_hash("Instant_Super_Wall_Jump_Launch")};
bool wall_jump(int type)
{
    return type == 4 || type == 6 || type == 7 || type == 8;
}
bool super_jump(int type)
{
    return type >= 15 && type <= 18;
}
void transition(als_inode *animation, string_hash category, bool immediate = false)
{
    animation->request_category_transition(category, layer, true, false, immediate);
}
void *__fastcall jump_delete(jump_state *self, void *, bool release)
{
    self->~jump_state();
    if (release)
        mem_dealloc(self, sizeof(jump_state));
    return self;
}
uint32_t __fastcall jump_type(jump_state *self)
{
    return self->get_virtual_type_enum();
}
bool __fastcall jump_subclass(jump_state *self, void *, mash::virtual_types_enum type)
{
    return self->is_subclass_of(type);
}
void __fastcall jump_activate(jump_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                              const mashed_state *previous, const param_block *params,
                              base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
void __fastcall jump_deactivate(jump_state *self, void *, const mashed_state *state)
{
    self->deactivate(state);
}
int __fastcall jump_frame(jump_state *self, void *, Float dt)
{
    return self->frame_advance(dt);
}
void __fastcall jump_list(jump_state *self, void *, info_node_desc_list &list)
{
    self->get_info_node_list(list);
}
int __fastcall jump_size(jump_state *self)
{
    return self->get_mash_sizeof();
}
int __fastcall jump_specific(jump_state *self, void *, Float dt)
{
    return self->frame_advance_jump_type_specifics(dt);
}
}  // namespace

void *jump_state::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 17> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), 16, result.data());
        result[2] = bit_cast<void *>(&jump_delete);
        result[3] = bit_cast<void *>(&jump_type);
        result[4] = bit_cast<void *>(&jump_subclass);
        result[6] = bit_cast<void *>(&jump_activate);
        result[7] = bit_cast<void *>(&jump_deactivate);
        result[8] = bit_cast<void *>(&jump_frame);
        result[9] = bit_cast<void *>(&jump_list);
        result[13] = bit_cast<void *>(&jump_size);
        result[16] = bit_cast<void *>(&jump_specific);
        return result;
    }();
    return const_cast<void **>(table.data());
}

jump_state::jump_state()
    : enhanced_state(), field_34(nullptr), field_40(nullptr), field_4C(nullptr), field_58(nullptr), field_64(nullptr)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00877158;
    field_80 = field_81 = field_82 = field_83 = field_84 = false;
}

#if STANDALONE_SYSTEM
static jump_param_t jump_params[21]{
    {"jump_fall_height", "jump_fall_distance"},
    {"jump_run_height", "jump_run_distance"},
    {"jump_double_height", "jump_double_distance"},
    {"jump_double_idle_height", "jump_double_idle_distance"},
    {"jump_off_wall_height", "jump_off_wall_distance"},
    {"jump_wall_release_height", "jump_wall_release_distance"},
    {"jump_wall_idle_height", "jump_wall_idle_distance"},
    {"jump_wall_run_height", "jump_wall_run_distance"},
    {"jump_wall_crawl_height", "jump_wall_crawl_distance"},
    {"jump_to_swing_height", "jump_to_swing_distance"},
    {"jump_slow_swing_height", "jump_slow_swing_distance"},
    {"jump_fast_swing_height", "jump_fast_swing_distance"},
    {"jump_zip_ground_height", "jump_zip_ground_distance"},
    {"jump_zip_air_height", "jump_zip_air_distance"},
    {"jump_glass_house_height", "jump_glass_house_distance"},
    {"jump_super_jump_run_height", "jump_super_jump_run_distance"},
    {"jump_super_jump_wall_idle_height", "jump_super_jump_wall_idle_distance"},
    {"jump_super_jump_wall_height", "jump_super_jump_wall_distance"},
    {"jump_instant_super_jump_wall_height", "jump_instant_super_jump_wall_distance"},
    {"jump_pole_swing_height", "jump_pole_swing_distance"},
    {"jump_flying_start_height", "jump_flying_start_distance"},
};
#else
static auto &jump_params = var<jump_param_t[21]>(0x00958CD0);
#endif

jump_state::jump_state(from_mash_in_place_constructor *a2)
    : enhanced_state(a2), field_34(a2), field_40(a2), field_4C(a2), field_58(a2), field_64(a2)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00877158;
}

void jump_state::apply_jets(Float dt)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00458890, this, dt);
        return;
    }
    if (std::abs(field_98) >= 1.1920928955078125e-7f)
        field_94 = std::clamp(field_94 + float(dt) * field_98, 0.0f, 1.0f);
    auto *physics = field_30->field_28;
    auto *controller = field_30->field_24;
    auto *animation = field_30->field_20;
    auto &params = get_core()->field_50;
    auto velocity = physics->get_velocity();
    auto horizontal = velocity;
    horizontal.y = 0.0f;
    const float speed = horizontal.length();
    if (speed > 0.0f)
        field_30->field_78 = bit_cast<int>(speed);
    const float force_limit = params.get_pb_float(string_hash{"jets_force_limit"});
    const float decay_limit = params.get_pb_float(string_hash{"jets_decay_limit"});
    const float feedback = params.get_pb_float(string_hash{"jets_force_feedback"});
    const float decay = params.get_pb_float(string_hash{"jets_decay_coeff"});
    const auto position = physics->get_abs_position();
    if (field_90 < -1000.0f && velocity.y < 0.0f)
        field_90 = position.y;
    field_78 = field_90 > position.y ? field_90 - position.y : -1.0f;
    als::param_list list;
    if (field_83)
        list.add_param(0x18u, -get_actor()->physical_ifc()->field_74);
    list.add_param({69, field_78});
    list.add_param({0, controller->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0)) ? 0.0f : 5.0f});
    const int type = field_30->field_50;
    if (super_jump(type) || wall_jump(type) || type == 1 || type == 2 || type == 3 || type == 19) {
        const auto button =
            controller->get_button(static_cast<controller_inode::eControllerButton>(super_jump(type) ? 12 : 7));
        if (!button.is_pressed())
            field_82 = true;
    }
    if (field_82) {
        const float power = params.get_pb_float(string_hash{"jump_mario_power"});
        const float peak = params.get_pb_float(string_hash{"jump_mario_peak"});
        if (velocity.y > -peak) {
            velocity.y -= power * float(dt);
            velocity.x -= velocity.x * 0.5f * float(dt);
            velocity.z -= velocity.z * 0.5f * float(dt);
            physics->set_velocity(velocity, false);
        }
    }
    update_jump_capsule();
    velocity = physics->get_velocity();
    if (!glass_house_manager::is_point_in_glass_house(position)) {
        auto toward = field_30->field_44->field_24 - position;
        const float distance = toward.length();
        if (distance > 0.0f)
            toward *= 1.0f / distance;
        velocity += toward * distance;
    } else if (field_83) {
        if (!controller->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0))) {
            auto direction = controller->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            direction.y = 0.0f;
            direction.normalize();
            const float vertical = velocity.y;
            velocity.y = 0.0f;
            field_34 = velocity;
            if (field_34.length2() < EPSILON)
                field_34 = get_actor()->get_abs_po().get_z_facing();
            field_34.normalize();
            if (direction.length2() < EPSILON)
                direction = get_actor()->get_abs_po().get_z_facing();
            list.add_param(0x1Bu, wall_jump(type) ? field_34 : direction);
            auto force = controller->get_axis(static_cast<controller_inode::eControllerAxis>(0));
            force.normalize();
            const float old_speed = velocity.length();
            force *= float(dt) * field_94 * 35.0f;
            velocity += force;
            field_40 = field_40 * feedback - force;
            if (field_40.length2() > force_limit * force_limit)
                field_40.set_length(force_limit);
            const float limit = old_speed <= 8.0f ? 8.0f : old_speed;
            if (velocity.length() > limit)
                velocity.set_length(limit);
            if ((physics->field_1C->field_C & 0x60u) != 0) {
                auto normal = physics->get_last_collision_normal();
                if (normal.x * normal.x + normal.z * normal.z > 9.99999905104687e-9f) {
                    normal.y = 0.0f;
                    normal.normalize();
                    const float inward = dot(normal, velocity);
                    if (inward < 0.0f)
                        velocity -= normal * inward;
                }
            }
            velocity.y = vertical;
        } else {
            auto force = field_40;
            field_40 *= decay;
            if (force.length() > decay_limit)
                force.set_length(decay_limit);
            velocity += force;
            list.add_param(0x1Bu, physics->get_z_facing());
        }
    }
    physics->set_velocity(velocity, false);
    velocity = physics->get_velocity();
    const float vertical = velocity.y;
    horizontal = velocity;
    horizontal.y = 0.0f;
    const float current_speed = horizontal.length();
    static float accumulated_deceleration = 0.0f;
    if (current_speed > 33.0f) {
        accumulated_deceleration += 2.5f * 0.1f * accumulated_deceleration * float(dt) + 23.0f * 0.1f * float(dt);
        velocity = horizontal * ((current_speed - accumulated_deceleration) / current_speed);
    } else {
        accumulated_deceleration = 0.0f;
    }
    velocity.y = vertical;
    physics->set_velocity(velocity, false);
    animation->get_als_layer(layer)->set_desired_params(list);
    params.get_pb_float(string_hash{"jump_gravity_initial"});
    const float peak_gravity = params.get_pb_float(string_hash{"jump_gravity_peak"});
    const float fall_gravity = params.get_pb_float(string_hash{"jump_gravity_fall"});
    const float gravity_speed = params.get_pb_float(string_hash{"jump_gravity_mod_speed"});
    if (physics->get_velocity().y < 0.0f && !field_80) {
        field_80 = true;
        field_7C = peak_gravity;
    }
    if (field_80 && field_7C < fall_gravity)
        field_7C = std::min(field_7C + gravity_speed * float(dt), fall_gravity);
    physics->set_gravity_multiplier(field_7C);
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
    if constexpr (1) {
        auto v5 = this->field_30->field_50;
        auto *v8 = this->get_core();
        auto a9 = v8->field_50.get_pb_float(jump_params[v5].m_height);

        auto *v12 = this->get_core();
        auto a10 = v12->field_50.get_pb_float(jump_params[v5].m_distance);

        vector3d v13 = ((a9 >= 0.0f) ? a4 : -a4);

        auto result = this->calculate_jump_vector(a3, v13, a9, a10);
        return result;
    } else {
        vector3d result;

        void(__fastcall * func)(const void *, void *, vector3d *, vector3d a3, vector3d a4) = CAST(func, 0x0044A640);
        func(this, nullptr, &result, a3, a4);

        return result;
    }
}

bool jump_state::process_flying(Float dt)
{
    if constexpr (!STANDALONE_SYSTEM)
        return THISCALL(0x00469EF0, this, dt);
    auto *controller = field_30->field_24;
    bool trigger = false;
    if (get_core()->field_50.get_pb_int(string_hash{"loco_allow_super_jump"}) != 1)
        trigger = controller->get_button(static_cast<controller_inode::eControllerButton>(7)).is_triggered();
    if (trigger) {
        const int type = field_30->field_50;
        const bool wall_ready = !wall_jump(type) || (field_83 && field_30->field_70 > 0.2f);
        bool initiated = false;
        if (wall_ready && type != 14 && type != 13 && type != 12 &&
            field_30->ought_to_jump_off_wall(field_30->field_1B0)) {
            field_30->set_jump_type(static_cast<eJumpType>(4), false);
            initiated = true;
        } else if (wall_ready && type != 2 && type != 3 && type != 14) {
            field_30->set_jump_type(
                static_cast<eJumpType>(
                    controller->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0)) ? 3 : 2),
                false);
            initiated = true;
        }
        if (initiated) {
            field_30->field_7C = false;
            field_30->field_70 = 0.0f;
            field_81 = field_82 = false;
            field_88 = 25.0f;
            field_8C = 3.0f;
            initiate();
        }
    }
    apply_jets(dt);
    return false;
}

bool jump_state::check_for_dive_fall()
{
    if constexpr (!STANDALONE_SYSTEM)
        return THISCALL(0x0044A150, this);
    const auto start = get_actor()->get_abs_position();
    const auto end = start - YVEC * 15.0f;
    vector3d point, normal;
    return !find_intersection(start,
                              end,
                              *local_collision::entfilter_entity_no_capsules,
                              *local_collision::obbfilter_lineseg_test,
                              &point,
                              &normal,
                              nullptr,
                              nullptr,
                              nullptr,
                              false);
}

void jump_state::initiate()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x004599C0, this);
        return;
    }
    auto *physics = field_30->field_28;
    auto *animation = field_30->field_20;
    if (field_30->field_50 != 17)
        physics->setup_for_jump(get_gravity_vector());
    field_30->field_58 = get_actor()->get_abs_position();
    field_30->field_64 = physics->get_y_facing();
    field_34 = physics->get_z_facing();
    field_90 = -1000000.0f;
    auto &params = get_core()->field_50;
    field_7C = params.get_pb_float(string_hash{"jump_gravity_initial"});
    params.get_pb_float(string_hash{"jump_gravity_peak"});
    params.get_pb_float(string_hash{"jump_gravity_fall"});
    params.get_pb_float(string_hash{"jump_gravity_mod_speed"});
    field_84 = true;
    field_83 = field_82 = field_80 = false;
    const int type = field_30->field_50;
    field_94 = type == 0 || (type >= 4 && type <= 8) ? 0.4f : 1.0f;
    field_98 = type >= 4 && type <= 8 ? 0.5f : 0.0f;
    switch (type) {
    case 0:
        initiate_from_ground();
        transition(animation, jump_in_air_cat);
        break;
    case 1:
        initiate_from_ground();
        transition(animation, jump_launch_cat);
        break;
    case 2:
    case 3:
        initiate_from_air();
        transition(animation, jump_double_cat);
        break;
    case 4:
    case 5:
    case 6:
    case 8:
        if (field_30->field_7C) {
            field_4C = calculate_off_wall_jump_force(field_30->field_1B0);
        } else {
            const bool allow_super = params.get_pb_int(string_hash{"loco_allow_super_jump"}) != 0;
            field_30->field_7C = true;
            field_4C = ZEROVEC;
            float delay = 0.5f;
            event_manager::raise_event(event::TRICK_WALL_JUMP, get_actor()->my_handle);
            auto direction = field_30->field_1B0.hit_norm;
            if (type == 4)
                direction = -direction;
            auto up = physics->get_y_facing();
            const auto position = field_30->field_1B0.hit_pos + field_30->field_1B0.hit_norm;
            auto category = jump_off_wall_cat;
            if (is_colinear(up, direction, 0.01f)) {
                if (is_colinear(direction, YVEC, 0.01f)) {
                    category = jump_off_ceiling_fly_cat;
                    direction = physics->get_z_facing();
                    field_30->set_jump_type(static_cast<eJumpType>(5), false);
                    delay = 0.1f;
                } else {
                    category = jump_off_wall_fly_cat;
                }
                up = YVEC;
            }
            if (field_30->field_50 != 5 || category == jump_off_ceiling_fly_cat) {
                if (allow_super)
                    category = jump_off_wall_fly_cat;
            } else {
                category = jump_release_cat;
                delay = 0.0f;
            }
            transition(animation, category, true);
            als::param_list list;
            list.add_param(0x1Bu, direction);
            list.add_param(0x18u, up);
            list.add_param(0x21u, position);
            animation->get_als_layer(layer)->set_desired_params(list);
            get_actor()->physical_ifc()->set_gravity_delay_timer(delay);
            get_actor()->physical_ifc()->cancel_all_velocity();
        }
        break;
    case 7:
        if (field_30->field_7C) {
            field_4C = calculate_off_wall_jump_force(field_30->field_1B0);
        } else {
            field_30->field_7C = true;
            field_4C = ZEROVEC;
            initiate_from_wall();
            transition(animation, jump_off_wall_fly_cat);
            event_manager::raise_event(event::TRICK_WALL_JUMP, get_actor()->my_handle);
            animation->get_system()->force_update();
            get_actor()->physical_ifc()->set_gravity_delay_timer(0.5f);
        }
        break;
    case 9:
        initiate_from_ground();
        transition(animation, string_hash{"Jump_To_Swing"});
        break;
    case 10:
    case 11:
        initiate_from_swing();
        transition(animation, string_hash{"Jump_From_Swing"});
        break;
    case 12:
        field_94 = 0.0f;
        initiate_from_ground();
        break;
    case 13:
        field_94 = 0.0f;
        field_98 = 2.0f;
        initiate_from_zip();
        break;
    case 14:
        field_94 = 0.0f;
        initiate_glass_house();
        animation->get_als_layer(layer)->force_als_state(string_hash{"JumpLaunch"}, static_cast<int>(0xDEADBEEF));
        break;
    case 15:
        if (field_30->field_7C) {
            initiate_super_jump();
        } else {
            field_30->field_7C = true;
            field_4C = ZEROVEC;
            transition(animation, super_jump_launch_cat);
        }
        break;
    case 16:
        initiate_super_jump();
        transition(animation, super_jump_launch_cat);
        break;
    case 17:
        if (field_30->field_7C) {
            initiate_from_ground();
        } else {
            field_30->field_7C = true;
            field_4C = ZEROVEC;
            if (get_actor()->has_physical_ifc()) {
                const auto gravity = -get_actor()->get_abs_po().get_y_facing();
                get_actor()->physical_ifc()->set_current_gravity_vector(gravity);
                set_gravity_vector(gravity, 0.0f);
            }
            transition(animation, super_wall_jump_launch_cat);
        }
        break;
    case 18:
        initiate_from_ground();
        if (get_actor()->has_physical_ifc()) {
            const auto gravity = -get_actor()->get_abs_po().get_y_facing();
            get_actor()->physical_ifc()->set_current_gravity_vector(gravity);
            set_gravity_vector(gravity, 0.0f);
        }
        transition(animation, instant_super_wall_jump_launch_cat);
        field_83 = true;
        break;
    case 19:
        initiate_from_pole_swing();
        transition(animation, jump_in_air_cat);
        break;
    default:
        break;
    }
    field_40 = ZEROVEC;
    if (!field_30->field_7C)
        extend_capsule_for_jump(get_actor());
    physics->apply_force_increment(field_4C, static_cast<physical_interface::force_type>(1), IGNORE_LOC, 0);
}

void jump_state::initiate_from_ground()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0044AB30, this);
        return;
    }
    auto *physics = field_30->field_28;
    auto direction = physics->get_z_facing();
    if (field_30->field_50 != 12) {
        auto axis = field_30->field_24->get_axis(static_cast<controller_inode::eControllerAxis>(0));
        float force = field_30->field_24->field_C->m_player_controller->get_motion_force();
        if (axis.length2() <= LARGE_EPSILON)
            force = LARGE_EPSILON;
        else
            direction = axis;
        direction *= force;
    }
    if (field_30->field_50 == 17 || field_30->field_50 == 18) {
        const float upward = dot(direction, YVEC);
        if (upward > 0.0f)
            direction = YVEC * upward + direction * (1.0f - upward);
    }
    field_4C = compute_force(direction, physics->get_y_facing());
    field_30->field_64 = physics->get_y_facing();
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
    if constexpr (STANDALONE_SYSTEM) {
        auto *v2 = this->field_30;
        auto *v3 = v2->field_20;
        auto *v4 = v2->field_24;
        auto &v5 = v2->field_28->get_abs_po();

        vector3d v27 = v5.get_y_facing();

        vector3d v31 = v5.get_z_facing();

        auto v36 = YVEC;

        vector3d a2;
        if (std::abs(dot(YVEC, v27)) < 0.80000001f) {
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
    if constexpr (1) {
        auto *v2 = this->get_core();

        static const string_hash jump_from_swing_y_bias_id{"jump_from_swing_y_bias"};

        auto jump_from_swing_y_bias = v2->field_50.get_pb_float(jump_from_swing_y_bias_id);

        static const string_hash jump_from_swing_vel_mul_id{"jump_from_swing_vel_mul"};

        auto *v3 = this->get_core();

        auto jump_from_swing_vel_mul = v3->field_50.get_pb_float(jump_from_swing_vel_mul_id);

        static const string_hash web_swing_jump_nerf_threshhold_id{"web_swing_jump_nerf_threshhold"};

        auto *v4 = this->get_core();

        auto web_swing_jump_nerf_threshhold = v4->field_50.get_pb_float(web_swing_jump_nerf_threshhold_id);

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


        if (swing_inode_ptr->m_swing_time < web_swing_jump_nerf_threshhold) {
            assert(not_equal(web_swing_jump_nerf_threshhold, 0.0f));

            auto v14 = swing_inode_ptr->m_swing_time / web_swing_jump_nerf_threshhold;

            new_vel *= v14 * v14;
        }

        als::param_list list{};
        list.add_param({14, entry});
        list.add_param({10, RAD_TO_DEG(swing_inode_ptr->field_8C)});
        list.add_param({11, RAD_TO_DEG(swing_inode_ptr->field_90)});

        auto *v15 = als_inode_ptr->get_als_layer(static_cast<als::layer_types>(0));

        v15->set_desired_params(list);


        physics_inode_ptr->set_velocity(new_vel, false);
        auto &v16 = physics_inode_ptr->get_y_facing();
        auto &v17 = physics_inode_ptr->get_z_facing();

        this->field_4C = this->compute_force(v17, v16);
    } else {
        THISCALL(0x0044ADA0, this);
    }
}

void jump_state::initiate_from_pole_swing()
{
    auto *v2 = this->field_30->field_28;
    [[maybe_unused]] auto *v3 = this->field_30->field_48;
    v2->set_velocity(ZEROVEC, false);

    auto dir = v2->get_z_facing();
    dir.y = 0.0;
    dir.normalize();

    static constexpr auto height = 10.0f;
    static constexpr auto distance = 15.0f;

    this->field_4C = this->calculate_jump_vector(dir, YVEC, height, distance);
}

void jump_state::initiate_from_air()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0044B220, this);
        return;
    }
    auto *physics = field_30->field_28;
    auto velocity = physics->get_velocity();
    const int previous = field_30->field_54;
    if (wall_jump(previous) || previous == 1 || previous == 2 || previous == 3 || previous == 19 || velocity.y < 0.0f) {
        velocity.y = 0.0f;
        physics->set_velocity(velocity, false);
    }
    field_4C = compute_force(physics->get_z_facing(), physics->get_y_facing());
}

void jump_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                          const param_block *params, base_state::activate_flag_e flags)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00469880, this, machine, state, previous, params, flags);
        return;
    }
    enhanced_state::activate(machine, state, previous, params, flags);
    field_30 = static_cast<hero_inode *>(get_core()->get_info_node(hero_inode::default_id, true));
    auto *animation = field_30->field_20;
    field_30->field_7C = false;
    field_84 = true;
    if (animation->is_cat_our_prev_cat(string_hash{"Loco_Combat_Jump"}, layer))
        field_30->set_jump_type(static_cast<eJumpType>(15), false);
    else if (animation->is_cat_our_prev_cat(string_hash{"Combat_Jump"}, layer) ||
             animation->get_category_id(layer) == string_hash{"Combat_Jump"})
        field_30->set_jump_type(static_cast<eJumpType>(1), false);
    else if (animation->get_category_id(layer) == string_hash{"Combat_Fall"})
        field_30->set_jump_type(static_cast<eJumpType>(0), false);
    field_78 = -1.0f;
    field_9C = 0;
    field_74 = field_70 = 0.0f;
    field_58 = get_actor()->physical_ifc()->field_74;
    field_64 = -YVEC;
    auto *controller = get_actor()->m_player_controller;
    if (controller->m_hero_type != VENOM)
        shrink_capsule_for_slanted_surfaces(get_actor());
    initiate();
    field_30->field_64 = get_actor()->get_abs_po().get_y_facing();
    controller->set_spidey_loco_mode(
        static_cast<eHeroLocoMode>(field_30->field_50 == 17 || field_30->field_50 == 18 ? 7 : 6));
    field_30->field_70 = 0.0f;
    field_81 = field_82 = false;
    field_88 = 25.0f;
    field_8C = 3.0f;
}

uint32_t jump_state::get_virtual_type_enum()
{
    return 303;
}

bool jump_state::is_subclass_of(mash::virtual_types_enum type)
{
    return type == 535 || type == 567 || type == 573;
}

void jump_state::deactivate(const ai::mashed_state *next)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x00449FA0, this, next);
        return;
    }
    auto *owner = get_actor();
    if (owner && owner->has_physical_ifc()) {
        auto *combat = field_30->field_2C;
        const auto offset = next && next->get_name() == combat_state::default_id ? 0x7C : 0x80;
        reinterpret_cast<void(__fastcall *)(combat_inode *, void *)>(get_vfunc(combat->m_vtbl, offset))(combat,
                                                                                                        nullptr);
        setup_hero_capsule(owner);
        if (super_jump(field_30->field_50)) {
            const auto distance = field_30->field_58 - owner->get_abs_position();
            g_game_ptr->gamefile->field_340.field_C4 += distance.length() * 0.0006213712f;
        }
        field_30->field_74 = field_78;
        owner->physical_ifc()->set_current_gravity_vector(-YVEC);
    }
    base_state::_deactivate(next);
}

void jump_state::set_gravity_vector(const vector3d &gravity, Float time)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0044A230, this, &gravity, time);
        return;
    }
    field_70 = field_74 = time;
    field_58 = get_actor()->physical_ifc()->field_74;
    field_64 = gravity;
}

int jump_state::frame_advance_jump_type_specifics(Float dt)
{
    if constexpr (!STANDALONE_SYSTEM)
        return THISCALL(0x00469AC0, this, dt);
    auto *animation = field_30->field_20;
    const int type = field_30->field_50;
    if (type == 17 || type == 18) {
        field_30->update_crawl_als_params();
        if (!field_83 && (animation->is_cat_our_prev_cat(super_wall_jump_launch_cat, layer) ||
                          (animation->get_category_id(layer) == instant_super_wall_jump_launch_cat &&
                           animation->get_eta_of_combat_signal(layer) <= 0.0f))) {
            field_30->field_28->setup_for_jump(get_gravity_vector());
            initiate();
            field_83 = true;
            field_30->field_7C = false;
        }
        static bool check_super_jump_direction = false;
        const auto displacement =
            field_30->field_58 - get_actor()->get_abs_position() + get_actor()->get_abs_po().get_y_facing();
        if (field_83 && ((check_super_jump_direction && dot(displacement, field_30->field_64) < 0.0f) ||
                         field_78 > 0.0f || field_1C > 0.75f)) {
            field_30->set_jump_type(static_cast<eJumpType>(0), false);
            set_gravity_vector(-YVEC, 0.5f);
            initiate();
            field_82 = true;
        }
    } else if (type == 15) {
        if (!field_83 && (animation->is_cat_our_prev_cat(super_jump_launch_cat, layer) ||
                          (animation->get_category_id(layer) == super_jump_launch_cat &&
                           animation->get_eta_of_combat_signal(layer) <= 0.0f))) {
            initiate();
            field_83 = true;
            field_30->field_7C = false;
        }
    } else if (type >= 4 && type <= 8) {
        if (!field_83 && !field_84 && animation->get_eta_of_combat_signal(layer) <= 0.0f) {
            if (animation->get_category_id(layer) == jump_off_wall_cat) {
                transition(animation, jump_off_wall_fly_cat, true);
                als::param_list list;
                list.add_param(0x1Bu, -get_actor()->get_abs_po().get_z_facing());
                list.add_param(0x18u, get_actor()->get_abs_po().get_y_facing());
                list.add_param(0x21u, get_actor()->get_abs_position());
                animation->get_als_layer(layer)->set_desired_params(list);
            }
            const auto category = animation->get_category_id(layer);
            if (category == jump_off_wall_fly_cat || category == jump_release_cat ||
                category == jump_off_ceiling_fly_cat) {
                initiate();
                field_83 = true;
                field_30->field_7C = false;
            }
        }
    } else if (type >= 0 && type <= 20) {
        field_83 = true;
        if (type == 0 && (field_30->field_54 == 17 || field_30->field_54 == 18) && field_70 <= 0.2f) {
            transition(animation, string_hash{"Crawl_Big_Jump_To_Normal_Big_Jump_Fly"}, true);
            field_30->set_jump_type(static_cast<eJumpType>(0), false);
        }
    }
    return TRANS_TOTAL_MSGS;
}

int jump_state::frame_advance(Float dt)
{
    if constexpr (!STANDALONE_SYSTEM)
        return THISCALL(0x00473E70, this, dt);
    const auto result = enhanced_state::frame_advance(dt);
    if (result != TRANS_TOTAL_MSGS)
        return result;
    auto *animation = field_30->field_20;
    update_gravity_vector(dt);
    field_30->field_70 += dt;
    const int has_dive = get_core()->field_50.get_pb_int(string_hash{"has_dive_fall"});
    if (field_30->field_50 == 0 && field_30->field_34->curr_status != 2 && !field_81 && check_for_dive_fall() &&
        has_dive) {
        field_81 = true;
        transition(animation, string_hash{"Jump_Dive_Fall"});
    }
    if (field_30->field_28->get_velocity().y < -10.0f)
        get_actor()->m_player_controller->set_spidey_loco_mode(FALLING);
    if (!animation->is_layer_interruptable(layer))
        return TRANS_TOTAL_MSGS;
    if (!field_81 || field_30->field_70 >= 0.2f)
        process_flying(dt);
    const auto advanced =
        reinterpret_cast<int(__fastcall *)(jump_state *, void *, Float)>(get_vfunc(m_vtbl, 0x40))(this, nullptr, dt);
    field_84 = false;
    return advanced;
}

void jump_state::get_info_node_list(info_node_desc_list &list)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0044A3B0, this, &list);
        return;
    }
    list.add_entry({hero_inode::default_id, 384});
    list.add_entry({als_inode::default_id, 333});
    list.add_entry({physics_inode::default_id, 402});
}

vector3d jump_state::get_gravity_vector() const
{
    if (field_70 <= 0.0f)
        return field_64;
    const float weight = (field_74 - field_70) / field_74;
    return field_64 * weight + field_58 * (1.0f - weight);
}

void jump_state::update_gravity_vector(Float dt)
{
    if (field_70 > 0.0f) {
        field_70 = std::max(field_70 - float(dt), 0.0f);
        get_actor()->physical_ifc()->set_current_gravity_vector(get_gravity_vector());
    }
}

vector3d jump_state::calculate_off_wall_jump_force(const line_info &line)
{
    auto *physics = field_30->field_28;
    auto velocity = physics->get_velocity();
    velocity.y = 0.0f;
    physics->set_velocity(velocity, false);
    auto direction = line.hit_norm;
    if (field_30->field_50 == 7) {
        direction = velocity;
        direction.normalize();
        if (direction == ZEROVEC)
            direction = line.hit_norm;
    }
    return compute_force(direction, physics->get_y_facing());
}

void jump_state::initiate_from_zip()
{
    auto *physics = field_30->field_28;
    const float distance_limit = get_core()->field_50.get_pb_float(string_hash{"web_zip_distance_limit"});
    const float arc_height = get_core()->field_50.get_pb_float(string_hash{"web_zip_arc_height"});
    const auto position = physics->get_abs_position();
    auto target = field_30->field_3C->field_1C.hit_pos;
    auto horizontal = target - position;
    const float height = horizontal.y;
    horizontal.y = 0.0f;
    if (horizontal.length2() > distance_limit * distance_limit) {
        horizontal.set_length(distance_limit);
        target = position + horizontal;
        target.y += height;
    }
    const float peak = std::max(target.y + arc_height, position.y + 0.5f);
    auto velocity = physics->get_velocity();
    const float vertical = velocity.y;
    velocity.y = 0.0f;
    if (velocity.length() > 10.0f)
        velocity.set_length(10.0f);
    velocity.y = vertical;
    physics->set_velocity(velocity, false);
    field_4C = physical_interface::calculate_perfect_force_vector(
        physics->get_abs_position(), target, peak, physics->get_gravity_multiplier());
}

void jump_state::initiate_glass_house()
{
    auto *physics = field_30->field_28;
    auto direction = field_30->field_44->field_24 - physics->get_abs_position();
    direction.y = 0.0f;
    direction.normalize();
    if (direction.length2() <= EPSILON)
        direction = ZVEC;
    als::param_list list;
    list.add_param(0x1Bu, direction);
    list.add_param(0x18u, YVEC);
    field_30->field_20->get_als_layer(layer)->set_desired_params(list);
    auto *owner = get_actor();
    if (!owner->has_physical_ifc() || (owner->physical_ifc()->field_C & 0x80000u) == 0) {
        po orientation;
        orientation.set_po(direction, YVEC, physics->get_abs_position());
        entity_set_abs_po(physics->field_C, orientation);
    }
    physics->set_velocity(ZEROVEC, false);
    field_4C = compute_force(direction, YVEC);
    field_4C.x *= 0.1f;
    field_4C.z *= 0.1f;
}

void jump_state::update_jump_capsule()
{
    auto *physics = field_30->field_28;
    const bool floor = (physics->field_1C->field_C & 0x60u) != 0 &&
                       dot(physics->get_last_collision_normal(), -get_actor()->physical_ifc()->field_74) > 0.93f;
    auto *capsule = get_core()->field_70;
    if (!floor || field_30->field_50 == 17 || field_30->field_50 == 18) {
        capsule->set_mode(static_cast<capsule_alter_sys::eAlterMode>(3));
    } else {
        capsule->set_avoid_floor(false);
        capsule->set_mode(static_cast<capsule_alter_sys::eAlterMode>(1));
        const vector3d base{0.0f, get_actor()->m_player_controller->m_hero_type == VENOM ? -0.8f : -0.4f, 0.0f};
        capsule->set_static_capsule(base, vector3d{0.0f, 0.2f, 0.0f}, 0.3f);
    }
}

int jump_state::get_mash_sizeof()
{
    return 164;
}

}  // namespace ai


void __fastcall set_velocity(ai::physics_inode *self, void *, const vector3d *a2, bool a3)
{
    self->field_1C->set_velocity(*a2, a3);

    {
        debug_variable_t v67{"jump_cap_vel", g_jump_cap_vel};
        g_jump_cap_vel = v67;

        debug_variable_t v68{"snow_balling", g_snow_balling};
        g_snow_balling = v68;

        debug_variable_t v88{"base_factor", g_base_factor};
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
