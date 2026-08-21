#pragma once

#include "progress.h"
#include "tlresource_location.h"
#include "worldly_resource_handler.h"

#include <cstdint>

struct worldly_pack_slot;
struct limited_timer;

struct base_tl_resource_handler : worldly_resource_handler {
    tlresource_type field_10;

    /* virtual */ ~base_tl_resource_handler() = default;

    //0x00562EC0
    /* virtual */ bool _handle(worldly_resource_handler::eBehavior a2, limited_timer *a3);

    void _pre_handle_resources(worldly_resource_handler::eBehavior);

    /* virtual */ void pre_handle_resources(worldly_resource_handler::eBehavior);

    /* virtual */ bool handle_resource(worldly_resource_handler::eBehavior, tlresource_location *);
};

extern limited_timer *&dword_95C824;
