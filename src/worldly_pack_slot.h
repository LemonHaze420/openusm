#pragma once

#include "resource_pack_slot.h"

#include "base_entity_resource_handler.h"
#include "worldly_resource_handler.h"

#include <vector.hpp>

#include <cstdint>

struct entity;
struct item;
struct limited_timer;
struct box_trigger;
struct worldly_resource_handler;
struct sound_alias_database_resource;

enum eWorldlyResourceType {
    MAX_WORLDLY_RESOURCE_TYPES = 21
};

struct worldly_pack_slot : resource_pack_slot {
    progress field_94;
    progress field_98;
    worldly_resource_handler *m_handlers[MAX_WORLDLY_RESOURCE_TYPES];
    _std::vector<entity *> *entity_instances;
    _std::vector<item *> *item_instances;
    _std::vector<box_trigger *> *box_trigger_instances;

    //0x00532220
    worldly_pack_slot();

    //0x005327E0
    //virtual
    ~worldly_pack_slot();

    //0x00558DF0
    _std::vector<item *> *get_item_instances();

    //0x00558DC0
    _std::vector<entity *> *get_entity_instances();

    void clear_progress();

    void _finalize(bool a2);

private:

    //0x0050ED20
    //virtual
    bool _on_load(limited_timer *a2) /*override*/;

    //0x0052AC90
    //virtual
    bool _on_unload(limited_timer *a2) /*override*/;

    //0x0050ECE0
    //virtual
    void _clear_slot() /*override*/;

    //0x0052AC40
    //virtual
    void _clear_pack() /*override*/;

public:

    //0x005382F0
    _std::vector<box_trigger *> *get_box_trigger_instances();
};

struct entity_resource_handler : base_entity_resource_handler {
    entity_resource_handler(worldly_pack_slot *a2);

    //virtual ~entity_resource_handler();

    //virtual
    bool _handle(worldly_resource_handler::eBehavior behavior, limited_timer *a5);

    //0x00563130
    /* virtual */ int _get_num_resources() /* override */;

    //0x0056BFA0
    /* virtual */ bool _handle_resource(worldly_resource_handler::eBehavior behavior) /* override */;

    //0x00572FF0
    /* virtual */ void _post_handle_resources(eBehavior) /* override */;
};

extern void worldly_pack_slot_patch();
