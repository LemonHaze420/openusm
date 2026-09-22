#include "base_ai_graph_manager.h"

#include "resource_manager.h"

namespace ai {
state_graph *state_graph_manager::find_state_graph_from_resource(resource_key resource_id, resource_pack_slot *a3)
{
    auto *__old_context = resource_manager::push_resource_context(a3);
    resource_id.m_type = static_cast<resource_key_type>(RESOURCE_KEY_TYPE_AI_STATE_GRAPH);
    auto *resource = bit_cast<state_graph *>(resource_manager::get_resource(resource_id, nullptr, nullptr));
    resource_manager::pop_resource_context();
    assert(resource_manager::get_resource_context() == __old_context);

    return resource;
}

bool state_graph_manager::can_get_graph(resource_key a2, resource_pack_slot *a3)
{
    auto resource_id = a2;
    auto *resource = ai::state_graph_manager::find_state_graph_from_resource(resource_id, a3);

    return resource != nullptr;
}


}  // namespace ai
