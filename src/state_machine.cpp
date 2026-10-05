#include "state_machine.h"

#include "actor.h"
#include "als_animation_logic_system.h"
#include "als_category.h"
#include "als_state.h"
#include "character_anim_inst.h"
#include "camera.h"
#include "custom_math.h"
#include "fakerootposedesc.h"
#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "layer_state_machine_shared.h"
#include "nal_generic.h"
#include "oldmath_po.h"
#include "memory.h"
#include "param_block.h"
#include "osassert.h"
#include "physical_interface.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cmath>

namespace als {

VALIDATE_SIZE(state_machine, 0x54);

state_machine::state_machine()
{
    this->bind_native_vtable(false);
    this->field_8.clear();
    this->field_14 = {};
    this->curr_req_data.clear();
    this->curr_req_data.field_C.field_0 = nullptr;
    this->shared_portion = nullptr;
    this->m_curr_state = nullptr;
    this->m_prev_state = nullptr;
    this->field_40.field_0 = new param[15];
    this->field_40.field_4 = 0;
    this->field_48.field_8 = nullptr;
}

state_machine::~state_machine()
{
    delete[] this->field_40.field_0;
    this->field_34.field_0.clear();
    this->field_8.requested_params.clear();
}

void state_machine::finalize(bool deallocate)
{
    this->~state_machine();
    if (deallocate) {
        mem_dealloc(this, sizeof(*this));
    }
}

int state_machine::get_pb_int(string_hash key) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(1));
    assert(block != nullptr);
    return block->get_pb_int(key);
}

const char *state_machine::get_pb_fixedstring(string_hash key) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(3));
    assert(block != nullptr);
    return block->get_pb_fixedstring(key);
}

string_hash state_machine::get_optional_pb_hash(
    const string_hash &key, const string_hash &fallback, bool *found) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(2));
    if (found != nullptr) *found = block != nullptr;
    return block != nullptr ? block->get_pb_hash(key) : fallback;
}

const char *state_machine::get_optional_pb_fixedstring(
    const string_hash &key, const char *fallback, bool *found) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(3));
    if (found != nullptr) *found = block != nullptr;
    return block != nullptr ? block->get_pb_fixedstring(key) : fallback;
}

const vector3d *state_machine::get_optional_pb_vector3d(
    const string_hash &key, const vector3d *fallback, bool *found) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(4));
    if (found != nullptr) *found = block != nullptr;
    return block != nullptr ? block->get_pb_vector3d(key) : fallback;
}

const variance_variable<float> *state_machine::get_optional_pb_float_variance(
    const string_hash &key, const variance_variable<float> *fallback, bool *found) const
{
    auto *block = this->find_param_block_with_param(key, static_cast<ai::param_types>(5));
    if (found != nullptr) *found = block != nullptr;
    return block != nullptr ? block->get_pb_float_variance(key) : fallback;
}

static layer_types __fastcall base_layer_id(state_machine *, void *)
{
    return static_cast<layer_types>(0);
}

static layer_types __fastcall additional_layer_id(state_machine *self, void *)
{
    return self->shared_portion->_get_layer_id();
}

static bool __fastcall base_request_satisfied(state_machine *, void *, animation_logic_system *)
{
    return true;
}

static bool __fastcall additional_request_satisfied(state_machine *self, void *, animation_logic_system *)
{
    return self->field_14.m_active && self->field_14.m_trans_succeed;
}

void state_machine::bind_native_vtable(bool layer)
{
    static void *tables[2][33];
    static const bool initialized = [] {
        void *common[] = {
            func_address(&state_machine::get_category_id),
            func_address(&state_machine::get_state_id),
            func_address(&state_machine::set_desired_params),
            func_address(&state_machine::set_desired_param),
            func_address(&state_machine::request_category_transition),
            func_address(&state_machine::is_interruptable),
            func_address(&state_machine::did_transition_succeed),
            func_address(&state_machine::is_request_satisfied),
            func_address(&state_machine::is_active),
            func_address(&state_machine::force_als_state),
            func_address(&state_machine::_kill_layer),
            func_address(&state_machine::does_category_exist),
            func_address(&state_machine::get_pb_float),
            func_address(&state_machine::get_pb_int),
            func_address(&state_machine::_get_pb_hash),
            func_address(&state_machine::get_pb_fixedstring),
            func_address(&state_machine::get_pb_vector3d),
            func_address(&state_machine::get_pb_float_variance),
            func_address(&state_machine::does_parameter_exist),
            func_address(&state_machine::get_parameter_data_type),
            func_address(&state_machine::get_time_to_end_of_anim),
            func_address(&state_machine::get_time_to_signal),
            func_address(&state_machine::is_cat_our_prev_cat),
            func_address(&state_machine::is_requesting_category),
            func_address(&state_machine::finalize),
            func_address(&state_machine::get_optional_pb_float),
            func_address(&state_machine::get_optional_pb_int),
            func_address(&state_machine::get_optional_pb_hash),
            func_address(&state_machine::get_optional_pb_fixedstring),
            func_address(&state_machine::get_optional_pb_vector3d),
            func_address(&state_machine::get_optional_pb_float_variance)};
        for (int i = 0; i < 31; ++i) {
            tables[0][i] = common[i];
            tables[1][i] = common[i];
        }
        tables[0][31] = reinterpret_cast<void *>(&base_layer_id);
        tables[1][31] = reinterpret_cast<void *>(&additional_layer_id);
        tables[0][32] = reinterpret_cast<void *>(&base_request_satisfied);
        tables[1][32] = reinterpret_cast<void *>(&additional_request_satisfied);
        return true;
    }();
    (void)initialized;
    this->m_vtbl = reinterpret_cast<std::intptr_t>(tables[layer ? 1 : 0]);
}

