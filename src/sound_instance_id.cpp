#include "sound_instance_id.h"

#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "sound_alias_database.h"
#include "sound_manager.h"

#include <algorithm>

VALIDATE_SIZE(sound_instance, 0x50);
VALIDATE_OFFSET(sound_instance, source_id, 0x4);
VALIDATE_OFFSET(sound_instance, wave_id, 0x8);
VALIDATE_OFFSET(sound_instance, alias, 0xC);
VALIDATE_OFFSET(sound_instance, volume, 0x18);
VALIDATE_OFFSET(sound_instance, pitch, 0x1C);
VALIDATE_OFFSET(sound_instance, flags, 0x30);
VALIDATE_SIZE(sound_instance_slot, 0x54);

#if STANDALONE_SYSTEM
static sound_instance_slot s_sound_instance_slot_storage[128]{};
static sound_instance_slot *s_sound_instance_slot_storage_ptr = s_sound_instance_slot_storage;
sound_instance_slot *&s_sound_instance_slots = s_sound_instance_slot_storage_ptr;
static uint16_t s_sound_instance_generation;
#else
sound_instance_slot *&s_sound_instance_slots = var<sound_instance_slot *>(0x0095C830);
#endif

void sound_instance::stop()
{
#if STANDALONE_SYSTEM
    nslStopSource(source_id);
    source_id = {};
    state = 5;
    const auto slot_index = static_cast<uint16_t>(handle);
    const auto handle_generation = static_cast<uint16_t>(handle >> 16);
    if (slot_index < 128 && s_sound_instance_slots[slot_index].field_50 == handle_generation) {
        s_sound_instance_slots[slot_index].field_50 = 0;
    }
#else
    THISCALL(0x0053E6F0, this);
#endif
}

void sound_instance::set_volume(Float value)
{
#if STANDALONE_SYSTEM
    volume = value < 0.0f ? 1.0f : static_cast<float>(value);
    if (source_id.value != -1) {
        const auto alias_volume = alias != nullptr ? alias->volume : 1.0f;
        const auto type_volume = field_4C < 8 ? sound_manager::get_source_type_volume(field_4C) : 1.0f;
        nslSetSourceVolume(source_id, volume * alias_volume * type_volume);
    }
#else
    THISCALL(0x0054D8F0, this, value);
#endif
}

void sound_instance::set_pitch(Float value)
{
#if STANDALONE_SYSTEM
    pitch = value < 0.0f ? 1.0f : static_cast<float>(value);
    if (source_id.value != -1) {
        const auto alias_pitch = alias != nullptr ? alias->pitch : 1.0f;
        nslSetSourcePitch(source_id, pitch * alias_pitch);
    }
#else
    THISCALL(0x0054D7A0, this, value);
#endif
}

void sound_instance::play()
{
#if STANDALONE_SYSTEM
    if (source_id.value == -1) {
        source_id = nslCreateSource(wave_id);
    }
    if (source_id.value == -1) {
        stop();
        return;
    }
    set_volume(volume);
    set_pitch(pitch);
    if (!nslPlaySource(source_id)) {
        stop();
        return;
    }
    state = 3;
    flags |= 1u;
#else
    THISCALL(0x00556100, this);
#endif
}

sound_instance_id create_native_sound_instance(uint32_t scope, nslWaveID wave_id, sound_alias *alias)
{
#if STANDALONE_SYSTEM
    update_native_sound_instances();
    uint32_t slot_index = 0;
    while (slot_index < 128 && s_sound_instance_slots[slot_index].field_50 != 0) {
        ++slot_index;
    }
    if (slot_index == 128) {
        return {};
    }
    if (++s_sound_instance_generation == 0) {
        ++s_sound_instance_generation;
    }

    auto &slot = s_sound_instance_slots[slot_index];
    slot.instance = {};
    slot.field_50 = s_sound_instance_generation;
    slot.instance.handle = (static_cast<uint32_t>(slot.field_50) << 16) | slot_index;
    slot.instance.source_id = {};
    slot.instance.wave_id = wave_id;
    slot.instance.alias = alias;
    slot.instance.state = 0;
    slot.instance.volume = 1.0f;
    slot.instance.pitch = 1.0f;
    slot.instance.doppler = 1.0f;
    slot.instance.min_distance = alias != nullptr ? alias->min_distance : 10.0f;
    slot.instance.max_distance = alias != nullptr ? alias->max_distance : 30.0f;
    slot.instance.pitch_variation = 1.0f;
    slot.instance.field_4C = scope;
    return sound_instance_id{slot.instance.handle};
#else
    sound_instance_id result{};
    CDECL_CALL(0x00556120, &result, scope, wave_id, alias);
    return result;
#endif
}

sound_instance_id sub_60B960(string_hash sound, Float volume, Float pitch)
{
    if (!g_game_ptr->level.load_completed) {
        return sound_instance_id{};
    }

#if STANDALONE_SYSTEM
    auto result = sound_manager::create_sound_instance(7, sound);
    if (auto *instance = result.get_sound_instance_ptr()) {
        instance->set_volume(volume);
        instance->set_pitch(pitch);
        instance->play();
    }
    return result;
#else
    sound_instance_id result;
    CDECL_CALL(0x0060B960, &result, sound, volume, pitch);
    return result;
#endif
}

sound_instance *sound_instance_id::get_sound_instance_ptr()
{
#if STANDALONE_SYSTEM
    const auto slot_index = static_cast<uint16_t>(field_0);
    const auto generation = static_cast<uint16_t>(field_0 >> 16);
    if (generation == 0 || slot_index >= 128 || s_sound_instance_slots == nullptr) {
        return nullptr;
    }
    auto &slot = s_sound_instance_slots[slot_index];
    return slot.field_50 == generation ? &slot.instance : nullptr;
#else
    return reinterpret_cast<sound_instance *>(THISCALL(0x0050EF00, this));
#endif
}

void update_native_sound_instances()
{
#if STANDALONE_SYSTEM
    for (auto &slot : s_sound_instance_slot_storage) {
        if (slot.field_50 != 0 && slot.instance.state == 3 && !nslSourceIsPlaying(slot.instance.source_id)) {
            slot.instance.stop();
        }
    }
#endif
}

void release_native_sound_instances()
{
#if STANDALONE_SYSTEM
    for (auto &slot : s_sound_instance_slot_storage) {
        if (slot.field_50 != 0) {
            slot.instance.stop();
        }
    }
    nslReleaseSources();
#endif
}
