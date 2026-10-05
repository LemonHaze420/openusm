#include "sound_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "sound_alias_database.h"
#include "sound_bank_slot.h"
#include "sound_instance_id.h"
#include "sound_source.h"
#include "os_developer_options.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector3d.h"
#include "web_sounds.h"

#include <algorithm>
#include <cmath>
static constexpr int SM_MAX_SOURCE_TYPES = 8;

struct sound_volume {
    float field_0;
    int field_4[7];
};

VALIDATE_SIZE(sound_volume, 0x20);

#if !STANDALONE_SYSTEM

static bool &s_sound_manager_initialized = var<bool>(0x0095C829);

static sound_volume (&s_volumes_by_type)[8] = var<sound_volume[8]>(0x0095C9A8);

#else

struct sound_type_fade {
    float start;
    float target;
    float current;
    float duration;
    float remaining;
    float slope;
};

static sound_type_fade s_type_fade{1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
static uint32_t s_fade_sound_types;
static uint32_t s_sound_manager_flags;

static unsigned int native_sound_type(nslWaveID wave)
{
    const auto *group = nslGetWaveGroupName(wave);
    static constexpr const char *names[] = {"", "SFX", "AMBIENT", "MUSIC", "VOICE", "SCENE", "CINEMATIC", "MOVIE"};
    if (group != nullptr) {
        for (unsigned int i = 1; i < 8; ++i) {
            if (_stricmp(group, names[i]) == 0) {
                return i;
            }
        }
    }
    return 0;
}
static bool &s_sound_manager_initialized = []() -> auto & {
    static bool s_sound_manager_initialized1{};
    return s_sound_manager_initialized1;
}();

static sound_volume (&s_volumes_by_type)[8] = []() -> auto & {
    static sound_volume s_volumes_by_type1[8]{};
    return s_volumes_by_type1;
}();

#endif

sound_alias_database *sound_manager::get_sound_alias_database()
{
    return s_sound_alias_database;
}

void sound_manager::set_sound_alias_database(sound_alias_database *a1)
{
    s_sound_alias_database = a1;
}

const vector3d &sound_manager::get_listener_position()
{
    return var<vector3d>(0x009604EC);
}

void sound_manager::set_listener_position(const vector3d &position)
{
    auto &listener = var<vector3d>(0x009604EC);
    listener = position.is_valid() ? position : ZEROVEC;
    var<vector3d>(0x009485EC) = listener;
    var<uint32_t>(0x00948598) |= 0x380000u;
}

void sound_manager::set_listener_velocity(const vector3d &velocity)
{
    auto &state = var<vector3d>(0x0096017C);
    state = velocity.is_valid() ? velocity : ZEROVEC;
    var<vector3d>(0x009485F8) = state;
    var<uint32_t>(0x00948598) |= 0x1C00000u;
}

void sound_manager::set_listener_orientation(const vector3d &forward, const vector3d &up)
{
    auto &front = var<vector3d>(0x009604E0);
    auto &top = var<vector3d>(0x0095CBD4);
    front = forward;
    top = up;
    front.normalize();
    top.normalize();
    if (!front.is_valid())
        front = ZVEC;
    if (!top.is_valid())
        top = YVEC;
    if (dot(top, front) > 0.99f) {
        front = ZVEC;
        top = YVEC;
    }
    const float front_length = front.length();
    const float top_length = top.length();

    var<vector3d>(0x00948658) = front_length > 1.1920928955078125e-7f ? front / front_length : ZEROVEC;
    var<vector3d>(0x00948664) = top_length > 1.1920928955078125e-7f ? top / top_length : ZEROVEC;
    var<uint32_t>(0x0094859C) |= 0xFC000u;
}

bool sound_manager::is_mission_sound_bank_ready()
{
    return s_sound_bank_slots()[11].m_state != 1;
}

void sound_manager::load_common_sound_bank(bool synchronous)
{
#if STANDALONE_SYSTEM
    static constexpr const char *bank_names[] = {
        "STREAMS_MUSIC",
        "STREAMS_AMBIENT1",
        "STREAMS_AMBIENT2",
        "STREAMS_SFX",
        "STREAMS_INTERFACE",
        "STREAMS_VOICE",
        "STREAMS_SCENE",
        "STREAMS_MISC",
    };
    for (size_t index = 0; index < 8; ++index) {
        s_sound_bank_slots()[index].load("STREAMS", bank_names[index], synchronous, 0);
    }
    auto *scene_name = sub_50F010();
    s_sound_bank_slots()[SB_TYPE_LEVEL_COMMON].load(scene_name, scene_name, synchronous, 0);
#else
    CDECL_CALL(0x0054DB10, synchronous);
#endif
}

void sound_manager::create_inst()
{
#if STANDALONE_SYSTEM
    s_type_fade = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    s_fade_sound_types = 0;
    s_sound_manager_flags = 0;
    for (auto &volume : s_volumes_by_type) {
        volume.field_0 = 1.0f;
    }

    for (auto &slot : s_sound_bank_slots()) {
        slot = {};
        slot.nsl_voice_bank_id = NSL_BANK_ID_INVALID;
        slot.nsl_non_voice_bank_id = NSL_BANK_ID_INVALID;
        slot.field_30 = NSL_BANK_ID_INVALID;
        slot.field_34 = NSL_BANK_ID_INVALID;
    }
    if (!g_is_the_packer)
        web_sounds_manager::create_inst();
    s_sound_manager_initialized = true;
#else
    CDECL_CALL(0x00543500);
#endif
}

sound_source sound_manager::get_sound_source(string_hash sound)
{
    if (g_is_the_packer || os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(0x67))) {
        return {};
    }
    sound_source result{nslFindWave(sound.source_hash_code), nullptr};
    if (result.wave_id.value == UINT32_MAX && s_sound_alias_database != nullptr) {
        result.alias = s_sound_alias_database->get_sound_alias(sound);
        if (result.alias != nullptr) {
            result.wave_id = nslFindWave(result.alias->field_4.source_hash_code);
        }
    }
    return result;
}
sound_instance_id sound_manager::create_sound_instance(uint32_t scope, string_hash sound)
{
#if STANDALONE_SYSTEM
    auto source = get_sound_source(sound);
    if (!source.is_valid()) {
        return {};
    }
    return create_native_sound_instance(scope, source.wave_id, source.alias);
#else
    sound_instance_id result{};
    CDECL_CALL(0x005561D0, &result, scope, sound);
    return result;
#endif
}


