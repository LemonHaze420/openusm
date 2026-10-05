#include "mission_table_container.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "common.h"
#include "entity_base_vhandle.h"
#include "mission_manager_script_data.h"
#include "oldmath_po.h"
#include "wds.h"
#include "mission_manager.h"
#include "parse_generic_mash.h"
#include "script_manager.h"
#include "glass_house_manager.h"
#include "game.h"
#include "camera.h"
#include "line_info.h"
#include "local_collision.h"
#include "trigger_manager.h"
#include "trigger.h"
#include <cmath>
#include <cstdlib>

VALIDATE_SIZE(mission_condition_instance, 0x68u);
VALIDATE_SIZE(mission_condition, 0x34u);
VALIDATE_SIZE(mission_table_game_state_entry, 0xC);
VALIDATE_OFFSET(mission_condition_instance, markers, 0x1C);
VALIDATE_OFFSET(mission_condition_instance, field_58, 0x58);
VALIDATE_OFFSET(mission_condition, num_positions, 0x27);
VALIDATE_OFFSET(mission_condition, field_28, 0x28);

VALIDATE_SIZE(mission_table_container, 0x48u);

bool mission_table_container::append_script_info(_std::vector<mission_table_container::script_info> *info)
{
    assert(info != nullptr);

    bool v9 = false;
    for (auto &v6 : this->field_38) {
        if (v6.applies_to_current_hero()) {
            auto v5 = v6.instances.size();
            for (auto i = 0u; i < v5; ++i) {
                script_info v3;
                v3.field_0 = v6.field_18;
                v3.field_4 = &v6.instances.at(i);
                v3.field_8 = i;
                info->push_back(v3);
                v9 = true;
            }
        }
    }

    return v9;
}

bool mission_table_container::append_nums(const char *a2, int a3, _std::vector<float> *nums) const
{
    assert(nums != nullptr);

    if constexpr (STANDALONE_SYSTEM) {
        bool result = false;
        for (auto &v6 : this->field_38) {
            auto *v4 = v6.field_18;
            if (!_strcmpi(v4, a2) && v6.append_nums(a3, nums)) {
                result = true;
            }
        }

        return result;
    } else {
        bool(__fastcall * func)(const void *, void *edx, const char *, int, _std::vector<float> *) =
            CAST(func, 0x005DAA90);
        return func(this, nullptr, a2, a3, nums);
    }
}

namespace {


template <class T>
void un_mash_values(mashable_vector<T> &values, generic_mash_data_ptrs *data)
{
    if (values.m_shared) {
        data->rebase_shared(4);
        values.m_data = data->get_from_shared<T>(values.m_size);
        data->rebase_shared(4);
    } else {
        data->rebase(4);
        values.m_data = data->get<T>(values.m_size);
        data->rebase(4);
    }
}

const char *un_mash_string(generic_mash_data_ptrs *data)
{
    data->rebase(4);
    const auto length = *data->get<uint32_t>();
    return data->get<char>(length);
}

void un_mash_states(mashable_vector<mission_table_game_state_entry> &states, generic_mash_data_ptrs *data)
{
    if (states.m_shared)
        return;
    data->rebase(4);
    states.m_data = data->get<mission_table_game_state_entry>(states.m_size);
    for (auto &state : states) {
        const int offset = reinterpret_cast<int>(state.field_0);
        state.field_0 =
            reinterpret_cast<float *>((state.field_8 & 0x40) != 0 ? script_manager::get_game_var_address(offset)
                                                                  : script_manager::get_shared_var_address(offset));
        if ((state.field_8 & 0x80) != 0)
            state.field_4.p =
                reinterpret_cast<float *>(script_manager::get_game_var_address(reinterpret_cast<int>(state.field_4.p)));
        else if ((state.field_8 & 0x100) != 0)
            state.field_4.p = reinterpret_cast<float *>(
                script_manager::get_shared_var_address(reinterpret_cast<int>(state.field_4.p)));
    }
    data->rebase(4);
}
}  // namespace

