#pragma once

#include "base_tl_resource_handler.h"

#include <cstdint>

struct worldly_pack_slot;

struct skeleton_resource_handler : base_tl_resource_handler {
    skeleton_resource_handler(worldly_pack_slot *a1);

    void finalize(bool);

    //virtual
    bool _handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);

    //0x0055F8E0
    /* virtual */ bool _handle_resource(worldly_resource_handler::eBehavior a2, tlresource_location *a3) /* override */;
};

extern void skeleton_resource_handler_patch();
