#pragma once

#include "base_tl_resource_handler.h"

struct scene_anim_resource_handler : base_tl_resource_handler {
    scene_anim_resource_handler(worldly_pack_slot *a2);

    //virtual
    void finalize(bool a2);

    bool _handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);

    //0x0055F990
    /* virtual */ bool _handle_resource(worldly_resource_handler::eBehavior a2,
                                        tlresource_location *loc) /* override */;
};

extern void scene_anim_resource_handler_patch();
