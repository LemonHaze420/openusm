#include "game_settings.h"

#include "common.h"
#include "actor.h"
#include "advanced_entity_ptrs.h"
#include "chuck_callbacks.h"
#include "damage_interface.h"
#include "femanager.h"
#include "frontendmenusystem.h"
#include "main_menu_memcard_check.h"
#include "func_wrapper.h"
#include "game.h"
#include "memory.h"
#include "mission_manager.h"
#include "mstring.h"
#include "marky_camera.h"
#include "nsl/src/nsl/nslsource.h"
#include "os_developer_options.h"
#include "ped_spawner.h"
#include "physical_interface.h"
#include "region.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "resource_partition.h"
#include "rumble_manager.h"
#include "script_manager.h"
#include "settings.h"
#include "sound_manager.h"
#include "trace.h"
#include "terrain.h"
#include "traffic.h"
#include "utility.h"
#include "variables.h"
#include "wds.h"

#include <cassert>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <malloc.h>
#include <windows.h>
#include <vector>
#if STANDALONE_SYSTEM
#include <map>

namespace {
std::map<std::uint32_t, float> standalone_numeric_game_settings;
}
#endif
namespace {
void __fastcall game_settings_observer_callback(
    MemoryUnitManager::Observer *observer,
    void *,
    MemoryUnitManager::eOperation operation)
{
    static_cast<game_settings *>(observer)->Callback(operation);
}

MemoryUnitManager::ObserverVTable game_settings_observer_vtable {
    &game_settings_observer_callback,
};


bool is_newer_save_timestamp(const game_save_timestamp &candidate,
                             const game_save_timestamp &reference)
{
    if (candidate.year != reference.year)
        return candidate.year > reference.year;
    if (candidate.month != reference.month)
        return candidate.month > reference.month;
    if (candidate.day != reference.day)
        return candidate.day > reference.day;
    if (candidate.hour != reference.hour)
        return candidate.hour > reference.hour;
    if (candidate.minute != reference.minute)
        return candidate.minute > reference.minute;
    return candidate.second > reference.second;
}

}

VALIDATE_SIZE(game_settings, 0x4CCu);

#if USE_CXX_CONSTRUCTOR

void __stdcall vector_constructor(void *a1, uint32_t size, int count, void(__fastcall *constructor)(void *),
                                  [[maybe_unused]] void(__fastcall *destructor)(void *))
{
    FUNC_ADDRESS(address, &game_data_essentials::initialize);

    constructor = bit_cast<decltype(constructor)>(address);

    for (int i{0}; i < count; ++i) {
        constructor(static_cast<int *>(a1));
        a1 = static_cast<char *>(a1) + size;
    }
}
#endif

game_settings::game_settings() : field_4{""}
{
    if constexpr (1) {
        m_vtbl = &game_settings_observer_vtable;

        this->field_4BF = false;
        this->field_4C0 = false;
        this->field_4C1 = false;
        this->field_4C2 = false;
        this->field_4BF = false;

        MemoryUnitManager::Initialize(0);

        [[maybe_unused]] auto v4 = os_developer_options::instance->get_string(os_developer_options::strings_t::SKU);

        MemoryUnitManager::RegisterObserver(this);

        this->set_script_buffer_size();

        strncpy(this->field_4A8, "", 12u);
        this->field_4A8[11] = '\0';

        this->field_4B8 = 0;
        this->m_slot_num = 0;
        this->field_4C8 = 0;
    } else {
        THISCALL(0x0057BF50, this);
    }
}

void sub_5288B0(void *Memory)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (Memory != nullptr) {
            mem_total_allocated -= _msize(Memory);
            std::free(Memory);
        }
    } else {
        CDECL_CALL(0x005288B0, Memory);
    }
}

game_settings::~game_settings()
{
    m_vtbl = &game_settings_observer_vtable;

    for (int i = 0; i < 3; ++i) {
        mem_freealign(this->field_49C[i]);
    }

    for (int i = 0; i < 2; ++i) {
        sub_5288B0(this->field_494[i]);
    }
}

