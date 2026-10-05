#include "std_fear_inode.h"

#include "common.h"
#include "func_wrapper.h"
#include "base_ai_core.h"
#include "ai_universal_soldier_inode.h"
#include "ai_team.h"
#include <algorithm>
#include <cstdlib>
#include "native_info_node_table.h"
#include "base_ai_state_machine.h"
#include "base_state.h"
#include "damage_interface.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(std_fear_inode, 0x7Cu);

namespace {
void __fastcall native_destruct(std_fear_inode *self, void *) { self->_destruct_mashed_class(); }
bool __fastcall native_needs_advance(std_fear_inode *self, void *) { return self->field_1C; }
void __fastcall native_activate(std_fear_inode *self, void *, ai_core *core) { self->_activate(core); }
void __fastcall native_advance(std_fear_inode *self, void *, Float dt) { self->_frame_advance(dt); }

void set_fear_flag(std_fear_inode *self, const char *name, bool &field, bool value)
{
    self->my_param_block.set_pb_int(string_hash{name}, value, true);
    field = value;
}
}

void *std_fear_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<std_fear_inode, 375> result;
        result[0] = reinterpret_cast<void *>(&native_destruct);
        result[6] = reinterpret_cast<void *>(&native_needs_advance);
        result[7] = reinterpret_cast<void *>(&native_advance);
        result[8] = reinterpret_cast<void *>(&native_activate);
        return result;
    }();
    return table.data();
}

std_fear_inode::~std_fear_inode() { finalize(mash::ALLOCATED); }

void std_fear_inode::finalize(mash::allocation_scope)
{
    remove_from_list(&all_fear_inodes());
    remove_from_list(&cowering_fear_inodes());
    remove_from_list(&fleeing_fear_inodes());
}

void std_fear_inode::_destruct_mashed_class()
{
    finalize(mash::FROM_MASH);
    info_node::_destruct_mashed_class();
}

void std_fear_inode::refresh_parameters()
{
    struct bool_parameter { string_hash name; bool std_fear_inode::*field; int value; };
    static const bool_parameter flags[] = {
        {"cowering_enabled", &std_fear_inode::field_1D, 1},
        {"cowering_forced", &std_fear_inode::field_1E, 0},
        {"cowering_stop_forced", &std_fear_inode::field_1F, 0},
        {"cowering_stop_disabled", &std_fear_inode::field_20, 0},
        {"fleeing_enabled", &std_fear_inode::field_21, 1},
        {"fleeing_forced", &std_fear_inode::field_22, 0},
        {"fleeing_entity_forced", &std_fear_inode::field_23, 0},
        {"fleeing_stop_forced", &std_fear_inode::field_24, 0},
        {"fleeing_stop_disabled", &std_fear_inode::field_25, 0},
        {"flee_to_location", &std_fear_inode::field_26, 0},
        {"can_steal_car_while_fleeing", &std_fear_inode::field_27, 1}
    };
    for (const auto &parameter : flags)
        this->*parameter.field = my_param_block.get_optional_pb_int(
            parameter.name, parameter.value, nullptr) != 0;
    field_28 = reinterpret_cast<std::intptr_t>(my_param_block.get_optional_pb_fixedstring(
        string_hash{"entity_to_flee_from_id"}, "", nullptr));
    field_2C = ZEROVEC;
    if (my_param_block.param_array) {
        if (auto *data = my_param_block.param_array->common_find_data(string_hash{"flee_destination"}))
            field_2C = *data->get_data_vector3d();
    }
    struct float_parameter { string_hash name; float std_fear_inode::*field; float value; };
    static const float_parameter values[] = {
        {"flee_entity_catch_min_time", &std_fear_inode::field_38, -1.0f},
        {"flee_entity_catch_min_dist", &std_fear_inode::field_3C, 0.0f},
        {"fear_decrement_speed", &std_fear_inode::field_40, 0.3f},
        {"fear_decrement_commander_mul", &std_fear_inode::field_44, 1.5f},
        {"internal_fear_mul", &std_fear_inode::field_48, 1.0f},
        {"bravado_increment_speed", &std_fear_inode::field_4C, 0.0f},
        {"bravado_increment_commander_mul", &std_fear_inode::field_50, 1.1f},
        {"internal_bravado_mul", &std_fear_inode::field_54, 1.0f},
        {"morale_break_value", &std_fear_inode::field_58, -0.75f},
        {"morale_break_chance", &std_fear_inode::field_5C, 0.75f},
        {"flee_chance_vs_cower", &std_fear_inode::field_60, 0.1f}
    };
    for (const auto &parameter : values)
        this->*parameter.field = my_param_block.get_optional_pb_float(
            parameter.name, parameter.value, nullptr);
}

void std_fear_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    refresh_parameters();
}

bool std_fear_inode::is_cowering() const
{
    auto *machine = field_8->my_base_machine;
    return machine && machine->my_curr_state && machine->my_curr_state->get_virtual_type_enum() == 374;
}

