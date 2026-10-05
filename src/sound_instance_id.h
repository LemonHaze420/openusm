#pragma once

#include "float.hpp"
#include "nslbank.h"
#include "nslsource.h"
#include "string_hash.h"

#include <cstdint>

struct sound_alias;
struct sound_interface;
struct sound_instance_id;

struct sound_instance {
    uint32_t handle;
    nslSourceID source_id;
    nslWaveID wave_id;
    sound_alias *alias;
    uint32_t emitter_id;
    int state;
    float volume;
    float pitch;
    float min_distance;
    float max_distance;
    float doppler;
    uint32_t field_2C;
    uint16_t flags;
    uint16_t field_32;
    uint32_t start_offset;
    float position[3];
    float elapsed;
    float pitch_variation;
    uint32_t field_4C;

    void stop();
    void set_volume(Float value);
    void set_pitch(Float value);
    void play();


    static sound_instance_id play_and_add(sound_interface *owner, string_hash sound, float volume, float pitch,
                                          float doppler, float min_distance, float max_distance);
};

struct sound_instance_slot {
    sound_instance instance;
    uint32_t field_50;
};

struct sound_instance_id {
    uint32_t field_0;

    sound_instance_id() = default;
    sound_instance_id(uint32_t value) : field_0(value) {}

    sound_instance *get_sound_instance_ptr();
};

[[nodiscard]] extern sound_instance_id sub_60B960(string_hash sound, Float volume, Float pitch);
extern sound_instance_id create_native_sound_instance(uint32_t scope, nslWaveID wave_id, sound_alias *alias);
extern void update_native_sound_instances();
extern void release_native_sound_instances();

extern sound_instance_slot *&s_sound_instance_slots;
