#pragma once

#include "base_entity_resource_handler.h"

#include "worldly_resource_handler.h"

#include <cstdint>

struct worldly_pack_slot;

struct box_trigger_resource_handler : base_entity_resource_handler {
    box_trigger_resource_handler(worldly_pack_slot *a2);

    //virtual
    void finalize(bool);

    //0x005630F0
    /* virtual */ int _get_num_resources() /* override */;

    //virtual
    bool _handle(worldly_resource_handler::eBehavior behavior, int a4, limited_timer *a5);

    //0x0056BEC0
    /* virtual */ bool _handle_resource(worldly_resource_handler::eBehavior behavior) /* override */;

    //0x0056FF20
    /* virtual */ void post_handle_resources(worldly_resource_handler::eBehavior a2) /* override */;
};

extern void box_trigger_resource_handler_patch();
