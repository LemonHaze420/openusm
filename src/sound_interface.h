#pragma once

#include "entity_base_interface.h"

#include "float.hpp"
#include "vector3d.h"
#include "sound_instance_id.h"
#include "string_hash.h"

#include <cstdint>

struct shared_sound_interface_info;
struct sound_source;

struct sound_emitter_id {
    int field_0;
};

enum eTerrainSoundType {
    TERRAIN_SOUND_GROUND_FOOTSTEP_L,
    TERRAIN_SOUND_GROUND_FOOTSTEP_R,
    TERRAIN_SOUND_WALL_FOOTSTEP_L,
    TERRAIN_SOUND_WALL_FOOTSTEP_R,
    TERRAIN_SOUND_WALL_CRAWLSTEP_L,
    TERRAIN_SOUND_WALL_CRAWLSTEP_R,
    TERRAIN_SOUND_ATTACK_IMPACT,
    TERRAIN_SOUND_PHYSICS_IMPACT,
    TERRAIN_SOUND_ATTACH_TO_WALL,
    TERRAIN_SOUND_JUMP_LAND,
    TERRAIN_SOUND_BODY_BULLET_HIT,
    TERRAIN_SOUND_BODY_BULLET_BLOCK,
};

struct sound_interface : entity_base_interface {
    int field_4;
    bool dynamic;
    sound_emitter_id field_C;
    shared_sound_interface_info *field_10;
    vector3d field_14;
    float field_20;
    float field_24;

    sound_interface();
    void copy(const sound_interface &source);


    sound_instance_id play_sound(sound_source source, float volume, float pitch, float doppler, float min_distance,
                                 float max_distance);
    sound_instance_id play_sound_grp(string_hash group, float volume, float pitch, float doppler, float min_distance,
                                     float max_distance);
    sound_instance_id play_sound_grp_at(string_hash group, const vector3d *position, float volume, float pitch,
                                        float doppler, float min_distance, float max_distance,
                                        sound_interface *emitter_owner = nullptr, uint32_t instance_scope = 0);

    //0x004D1910
    static void frame_advance_all_sound_ifc(Float a3);


    sound_instance_id play_terrain_sound(eTerrainSoundType type, string_hash terrain, float volume,
                                         const vector3d *position = nullptr);
};

void release_native_sound_emitter(sound_interface *owner);
void frame_advance_native_sound_emitter(sound_interface *owner, Float elapsed);
bool native_sound_emitter_count(uint32_t emitter_id, unsigned &count);
bool stop_first_native_emitter_sound(sound_interface *owner);


extern void sound_interface_patch();