void mission_condition_instance::un_mash(generic_mash_data_ptrs *data, mission_table_container *container)
{
    data->rebase(16);
    data->rebase(4);
    field_4 = data->get<po>();
    un_mash_states(field_58, data);
    if (is_flag_set(2 | 4))
        key_name = un_mash_string(data);
    if (is_flag_set(8))
        script_data_name = un_mash_string(data);
    const auto relocate = [](auto *&pointer, int count, const auto &values) {
        if (count > 0)
            pointer = values.m_data + (reinterpret_cast<uintptr_t>(pointer) & 0xFFFF);
    };
    relocate(markers, num_markers, container->field_0);
    relocate(camera_markers, num_camera_markers, container->multi_array_camera_markers);
    relocate(transform_markers, num_transform_markers, container->field_10);
    relocate(camera_transform_markers, num_camera_transform_markers, container->field_18);
    relocate(nums, num_nums, container->multi_array_nums);
    relocate(strings, num_strings, container->multi_array_strings);
    relocate(positions, num_positions, container->multi_array_positions);
}

void mission_condition::un_mash(generic_mash_data_ptrs *data, mission_table_container *container)
{
    if (!instances.m_shared) {
        data->rebase(4);
        instances.m_data = data->get<mission_condition_instance>(instances.m_size);
        for (auto &instance : instances)
            instance.un_mash(data, container);
        data->rebase(4);
    }
    un_mash_states(field_10, data);
    field_18 = un_mash_string(data);
    field_1C = un_mash_string(data);
    field_2C = un_mash_string(data);
    field_30 = un_mash_string(data);
}

void mission_table_container::un_mash(generic_mash_header *header, void *context, void *object,
                                      generic_mash_data_ptrs *data)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (!field_0.m_shared)
            un_mash_values(field_0, data);
        if (!multi_array_camera_markers.m_shared)
            un_mash_values(multi_array_camera_markers, data);
        if (!field_10.m_shared)
            un_mash_values(field_10, data);
        if (!field_18.m_shared)
            un_mash_values(field_18, data);
        un_mash_values(multi_array_nums, data);
        if (!multi_array_strings.m_shared) {
            data->rebase(4);
            multi_array_strings.m_data = data->get<const char *>(multi_array_strings.m_size);
            for (auto &string : multi_array_strings) {
                data->rebase_shared(4);
                const auto length = *data->get_from_shared<uint32_t>();
                string = data->get_from_shared<char>(length);
            }
            data->rebase(4);
        }
        un_mash_values(multi_array_positions, data);
        if (!field_38.m_shared) {
            data->rebase(4);
            field_38.m_data = data->get<mission_condition>(field_38.m_size);
            for (auto &condition : field_38)
                condition.un_mash(data, this);
            data->rebase(4);
        }
        field_44 = nullptr;
    } else {
        THISCALL(0x005C6010, this, header, context, object, data);
    }
}