void std_fear_inode::_frame_advance(Float dt)
{
    if (my_param_block.field_0 >= g_world_ptr->time_manager.field_C - 1)
        refresh_parameters();
    if (field_68.get_volatile_ptr() && !field_68.get_volatile_ptr())
        field_68.field_0 = 0;
    auto *actor = field_8->field_64;
    auto *damage = actor->has_damage_ifc() ? actor->damage_ifc() : nullptr;
    if (damage && (!damage->is_alive() || damage->is_subdued())) {
        field_74 = 0.0f;
        field_70 = 0.0f;
        field_6C = 0.0f;
    } else {
        field_74 -= dt;
        if (field_74 > 0.0f) {
            if (field_68.get_volatile_ptr() && field_23) {
                set_fear_flag(this, "fleeing_forced", field_22, true);
                set_fear_flag(this, "can_steal_car_while_fleeing", field_27, false);
            }
        } else {
            if (field_6C < field_58 - EPSILON && !field_22 && !field_24 &&
                !field_1E && !field_1F && !is_cowering() && !field_64) {
                const float amount = (field_6C - field_58) / (-1.0f - field_58);
                const float chance = amount < 0.0f ? 0.0f : amount * field_5C;
                constexpr float random_scale = 3.0518509447574615e-05f;
                if (std::rand() * random_scale <= chance) {
                    if (field_21 && (!field_1D || std::rand() * random_scale <= field_60))
                        set_fear_flag(this, "fleeing_forced", field_22, true);
                    else if (field_1D)
                        set_fear_flag(this, "cowering_forced", field_1E, true);
                }
            }
            field_74 = 0.25f;
        }
        field_70 -= dt;
        if (field_70 <= 0.0f) {
            field_70 = 0.0f;
            auto *soldier = static_cast<universal_soldier_inode *>(
                field_8->get_info_node(universal_soldier_inode::default_id, false));
            const bool has_commander = soldier && soldier->field_30;
            if (field_6C < 0.0f) {
                const float speed = has_commander ? field_40 * field_44 : field_40;
                field_6C = std::min(0.0f, field_6C + speed * dt);
            } else if (field_4C > EPSILON) {
                const float speed = has_commander ? field_4C * field_50 : field_4C;
                field_6C = std::min(1.0f, field_6C + speed * dt);
            } else if (field_6C > 0.0f) {
                field_6C = 0.0f;
            }
        }
    }
    if (field_64 && !field_79) {
        add_to_list(&fleeing_fear_inodes());
        field_79 = true;
    } else if (!field_64 && field_79) {
        remove_from_list(&fleeing_fear_inodes());
        field_79 = false;
    }
    if (is_cowering()) {
        if (!field_78) {
            add_to_list(&cowering_fear_inodes());
            field_78 = true;
        }
    } else if (field_78) {
        remove_from_list(&cowering_fear_inodes());
        field_78 = false;
    }
}

std_fear_inode::std_fear_inode() : info_node()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[375]);
    this->field_1C = 0;
    this->field_1D = 0;
    this->field_1E = 0;
    this->field_1F = 0;
    this->field_20 = 0;
    this->field_21 = 0;
    this->field_22 = 0;
    this->field_23 = 0;
    this->field_24 = 0;
    this->field_25 = 0;
    this->field_26 = 0;
    this->field_27 = 0;
    this->field_64 = 0;
    this->field_68 = {0};
    this->field_78 = false;
    this->field_79 = false;
    this->initialize(mash::ALLOCATED);
}

std_fear_inode::std_fear_inode(from_mash_in_place_constructor *a2) : info_node(a2), field_2C(a2)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[375]);
    this->field_1C = 0;
    this->field_1D = 0;
    this->field_1E = 0;
    this->field_1F = 0;
    this->field_20 = 0;
    this->field_21 = 0;
    this->field_22 = 0;
    this->field_23 = 0;
    this->field_24 = 0;
    this->field_25 = 0;
    this->field_26 = 0;
    this->field_27 = 0;
    this->field_68 = {0};
    this->initialize(mash::ALLOCATED);
}

void std_fear_inode::initialize(mash::allocation_scope)
{
    set_fear_flag(this, "cowering_forced", field_1E, false);
    set_fear_flag(this, "cowering_stop_forced", field_1F, false);
    set_fear_flag(this, "cowering_stop_disabled", field_20, false);
    set_fear_flag(this, "fleeing_forced", field_22, false);
    set_fear_flag(this, "fleeing_stop_forced", field_24, false);
    set_fear_flag(this, "flee_to_location", field_26, false);
    my_param_block.set_pb_fixedstring(string_hash{"entity_to_flee_from_id"}, "", true);
    field_28 = reinterpret_cast<std::intptr_t>("");
    field_68.field_0 = 0;
    field_6C = 0.0f;
    field_74 = 0.0f;
    field_78 = false;
    field_79 = false;
    field_64 = false;
    add_to_list(&all_fear_inodes());
}

void std_fear_inode::add_to_list(_std::vector<std_fear_inode *> **list)
{
    if (!*list)
        *list = new _std::vector<std_fear_inode *>;
    (*list)->push_back(this);
}

