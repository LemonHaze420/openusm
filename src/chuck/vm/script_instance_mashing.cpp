#include "script_instance_mashing.h"

#include "common.h"
#include "custom_math.h"
#include "entity_handle_manager.h"
#include "mission_manager.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "resource_manager.h"
#include "resource_pack_slot.h"
#include "script_executable.h"
#include "script_executable_entry.h"
#include "script_manager.h"
#include "script_object.h"
#include "trigger.h"
#include "trigger_manager.h"
#include "variables.h"

#include <cmath>
#include <cstring>

struct script_instance_mash_record {
    uint32_t name_offset;
    char *buffer;
    uint16_t parent_offset;
    uint8_t parent_index;
    uint8_t member_count;
};

struct script_member_initializer {
    uint16_t offset;
    uint16_t type;
    uintptr_t value;


    void un_mash(generic_mash_data_ptrs *data)
    {
        uint32_t size;
        switch (type & 0xFF) {
        case 0:
            size = sizeof(float);
            break;
        case 3:
            size = sizeof(vector3d);
            break;
        case 4:
            size = sizeof(float) * 4;
            break;
        default:
            return;
        }

        data->rebase(4);
        value = reinterpret_cast<uintptr_t>(data->get<uint8_t>(size));
    }


    bool init(char *buffer, const char *strings, const po &key_po) const
    {
        char *destination = buffer + offset;
        switch (type) {
        case 0:
            std::memcpy(destination, reinterpret_cast<const void *>(value), sizeof(float));
            return true;
        case 2: {
            const char *string = strings + value;
            std::memcpy(destination, &string, sizeof(string));
            return true;
        }
        case 3:
            std::memcpy(destination, reinterpret_cast<const void *>(value), sizeof(vector3d));
            return true;
        case 4:
            std::memcpy(destination, reinterpret_cast<const void *>(value), sizeof(float) * 4);
            return true;
        case 5: {
            const string_hash name{strings + value};
            auto *entity = entity_handle_manager::find_entity(name, IGNORE_FLAVOR, true);
            if (entity == nullptr)
                return false;
            const auto handle = entity->my_handle.get_goodies();
            std::memcpy(destination, &handle, sizeof(handle));
            return true;
        }
        case 6: {
            const mString name{strings + value};
            auto *instance = trigger_manager::instance->find_instance(name);
            const uint32_t handle = instance != nullptr ? instance->my_handle.get_goodies() : 0;
            std::memcpy(destination, &handle, sizeof(handle));
            return instance != nullptr;
        }
        case 0x8003: {
            const auto position = key_po.slow_xform(*reinterpret_cast<const vector3d *>(value));
            std::memcpy(destination, &position, sizeof(position));
            return true;
        }
        case 0x8004: {
            const auto *position_facing = reinterpret_cast<const float *>(value);
            po relative;
            relative.set_rotate_y(position_facing[3]);
            relative.set_position(vector3d{position_facing[0], position_facing[1], position_facing[2]});
            const po world = relative.sub_4BAB00(key_po);


            float x = world.m[2][0];
            float z = world.m[2][2];

            constexpr float facing_epsilon = 9.99999944e-11f;
            const double length_squared = static_cast<double>(x) * x + static_cast<double>(z) * z;
            if (length_squared > facing_epsilon) {
                const double inverse_length = 1.0 / std::sqrt(length_squared);
                x = static_cast<float>(x * inverse_length);
                z = static_cast<float>(z * inverse_length);
            }
            const float transformed[4] = {world.m[3][0], world.m[3][1], world.m[3][2], -sub_48A720(x, z)};
            std::memcpy(destination, transformed, sizeof(transformed));
            return true;
        }
        default:
            return false;
        }
    }
};

VALIDATE_SIZE(script_instance_mash_record, 0xC);
VALIDATE_SIZE(script_member_initializer, 0x8);
VALIDATE_SIZE(script_instance_info, 0x18);

