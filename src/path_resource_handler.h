#pragma once

#include "base_engine_resource_handler.h"

struct resource_location;

struct path_resource_handler : base_engine_resource_handler {
    path_resource_handler(worldly_pack_slot *a2);

    //virtual
    bool _handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);

    //0x0056FF90
    //virtual
    bool _handle_resource(worldly_resource_handler::eBehavior a2, resource_location *a3);
};

extern void path_resource_handler_patch();