void state_machine::set_anim_handle(animation_controller::anim_ctrl_handle &a2)
{
    this->field_48 = a2;
}

param_node *state_machine::find_external_param(external_parameter_types a2) const
{
    TRACE("als::state_machine::find_external_param");

    if constexpr (1) {
        if (!this->field_8.requested_params.is_empty() && this->is_interruptable()) {
            auto *result = this->field_8.requested_params.field_0;
            while (1) {
                if (result->field_0.field_0 == a2) {
                    return result;
                }

                result = result->field_8;
                if (result == this->field_8.requested_params.field_0) {
                    break;
                }
            }
        }

        if (this->field_34.field_0.is_empty()) {
            return nullptr;
        }

        auto *result = this->field_34.field_0.field_0;
        while (result->field_0.field_0 != a2) {
            result = result->field_8;
            if (result == this->field_34.field_0.field_0) {
                return nullptr;
            }
        }

        return result;

    } else {
        return (param_node *)THISCALL(0x00493660, this, a2);
    }
}

float state_machine::get_internal_param(animation_logic_system *a3, internal_parameter_types a4) const
{
    TRACE("state_machine::get_internal_param");

    if constexpr (STANDALONE_SYSTEM) {
        constexpr float radians_to_degrees = 57.2957763671875f;
        constexpr float degrees_to_radians = 0.01745329238474369f;
        constexpr float minimum_direction_length_squared = 0.000001f;
        const auto vector_param = [this, a3](uint32_t id) {
            return this->get_vector_param(a3, id);
        };


        const auto heading = [a3](const vector3d &direction) {
            constexpr float full_turn = 6.2831854820251465f;
            constexpr float radians_to_degrees = 57.2957763671875f;
            const auto &pose = a3->get_actor()->get_abs_po();
            const double turns = static_cast<double>(
                sub_48A720(dot(pose.get_x_facing(), direction),
                           dot(pose.get_z_facing(), direction))) / full_turn;
            const float stored_turns = static_cast<float>(turns);
            const double angle = (stored_turns - std::floor(turns)) *
                                 full_turn * radians_to_degrees;
            return static_cast<float>(angle > 180.0 ? angle - 360.0 : angle);
        };

        switch (static_cast<int>(a4)) {
        case 91:
            return field_48.is_anim_active() ? field_48.get_anim_total_time_in_sec() : 0.0f;
        case 92:
            return field_48.is_anim_active() ? field_48.get_anim_time_in_sec() : 0.0f;
        case 93:
            return field_48.is_anim_active() ? field_48.get_anim_norm_time() : 0.0f;
        case 94:
            return field_48.is_anim_active()
                       ? field_48.get_anim_time_in_sec() / field_48.get_anim_duration()
                       : 0.0f;
        case 95: {
            const auto forward = vector_param(115);
            const auto desired = vector_param(27);
            const auto up = vector_param(108);
            auto projected = desired - up * dot(up, desired);
            projected.normalize();
            const auto cross = vector3d::cross(forward, projected);
            double angle = std::atan2(cross.length(), dot(projected, forward));
            if (dot(cross, up) <= 0.0f)
                angle = -angle;
            return static_cast<float>(angle * radians_to_degrees);
        }
        case 96: {
            auto up = vector_param(108);
            auto desired_up = vector_param(36);
            up.normalize();
            desired_up.normalize();
            return static_cast<float>(
                std::asin(vector3d::cross(desired_up, up).length()) * radians_to_degrees);
        }
        case 97: {
            const float angle = get_internal_param(a3, static_cast<internal_parameter_types>(96)) *
                                degrees_to_radians;
            const auto desired_up = vector_param(36);
            const auto forward = vector_param(115);

            return (dot(desired_up, forward) < 0.0f ? -angle : angle) * radians_to_degrees;
        }
        case 98:


            break;
        case 99: {
            const auto line_direction = vector_param(48);
            if (line_direction.length2() < minimum_direction_length_squared)
                return -1.0f;
            auto *actor = a3->get_actor();
            const auto feet = actor->get_abs_position() -
                              vector_param(108) * actor->get_floor_offset();
            const auto line_origin = vector_param(45);
            const float parameter =
                closest_point_infinite_line_point(line_origin, line_direction, feet);
            return (feet - (line_direction * parameter + line_origin)).length();
        }
        case 100: {
            const auto line_direction = vector_param(48);
            if (line_direction.length2() < minimum_direction_length_squared)
                return -1000.0f;
            auto forward = vector_param(115);
            auto up = vector_param(108);
            forward.normalize();
            up.normalize();
            const float angle = static_cast<float>(std::asin(dot(forward, line_direction)));
            const auto cross = vector3d::cross(forward, up);

            return (dot(line_direction, cross) > 0.0f ? -angle : angle) * radians_to_degrees;
        }
        case 101:
        case 102:
            return heading(vector_param(58));
        case 103: {
            auto *view_camera = g_game_ptr->get_current_view_camera(0);
            if (view_camera == nullptr)
                return 0.0f;
            const auto &pose = a3->get_actor()->get_abs_po();
            return static_cast<float>(calculate_xz_angle_relative_to_local_po(
                       pose, pose.get_z_facing(), view_camera->get_abs_po().get_z_facing()) *
                       radians_to_degrees);
        }
        case 104: {
            if (!field_48.is_anim_active())
                return 0.0f;
            const float remaining = 1.0f - field_48.get_anim_norm_time();
            auto *anim = static_cast<nalAnimClass<nalAnyPose> *>(field_48.get_anim_ptr());
            return anim != nullptr ? remaining * anim->field_38 * 30.0f : remaining * 30.0f;
        }
        case 105:
            return a3->get_actor()->get_abs_position().x;
        case 106:
        case 107:

            return a3->get_actor()->get_abs_position().y;
        case 108:
        case 109:
        case 110:
            return a3->get_actor()->get_abs_po().get_y_facing()[static_cast<int>(a4) - 108];
        case 111:
        case 112:
        case 113:
            return a3->get_actor()->get_abs_po().get_x_facing()[static_cast<int>(a4) - 111];
        case 114: {
            auto up = vector_param(108);
            auto desired = vector_param(27);
            up.normalize();
            desired.normalize();

            const float product = up.y * desired.y;
            if (std::fabs(product) < EPSILON)
                return 0.0f;
            return static_cast<float>(90.0 - std::acos(product) * radians_to_degrees);
        }
        case 115:
        case 116:
        case 117:
            return a3->get_actor()->get_abs_po().get_z_facing()[static_cast<int>(a4) - 115];
        case 118:
        case 119:
        case 120:
            return a3->get_actor()->physical_ifc()->get_velocity()[static_cast<int>(a4) - 118];
        case 121: {
            auto *actor = a3->get_actor();
            if (!actor->has_physical_ifc())
                return 0.0f;
            auto *physical = actor->physical_ifc();
            const float height = physical->calc_height_above_ground();
            return std::equal_to<float>{}(height, -10000.0f) ? physical->field_F0 : height;
        }
        case 122: {
            auto *actor = a3->get_actor();
            if (!actor->has_physical_ifc())
                return 0.0f;
            auto *physical = actor->physical_ifc();
            const float height = physical->calc_height_above_ground();
            if (height <= 0.0f)
                return 0.0f;


            const float velocity = physical->get_velocity().y;
            const float acceleration =
                g_gravity * physical->m_gravity_multiplier * -0.5f;
            if (std::equal_to<float>{}(acceleration, 0.0f))
                return -1.0f;
            const double discriminant = static_cast<double>(velocity) * velocity -
                                        static_cast<double>(acceleration) * height * 4.0;
            if (discriminant < 0.0)
                return -1.0f;
            const double root = std::sqrt(discriminant);
            const double denominator = static_cast<double>(acceleration) * 2.0;
            const float first = static_cast<float>((-velocity - root) / denominator);
            const float second = static_cast<float>((root - velocity) / denominator);
            if (first >= second) {
                if (second < 0.0f)
                    return first;
            } else if (first >= 0.0f) {
                return first;
            }
            return second;
        }
        case 124:
            return heading(-vector_param(55));
        case 125:
            return heading(vector_param(30));
        case 126:
            return (vector_param(33) - a3->get_actor()->get_abs_position()).length();
        case 127:
            return heading(vector_param(27));
        case 128: {
            const float angle =
                get_internal_param(a3, static_cast<internal_parameter_types>(127));
            if (angle >= -45.0f && angle < 45.0f)
                return 0.0f;
            if (angle >= 45.0f)
                return angle < 135.0f ? 1.0f : 2.0f;
            if (angle >= 135.0f || angle < -135.0f)
                return 2.0f;
            return angle >= -45.0f ? 0.0f : 3.0f;
        }
        case 129: {
            auto *physical = a3->get_actor()->physical_ifc();
            return physical != nullptr && physical->is_flag(0x80000u) &&
                   physical->is_biped_stable() ? 1.0f : 0.0f;
        }
        case 123:
        default:
            return 0.0f;
        }
    }

    float(__fastcall * func)(const void *, void *, animation_logic_system *, internal_parameter_types) =
        CAST(func, 0x0049CFD0);
    return func(this, nullptr, a3, a4);
}

