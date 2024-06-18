#pragma once

#include "string_hash.h"
#include "simple_classes.h"

#include <hash_map.hpp>

struct entity_base_vhandle;
struct entity_base;

enum entity_flavor_t {
    ACTOR = 0,
    SIGNALLER = 1,
    CAMERA = 2,
    BASE = 3,
    ENTITY = 4,
    MARKER = 5,
    PARKING_MARKER = 6,
    WATER_EXIT_MARKER = 7,
    MIC = 8,
    LIGHT_SOURCE = 9,
    PFX = 10,
    ENTITY_ITEM = 11,
    CONGLOMERATE = 12,
    CONGLOMERATE_CLONE = 13,
    BEAM = 14,
    MANIP = 15,
    SWITCH = 16,
    POLYTUBE = 17,
    LENSFLARE = 18,
    NEOLIGHT = 19,
    GAME_CAMERA = 20,
    SPIDERMAN_CAMERA = 21,
    MARKY_CAMERA = 22,
    SNIPER_CAMERA = 23,
    AI_CAMERA = 24,
    ANCHOR_MARKER = 25,
    LINE_ANCHOR = 26,
    AI_COVER_MARKER = 27,
    NULL_FLAVOR = 28,
    IGNORE_FLAVOR = 29,
};

struct entity_slot {
    entity_base *my_ptr;
    uint32_t my_id;

    int occupy(entity_base *a2);

    inline bool in_use() {
        return this->my_ptr != nullptr;
    }

    void vacate(const entity_base_vhandle &ent_handle);
};

inline constexpr auto MAX_ENTITIES = 10240;

struct entity_handle_manager {
    //0x004EF970
    static void register_entity(entity_base *a1);

    //0x004DC510
    static void deregister_entity(entity_base *a1);

    //0x004DC300
    static entity_base *find_entity(const string_hash &arg0, entity_flavor_t a2, bool a3);

    static stdext::hash_map<string_hash, entity_base *> & the_map;

    //0x004D0F20
    static void create_inst();

    //0x004C2A80
    static void delete_inst();

    //0x004CCF60
    static int add_entity(entity_base *a1);

    //0x004CCE40
    static void clear_ent_slots();

    //0x004CCF20
    static int get_free_slot();

    //0x004CCFC0
    static void remove_entity(const entity_base_vhandle &ent_handle);

    static entity_slot *& ent_slots;

    static entity_slot *& ENTS;

    static int & curr_idx;

    static bool & check_world_lists;

    static simple_queue<int, 32> & free_slot_indices;
};

//0x004DCE50
extern entity_base *find_marker(const string_hash &a1);

//0x0050B9D0
extern mic *find_mic(const string_hash &a1);

inline auto & entity_flavor_names = var<const char *[30]>(0x0091FD38);

extern void entity_handle_manager_patch();
