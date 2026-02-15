#pragma once

#include "base_tl_resource_handler.h"

#include "worldly_resource_handler.h"

#include <cstdint>

struct tlresource_location;
struct limited_timer;
struct resource_pack_slot;

struct mesh_file_resource_handler : base_tl_resource_handler {
    mesh_file_resource_handler(worldly_pack_slot *a2);

    //virtual
    void finalize(bool);

    //0x0056BD00
    //virtual
    bool _handle_resource(worldly_resource_handler::eBehavior behavior,
                                       tlresource_location *loc);

    /* virtual */ bool handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);
};

extern void mesh_file_resource_handler_patch();