bool state_machine::is_curr_state_interruptable(animation_logic_system *a2) const
{
    auto func = [](const auto *self) {
        return !self->is_flag_set(static_cast<state_flags>(1));
    };

    return a2->sub_49F2A0() && func(this->m_curr_state);
}

bool state_machine::has_ext_param_been_set(uint32_t a2) const
{
    return this->find_external_param(static_cast<external_parameter_types>(a2)) != nullptr;
}

ai::param_block *state_machine::find_param_block_with_param(string_hash a2) const
{
    if (this->is_active()) {
        ai::param_block *v4 = this->m_curr_state->field_10;
        if (v4 != nullptr && v4->does_parameter_exist(a2)) {
            return v4;
        } else {
            auto *curr_category = this->get_curr_category();
            auto *v4 = curr_category->field_C;

            if (v4 != nullptr && v4->does_parameter_exist(a2)) {
                return v4;
            }
        }
    }

    return nullptr;
}

ai::param_block *state_machine::find_param_block_with_param(string_hash a2, ai::param_types a3) const
{
    if (this->is_active()) {
        ai::param_block *v5 = this->m_curr_state->field_10;
        if (v5 != nullptr && v5->does_parameter_exist(a2) && v5->get_parameter_data_type(a2) == a3) {
            return v5;
        } else {
            ai::param_block *v5 = this->get_curr_category()->field_C;

            if (v5 != nullptr && v5->does_parameter_exist(a2) && v5->get_parameter_data_type(a2) == a3) {
                return v5;
            }
        }
    }

    return nullptr;
}