void game_settings::Callback(MemoryUnitManager::eOperation a2)
{
    const auto status = MemoryUnitManager::GetLastError();
    if (status == MemoryUnitManager::STATUS_OK ||
        (status == MemoryUnitManager::STATUS_FILE_NOT_FOUND &&
         a2 == MemoryUnitManager::OPERATION_SAVE)) {
        if (a2 == MemoryUnitManager::OPERATION_LOAD) {
            for (int i = 0; i < 3; ++i) {
                std::memcpy(
                    &field_28C[i], field_49C[i],
                    sizeof(game_data_essentials));
                m_game_data_valid[i] = true;
            }
            field_4BF = false;
            if (g_femanager.m_fe_menu_system != nullptr)
                static_cast<main_menu_memcard_check *>(
                    g_femanager.m_fe_menu_system->field_4[2])
                    ->OnSuccessfulLoad();
        }
        return;
    }

    if (a2 == MemoryUnitManager::OPERATION_LOAD)
        field_4BF = false;
    if (g_femanager.m_fe_menu_system != nullptr)
        static_cast<main_menu_memcard_check *>(
            g_femanager.m_fe_menu_system->field_4[2])
            ->OperationFailed(a2, status);
}

void game_settings::init_script_buffer()
{
    TRACE("game_settings::init_script_buffer");

    if constexpr (1) {
        this->sub_579990();
        script_manager::save_game_var_buffer(this->field_494[0]);
        script_manager::save_game_var_buffer(this->field_494[1]);

    } else {
        THISCALL(0x005799E0, this);
    }
}

void game_settings::update_miles_crawled_venom(Float a2)
{
    this->field_340.field_C0 += a2 * 0.0006213712;
}

void game_settings::update_miles_crawled_spidey(Float a2)
{
    this->field_340.field_98 += a2 * 0.0006213712;
}

