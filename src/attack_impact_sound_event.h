#pragma once

#include "event.h"
#include "entity_base_vhandle.h"

struct from_mash_in_place_constructor;

struct attack_impact_sound_event : event {
    string_hash sound;
    float volume;
    entity_base_vhandle source;

    attack_impact_sound_event();
    explicit attack_impact_sound_event(from_mash_in_place_constructor *tag);
    static void *native_vtable();
};