void sound_manager::delete_inst()
{
    TRACE("sound_manager::delete_inst");
#if STANDALONE_SYSTEM
    if (!g_is_the_packer)
        web_sounds_manager::delete_inst();
    release_native_sound_instances();
    for (auto &slot : s_sound_bank_slots()) {
        slot.unload();
    }
    s_sound_manager_initialized = false;
#else
    CDECL_CALL(0x00543EF0);
#endif
}

void sound_manager::frame_advance(Float a1)
{
    TRACE("sound_manager::frame_advance");
#if STANDALONE_SYSTEM
    if (g_is_the_packer)
        return;
    web_sounds_manager::frame_advance(a1);
    for (auto &slot : s_sound_bank_slots()) {
        slot.frame_advance(a1);
    }
    if ((s_sound_manager_flags & 1u) != 0 && s_type_fade.remaining > 0.0f) {
        s_type_fade.remaining -= float(a1);
        if (s_type_fade.remaining <= 0.0f) {
            s_type_fade.remaining = 0.0f;
            s_type_fade.current = s_type_fade.target;
        } else {
            s_type_fade.current += float(a1) * s_type_fade.slope;
        }
    }
    for (int i = 0; i < 128; ++i) {
        auto &slot = s_sound_instance_slots[i];
        if (slot.field_50 != 0) {
            slot.instance.set_volume(slot.instance.volume);
        }
    }
    if ((s_sound_manager_flags & 1u) != 0 && std::fpclassify(s_type_fade.current - s_type_fade.target) == FP_ZERO) {
        s_sound_manager_flags &= ~1u;
        if ((s_sound_manager_flags & 2u) != 0) {
            for (int i = 0; i < 128; ++i) {
                auto &slot = s_sound_instance_slots[i];
                if (slot.field_50 != 0 &&
                    (s_fade_sound_types & (1u << native_sound_type(slot.instance.wave_id))) != 0) {
                    nslPauseSource(slot.instance.source_id);
                    slot.instance.state = 4;
                }
            }
            s_sound_manager_flags &= ~2u;
        }
    }
    update_native_sound_instances();
    nslUpdate();
#else
    CDECL_CALL(0x00551C20, a1);
#endif
}

void sound_manager::load_hero_sound_bank(const char *a1, bool a2)
{
    assert(s_sound_bank_slots()[SB_TYPE_LEVEL_COMMON].get_state() == SB_STATE_LOADED);

    char *v11 = sub_50F010();

    s_sound_bank_slots()[SB_TYPE_HERO].load(v11, a1, a2, 0);
}