float state_machine::get_param(animation_logic_system *a2, uint32_t a3) const
{
    TRACE("als::state_machine::get_param");

    if constexpr (1) {
        auto func = [](const param_cache &self, int a2) -> int {
            int i;
            auto num_params_in_cache = self.get_num_params_in_cache();
            for (i = 0; i < num_params_in_cache; ++i) {
                if (self.field_0[i].field_0 == a2) {
                    return i;
                }
            }

            return -1;
        };

        auto v5 = func(this->field_40, a3);
        if (v5 != -1) {
            return this->field_40.get_from_cache(v5);
        }

        if (a3 < 91) {
            auto *external_param = this->find_external_param((external_parameter_types)a3);
            if (external_param != nullptr) {
                return bit_cast<param_cache *>(&this->field_40)->cache_param(a3, external_param->field_0.field_4);
            } else {
                return 0.0;
            }
        } else {
            auto internal_param = this->get_internal_param(a2, static_cast<internal_parameter_types>(a3));
            return bit_cast<param_cache *>(&this->field_40)->cache_param(a3, internal_param);
        }

    } else {
        float(__fastcall * func)(const void *, void *, animation_logic_system *, uint32_t) = CAST(func, 0x0049FB00);
        return func(this, nullptr, a2, a3);
    }
}

vector3d state_machine::get_vector_param(animation_logic_system *a2, uint32_t a3) const
{
    if (a3 >= 91) {
        vector3d result;
        switch (a3) {
        case 108u:
        case 109u:
        case 110u: {
            auto *v9 = a2->get_actor();
            result = v9->get_abs_po().get_y_facing();
            break;
        }
        case 111u:
        case 112u:
        case 113u: {
            auto *v11 = a2->get_actor();
            result = v11->get_abs_po().get_x_facing();
            break;
        }
        case 115u:
        case 116u:
        case 117u: {
            auto *v10 = a2->get_actor();
            result = v10->get_abs_po().get_z_facing();
            break;
        }
        default:
            assert("Internal Vector parameter has not been added to get_vector_param");
            result = vector3d{0.0, 0.0, 0.0};
            break;
        }

        return result;
    }

    auto a3a = this->get_param(a2, a3 + 2);
    auto a2a = this->get_param(a2, a3 + 1);
    auto a1a = this->get_param(a2, a3);
    vector3d result{a1a, a2a, a3a};
    return result;
}

bool state_machine::did_do_transition() const
{
    return this->field_14.m_trans_succeed;
}

void state_machine::request_category_transition(string_hash a2)
{
    if (!this->field_8.is_force_state) {
        this->field_8.is_request_or_force = true;
        this->field_8.m_cat_id = a2;
    }
}

bool state_machine::is_interruptable() const
{
    return this->field_14.m_curr_state_interruptable || !this->field_14.m_active;
}

bool state_machine::did_transition_succeed() const
{
    TRACE("als::state_machine::did_transition_succeed");

    return this->field_14.m_trans_succeed;
}

bool state_machine::is_request_satisfied() const
{
    TRACE("als::state_machine::is_request_satisfied");

    if constexpr (1) {
        return !this->field_14.m_request_not_satisfied;
    } else {
        bool(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
        return func(this);
    }
}

bool state_machine::is_active() const
{
    return this->field_14.m_active;
}

void state_machine::kill_layer()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x28));
    func(this);
}

void state_machine::_kill_layer()
{
    this->field_8.is_set_kill = true;
}

void state_machine::force_als_state(string_hash a2, int)
{
    TRACE("als::state_machine::force_als_state");

    this->field_8.is_request_or_force = true;
    this->field_8.is_force_state = true;
    this->field_8.m_cat_id = a2;
}

bool state_machine::does_category_exist(string_hash a2) const
{
    TRACE("als::state_machine::does_category_exist");

    assert(this->shared_portion != nullptr);

    auto &cat_list = this->shared_portion->category_list;
    auto begin = cat_list.m_data;
    auto end = begin + cat_list.size();
    auto it = std::find_if(begin, end, [a2](auto &cat) { return cat->field_4 == a2; });

    return it != end;
}

float state_machine::get_pb_float(string_hash a1) const
{
    auto *the_pblock = this->find_param_block_with_param(a1, static_cast<ai::param_types>(0));
    assert(the_pblock != nullptr && "Asking for a parameter that doesn't exist.");

    return the_pblock->get_pb_float(a1);
}

string_hash state_machine::get_pb_hash(string_hash a3) const
{
    return this->_get_pb_hash(a3);
}

string_hash state_machine::_get_pb_hash(string_hash a3) const
{
    auto *pblock = this->find_param_block_with_param(a3, static_cast<ai::param_types>(2));
    auto a2 = pblock->get_pb_hash(a3);
    return a2;
}

vector3d *state_machine::get_pb_vector3d(string_hash a2) const
{
    auto *the_pblock = this->find_param_block_with_param(a2, static_cast<ai::param_types>(4));
    assert(the_pblock != nullptr && "Asking for a parameter that doesn't exist.");

    return the_pblock->get_pb_vector3d(a2);
}

variance_variable<float> *state_machine::get_pb_float_variance(string_hash a2) const
{
    auto *the_pblock = this->find_param_block_with_param(a2, static_cast<ai::param_types>(5));
    assert(the_pblock != nullptr && "Asking for a parameter that doesn't exist.");

    return the_pblock->get_pb_float_variance(a2);
}

bool state_machine::does_parameter_exist(string_hash a1) const
{
    return this->find_param_block_with_param(a1) != nullptr;
}

int state_machine::get_parameter_data_type(string_hash a2) const
{
    auto *the_param_block = this->find_param_block_with_param(a2);
    if (the_param_block != nullptr) {
        return the_param_block->get_parameter_data_type(a2);
    }

    return 9;
}

float state_machine::get_time_to_end_of_anim() const
{
    const auto &handle = this->get_anim_handle();
    const auto *anim = static_cast<const nalAnimClass<nalAnyPose> *>(handle.get_anim_ptr());
    if ((anim->field_34 & 1) != 0) {
        return -1.0f;
    }
    const float remaining_time = anim->field_38 - handle.get_anim_time_in_sec();
    return remaining_time * (1.0f / handle.get_anim_speed());
}

