#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"

struct marker : entity {
    using base_type = vhandle_type<marker>;

    marker(const string_hash &a2, uint32_t a3);
};