void sound_manager::unload_hero_sound_bank()
{
    assert(s_sound_bank_slots()[SB_TYPE_LEVEL_COMMON].get_state() == SB_STATE_LOADED);

    s_sound_bank_slots()[SB_TYPE_HERO].unload();
}

float sound_manager::get_source_type_volume(unsigned int source_type)
{
    assert(s_sound_manager_initialized);
    assert(source_type < SM_MAX_SOURCE_TYPES);

    return s_volumes_by_type[source_type].field_0;
}

float sound_manager::get_effective_source_type_volume(unsigned int source_type)
{
    const auto volume = get_source_type_volume(source_type);
#if STANDALONE_SYSTEM
    if ((s_sound_manager_flags & 1u) != 0 && (s_fade_sound_types & (1u << source_type)) != 0 &&
        ((volume < s_type_fade.current || volume > s_type_fade.target) &&
         (volume > s_type_fade.current || volume < s_type_fade.target))) {
        return std::clamp(s_type_fade.current * volume, 0.0f, 1.0f);
    }
#endif
    return volume;
}
#if STANDALONE_SYSTEM
float sound_manager::get_wave_type_volume(nslWaveID wave)
{
    return get_effective_source_type_volume(native_sound_type(wave));
}
#endif
void sound_manager::set_source_type_volume(unsigned int source_type, Float value, Float duration)
{
    assert(s_sound_manager_initialized);
    assert(source_type < SM_MAX_SOURCE_TYPES);
#if STANDALONE_SYSTEM
    (void)duration;
    s_volumes_by_type[source_type].field_0 = value;
#else
    CDECL_CALL(0x0050FC50, source_type, value, duration);
#endif
}

void sound_manager::unpause_all_sounds()
{
#if STANDALONE_SYSTEM
    for (int i = 0; i < 128; ++i) {
        auto &slot = s_sound_instance_slots[i];
        if (slot.field_50 != 0) {
            nslUnpauseSource(slot.instance.source_id);
            slot.instance.state = nslSourceIsPaused(slot.instance.source_id)    ? 4
                                  : nslSourceIsPlaying(slot.instance.source_id) ? 3
                                                                                : 0;
        }
    }
#else
    CDECL_CALL(0x00520520);
#endif
}

int sound_manager::fade_sounds_by_type(uint32_t a1, Float a2, Float a3, bool a4)
{
#if STANDALONE_SYSTEM
    s_fade_sound_types = a1;
    s_type_fade.start = s_type_fade.current;
    s_type_fade.target = a2;
    if (a3 <= 0.0f) {
        s_type_fade.current = a2;
        s_type_fade.remaining = 0.0f;
    } else {
        s_type_fade.duration = a3;
        s_type_fade.remaining = a3;
        s_type_fade.slope = (float(a2) - s_type_fade.current) / float(a3);
    }
    s_sound_manager_flags = (s_sound_manager_flags | 1u) & ~2u;
    if (a4) {
        s_sound_manager_flags |= 2u;
    }
    return s_sound_manager_flags;
#else
    int(__cdecl * func)(uint32_t a1, Float a2, Float a3, bool a4) = CAST(func, 0x0050FA50);
    return func(a1, a2, a3, a4);
#endif
}

char *sub_50F010()
{
    if constexpr (1) {
        int curr_char = strlen(g_scene_name) - 1;
        if (curr_char > 0) {
            while (g_scene_name[curr_char] != '\\') {
                if (--curr_char <= 0) {
                    goto LABEL_4;
                }
            }
            return &g_scene_name[curr_char + 1];
        }
    LABEL_4:
        if (g_scene_name[curr_char] == '\\') {
            return &g_scene_name[curr_char + 1];
        }

        return &g_scene_name[curr_char];
    } else {
        char *(__cdecl * func)() = CAST(func, 0x0050F010);
        return func();
    }
}

void sub_54DC10(const char *a1, bool a2)
{
    assert(s_sound_bank_slots()[SB_TYPE_LEVEL_COMMON].get_state() == SB_STATE_LOADED);

    assert(s_sound_bank_slots()[SB_TYPE_MOVIE].get_state() == SB_STATE_EMPTY);

    auto *v2 = sub_50F010();
    s_sound_bank_slots()[SB_TYPE_MISSION].load(v2, a1, a2, 0);
}

int sub_79A160()
{
    if constexpr (0) {
    } else {
        return CDECL_CALL(0x0079A160);
    }
}

void sound_manager_patch()
{
    {
        REDIRECT(0x0055D6F4, sound_manager::frame_advance);
        REDIRECT(0x00559F87, sound_manager::frame_advance);
    }
}