mission_condition_instance *mission_condition::find_best_instance(mission_manager_script_data *data) const
{
    if constexpr (!STANDALONE_SYSTEM) {
        mission_condition_instance *(__fastcall * func)(const void *, void *, mission_manager_script_data *) =
            CAST(func, 0x005DD780);
        return func(this, nullptr, data);
    }
    auto *manager = mission_manager::s_inst;
    const auto hero_position = g_world_ptr->get_hero_ptr(0)->get_abs_position();
    const float min_distance2 = field_0 * field_0;
    const float max_distance2 = field_4 * field_4;
    mission_condition_instance *best = nullptr;
    float best_distance2 = 0.0f;
    float eligible_count = 0.0f;
    for (int index = 0; index < instances.size(); ++index) {
        auto &instance = instances.at(index);
        if (manager->field_80) {
            if (manager->field_A8 == fixedstring<8>{""}) {
                if (manager->field_C8 != index)
                    continue;
            } else if (!instance.is_flag_set(8) || instance.script_data_name == nullptr ||
                       _strcmpi(manager->field_A8.to_string(), instance.script_data_name) != 0) {
                continue;
            }
        }


        const auto in_glass_house = [](const vector3d &position) {
            return position.length2() <= 0.0100000007f || glass_house_manager::is_point_in_glass_house(position);
        };
        if (!glass_house_manager::is_enabled()) {
            bool valid = true;
            for (int i = 0; valid && i < num_positions; ++i)
                valid = in_glass_house(instance.positions[i]);
            for (int i = 0; valid && i < num_camera_markers; ++i)
                valid = in_glass_house(instance.camera_markers[i].base_position);
            for (int i = 0; valid && i < num_transform_markers; ++i)
                valid = in_glass_house(instance.transform_markers[i].base_position);
            for (int i = 0; valid && i < num_camera_transform_markers; ++i)
                valid = in_glass_house(instance.camera_transform_markers[i].base_position);
            if (!valid)
                continue;
        }
        if (!instance.check_game_state())
            continue;

        vector3d position{};
        trigger *key_trigger = nullptr;
        bool in_range = false;
        bool no_position = is_flag_set(0x2000);
        if (!no_position) {
            if (instance.is_flag_set(1)) {
                position = instance.field_4->get_position();
                if (!in_glass_house(position))
                    continue;
            } else if (instance.is_flag_set(2)) {
                auto *key_entity =
                    entity_handle_manager::find_entity(string_hash{instance.key_name}, IGNORE_FLAVOR, true);
                if (key_entity == nullptr)
                    continue;
                position = key_entity->get_abs_position();
            } else if (instance.is_flag_set(4)) {
                key_trigger = trigger_manager::instance->find_instance(mString{instance.key_name});
                if (key_trigger == nullptr)
                    continue;
                position = key_trigger->get_position();
                if ((key_trigger->field_4 & 0x2000) != 0)
                    in_range = key_trigger->contains(hero_position);
            } else {
                no_position = true;
            }
        }
        const float dx = hero_position.x - position.x;
        const float dz = hero_position.z - position.z;
        const float distance2 = dx * dx + dz * dz;
        if (key_trigger == nullptr && distance2 >= min_distance2 && distance2 <= max_distance2)
            in_range = true;
        if (manager->field_80 && manager->field_C8 == index) {
            if (!in_range && !no_position) {
                manager->field_80 = false;
                return nullptr;
            }
            best = &instance;
            break;
        }
        if (!no_position) {
            if (!in_range)
                continue;
            if (is_flag_set(0x100)) {
                auto *view = g_game_ptr->get_current_view_camera(0);
                auto forward = view->get_abs_po().get_z_facing();
                auto direction = position - view->get_abs_position();
                forward.normalize();
                direction.normalize();
                static const float view_cosine = std::cos(1.134464f);
                const bool in_view = dot(forward, direction) > view_cosine;
                if (in_view != is_flag_set(0x80))
                    continue;
            }
            if (is_flag_set(0x40)) {
                line_info sight;
                sight.clear();
                sight.field_0 = hero_position;
                sight.field_C = position;
                sight.sub_48B410(99.900002f);
                const bool collision = sight.check_collision(
                    *local_collision::entfilter_blocks_ai_los, *local_collision::obbfilter_lineseg_test, nullptr);
                if (collision == is_flag_set(0x20))
                    continue;
            }
        }
        if (manager->field_80)
            continue;
        eligible_count += 1.0f;
        if (best == nullptr) {
            best = &instance;
            best_distance2 = distance2;
            if (is_flag_set(8))
                break;
        } else if (is_flag_set(0x10)) {
            if (static_cast<double>(std::rand()) * (1.0f / RAND_MAX) <= 1.0f / eligible_count)
                best = &instance;
        } else if ((is_flag_set(2) && distance2 < best_distance2) || (is_flag_set(4) && distance2 > best_distance2)) {
            best = &instance;
            best_distance2 = distance2;
        }
    }
    if (best != nullptr && data != nullptr) {
        data->field_B4 = best->is_flag_set(0x40);
        if (best->is_flag_set(1))
            *data->field_94 = *best->field_4;
        else if (best->is_flag_set(2 | 4))
            data->field_84 = best->key_name;
        if (best->is_flag_set(0x20))
            data->field_98 = best->field_8;
        if (best->is_flag_set(8))
            data->field_A4 = resource_key{string_hash{best->script_data_name}, static_cast<resource_key_type>(16)};
        const auto append = [](auto &destination, const auto *source, int count) {
            for (int i = 0; i < count; ++i)
                destination.push_back(source[i]);
        };
        append(data->pos, best->positions, best->num_positions);
        best->append_nums(this, &data->nums);
        append(data->markers, best->markers, best->num_markers);
        append(data->camera_markers, best->camera_markers, best->num_camera_markers);
        append(data->transform_markers, best->transform_markers, best->num_transform_markers);
        append(data->camera_transform_markers, best->camera_transform_markers, best->num_camera_transform_markers);
        best->append_strings(this, &data->strings);
    }
    return best;
}

