#pragma once

#include "base_engine_resource_handler.h"

struct gab_database_resource_handler : base_engine_resource_handler {
    gab_database_resource_handler(worldly_pack_slot *a2);

    /* virtual */ bool _handle_resource(worldly_resource_handler::eBehavior a2, resource_location *a3) /* override */;
};

extern void gab_database_resource_handler_patch();
