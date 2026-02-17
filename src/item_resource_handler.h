#pragma once

#include "base_entity_resource_handler.h"

#include "worldly_resource_handler.h"

#include <cstdint>

struct worldly_pack_slot;

struct item_resource_handler : base_entity_resource_handler {
    item_resource_handler(worldly_pack_slot *a2);

    void finalize(bool a2);

    //0x00563110
    /* virtual */ int _get_num_resources() /* override */;

    //virtual
    bool _handle(worldly_resource_handler::eBehavior behavior, int a4, limited_timer *a5);

    //0x0056BF00
    /* virtual */ bool _handle_resource(eBehavior behavior) /* override */;

    //0x00572F70
    /* virtual */ void _post_handle_resources(eBehavior) /* override */;
};

extern void item_resource_handler_patch();
