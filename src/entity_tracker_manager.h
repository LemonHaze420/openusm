#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "slot_pool.h"

#include <cstdint>
#include <map.hpp>

struct entity_tracker;
struct vector3d;

struct entity_tracker_manager {
    _std::map<entity_base_vhandle, uint32_t> field_0;
    slot_pool<entity_tracker *, unsigned int> tracker_slot_pool;
    vhandle_type<entity> field_48;
    bool field_4C;

    //0x00638310
    entity_tracker_manager();
    ~entity_tracker_manager();

    //0x00629E30
    entity_tracker *id_to_ptr(uint32_t a2);
    // 0x00641500
    uint32_t create_entity_tracker(entity_base_vhandle handle);

    // 0x0063A270
    void destroy_entity_tracker(uint32_t id);
    void set_entity(uint32_t id, entity *owner);

    //0x0062EE10
    bool get_the_arrow_target_pos(vector3d *);

    void place_poi_reticles();
};
