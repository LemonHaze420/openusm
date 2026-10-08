#include "venom_inode.h"

#include "actor.h"
#include "ai_std_combat_target.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "damage_interface.h"
#include "from_mash_in_place_constructor.h"
#include "native_info_node_table.h"
#include "state_machine.h"
#include "trigger_manager.h"
#include "variable.h"
#include "vtbl.h"
#include "trigger.h"
#include "utility.h"

#include <algorithm>

#include <cmath>
#include <limits>

VALIDATE_SIZE(venom_inode, 0xBC);
VALIDATE_OFFSET(venom_inode, field_98, 0x98);
VALIDATE_OFFSET(venom_inode, field_B8, 0xB8);

namespace {
void __fastcall venom_advance(venom_inode *self, void *, Float elapsed)
{
    self->_frame_advance(elapsed);
}

void __fastcall venom_activate(venom_inode *self, void *, ai::ai_core *core)
{
    self->_activate(core);
}

void __fastcall venom_deactivate(venom_inode *self, void *)
{
    self->_deactivate();
}
}

void *venom_inode::native_vtable()
{
    static auto table = [] {
        ai::native_inode::table<venom_inode, 440> result;
        result[6] = reinterpret_cast<void *>(&ai::native_inode::always_advance);
        result[7] = reinterpret_cast<void *>(&venom_advance);
        result[8] = reinterpret_cast<void *>(&venom_activate);
        result[9] = reinterpret_cast<void *>(&venom_deactivate);
        return result;
    }();
    return table.data();
}

venom_inode::venom_inode(from_mash_in_place_constructor *a2)
    : ai::info_node(a2), base_1(a2), predicted_target_position(a2), field_8C(a2)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    m_vtbl = 0x0087DE38;
#endif
    field_98.field_0 = 0;
    field_9C.field_0 = 0;
    field_A0.field_0 = 0;
    field_A4 = nullptr;
}


void venom_inode::_activate(ai::ai_core *core)
{
    ai::info_node::_activate(core);
    n11 = 0;
    field_20 = 0;
    field_22 = false;
    field_98.field_0 = 0;
    field_9C.field_0 = 0;
    field_A0.field_0 = 0;
    n2 = 0;
    n2_1 = 0;
    n2_2 = 0;
    field_B8 = nullptr;
    var<bool>(0x96BE78) = true;

    auto &params = core->field_50;
    optional_pb_float_1 =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("prediction_time"))}, 0.5f, nullptr);
    optional_pb_float_2 =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("approach_distance"))}, 3.5f, nullptr);
    optional_pb_float_3 =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("damage_dealt_counter"))}, 10.0f, nullptr);
    optional_pb_float_4 =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("damage_received_counter"))}, 10.0f, nullptr);
    optional_pb_float =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("time_to_attack"))}, 0.5f, nullptr);
    field_38 = 0.0f;
    float_NULL = params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("engage_timer"))}, 0.0f, nullptr);
    float_NULL_1 =
        params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("prop_throw_timeout"))}, 15.0f, nullptr);

    if (params.does_parameter_exist(string_hash{static_cast<int>(to_hash("venom_s01"))})) {
        n11 = 1;
        float_NULL_3 = -1.0f;
    } else if (params.does_parameter_exist(string_hash{static_cast<int>(to_hash("venom_s11"))})) {
        n11 = 11;
        optional_pb_float_5 =
            params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("engage_distance"))}, 35.0f, nullptr);
    } else if (params.does_parameter_exist(string_hash{static_cast<int>(to_hash("venom_s13"))})) {
        n11 = 13;
        const char *name = params.get_pb_fixedstring(string_hash{static_cast<int>(to_hash("arena_trigger"))});
        if (name && *name)
            field_A8 = trigger_manager::instance->find_instance(mString{name});
        name = params.get_pb_fixedstring(string_hash{static_cast<int>(to_hash("fire1_trigger"))});
        if (name && *name)
            field_AC = trigger_manager::instance->find_instance(mString{name});
        name = params.get_pb_fixedstring(string_hash{static_cast<int>(to_hash("fire2_trigger"))});
        if (name && *name)
            field_B0 = trigger_manager::instance->find_instance(mString{name});
        name = params.get_pb_fixedstring(string_hash{static_cast<int>(to_hash("platform_trigger"))});
        if (name && *name)
            field_B4 = trigger_manager::instance->find_instance(mString{name});
    }
    field_8C = {-134.0f, 182.3f, 1342.8f};
}


void venom_inode::_deactivate()
{
    var<bool>(0x96BE78) = false;
}


bool venom_inode::is_sable_in_engage_range() const
{
    auto *sable = field_9C.get_volatile_ptr();
    if (!sable)
        return true;
    const vector3d position = field_C->get_abs_position();
    const vector3d sable_position = sable->get_abs_position();
    const double x = double(position.x) - sable_position.x;
    const double y = double(position.y) - sable_position.y;
    const double z = double(position.z) - sable_position.z;
    return !(x * x + y * y + z * z > optional_pb_float_5);
}