bool mission_condition::get_key_po(int instance, po *p) const
{
    assert(p != nullptr);

    assert(instance >= 0);

    if (instance >= this->instances.size()) {
        return false;
    }

    auto &v4 = this->instances.at(instance);
    return v4.get_key_po(this, p);
}

bool mission_condition::applies_to_current_hero() const
{
    assert(g_world_ptr->get_hero_ptr(0) != nullptr);

    assert(bit_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller() != nullptr);

    auto *v8 = bit_cast<actor *>(g_world_ptr->get_hero_ptr(0))->get_player_controller();
    auto v15 = v8->m_hero_type;
    if (v15 == 1) {
        if (this->is_flag_set(0x8000)) {
            return true;
        }
    } else if (v15 == 2) {
        if (this->is_flag_set(0x4000)) {
            return true;
        }
    } else if (v15 == 3 && this->is_flag_set(0x10000)) {
        return true;
    }

    return false;
}

bool mission_condition::check_condition(mission_manager_script_data *data) const
{
    assert(data != nullptr);

    if (g_world_ptr->get_hero_ptr(0) == nullptr || !this->applies_to_current_hero()) {
        return false;
    }

    data->uses_script_stack = (this->is_flag_set(0x1000) || this->is_flag_set(0x2000));

    bool v11 = false;
    auto *v4 = mission_manager::s_inst;
    if (v4->field_80) {
        if (_strcmpi(this->field_18, v4->field_88.to_string()) != 0) {
            return false;
        }

        v11 = true;
    }

    auto v12 = this->field_28;
    if ((v12 & 0x800) != 0) {
        auto v13 = v4->sub_5C5BD0();
        if (v13 != 0) {
            if (v13 == 1) {
                if (!this->is_flag_set(0x200) && !v11) {
                    return false;
                }
            } else if (v13 == 2) {
                if (!this->is_flag_set(0x400) && !v11) {
                    return false;
                }
            } else {
                assert(0 && "unknown time of day!!!");
            }
        }
    }

    for (auto &entry : this->field_10) {
        if (!entry.check()) {
            return false;
        }
    }

    auto *best_instance = this->find_best_instance(data);
    if (best_instance == nullptr) {
        return false;
    }

    data->field_0 = this->field_18;
    data->field_B8 = this->field_1C;
    data->field_10 = this->field_20;
    data->field_C8 = this->field_2C;
    data->field_D8 = this->field_30;
    data->field_B0 = ((this->field_28 & 0x2000) != 0 ? best_instance->get_patrol_num() : -1);
    return true;
}