void game_settings::start_new_game()
{
#if STANDALONE_SYSTEM
    os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(100));
    auto *mission_streamer =
        resource_manager::get_partition_pointer(RESOURCE_PARTITION_MISSION)->get_streamer();
    auto *mission_slots = mission_streamer->get_pack_slots();
    if (mission_slots != nullptr && !mission_slots->empty()) {
        auto *context = resource_manager::get_best_context(RESOURCE_PARTITION_MISSION);
        resource_manager::push_resource_context(context);
        resource_manager::pop_resource_context();
    }

    if (!g_is_the_packer)
        nslReleaseSources();

    auto *the_terrain = g_world_ptr->the_terrain;
    auto *district_streamer =
        resource_manager::get_partition_pointer(RESOURCE_PARTITION_DISTRICT)->get_streamer();
    auto *strip_streamer =
        resource_manager::get_partition_pointer(RESOURCE_PARTITION_STRIP)->get_streamer();
    do {
        district_streamer->flush(game::render_empty_list, 0.02f);
        strip_streamer->flush(game::render_empty_list, 0.02f);
    } while (!district_streamer->is_idle() || !strip_streamer->is_idle());

    auto *district_slots = district_streamer->get_pack_slots();
    for (unsigned int i = 0; i < district_slots->size(); ++i) {
        if (!(*district_slots)[i]->is_empty())
            district_streamer->unload_internal(i);
        district_streamer->flush(game::render_empty_list, 0.02f);
        the_terrain->force_streamer_refresh();
    }

    g_game_ptr->enable_marky_cam(false, false, g_world_ptr->field_28.field_44->field_1D8, 0.0f);
    g_game_ptr->field_15D = false;
    field_4C0 = false;
    field_340.init();
    traffic::enable_traffic(false, true);
    ped_spawner::cleanup();

    const resource_key no_context{};
    const auto world_script = create_resource_key_from_path(
        g_world_ptr->field_140.field_8.c_str(), RESOURCE_KEY_TYPE_SCRIPT);
    script_manager::un_load(world_script, false, no_context);


    std::vector<entity *> actors_to_remove;
    for (auto &entities : g_world_ptr->ent_mgr.entities.field_0) {
        for (auto *ent : entities) {
            if (ent == nullptr || !ent->is_an_actor())
                continue;
            auto *act = static_cast<actor *>(ent);
            if ((act->adv_ptrs != nullptr && act->adv_ptrs->my_script != nullptr) ||
                (act->has_physical_ifc() && act->physical_ifc()->is_prop_physics_running()) ||
                (act->field_4 & 0x10000u) != 0) {
                actors_to_remove.push_back(ent);
            }
        }
    }
    for (auto *ent : actors_to_remove) {
        if ((ent->field_8 & 0x200u) == 0)
            g_world_ptr->ent_mgr.destroy_entity(ent);
    }

    script_manager::clear();
    register_chuck_callbacks();
    script_manager::reinit_script_vars();
    for (int i = 0; i < the_terrain->total_regions; ++i)
        the_terrain->set_district_variant(the_terrain->regions[i]->district_id, 0, false);

    script_manager::init_game_var();
    script_manager::load(resource_key{string_hash{"init_gv"}, RESOURCE_KEY_TYPE_SCRIPT}, 0,
                         resource_manager::get_best_context(RESOURCE_PARTITION_COMMON), no_context);
    script_manager::load(resource_key{string_hash{"init_sv"}, RESOURCE_KEY_TYPE_SCRIPT}, 0,
                         resource_manager::get_best_context(RESOURCE_PARTITION_COMMON), no_context);
    script_manager::link();
    script_manager::run(0.0f, false);
    script_manager::clear();
    register_chuck_callbacks();
    script_manager::load(
        create_resource_key_from_path(g_world_ptr->field_140.field_8.c_str(), RESOURCE_KEY_TYPE_SCRIPT),
        1, resource_manager::get_best_context(RESOURCE_PARTITION_COMMON), no_context);
    script_manager::link();
    sub_579990();
    script_manager::save_game_var_buffer(field_494[0]);
    script_manager::save_game_var_buffer(field_494[1]);
    g_world_ptr->field_140.hook_up_global_script_object();
    mission_manager::s_inst->set_real_time();
#else
    THISCALL(0x0057EAB0, this);
#endif
}

void game_settings::frame_advance(Float a2)
{
    TRACE("game_settings::frame_advance");

    if constexpr (1) {
        if (this->field_4C2 && ++this->field_4C8 > 2) {
            this->load_game(this->m_slot_num);
        }

    } else {
        THISCALL(0x005802D0, this, a2);
    }
}

