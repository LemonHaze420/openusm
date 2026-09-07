#pragma once

#include "pfx_interface.h"
#include "string_hash.h"
#include "mashable_vector.h"
#include "resource_pack_group.h"
#include <array>
#include <cstddef>
#include <cstdint>

struct web_sound_params;
// PC shared sound stream: 0x4D9B30, 0x4C64D0, 0x4C6BE0.
struct sound_interface_resource_info {
    std::array<std::uint32_t, 4> field_0;
    std::uint32_t flags;
};

struct sound_interface_event_info {
    std::array<std::uint32_t, 3> field_0;
    mashable_vector<sound_interface_resource_info> resources;
    std::uint32_t field_14;
};

struct shared_sound_interface_info {
    std::uint32_t field_0;
    std::uint32_t references;
    mashable_vector<sound_interface_event_info> events;
    mashable_vector<std::array<std::uint32_t, 4>> web_parameters;
    mashable_vector<resource_pack_group> groups;
    web_sound_params *get_web_sound_params(string_hash a2);
};

static_assert(sizeof(sound_interface_resource_info) == 20);
static_assert(sizeof(sound_interface_event_info) == 24);
static_assert(offsetof(sound_interface_event_info, resources) == 12);
static_assert(sizeof(shared_sound_interface_info) == 32);
static_assert(offsetof(shared_sound_interface_info, events) == 8);
static_assert(offsetof(shared_sound_interface_info, web_parameters) == 16);
static_assert(offsetof(shared_sound_interface_info, groups) == 24);

struct sound_and_pfx_interface : pfx_interface {
    sound_and_pfx_interface();

    web_sound_params *get_web_sound_params(string_hash a1);

    //virtual
    void release_ifc();
};