void venom_inode::_frame_advance(Float elapsed)
{
    float_NULL_1 -= elapsed;
    optional_pb_float -= elapsed;
    auto *target_node =
        static_cast<ai::combat_target_inode *>(field_8->get_info_node(ai::combat_target_inode::default_id, true));
    vhandle_type<actor> target_handle;
    using target_fn = vhandle_type<actor> *(__fastcall *)(ai::combat_target_inode *, void *, vhandle_type<actor> *);
    reinterpret_cast<target_fn>(get_vfunc(target_node->m_vtbl, 0x38))(target_node, nullptr, &target_handle);
    auto *target = target_handle.get_volatile_ptr();
    const vector3d position = field_C->get_abs_position();
    var<vector3d>(0x96BFF8) = {position.x, position.y + 1.0f, position.z};
    if (target) {
        base_1 = target->get_abs_position();
        const double x = double(position.x) - base_1.x;
        const double y = double(position.y) - base_1.y;
        const double z = double(position.z) - base_1.z;
        float_NULL_2 = static_cast<float>(x * x + y * y + z * z);
        const vector3d velocity = target->get_velocity();
        const float predicted_velocity_x = optional_pb_float_1 * velocity.x;
        const float predicted_velocity_y = optional_pb_float_1 * velocity.y;
        predicted_target_position = {static_cast<float>(double(predicted_velocity_x) + base_1.x),
                                     static_cast<float>(double(predicted_velocity_y) + base_1.y),
                                     static_cast<float>(double(optional_pb_float_1) * velocity.z + base_1.z)};
        const double predicted_x = double(position.x) - predicted_target_position.x;
        const double predicted_y = double(position.y) - predicted_target_position.y;
        const double predicted_z = double(position.z) - predicted_target_position.z;
        a3b = static_cast<float>(predicted_x * predicted_x + predicted_y * predicted_y + predicted_z * predicted_z);
        field_68 = base_1.y - position.y;
    }

    auto &params = field_8->field_50;
    if (n11 == 1) {
        if (!field_B8) {
            const char *name = params.get_optional_pb_fixedstring(
                string_hash{static_cast<int>(to_hash("spark_trigger"))}, nullptr, nullptr);
            if (name)
                field_B8 = trigger_manager::instance->find_instance(mString{name});
        }
        if (float_NULL_3 < 0.0f) {
            const float health = field_C->damage_ifc()->field_1FC.field_0[0];
            float_NULL_3 =
                health - params.get_optional_pb_float(string_hash{static_cast<int>(to_hash("knock_down_hits"))},
                                                      std::numeric_limits<float>::max(),
                                                      nullptr);
        }
        const vector3d current_position = field_C->get_abs_position();
        const float height = static_cast<float>(std::fabs(double(base_1.y) - current_position.y));
        const float allowed_height =
            params.get_pb_int(string_hash{static_cast<int>(to_hash("venom_s08"))}) == 1 ? 8.0f : 6.0f;
        if (!(height <= allowed_height))
            float_NULL -= elapsed;
    }

    if (n11 == 11) {
        if (!field_98.get_volatile_ptr()) {
            auto *hero =
                entity_handle_manager::find_entity(string_hash{static_cast<int>(to_hash("hero"))}, IGNORE_FLAVOR, true);
            field_98 = hero ? hero->my_handle : INVALID_HANDLE;
        }
        if (field_9C.get_volatile_ptr()) {
            if (!is_sable_in_engage_range())
                float_NULL -= elapsed;
        } else {
            const char *name = params.get_optional_pb_fixedstring(
                string_hash{static_cast<int>(to_hash("sable_entity"))}, nullptr, nullptr);
            if (name) {
                auto *sable = entity_handle_manager::find_entity(string_hash{name}, IGNORE_FLAVOR, true);
                field_9C = sable ? sable->my_handle : INVALID_HANDLE;
            }
        }
    }

    if (n11 == 13) {
        auto *damage = field_C->damage_ifc();
        const double health_percent = double(damage->field_1FC.field_0[0]) / damage->field_1FC.field_0[2] * 100.0;
        n2 = health_percent > 66.0 ? 1 : health_percent > 33.0 ? 2 : 3;
        if (field_21) {
            auto *als_node = static_cast<ai::als_inode *>(field_8->get_info_node(ai::als_inode::default_id, true));
            als_node->get_als_layer(static_cast<als::layer_types>(0))->set_desired_param({1, 2.0f});
        }
        if (field_A0.get_volatile_ptr()) {
            field_38 -= elapsed;
        } else {
            const char *name = params.get_pb_fixedstring(string_hash{static_cast<int>(to_hash("traskcopter_entity"))});
            if (name && *name) {
                auto *copter = entity_handle_manager::find_entity(string_hash{name}, IGNORE_FLAVOR, true);
                field_A0 = copter ? copter->my_handle : INVALID_HANDLE;
            }
        }
        if (float_NULL_2 > 625.0f)
            float_NULL -= elapsed;
    }
}