namespace {
bool initialize_record(script_instance_info &info, script_instance_mash_record &record, const script_executable *exec,
                       const po &key_po, uint16_t &member_index)
{
    const string_hash name{info.field_10 + record.name_offset};
    auto *object = exec->find_object(name, nullptr);
    if (object == nullptr)
        return false;

    script_instance *instance;
    if (record.parent_index == 0xFF) {
        instance = object->instances->_first_element;
    } else {
        static const string_hash auto_instance_name{"_auto_inst_mjd"};
        instance = new script_instance{auto_instance_name, object->data_blocksize, 0};
        object->add(instance);
        auto &parent = info.field_0[record.parent_index];
        std::memcpy(parent.buffer + record.parent_offset, &instance, sizeof(instance));
    }

    record.buffer = instance->get_buffer();
    for (unsigned int i = 0; i < record.member_count; ++i)
        info.field_8[member_index++].init(record.buffer, info.field_10, key_po);
    return true;
}
}


void script_instance_info::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *data)
{
    if (!field_0.is_shared()) {
        data->rebase(4);
        field_0.m_data = data->get<script_instance_mash_record>(field_0.m_size);
        data->rebase(4);
    }
    if (!field_8.is_shared()) {
        data->rebase(4);
        field_8.m_data = data->get<script_member_initializer>(field_8.m_size);
        for (auto &member : field_8)
            member.un_mash(data);
        data->rebase(4);
    }
    data->rebase(4);
    field_10 = data->get<char>(field_14);
}

void script_instance_info::un_mash_start(generic_mash_header *header, void *object, generic_mash_data_ptrs *data,
                                         void *)
{
    un_mash(header, object, data);
}


bool script_instance_info::initialize_single(const script_executable *exec, string_hash requested, const po &key_po)
{
    uint16_t member_index = 0;
    for (auto &record : field_0) {
        const auto &owner = record.parent_index == 0xFF ? record : field_0[record.parent_index];
        const string_hash name{field_10 + owner.name_offset};
        if (name == requested && initialize_record(*this, record, exec, key_po, member_index))
            continue;
        member_index += record.member_count;
    }
    return true;
}


bool script_instance_info::initialize(const script_executable *exec, const po &key_po)
{
    uint16_t member_index = 0;
    for (auto &record : field_0)
        initialize_record(*this, record, exec, key_po, member_index);
    return true;
}


bool initialize_game_init_instances(const script_executable *exec, string_hash requested)
{
    if (g_is_the_packer)
        return true;

    auto *entry = script_manager::find_entry(exec);
    assert(entry != nullptr);
    const auto &key = entry->field_C;
    if (key.m_hash == string_hash{})
        return true;

    const po key_po = mission_manager::s_inst->get_mission_key_po();
    auto *resource = reinterpret_cast<generic_mash_header *>(resource_manager::get_resource(key, nullptr, nullptr));
    if (resource == nullptr) {
        auto *partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_DISTRICT);
        const auto &slots = partition->get_pack_slots();
        for (unsigned int i = 0; i < slots.size() && resource == nullptr; ++i) {
            auto *slot = slots[i];
            if (slot->is_pack_ready()) {
                resource_manager::push_resource_context(slot);
                resource =
                    reinterpret_cast<generic_mash_header *>(resource_manager::get_resource(key, nullptr, nullptr));
                resource_manager::pop_resource_context();
            }
        }
        if (resource == nullptr)
            return false;
    }

    script_instance_info *info = nullptr;
    if (resource->field_4 < 0) {
        info = reinterpret_cast<script_instance_info *>(resource + 1);
    } else {
        [[maybe_unused]] const bool allocated_mem =
            parse_generic_object_mash(info, resource, nullptr, nullptr, nullptr, 0, 0, nullptr);
        assert(!allocated_mem);
    }

    if (requested != string_hash{})
        info->initialize_single(exec, requested, key_po);
    else
        info->initialize(exec, key_po);
    return true;
}
