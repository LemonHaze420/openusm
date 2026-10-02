#pragma once

#include "resource_key.h"

struct resource_pack_slot;

namespace ai {

struct state_graph;

struct state_graph_manager {
    static state_graph *find_state_graph_from_resource(resource_key a2, resource_pack_slot *a3);

    //0x00688200
    static bool can_get_graph(resource_key a2, resource_pack_slot *a3);
};
}  // namespace ai