bool venom_inode::accepts_knockdown_attack(string_hash attack)
{
    if (n11 != 1)
        return false;
    if (attack == string_hash{static_cast<int>(to_hash("Punch1"))} ||
        attack == string_hash{static_cast<int>(to_hash("Kick1"))}) {
        n2_2 = 1;
        return false;
    }
    if (attack == string_hash{static_cast<int>(to_hash("Punch2"))} ||
        attack == string_hash{static_cast<int>(to_hash("Kick2"))}) {
        if (n2_2 == 1)
            n2_2 = 2;
        return false;
    }
    if (attack == string_hash{static_cast<int>(to_hash("Punch3"))} ||
        attack == string_hash{static_cast<int>(to_hash("Kick3"))}) {
        if (n2_2 == 2)
            n2_2 = 3;
        return field_8->field_50.get_pb_int(string_hash{static_cast<int>(to_hash("venom_s08"))}) != 1;
    }
    n2_2 = 0;
    return false;
}


void venom_inode::reset_attack_timer()
{
    optional_pb_float =
        field_8->field_50.get_optional_pb_float(string_hash{int(to_hash("time_to_attack"))}, 0.5f, nullptr);
}


bool venom_inode::should_throw_prop()
{
    bool should_throw = n11 == 11 && float_NULL_1 <= 0.0f && float_NULL_2 >= 100.0f && float_NULL_2 < 9025.0f;
    if (n11 == 13) {
        if (n2 == 1) {
            if (float_NULL_1 >= 0.0f) {
                float_NULL_1 = -1.0f;
                return true;
            }
            return false;
        }
        should_throw = float_NULL_1 <= 0.0f && (n2 == 2 ? float_NULL_2 > 100.0f : float_NULL_2 > 225.0f);
    }
    if (should_throw) {
        float_NULL_1 =
            field_8->field_50.get_optional_pb_float(string_hash{int(to_hash("prop_throw_timeout"))}, 15.0f, nullptr);
        return true;
    }
    return false;
}


bool venom_inode::should_chase()
{
    if (n11 == 1 && field_8->field_50.get_pb_int(string_hash{int(to_hash("venom_s08"))}) == 1 &&
        std::fabs(field_C->get_abs_position().y - predicted_target_position.y) < 8.0f)
        float_NULL = std::min(float_NULL + 0.1f, 2.0f);
    return float_NULL <= 0.0f;
}


bool venom_inode::should_jump_attack()
{
    bool no_combat_target = false;
    if (n11 == 13) {
        if (n2 == 1) {
            auto *target = static_cast<ai::combat_target_inode *>(
                field_8->get_info_node(ai::combat_target_inode::default_id, true));
            if (reinterpret_cast<bool(__fastcall *)(ai::combat_target_inode *, void *)>(
                    get_vfunc(target->m_vtbl, 0x6C))(target, nullptr))
                return false;
            no_combat_target = true;
        }
        if (!field_A8->contains(base_1))
            return false;
        const bool far_enough =
            n2 >= 2 && (n2_1 >= 2 ? float_NULL_2 > 100.0f : float_NULL <= 0.0f && float_NULL_2 > 225.0f);
        if (!far_enough && !no_combat_target)
            return false;
    } else if (float_NULL_2 <= 225.0f) {
        return false;
    }
    const float height = field_C->get_abs_position().y;
    const float target_height = base_1.y - height;
    const float predicted_height = predicted_target_position.y - height;
    return target_height >= -2.0f && target_height <= 2.0f && predicted_height >= -2.0f && predicted_height <= 2.0f;
}


void venom_inode::record_target_damage()
{
    const float previous_health = bit_cast<float>(field_6C);
    if (previous_health > 0.0f) {
        auto *node =
            static_cast<ai::combat_target_inode *>(field_8->get_info_node(ai::combat_target_inode::default_id, true));
        vhandle_type<actor> handle;
        reinterpret_cast<vhandle_type<actor> *(__fastcall *)(ai::combat_target_inode *, void *, vhandle_type<actor> *)>(
            get_vfunc(node->m_vtbl, 0x38))(node, nullptr, &handle);
        auto *target = handle.get_volatile_ptr();
        if (target && std::not_equal_to<float>{}(previous_health, target->damage_ifc()->field_1FC.field_0[0]))
            field_22 = true;
    }
    field_6C = 0;
}


bool venom_inode::should_feed() const
{
    if ((n11 == 1 && field_8->field_50.get_pb_int(string_hash{int(to_hash("phase"))}) == 1) || n11 == 13)
        return false;
    const float threshold =
        field_8->field_50.get_optional_pb_float(string_hash{int(to_hash("feed_below_percent"))}, 0.0f, nullptr);
    auto *damage = field_C->damage_ifc();
    return double(damage->field_1FC.field_0[0]) / damage->field_1FC.field_0[2] * 100.0 <= threshold;
}