bool mission_condition::append_nums(int instance, _std::vector<float> *nums) const
{
    assert(nums != nullptr);

    assert(instance >= 0);

    if (instance >= this->instances.size()) {
        return false;
    }

    auto &v4 = this->instances.at(instance);
    return v4.append_nums(this, nums);
}

bool mission_condition_instance::get_key_po(const mission_condition *, po *p) const
{
    assert(this->sentinel == 0x31415926 && "corruption!");

    assert(p != nullptr);

    if (!this->is_flag_set(1)) {
        return false;
    }

    *p = *this->field_4;
    return true;
}

bool mission_condition_instance::check_game_state() const
{
    for (auto &entry : this->field_58) {
        if (!entry.check()) {
            return false;
        }
    }

    return true;
}

const char *mission_condition_instance::get_script_data_name() const
{
    if (!this->is_flag_set(8)) {
        return nullptr;
    }

    assert(script_data_name != nullptr);

    return this->script_data_name;
}

bool mission_condition_instance::append_nums(const mission_condition *, _std::vector<float> *num_list) const
{
    assert(this->sentinel == 0x31415926 && "corruption!");

    assert(num_list != nullptr);

    const auto v4 = this->num_nums;
    auto begin = this->nums;
    auto end = this->nums + v4;
    std::for_each(begin, end, [&](const float v) { num_list->push_back(v); });

    return v4 > 0;
}

bool mission_condition_instance::append_strings(const mission_condition *, _std::vector<mString> *str_list) const
{
    assert(this->sentinel == 0x31415926 && "corruption!");

    assert(str_list != nullptr);

    auto v4 = this->num_strings;
    auto begin = this->strings;
    auto end = begin + v4;
    std::for_each(begin, end, [&](const auto &v) { str_list->push_back(v); });

    //std::copy(begin, end, std::back_inserter(*str_list));

    return v4 > 0;
}

bool mission_table_game_state_entry::check() const
{
    auto sub_6786F0 = [](const mission_table_game_state_entry *self, uint32_t a2) -> bool {
        return (a2 & self->field_8) != 0;
    };

    auto v1 = mission_manager::s_inst->field_80;
    auto v2 = this->field_8;
    bool v3 = false;
    float v4;
    if ((v2 & 0x80u) != 0) {
        v4 = *this->field_4.p;
    } else if ((v2 & 0x100) != 0) {
        v4 = *this->field_4.p;
    } else {
        v4 = this->field_4.f;
    }

    if (sub_6786F0(this, 1)) {
        if (v1) {
            *this->field_0 = v4;
        }

        v3 = (*this->field_0 <= v4 && *this->field_0 >= v4);
    } else if (sub_6786F0(this, 2)) {
        if (v1 && *this->field_0 <= v4 && *this->field_0 >= v4) {
            *this->field_0 = 1.0f + v4;
        }

        v3 = !(*this->field_0 <= v4 && *this->field_0 >= v4);
    } else if (sub_6786F0(this, 4)) {
        if (v1 && v4 >= *this->field_0) {
            *this->field_0 = 1.0f + v4;
        }

        v3 = (*this->field_0 > v4);
    } else if (sub_6786F0(this, 8)) {
        if (v1 && v4 <= *this->field_0) {
            *this->field_0 = v4 - 1.0;
        }

        v3 = (*this->field_0 < v4);
    } else if (sub_6786F0(this, 0x10)) {
        if (v1 && v4 > *this->field_0) {
            *this->field_0 = v4;
        }

        v3 = (*this->field_0 >= v4);
    } else if (sub_6786F0(this, 0x20)) {
        if (v1 && v4 < *this->field_0) {
            *this->field_0 = v4;
        }

        v3 = (*this->field_0 <= v4);
    } else {
        assert(0 && "unknown game state operator");
    }

    if (v1) {
        auto v5 = mission_manager::s_inst->field_60;
        if (this->field_0 == v5) {
            mission_manager::s_inst->field_5C = *v5;
        }
    }

    return v3;
}