bool state_machine::is_cat_our_prev_cat(string_hash a2) const
{
    TRACE("als::state_machine::is_cat_our_prev_cat");

    if constexpr (1) {
        if (!this->field_14.m_trans_succeed) {
            return false;
        }

        return a2 == this->field_14.m_cat_id && a2 != this->get_category_id();
    } else {
        bool(__fastcall * func)(const void *, void *, string_hash) = CAST(func, get_vfunc(m_vtbl, 0x58));
        return func(this, nullptr, a2);
    }
}

bool state_machine::is_requesting_category(string_hash a2) const
{
    TRACE("als::state_machine::is_requesting_category");

    if constexpr (1) {
        return (this->field_8.is_request_or_force && this->field_8.m_cat_id == a2);
    } else {
        bool(__fastcall * func)(const void *, void *, string_hash) = CAST(func, get_vfunc(m_vtbl, 0x5C));
        return func(this, nullptr, a2);
    }
}

string_hash state_machine::get_category_id() const
{
    TRACE("als::state_machine::get_category_id");

    if constexpr (1) {
        if (this->is_active()) {
            return this->m_curr_state->get_category_id();
        }

        return string_hash{0};
    } else {
        void(__fastcall * func)(const void *, void *, string_hash *) = CAST(func, get_vfunc(m_vtbl, 0x0));

        string_hash id;
        func(this, nullptr, &id);
        return id;
    }
}

double get_character_time_to_signal(const animation_controller::anim_ctrl_handle &handle, string_hash signal,
                                    bool from_start)
{
    auto *anim = static_cast<nalChar::nalCharAnim *>(handle.get_anim_ptr());
    if (anim == nullptr) {
        return -1.0;
    }
    const bool looping = (anim->field_34 & 1) != 0;
    auto *data = static_cast<const FakerootPoseDesc::PerAnimData *>(
        anim->GetPerAnimDataByName(CharComponentBase::Names::FakerootEntropyCompressed));
    if (data == nullptr) {
        return -1.0;
    }
    const uint32_t frames = static_cast<uint32_t>(anim->GetTotalFrames()) - (looping ? 0u : 1u);
    const float frames_per_second = static_cast<double>(frames) / anim->field_38;
    auto iter = data->GetStartIterator();
    if (!iter.IsIteratorValid()) {
        return -1.0;
    }
    const float current_time = from_start ? 0.0f : handle.get_anim_time_in_sec();
    const int first_frame = static_cast<int>(std::ceil(static_cast<double>(current_time) * frames_per_second));
    if (!from_start) {
        while (!iter.field_8 && iter.GetSignalFrame() < first_frame) {
            ++iter;
        }
        if (iter.field_8 && !looping) {
            return -1.0;
        }
    }
    for (uint32_t i = 0; i < static_cast<uint32_t>(data->numTotalSignals); ++i) {
        if (static_cast<uint32_t>(iter.GetNameOfSignal()) == signal.source_hash_code) {
            const double signal_time = static_cast<double>(iter.GetSignalFrame()) / frames_per_second;
            return signal_time >= current_time ? signal_time - current_time
                                               : signal_time + (static_cast<double>(anim->field_38) - current_time);
        }
        ++iter;
        if (iter.field_8 && !looping) {
            return -1.0;
        }
    }
    return -1.0;
}

double get_generic_time_to_signal(const animation_controller::anim_ctrl_handle &a1, string_hash a2, bool a3)
{
    TRACE("get_generic_time_to_signal");

    double(__cdecl * func)(const animation_controller::anim_ctrl_handle *, string_hash, bool) = CAST(func, 0x0049DF60);
    return func(&a1, a2, a3);
}


float state_machine::get_time_to_signal(string_hash a2)
{
    TRACE("als::state_machine::get_time_to_signal");

    if constexpr (1) {
        auto &the_handle = this->get_anim_handle();
        assert(the_handle.is_anim_active());

        float time_to_signal = (the_handle.is_same_animtype(tlFixedString{"Character"})
                                    ? get_character_time_to_signal(the_handle, a2, false)
                                    : get_generic_time_to_signal(the_handle, a2, false));

        auto result = (time_to_signal < 0.0f ? time_to_signal : time_to_signal * (1.0f / the_handle.get_anim_speed()));
        return result;
    } else {
        float(__fastcall * func)(void *, void *, string_hash) = CAST(func, 0x0049F4E0);
        auto result = func(this, nullptr, a2);

        return result;
    }
}

void state_machine::set_desired_params(param_list &a2)
{
    param_list &v2 = this->field_8.requested_params;
    v2.concat_list(a2);
    v2.cull_duplicates_keep_last();
}

void state_machine::set_desired_param(const param &a2)
{
    if constexpr (1) {
        param_list v8{};
        auto &v7 = a2;

        v8.add_param(v7);

        auto *v4 = &this->field_8.requested_params;
        v4->concat_list(v8);

        v4->cull_duplicates_keep_last();

        v8.clear();
    } else {
        THISCALL(0x004A6A80, this, &a2);
    }
}

string_hash state_machine::get_state_id() const
{
    TRACE("als::state_machine::get_state_id");

    if constexpr (1) {
        if (this->is_active()) {
            auto *curr_state = this->get_curr_state();
            if (curr_state->is_flag_set(static_cast<state_flags>(0x1000))) {
                return curr_state->get_state_id();
            }

            return string_hash{0};
        } else {
            return string_hash{0};
        }

    } else {
        void(__fastcall * func)(const void *, void *, string_hash *) = CAST(func, get_vfunc(this->m_vtbl, 0x4));

        string_hash id;
        func(this, nullptr, &id);
        return id;
    }
}

