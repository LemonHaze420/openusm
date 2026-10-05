#pragma once

#include "entity_base_vhandle.h"
#include "float.hpp"
#include "sound_instance_id.h"
#include "string_hash.h"
#include "variable.h"
#include "vector3d.h"

#include <list.hpp>

struct actor;
struct fixed_pool;

namespace web_sounds_manager {
void create_inst();
void frame_advance(Float elapsed);
void delete_inst();
void add_web_sound(actor *owner, const vector3d &anchor, string_hash category);
}


struct web_sound_params {
    string_hash category;
    string_hash launch_group;
    string_hash travel_group;
    string_hash impact_group;
    float travel_factor;
    uint32_t field_14;
    uint32_t field_18;
    uint32_t field_1C;
};

struct web_sound {
    entity_base_vhandle owner;
    sound_instance_id travel_sound;
    float remaining_time;
    vector3d anchor;
    float travel_time;
    web_sound_params *params;

    web_sound(web_sound_params *params, actor *owner, const vector3d &anchor);
    bool frame_advance(Float elapsed);
    void *operator new(size_t);
    void operator delete(void *storage);
    static fixed_pool &pool;
};

extern _std::list<web_sound *> *&s_web_sounds;