void std_fear_inode::remove_from_list(_std::vector<std_fear_inode *> **list)
{
    if (!*list)
        return;
    auto it = std::find((*list)->begin(), (*list)->end(), this);
    if (it != (*list)->end())
        (*list)->erase(it);
    if ((*list)->empty()) {
        delete *list;
        *list = nullptr;
    }
}

void std_fear_inode::set_cowering_enabled(bool a2)
{
    static const string_hash cowering_enabled_hash{int(to_hash("cowering_enabled"))};

    this->my_param_block.set_pb_int(cowering_enabled_hash, a2, true);
    if (a2 != this->field_1D) {
        this->field_1D = a2;
    }
}

void std_fear_inode::post_event(int event, float magnitude)
{
    struct fear_parameter {
        string_hash name;
        float mean;
        float variation;
    };
    static const fear_parameter parameters[] = {
        {string_hash{"FEAR_SELF_DAMAGE"}, -0.15f, 0.05f},
        {string_hash{"FEAR_SURPRISE"}, -0.25f, 0.0f},
        {string_hash{"FEAR_FRIEND_DAMAGE"}, -0.05f, 0.05f},
        {string_hash{"FEAR_FRIEND_COWER"}, -0.1f, 0.05f},
        {string_hash{"FEAR_FRIEND_FLEE"}, -0.15f, 0.1f},
        {string_hash{"FEAR_FRIEND_SUBDUED"}, -0.2f, 0.1f},
        {string_hash{"FEAR_COMMANDER_DAMAGE"}, -0.2f, 0.1f},
        {string_hash{"FEAR_COMMANDER_COWER"}, -0.15f, 0.1f},
        {string_hash{"FEAR_COMMANDER_FLEE"}, -0.35f, 0.2f},
        {string_hash{"FEAR_COMMANDER_SUBDUED"}, -0.5f, 0.2f},
        {string_hash{"BRAVADO_SEPERATOR"}, 0.0f, 0.0f},
        {string_hash{"BRAVADO_COMMANDER_MORALE_BOOST"}, 0.75f, 0.0f}
    };
    double value = 0.0;
    if (event >= 0 && event < 12) {
        const auto &parameter = parameters[event];
        float mean = parameter.mean;
        float variation = parameter.variation;
        if (my_param_block.param_array != nullptr) {
            if (auto *data = my_param_block.param_array->common_find_data(parameter.name)) {
                const auto *variance = data->get_data_float_variance();
                mean = variance->field_0;
                variation = variance->field_4;
            }
        }
        const int random = std::rand();
        constexpr float random_scale = 3.0518509447574615e-05f;
        value = ((static_cast<double>(random) * random_scale) * 2.0 - 1.0) * variation + mean;
    }
    value *= magnitude;
    if (event >= 10) {
        field_6C = static_cast<float>(value * field_54 + field_6C);
        if (field_6C > 1.0f)
            field_6C = 1.0f;
        field_74 = 0.5f;
        field_70 = 0.1f;
    } else {
        field_6C = static_cast<float>(value * field_48 + field_6C);
        if (field_6C < -1.0f)
            field_6C = -1.0f;
        field_74 = 0.0f;
        field_70 = 0.5f;
    }
    if (event == 0)
        post_event_to_others(2, magnitude, true, false, false);
}

void std_fear_inode::post_event_to_others(int event, float magnitude, bool friends, bool enemies, bool neutrals)
{
    if (friends) {
        auto *soldier = static_cast<universal_soldier_inode *>(
            field_8->get_info_node(string_hash{"universal_soldier"}, false));
        if (soldier != nullptr && (soldier->field_5C & 0xFF) != 0 &&
            soldier->field_2C != nullptr && !soldier->field_2C->empty()) {
            const int group_event = event >= 2 && event <= 5 ? event + 4 : event;
            for (auto *member : *soldier->field_2C) {
                if (member->field_40 != nullptr)
                    member->field_40->post_event(group_event, magnitude);
            }
            friends = false;
        }
    }
    if (!friends && !enemies && !neutrals)
        return;
    const string_hash team_parameter{"team"};
    if (!field_8->get_param_block()->does_parameter_exist(team_parameter))
        return;
    const auto observer_team = team::manager::get_team_enum_by_hash(
        field_8->get_param_block()->get_pb_hash(team_parameter));
    for (auto *other : *all_fear_inodes()) {
        if (other == this || !other->field_8->get_param_block()->does_parameter_exist(team_parameter))
            continue;
        const auto candidate_team = team::manager::get_team_enum_by_hash(
            other->field_8->get_param_block()->get_pb_hash(team_parameter));
        if ((friends && team::manager::is_friend(observer_team, candidate_team)) ||
            (enemies && team::manager::is_enemy(observer_team, candidate_team)) ||
            (neutrals && team::manager::is_neutral(observer_team, candidate_team)))
            other->post_event(event, magnitude);
    }
}

}  // namespace ai