float state_machine::get_optional_pb_float(const string_hash &a2, Float a3, bool *a4) const
{
    auto *the_param_block = this->find_param_block_with_param(a2, static_cast<ai::param_types>(0));
    if (a4 != nullptr) {
        *a4 = (the_param_block != nullptr);
    }

    if (the_param_block != nullptr) {
        return the_param_block->get_pb_float(a2);
    }

    return a3;
}

int state_machine::get_optional_pb_int(const string_hash &a2, int a3, bool *a4)
{
    auto *block = this->find_param_block_with_param(a2, static_cast<ai::param_types>(1));
    if (a4 != nullptr) {
        *a4 = block != nullptr;
    }
    return block != nullptr ? block->get_pb_int(a2) : a3;
}

layer_types state_machine::get_layer_id()
{
    layer_types(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x7C));
    return func(this);
}

bool state_machine::determine_if_request_satisfied(animation_logic_system *a2) const
{
    bool(__fastcall * func)(const void *, void *, animation_logic_system *) = CAST(func, get_vfunc(m_vtbl, 0x80));
    return func(this, nullptr, a2);
}

state *state_machine::get_curr_state() const
{
    return m_curr_state;
}

void state_machine::process_requests(animation_logic_system *a2)
{
    TRACE("als::state_machine::process_requests");

    if constexpr (1) {
        this->curr_req_data.clear();
        this->field_40.clear_cache();
        if (this->field_8.is_set_kill) {
            layer_types v4 = this->get_layer_id();
            a2->sub_4A6630(v4);
            this->field_34.clear();
        } else {
            if (this->field_8.is_request_or_force) {
                if (this->field_8.is_force_state) {
                    this->do_force_state_trans(a2);
                } else {
                    auto *cat = this->find_category(this->field_8.m_cat_id);
                    if (cat != nullptr) {
                        if (cat->is_flag_set(1u)) {
                            this->do_cat_force_trans(a2);
                        } else {
                            this->do_explicit_trans(a2);
                        }
                    } else {
                        this->do_implicit_trans(a2);
                    }
                }
            } else if (this->is_active()) {
                this->do_implicit_trans(a2);
            }
        }
    } else {
        THISCALL(0x004A6BA0, this, a2);
    }
}

void state_machine::process_post_requests(animation_logic_system *a2)
{
    if (this->curr_req_data.field_C.field_0 != nullptr) {
        if (this->curr_req_data.post_req_for_category) {
            auto *v3 = this->m_prev_state;
            auto v5 = v3->m_cat_id;
            auto *category = this->find_category(v5);
            category->do_post_trans(a2, this, this->curr_req_data.field_C);
        } else {
            this->m_prev_state->do_post_trans(a2, this, this->curr_req_data.field_C);
        }
    }
}

void state_machine::process_layer_response_rules(als::animation_logic_system *a2)
{
    auto v9 = this->m_curr_state->do_layer_trans(a2, this);
    if (!v9.did_transition_occur && v9.ignore_no_transition) {
        auto v8 = this->get_category_id();
        auto *the_category = this->find_category(v8);
        auto v6 = the_category->do_layer_trans(a2, this);
        v9 = v6;
    }

    if (v9.did_transition_occur) {
        auto *the_state = this->find_state(v9.field_8);
        this->change_state(a2, the_state);
    }
}

void state_machine::do_force_state_trans(animation_logic_system *a2)
{
    this->curr_req_data.clear();
    this->curr_req_data.did_transition_occur = true;
    this->curr_req_data.field_C = {};
    this->curr_req_data.field_8 = this->field_8.m_cat_id;
    this->change_state(a2, this->find_state(this->field_8.m_cat_id));
    this->field_34.clear();
}

void state_machine::do_cat_force_trans(animation_logic_system *a2)
{
    auto *cat = this->find_category(this->field_8.m_cat_id);
    request_data result;
    void(__fastcall *select_state)(category *, void *, request_data *, animation_logic_system *,
        state_machine *, state *) = CAST(select_state, get_vfunc(cat->m_vtbl, 0x2C));
    select_state(cat, nullptr, &result, a2, this, this->m_curr_state);
    this->curr_req_data = result;
    this->change_state(a2, this->find_state(result.field_8));
    this->field_14.m_trans_succeed = true;
    this->field_14.m_active = true;
    this->field_34.clear();
}

bool category_binary_search(string_hash key, category **data, int size, int *found_idx)
{
    int first = 0;
    int last = size;
    while (first < last) {
        const int middle = (first + last) / 2;
        const auto hash = data[middle]->field_4;
        if (key < hash) {
            last = middle;
        } else if (hash < key) {
            first = middle + 1;
        } else {
            if (found_idx != nullptr) {
                *found_idx = middle;
            }
            return true;
        }
    }
    if (found_idx != nullptr) {
        *found_idx = last;
    }
    return false;
}

category *state_machine::find_category(string_hash a2) const
{
    assert(this->shared_portion != nullptr);

    auto size = this->shared_portion->category_list.size();
    auto **data = this->shared_portion->category_list.m_data;
    int v8;
    if (category_binary_search(a2, data, size, &v8)) {
        return this->shared_portion->category_list.at(v8);
    }

    auto *v4 = a2.to_string();
    sp_log("Could not find category (%s).", v4);
    return nullptr;
}

category *state_machine::get_curr_category() const
{
    auto *curr_state = this->m_curr_state;
    auto v4 = curr_state->get_category_id();
    return this->find_category(v4);
}

scripted_trans_group *state_machine::get_trans_group(int idx) const
{
    return bit_cast<scripted_trans_group *>(this->shared_portion->trans_group_list.at(idx));
}

void state_machine::set_active(animation_logic_system *a2, string_hash a3)
{
    TRACE("als::state_machine::set_active");

    this->field_14.m_active = true;
    auto *the_state = this->find_state(a3);
    this->change_state(a2, the_state);
    this->field_14.m_trans_succeed = true;
    this->field_34.clear();
}

