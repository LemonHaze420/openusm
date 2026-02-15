#pragma once

#include "worldly_resource_handler.h"

#include "resource_key.h"

struct resource_location;

struct base_engine_resource_handler : worldly_resource_handler {
    resource_key_type field_10;

    base_engine_resource_handler();

    //0x00562DF0
    /* virtual */ bool _handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);

    void _pre_handle_resources(worldly_resource_handler::eBehavior);

    /* virtual */ void pre_handle_resources(worldly_resource_handler::eBehavior);

    /* virtual */ bool handle_resource(worldly_resource_handler::eBehavior a2,
                                       resource_location *a3) /* = 0 */;
};
