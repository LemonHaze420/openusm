#include "core_ai_resource.h"

#include "combo_system.h"
#include "common.h"
#include "func_wrapper.h"
#include "resource_key.h"
#include "trace.h"
#include "utility.h"

#include <cassert>
#include <array>
#include <cstring>
#include <memory>
#include <vector>

namespace ai {

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
namespace {

std::vector<std::unique_ptr<uint8_t[]>> converted_core_ai_buffers;

uint32_t read_u32(const uint8_t *data)
{
    uint32_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return value;
}

bool convert_web_zip_inode(std::vector<uint8_t> &data)
{
    constexpr auto xbox_hash = to_hash("web_zip_inode");
    constexpr int xbox_object_size = 0xB4;
    constexpr int inline_array_size = 0x30;
    constexpr int pc_object_size = 0xE0;
    constexpr int xbox_total_size = xbox_object_size + inline_array_size;

    if (data.size() < xbox_total_size) {
        return false;
    }

    bool converted = false;
    for (size_t offset = 0; offset + xbox_total_size <= data.size(); offset += 4)
    {
        if (read_u32(data.data() + offset) != xbox_hash) {
            continue;
        }

        const auto pointer_offset = offset + pc_object_size - inline_array_size;
        const auto xbox_array_pointer = read_u32(data.data() + pointer_offset);
        if (xbox_array_pointer == 0) {
            continue;
        }

        data.erase(data.begin() + pointer_offset,
                   data.begin() + pointer_offset + sizeof(uint32_t));
        converted = true;
        offset += pc_object_size - 4;
    }

    return converted;
}

void expand_swing_inode(const uint8_t *source, uint8_t *destination)
{
    std::memcpy(destination, source, 0x1C);

    destination[0x1C] = source[0x87];
    std::memcpy(destination + 0x20, source + 0x1C, 4);
    destination[0x24] = source[0x85];
    std::memcpy(destination + 0x28, source + 0x20, 0x1C);

    destination[0x44] = source[0x84];
    std::memcpy(destination + 0x48, source + 0x3C, 0x0C);
    destination[0x54] = source[0x86];
    std::memcpy(destination + 0x58, source + 0x48, 0x3C);
}

bool convert_swing_inode(std::vector<uint8_t> &data)
{
    constexpr auto swing_hash = to_hash("swing_inode");
    constexpr auto following_hash = to_hash("std_puppet_inode");
    constexpr size_t xbox_object_size = 0x88;
    constexpr size_t pc_object_size = 0x94;

    if (data.size() < xbox_object_size + sizeof(uint32_t)) {
        return false;
    }

    bool converted = false;
    for (size_t offset = 0;
         offset + xbox_object_size + sizeof(uint32_t) <= data.size();
         offset += 4)
    {
        const auto *source = data.data() + offset;
        if (read_u32(source) != swing_hash
            || read_u32(source + xbox_object_size) != following_hash) {
            continue;
        }

        std::array<uint8_t, pc_object_size> expanded {};
        expand_swing_inode(source, expanded.data());

        data.erase(data.begin() + offset,
                   data.begin() + offset + xbox_object_size);
        data.insert(data.begin() + offset, expanded.begin(), expanded.end());
        converted = true;
        offset += pc_object_size - 4;
    }

    return converted;
}

void expand_results(const uint8_t *source, uint8_t *destination)
{
    constexpr size_t xbox_size = 0x78;
    constexpr size_t pc_size = 0x7C;
    constexpr size_t pc_only_string_word = 0x10;

    std::memcpy(destination, source, pc_only_string_word);
    std::memset(destination + pc_only_string_word, 0, sizeof(uint32_t));
    std::memcpy(destination + pc_only_string_word + sizeof(uint32_t),
                source + pc_only_string_word,
                xbox_size - pc_only_string_word);
    static_assert(pc_only_string_word + sizeof(uint32_t)
                      + xbox_size - pc_only_string_word
                  == pc_size);
}

void expand_incoming_move(const uint8_t *source, uint8_t *destination)
{
    constexpr size_t prefix_size = 0x14;
    constexpr size_t xbox_results_size = 0x78;
    constexpr size_t pc_results_size = 0x7C;
    constexpr size_t pc_size = 0x94;

    std::memcpy(destination, source, prefix_size);
    expand_results(source + prefix_size, destination + prefix_size);
    std::memset(destination + prefix_size + pc_results_size,
                0,
                pc_size - prefix_size - pc_results_size);
    static_assert(prefix_size + xbox_results_size == 0x8C);
}

void expand_combat_state(const uint8_t *source, uint8_t *destination)
{
    std::memcpy(destination, source, 0x4C);
    std::memset(destination + 0x4C, 0, 4);
    std::memcpy(destination + 0x50, source + 0x4C, 0x30);
    std::memset(destination + 0x80, 0, 8);
    std::memcpy(destination + 0x88, source + 0x7C, 0x10);

    const auto *xbox_state = source + 0x8C;
    auto *pc_state = destination + 0x98;
    std::memcpy(pc_state, xbox_state, 0x1C);
    std::memset(pc_state + 0x1C, 0, 4);
    std::memcpy(pc_state + 0x20, xbox_state + 0x1C, 0x18);
    pc_state[0x38] = xbox_state[0x3D];
    std::memset(pc_state + 0x39, 0, 3);
    std::memcpy(destination + 0xD4, source + 0xCC, 4);
}

bool convert_player_combat_inode(std::vector<uint8_t> &data)
{
    constexpr auto player_hash = to_hash("player_combat_inode");
    constexpr auto combat_hash = to_hash("combat_inode");
    constexpr auto incoming_hash = to_hash("combat_inode::incoming_move");

    constexpr size_t xbox_object_size = 0xDC;
    constexpr size_t pc_object_size = 0x330;
    constexpr size_t parameter_data_size = 0x28;
    constexpr size_t xbox_incoming_size = 0x90;
    constexpr size_t pc_incoming_size = 0x94;
    constexpr size_t incoming_count = 4;
    constexpr size_t xbox_array_offset = xbox_object_size + parameter_data_size;
    constexpr size_t xbox_total_size =
        xbox_array_offset + xbox_incoming_size * incoming_count;
    constexpr size_t pc_total_size = pc_object_size + parameter_data_size;

    static_assert(xbox_array_offset == 0x104);
    static_assert(xbox_total_size == 0x344);
    static_assert(pc_total_size == 0x358);

    if (data.size() < xbox_total_size) {
        return false;
    }

    bool converted = false;
    for (size_t offset = 0; offset + xbox_total_size <= data.size(); offset += 4)
    {
        const auto *source = data.data() + offset;
        if (read_u32(source) != player_hash || read_u32(source + 4) != combat_hash) {
            continue;
        }

        bool has_expected_moves = true;
        for (size_t i = 0; i < incoming_count; ++i) {
            if (read_u32(source + xbox_array_offset + i * xbox_incoming_size)
                != incoming_hash) {
                has_expected_moves = false;
                break;
            }
        }
        if (!has_expected_moves) {
            continue;
        }

        std::array<uint8_t, pc_total_size> expanded {};
        expand_combat_state(source, expanded.data());

        for (size_t i = 0; i < incoming_count; ++i) {
            expand_incoming_move(
                source + xbox_array_offset + i * xbox_incoming_size,
                expanded.data() + 0xD8 + i * pc_incoming_size);
        }

        std::memcpy(expanded.data() + 0x328, source + 0xD4, 8);
        std::memcpy(expanded.data() + pc_object_size,
                    source + xbox_object_size,
                    parameter_data_size);

        data.erase(data.begin() + offset, data.begin() + offset + xbox_total_size);
        data.insert(data.begin() + offset, expanded.begin(), expanded.end());
        converted = true;
        offset += pc_total_size - 4;
    }

    return converted;
}

bool convert_spidey_combat_inode(std::vector<uint8_t> &data)
{
    constexpr auto spidey_hash = to_hash("spidey_combat_inode");
    constexpr auto combat_hash = to_hash("combat_inode");
    constexpr auto incoming_hash = to_hash("combat_inode::incoming_move");

    constexpr size_t xbox_object_size = 0xFC;
    constexpr size_t pc_object_size = 0x350;
    constexpr size_t xbox_incoming_size = 0x90;
    constexpr size_t pc_incoming_size = 0x94;
    constexpr size_t incoming_count = 4;
    constexpr size_t xbox_total_size =
        xbox_object_size + xbox_incoming_size * incoming_count;

    static_assert(xbox_total_size == 0x33C);

    if (data.size() < xbox_total_size) {
        return false;
    }

    bool converted = false;
    for (size_t offset = 0; offset + xbox_total_size <= data.size(); offset += 4)
    {
        const auto *source = data.data() + offset;
        if (read_u32(source) != spidey_hash
            || read_u32(source + 4) != combat_hash) {
            continue;
        }

        bool has_expected_moves = true;
        for (size_t i = 0; i < incoming_count; ++i) {
            if (read_u32(source + xbox_object_size + i * xbox_incoming_size)
                != incoming_hash) {
                has_expected_moves = false;
                break;
            }
        }
        if (!has_expected_moves) {
            continue;
        }

        std::array<uint8_t, pc_object_size> expanded {};
        expand_combat_state(source, expanded.data());

        for (size_t i = 0; i < incoming_count; ++i) {
            expand_incoming_move(
                source + xbox_object_size + i * xbox_incoming_size,
                expanded.data() + 0xD8 + i * pc_incoming_size);
        }

        std::memcpy(expanded.data() + 0x328, source + 0xD4, 8);
        std::memcpy(expanded.data() + 0x330, source + 0xDC, 0x20);

        data.erase(data.begin() + offset, data.begin() + offset + xbox_total_size);
        data.insert(data.begin() + offset, expanded.begin(), expanded.end());
        converted = true;
        offset += pc_object_size - 4;
    }

    return converted;
}

void convert_core_ai_data(core_ai_resource &resource)
{
    auto *source = reinterpret_cast<const uint8_t *>(resource.field_C);
    if (source == nullptr || resource.field_40 <= 0) {
        return;
    }

    std::vector<uint8_t> converted_data(source, source + resource.field_40);
    bool converted = convert_web_zip_inode(converted_data);
    converted = convert_swing_inode(converted_data) || converted;
    converted = convert_player_combat_inode(converted_data) || converted;
    converted = convert_spidey_combat_inode(converted_data) || converted;
    if (!converted) {
        return;
    }

    auto storage = std::make_unique<uint8_t[]>(converted_data.size());
    std::memcpy(storage.get(), converted_data.data(), converted_data.size());
    resource.field_C = reinterpret_cast<intptr_t>(storage.get());
    resource.field_40 = static_cast<int>(converted_data.size());

    converted_core_ai_buffers.push_back(std::move(storage));
}

} // namespace
#endif

VALIDATE_SIZE(core_ai_resource, 0x48);
VALIDATE_OFFSET(core_ai_resource, my_locomotion_graphs, 0x28);

core_ai_resource::core_ai_resource(from_mash_in_place_constructor *a2) {
    THISCALL(0x006D9A10, this, a2);
}

int core_ai_resource::destruct_mashed_class() {
    return THISCALL(0x006D71A0, this);
}

void core_ai_resource::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    TRACE("ai::core_ai_resource::unmash");

#if OPENUSM_XBOX_MASH_FORMAT
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->my_base_graphs, this);
    a1->unmash_class_in_place(this->my_locomotion_graphs, this);

    uint8_t class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
    assert(class_mashed == 0xAF || class_mashed == 0);

    if (this->field_10 != nullptr) {
        a1->unmash_class(this->field_10, this, mash::NORMAL_BUFFER);
    }

    a1->align_buffer(mash::SHARED_BUFFER, 4);
    const auto normal_data_size =
        *reinterpret_cast<int *>(a1->read_from_buffer(mash::SHARED_BUFFER, 4, 4));
    const auto shared_data_size =
        *reinterpret_cast<int *>(a1->read_from_buffer(mash::SHARED_BUFFER, 4, 4));

    a1->align_buffer(mash::NORMAL_BUFFER, 16);
    a1->align_buffer(mash::SHARED_BUFFER, 16);

    this->field_40 = normal_data_size;
    this->field_C = (int)&a1->mash_image_ptr[0][a1->buffer_size_used[0]];
    a1->advance_buffer(mash::NORMAL_BUFFER, normal_data_size);
    a1->advance_buffer(mash::SHARED_BUFFER, shared_data_size);

    const auto normal_sentry =
        *reinterpret_cast<uint32_t *>(a1->read_from_buffer(mash::NORMAL_BUFFER, 4, 4));
    const auto shared_sentry =
        *reinterpret_cast<uint32_t *>(a1->read_from_buffer(mash::SHARED_BUFFER, 4, 4));
    assert(normal_sentry == mash::CUSTOM_MASH_SENTRY);
    assert(shared_sentry == mash::CUSTOM_MASH_SENTRY);

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
    convert_core_ai_data(*this);
#endif
#else
    THISCALL(0x006D71F0, this, a1, a3);
#endif
}

bool core_ai_resource::does_base_graph_exist(resource_key the_graph) const
{
    assert(the_graph.get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

    for (auto &curr : this->my_base_graphs)
    {
        assert(curr->get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

        if (*curr == the_graph) {
            return true;
        }
    }

    return false;
}

bool core_ai_resource::does_locomotion_graph_exist(resource_key the_graph) const
{
    assert(the_graph.get_type() == RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

    for (auto &curr : this->my_locomotion_graphs)
    {
        assert(curr->get_type() != RESOURCE_KEY_TYPE_AI_STATE_GRAPH);

        if (*curr == the_graph) {
            return true;
        }
    }

    return false;
}

} // namespace ai

void core_ai_resource_patch()
{
    {
        FUNC_ADDRESS(address, &ai::core_ai_resource::unmash);
        REDIRECT(0x00568A67, address);
    }
}