void state_machine::set_pending_params(param_list &params)
{
    auto &pending = this->field_34.field_0;
    auto take_front = [&params]() {
        auto *node = params.field_0;
        if (node->field_8 == node) {
            params.field_0 = nullptr;
        } else {
            params.field_0 = node->field_8;
            node->field_C->field_8 = node->field_8;
            node->field_8->field_C = node->field_C;
        }
        return node;
    };
    if (params.is_empty()) {
        return;
    }
    if (pending.is_empty()) {
        pending.insert_node(take_front());
    }
    auto *cursor = pending.field_0;
    while (!params.is_empty()) {
        auto *node = take_front();
        auto *start = cursor;
        do {
            if (cursor->field_0.field_0 == node->field_0.field_0) {
                break;
            }
            cursor = cursor->field_8;
        } while (cursor != start);
        if (cursor->field_0.field_0 == node->field_0.field_0) {
            cursor->field_0 = node->field_0;
            mem_dealloc(node, sizeof(*node));
            cursor = cursor->field_8;
        } else {
            pending.insert_node(node);
        }
    }
}

void state_machine::update_pending_params(animation_logic_system *a2)
{
    TRACE("state_machine::update_pending_params");

    this->field_14.m_request_not_satisfied = !this->determine_if_request_satisfied(a2);
    auto v5 = this->field_14.m_curr_state_interruptable;
    if (this->field_14.m_active) {
        this->field_14.m_curr_state_interruptable = this->is_curr_state_interruptable(a2);
    }

    if (v5 || this->field_14.m_curr_state_interruptable) {
        this->set_pending_params(this->field_8.requested_params);
    }

    this->field_8.clear();

    this->field_40.clear_cache();
}

void state_machine::do_implicit_trans(animation_logic_system *a2)
{
    TRACE("als::state_machine::do_implicit_trans");

    if constexpr (1) {
        this->curr_req_data = this->m_curr_state->do_implicit_trans(a2, this);

        if (this->curr_req_data.did_rule_pass() || !this->curr_req_data.ignore_no_transition) {
            if (this->curr_req_data.did_rule_pass()) {
                if (this->curr_req_data.is_trans_to_category) {
                    auto *cat = this->find_category(this->curr_req_data.field_8);
                    this->curr_req_data = cat->do_incoming_trans(a2, this);
                    if (this->curr_req_data.did_rule_pass()) {
                        auto *the_state = this->find_state(this->curr_req_data.field_8);
                        this->change_state(a2, the_state);
                    } else {
                        assert(0 && "Implicit transition to category specified failed.");
                    }
                } else {
                    auto *the_state = this->find_state(this->curr_req_data.field_8);
                    this->change_state(a2, the_state);
                }
            } else {
                this->field_14.m_trans_succeed = false;
            }

        } else {
            auto v16 = this->m_curr_state->get_category_id();
            auto *v6 = this->find_category(v16);
            this->curr_req_data = v6->do_implicit_trans(a2, this);
            if (this->curr_req_data.did_rule_pass()) {
                if (this->curr_req_data.is_trans_to_category) {
                    auto *v10 = this->find_category(this->curr_req_data.field_8);
                    this->curr_req_data = v10->do_incoming_trans(a2, this);
                    if (this->curr_req_data.did_rule_pass()) {
                        auto *the_state = this->find_state(this->curr_req_data.field_8);
                        this->change_state(a2, the_state);
                    } else {
                        assert(0 && "Implicit transition to category specified failed.");
                    }
                } else {

                    auto *the_state = this->find_state(this->curr_req_data.field_8);
                    this->change_state(a2, the_state);
                }
            } else {
                this->field_14.m_trans_succeed = false;
            }
        }

    } else {
        THISCALL(0x0049CD30, this, a2);
    }
}

void state_machine::do_explicit_trans(animation_logic_system *a2)
{
    TRACE("als::state_machine::do_explicit_trans");

    if constexpr (1) {
        if (!this->is_active()) {
            auto v4 = this->field_8.m_cat_id;
            auto *the_category = this->find_category(v4);
            auto dst_state = the_category->get_default_state();

            assert(this->does_state_exist(dst_state) && "This category does not have a start state set");

            auto *the_state = this->find_state(dst_state);
            this->change_state(a2, the_state);
            this->curr_req_data.clear();
            this->curr_req_data.did_transition_occur = true;
            this->curr_req_data.field_C = {};
            this->curr_req_data.post_req_for_category = false;
            this->curr_req_data.field_8 = dst_state;
            return;
        }

        if (this->is_interruptable()) {
            if (auto *the_state = this->get_curr_state(); (the_state->field_C & 0x400) == 0) {
                auto v13 = this->field_8.m_cat_id;
                if (this->get_category_id() == v13) {
                    this->curr_req_data.clear();
                    return;
                }
            }

            this->curr_req_data = this->m_curr_state->do_explicit_trans(a2, this, this->field_8.m_cat_id);
            if (this->curr_req_data.did_rule_pass() || !this->curr_req_data.ignore_no_transition) {
                if (this->curr_req_data.did_rule_pass()) {
                    if (this->curr_req_data.is_trans_to_category) {
                        auto *v24 = this->find_category(this->curr_req_data.field_8);
                        this->curr_req_data = v24->do_incoming_trans(a2, this);
                        if (this->curr_req_data.did_rule_pass()) {
                            auto *v27 = this->find_state(this->curr_req_data.field_8);
                            this->change_state(a2, v27);
                        } else {
                            assert(0 && "Explicit transition to category specified failed.");
                        }
                    } else {
                        auto *v27 = this->find_state(this->curr_req_data.field_8);
                        this->change_state(a2, v27);
                    }
                } else {
                    this->field_14.m_trans_succeed = false;
                }
            } else {
                auto v34 = this->m_curr_state->m_cat_id;
                auto *v19 = this->find_category(v34);
                this->curr_req_data = v19->do_explicit_trans(a2, this, this->field_8.m_cat_id);
                if (this->curr_req_data.did_rule_pass()) {
                    if (this->curr_req_data.is_trans_to_category) {
                        auto *v24 = this->find_category(this->curr_req_data.field_8);
                        this->curr_req_data = v24->do_incoming_trans(a2, this);
                        if (this->curr_req_data.did_rule_pass()) {
                            auto *v27 = this->find_state(this->curr_req_data.field_8);
                            this->change_state(a2, v27);
                        } else {
                            assert(0 && "Explicit transition to category specified failed.");
                        }
                    } else {
                        auto *v27 = this->find_state(this->curr_req_data.field_8);
                        this->change_state(a2, v27);
                    }
                } else {
                    this->field_14.m_trans_succeed = false;
                }
            }
        }

        if (this->field_14.m_trans_succeed) {
            auto func = [](auto *a1) -> void {
                a1->field_0.clear();
                a1->field_4 = string_hash{0};
            };
            func(&this->field_34);
            return;
        }

        auto *v29 = this->find_category(this->field_8.m_cat_id);
        this->curr_req_data = v29->do_incoming_trans(a2, this);
        if (this->curr_req_data.did_rule_pass()) {
            auto *v32 = this->find_state(this->curr_req_data.field_8);
            this->change_state(a2, v32);

            auto func = [](auto *a1) -> void {
                a1->field_0.clear();
                a1->field_4 = string_hash{0};
            };
            func(&this->field_34);
            return;
        }

        this->field_14.m_trans_succeed = false;
        this->do_implicit_trans(a2);
    } else {
        THISCALL(0x0049F5A0, this, a2);
    }
}