void game_settings::export_game_options()
{
    sound_manager::set_source_type_volume(0, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(1u, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(2u, Settings::MusicVolume, 0.0);
    sound_manager::set_source_type_volume(3u, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(4u, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(5u, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(6u, Settings::GameSoundVolume, 0.0);
    sound_manager::set_source_type_volume(7u, Settings::GameSoundVolume, 0.0);
    auto *v2 = input_mgr::instance->rumble_ptr;
    if (this->field_340.field_31) {
        v2->enable_vibration();
    } else {
        v2->disable_vibration();
    }

    if (g_world_ptr->get_hero_ptr(0) != nullptr) {
        mString a1{"gv_hero_spawn_point"};

        auto *v3 = (const vector3d *)script_manager::get_game_var_address(a1, nullptr, nullptr);

        g_world_ptr->malor_point(*v3, 0, false);
    }
}

void game_settings::export_game_settings()
{
    if (g_world_ptr != nullptr) {
        auto *v1 = g_world_ptr->get_hero_ptr(0);
        if (v1 != nullptr) {
            if (v1->has_damage_ifc()) {
                auto &v3 = v1->damage_ifc()->field_1FC.field_0[2];
                auto *v2 = v1->damage_ifc();
                v2->field_1FC.sub_48BFB0(v3);
            }
        }
    }
}

static constexpr auto NUM_SOFT_SAVE_BUFFERS = 2;

void game_settings::soft_load(uint32_t soft_save_type)
{
    if constexpr (1) {
        assert(soft_save_type < NUM_SOFT_SAVE_BUFFERS);

        if (soft_save_type == 1) {
            std::memcpy(this->field_494[0], this->field_494[1], this->field_4B4);
        }

        g_game_ptr->field_15D = false;
        this->field_4C0 = false;
        this->export_game_settings();
        script_manager::load_game_var_buffer(this->field_494[soft_save_type]);
        mission_manager::s_inst->set_real_time();

    } else {
        THISCALL(0x0057C1E0, this, soft_save_type);
    }
}

void game_settings::update_miles_run_venom(Float a2)
{
    this->field_340.field_BC += a2 * 0.0006213712f;
}

void game_settings::update_miles_web_zipping(Float a2)
{
    this->field_340.m_miles_web_zipping += a2 * 0.0006213712f;
}

void game_settings::update_miles_run_spidey(Float a2)
{
    this->field_340.field_94 += a2 * 0.0006213712f;
}

void game_settings::set_script_buffer_size()
{
    TRACE("game_settings::set_script_buffer_size");

    this->field_4B4 = script_manager::save_game_var_buffer(nullptr);
}

void game_settings::update_web_fluid_used(Float a2)
{
    this->field_340.m_web_fluid_used += a2;
}

void game_settings::reset_container(bool)
{
    field_4.Reset("Save");
    if (field_4B4 == 0)
        field_4B4 = script_manager::save_game_var_buffer(nullptr);
    const unsigned int buffer_size = MemoryUnitManager::GetGameSaveSize(
        std::max(0x4000, 2 * field_4B4 + 400));
    for (int i = 0; i < 3; ++i) {
        if (field_49C[i] == nullptr)
            field_49C[i] = static_cast<char *>(arch_memalign(0x20, buffer_size));
        std::memset(field_49C[i], 0, buffer_size);
        char filename[16]{};
        std::snprintf(filename, sizeof(filename), "Save%d", i);
        field_4.AddFile(filename, field_49C[i], buffer_size);
        m_game_data_valid[i] = false;
    }
}

int game_settings::load()
{
    this->reset_container(true);
    return static_cast<int>(MemoryUnitManager::LoadGame(this->field_4));
}

void game_settings::load_game(int slot_num)
{
    assert(this->m_game_data_valid[slot_num]);

    if constexpr (1) {
        if (this->field_4C2) {
            if (this->field_4C8 > 2) {
                this->field_4C2 = false;
                this->start_new_game();
                mission_manager::s_inst->lock();
                this->field_4C1 = false;
                std::memcpy(&this->field_340,
                            this->field_49C[slot_num] + sizeof(game_data_essentials),
                            sizeof(this->field_340));
                this->sub_579990();
                std::memcpy(this->field_494[0],
                            this->field_49C[slot_num] + sizeof(game_data_essentials) + sizeof(game_data_meat),
                            this->field_4B4);
                std::memcpy(this->field_494[1],
                            this->field_49C[slot_num] + this->field_4B4 + sizeof(game_data_essentials) +
                                sizeof(game_data_meat),
                            this->field_4B4);
                this->field_4BF = true;
                this->soft_load(0);
                this->export_game_settings();
                this->export_game_options();

                std::memcpy(this->field_4A8, this->field_28C[slot_num].field_2E, sizeof(this->field_4A8));

                this->field_4B8 = slot_num;
                auto *v4 = g_world_ptr->get_chase_cam_ptr(0);
                g_game_ptr->set_current_camera(v4, true);
            }
        } else {
            this->field_4C2 = true;
            this->field_4C8 = 0;
            this->m_slot_num = slot_num;
            mission_manager::s_inst->unload_script_now();
            auto v3 = g_world_ptr->num_players;
            if (v3 > 0) {
                g_world_ptr->remove_player(v3 - 1);
            }
        }

    } else {
        THISCALL(0x0057F410, this, slot_num);
    }
}

int game_settings::get_most_recent_game_slot() const
{
    game_save_timestamp most_recent {1990, 1, 1, 0, 0, 0};
    int most_recent_slot = -1;

    for (int slot = 0; slot < 3; ++slot) {
        if (this->m_game_data_valid[slot] &&
            is_newer_save_timestamp(this->field_28C[slot].timestamp, most_recent)) {
            most_recent = this->field_28C[slot].timestamp;
            most_recent_slot = slot;
        }
    }

    return most_recent_slot;
}

void game_settings::load_most_recent_game()
{
    const auto slot = this->get_most_recent_game_slot();
    if (slot >= 0)
        this->load_game(slot);
}

game_save_timestamp *GetSystemDate(game_save_timestamp *out)
{
    if constexpr (STANDALONE_SYSTEM) {
        SYSTEMTIME local_time{};
        GetLocalTime(&local_time);
        out->year = local_time.wYear;
        out->month = local_time.wMonth;
        out->day = local_time.wDay;
        out->hour = local_time.wHour;
        out->minute = local_time.wMinute;
        out->second = local_time.wSecond;
        return out;
    } else {
        return (game_save_timestamp *)CDECL_CALL(0x00573510, out);
    }
}

void game_settings::collect_game_settings()
{
    if constexpr (1) {
        if (g_world_ptr != nullptr) {
            auto *hero_ptr = g_world_ptr->get_hero_ptr(0);
            if (hero_ptr != nullptr) {
                if (hero_ptr->has_damage_ifc()) {
                    this->field_340.m_hero_health = hero_ptr->damage_ifc()->field_1FC.field_0[0];
                    this->field_4BF = true;
                    return;
                }

                this->field_340.m_hero_health = 0.0;
            }
        }

        this->field_4BF = true;

    } else {
        THISCALL(0x00579B90, this);
    }
}

void game_settings::sub_579990()
{
    if constexpr (1) {
        if (this->field_494[0] == nullptr) {
            if (this->field_4B4 == 0) {
                this->set_script_buffer_size();
            }

            this->field_494[0] = static_cast<char *>(arch_malloc(this->field_4B4));
            this->field_494[1] = static_cast<char *>(arch_malloc(this->field_4B4));
        }

    } else {
        THISCALL(0x00579990, this);
    }
}

void game_settings::collect_game_options()
{
    THISCALL(0x00579BF0, this);
}

char *game_settings::get_buffer(int a2)
{
    return this->field_49C[a2];
}

int game_settings::soft_save(uint32_t soft_save_type)
{
    assert(soft_save_type < NUM_SOFT_SAVE_BUFFERS);

    this->collect_game_settings();
    this->sub_579990();
    return script_manager::save_game_var_buffer(this->field_494[soft_save_type]);
}

static constexpr auto MAX_GAME_SIZE = 16384;

int game_settings::get_game_size()
{
    if (this->field_4B4 == 0) {
        this->field_4B4 = script_manager::save_game_var_buffer(nullptr);
    }

    auto size = 2 * this->field_4B4 + 400;
    assert(size <= MAX_GAME_SIZE && "Uh-oh!!  We've exceeded the maximum size for our save game!!!");

    if (size < MAX_GAME_SIZE) {
        return MAX_GAME_SIZE;
    }

    return size;
}

void game_settings::save(int slot_num)
{
    if constexpr (1) {
        static constexpr auto MAX_GAMEFILE_SLOTS = 3;

        assert(slot_num >= 0 && slot_num <= MAX_GAMEFILE_SLOTS);

        auto &v1 = this->field_28C[slot_num];

        strncpy(v1.field_14, "02:29:05", 25u);
        strncpy(v1.field_2E, this->field_4A8, 12u);

        auto a2a = *(float *)script_manager::get_game_var_address(mString{"real_world_timer"}, nullptr, nullptr);

        v1.field_C = (int)a2a;

        GetSystemDate(&v1.timestamp);
        v1.field_10 = this->field_4B4;
        this->m_game_data_valid[slot_num] = true;
        this->soft_save(0);
        this->collect_game_options();
        auto size = this->get_game_size();

        memset(this->field_49C[slot_num], 0, MemoryUnitManager::GetGameSaveSize(size));
        std::memcpy(this->field_49C[slot_num], &v1, sizeof(game_data_essentials));
        std::memcpy(this->field_49C[slot_num] + sizeof(game_data_essentials), &this->field_340, sizeof(game_data_meat));
        std::memcpy(this->field_49C[slot_num] + sizeof(game_data_essentials) + sizeof(game_data_meat),
                    this->field_494[0],
                    this->field_4B4);
        std::memcpy(&this->field_49C[slot_num][this->field_4B4 + 400], this->field_494[1], this->field_4B4);

        if (!MemoryUnitManager::SaveGame(this->field_4)) {
            g_game_ptr->field_15D = false;
        }

        this->field_4B8 = slot_num;

    } else {
        THISCALL(0x0057D460, this, slot_num);
    }
}

bool game_settings::set_str(const resource_key &att, const mString &a3)
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    static resource_key pstring_set[2]{
        resource_key{string_hash{int(to_hash("HERO_NAME"))}, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE},
        resource_key{string_hash{int(to_hash("DISTRICT_NAME"))}, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE},
    };

    if (att == pstring_set[0]) {
        this->field_340.m_hero_name = a3.c_str();
        return true;
    } else if (att == pstring_set[1]) {
        this->field_340.m_district_name = a3.c_str();
        return true;
    } else {
        assert(0 && "invalid game setting");

        return false;
    }
}

bool game_settings::get_str(const resource_key &att, mString &a3) const
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    static resource_key pstring_get[2]{
        resource_key{string_hash{int(to_hash("HERO_NAME"))}, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE},
        resource_key{string_hash{int(to_hash("DISTRICT_NAME"))}, RESOURCE_KEY_TYPE_IFC_ATTRIBUTE},
    };

    if (att == pstring_get[0]) {
        a3 = this->field_340.m_hero_name.to_string();
        return true;
    } else if (att == pstring_get[1]) {
        a3 = this->field_340.m_district_name.to_string();
        return true;
    } else {
        assert(0 && "invalid game setting");

        return false;
    }
}

bool game_settings::set_num(const resource_key &att, Float a3)
{
    TRACE("game_settings::set_num", att.m_hash.to_string());

    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

#if STANDALONE_SYSTEM
    standalone_numeric_game_settings[att.m_hash.source_hash_code] = a3;
    return true;
#else
    bool(__fastcall * func)(const void *, void *edx, const resource_key *, Float) = CAST(func, 0x00573AE0);
    return func(this, nullptr, &att, a3);
#endif
}

bool game_settings::get_num(const resource_key &att, float &a3, [[maybe_unused]] bool a4) const
{
    TRACE("game_settings::get_num", att.m_hash.to_string());

    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

#if STANDALONE_SYSTEM
    const auto found = standalone_numeric_game_settings.find(att.m_hash.source_hash_code);
    if (found == standalone_numeric_game_settings.end()) {
        return false;
    }
    a3 = found->second;
    return true;
#else
    bool(__fastcall * func)(const void *, void *edx, const resource_key *att, float *a3, bool a4) =
        CAST(func, 0x00575930);
    bool result = func(this, nullptr, &att, &a3, a4);
    sp_log("%f", a3);

    if (!result) {
        assert(0 && "invalid game setting");
    }

    return result;
#endif
}

void game_settings_patch()
{
    {
        FUNC_ADDRESS(address, &game_settings::frame_advance);
        REDIRECT(0x0055D770, address);
    }

    {
        FUNC_ADDRESS(address, &game_settings::set_num);
        REDIRECT(0x00663BED, address);
    }

    {
        FUNC_ADDRESS(address, &game_settings::get_num);
        REDIRECT(0x00663C6A, address);
    }

    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &game_settings::load_game);
            //REDIRECT(0x005802F8, address);
        }
    }
}
