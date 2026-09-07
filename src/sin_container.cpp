#include "sin_container.h"

#include "common.h"
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "utility.h"
#include "chuck_callbacks.h"
#include "dynamic_rtree.h"
#include "region.h"
#include "region_mash_info.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "script_manager.h"
#include "sin_district_container.h"
#include "sin_strip_container.h"
#include "terrain.h"
#include "wds.h"

#include <cstdio>
#include <limits>
#include <type_traits>

VALIDATE_SIZE(sin_container, 0x30);
VALIDATE_OFFSET(sin_container, field_24, 0x24);

void sin_container::setup_world()
{
    TRACE("sin_container::setup_world");
    if (g_world_ptr->the_terrain != nullptr) {
        return;
    }

    auto *terrain = g_world_ptr->create_terrain(mString{field_4});
    auto strip_count = field_24.size();
    delete[] terrain->strips;
    using terrain_strip = std::remove_reference_t<decltype(*terrain->strips)>;
    terrain->strips = new terrain_strip[strip_count];
    terrain->total_strips = 0;

    for (int strip_index = 0; strip_index < strip_count; ++strip_index) {
        auto &strip = field_24[strip_index];
        auto runtime_strip = terrain->add_strip(mString{strip.strip_name});

        for (int district_index = 0; district_index < strip.field_4.size(); ++district_index) {
            auto &district = strip.field_4[district_index];
            auto *reg = terrain->find_region(string_hash{district.district_name});
            assert(reg != nullptr);

            if (district.field_0[1] >= 0 && reg->mash_info != nullptr) {
                auto packed = static_cast<uint32_t>(district.field_0[1]);
                reg->mash_info->field_20 = color{
                    static_cast<float>(packed & 0xFF) / 255.0f,
                    static_cast<float>((packed >> 8) & 0xFF) / 255.0f,
                    static_cast<float>((packed >> 16) & 0xFF) / 255.0f,
                    1.0f};
            }

            auto ground_level = bit_cast<float>(district.field_0[4]);
            if (static_cast<uint32_t>(district.field_0[4]) != 0x7F7FFFFFu) {
                reg->field_BC = ground_level;
            }

            auto flags = district.field_0[6];
            if (flags & 2)
                reg->flags |= 0x100;
            if (flags & 0x20)
                reg->flags |= 0x40000;
            if (flags & 8)
                reg->flags |= 1;
            if (flags & 0x10)
                reg->flags |= 0x8000;
            reg->flags |= 0x4000;
            if (flags & 4)
                reg->flags |= 0x20;

            reg->multiblock_number = district.field_0[5];
            reg->district_id = district.field_0[0];
            reg->strip_id = runtime_strip;
        }
    }

    script_manager::clear();
    register_chuck_callbacks();
    resource_key master_key{string_hash{master_script_name}, RESOURCE_KEY_TYPE_SCRIPT};
    resource_key empty_key{};
    auto *common_slot = resource_manager::get_best_context(RESOURCE_PARTITION_COMMON);
    script_manager::load(master_key, 1u, common_slot, empty_key);
    mString master_script;
    get_master_script_name(&master_script);
    g_world_ptr->field_140.field_8 = master_script;
    g_world_ptr->create_water_kill_trigger();
}

void sin_container::un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *)
{
    this->un_mash(a2, a3, a4);
}

void sin_container::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    if constexpr (1) {
        a4->rebase(4u);

        auto v5 = *a4->get<int>();
        this->master_script_name = a4->get<char>(v5);
        a4->rebase(4u);

        auto v9 = *a4->get<int>();
        this->field_4 = a4->get<char>(v9);

        this->field_24.un_mash(a2, &this->field_24, a4, nullptr);
    } else {
        THISCALL(0x00520B00, this, a2, a3, a4);
    }
}

mString sin_container::sub_55F530()
{
    mString a2{this->master_script_name};
    return a2;
}

mString *sin_container::get_master_script_name(mString *out)
{
    *out = {this->master_script_name};
    return out;
}

void sin_container_patch()
{
    {
        FUNC_ADDRESS(address, &sin_container::setup_world);
        REDIRECT(0x0055CA45, address);
    }
}