bool state_machine::does_state_exist(string_hash a1) const
{
    assert(this->shared_portion != nullptr);

    for (int i = 0; i < this->shared_portion->state_list.size(); ++i) {
        auto v3 = this->shared_portion->state_list.at(i);
        auto v5 = v3->m_state_id;
        if (v5 == a1) {
            return true;
        }
    }

    return false;
}

state *state_machine::find_state(string_hash a2) const
{
    TRACE("als::state_machine::find_state", a2.to_string());

    assert(this->shared_portion != nullptr);

    auto &state_list = this->shared_portion->state_list;
    auto begin = state_list.m_data;
    auto end = begin + state_list.size();
    auto it = std::find_if(begin, end, [a2](auto *the_state) { return (the_state->get_state_id() == a2); });

    if (it != end) {
        return (*it);
    }

    auto *v5 = a2.to_string();
    error("Could not find state (%s).", v5);
    return nullptr;
}

void state_machine::change_state(animation_logic_system *a2, state *a3)
{
    TRACE("als::state_machine::change_state");

    if (a2 != nullptr) {}

    if (!this->field_14.m_trans_succeed) {
        this->m_prev_state = this->m_curr_state;
    }

    this->m_curr_state = a3;
    this->field_14.m_trans_succeed = true;
    this->field_14.m_active = (this->m_curr_state != nullptr);
    auto *prev_state = this->m_prev_state;
    this->field_14.m_cat_id = (prev_state != nullptr ? prev_state->get_category_id() : string_hash{0});
}

}  // namespace als


string_hash *als_state_machine_get_state_id(als::state_machine *self, void *, string_hash *out)
{
    *out = self->get_state_id();
    return out;
}

void als_state_machine_patch()
{
    auto REPLACE = [](auto addr, auto func) -> void {
        FUNC_ADDRESS(address, func);
        SET_JUMP(addr, address);
    };

    REPLACE(0x0049FB00, &als::state_machine::get_param);

    REPLACE(0x00493850, &als::state_machine::change_state);

    REPLACE(0x0049F9B0, &als::state_machine::set_active);

    REPLACE(0x004934F0, &als::state_machine::did_transition_succeed);

    if constexpr (0) {
        auto address = int(&als_state_machine_get_state_id);
        SET_JUMP(0x00499320, address);
    }

    REPLACE(0x00493500, &als::state_machine::is_request_satisfied);

    {
        FUNC_ADDRESS(address, &als::state_machine::find_external_param);
        REDIRECT(0x0049FB7E, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::get_internal_param);
        REDIRECT(0x0049FB39, address);
        REDIRECT(0x0049D3CD, address);
        REDIRECT(0x0049DCBD, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::get_time_to_end_of_anim);
        set_vfunc(0x00881428, address);
        set_vfunc(0x00881500, address);
        set_vfunc(0x00881588, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::process_requests);
        REDIRECT(0x004A910D, address);
    }

    return;

    {
        FUNC_ADDRESS(address, &als::state_machine::get_time_to_signal);
        set_vfunc(0x0088142C, address);
        set_vfunc(0x00881504, address);
        set_vfunc(0x0088158C, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::force_als_state);
        SET_JUMP(0x00493520, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::does_category_exist);
        SET_JUMP(0x00499370, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::find_state);
        SET_JUMP(0x004994C0, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::is_cat_our_prev_cat);
        SET_JUMP(0x00493550, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::is_requesting_category);
        SET_JUMP(0x00493470, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::do_force_state_trans);
        REDIRECT(0x004A6C04, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::do_implicit_trans);
        REDIRECT(0x0049F82D, address);
        REDIRECT(0x004A6C55, address);
        REDIRECT(0x004A6C72, address);
    }

    {
        FUNC_ADDRESS(address, &als::state_machine::do_explicit_trans);
        REDIRECT(0x004A6C43, address);
    }
}
