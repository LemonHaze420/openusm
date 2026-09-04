#pragma once

#include "entity_base_vhandle.h"
#include "marker.h"

struct parking_marker : marker {
    using base_type = vhandle_type<marker>;

    parking_marker(const string_hash &a2, uint32_t a3);
};
