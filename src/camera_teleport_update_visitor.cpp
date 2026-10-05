#include "camera_teleport_update_visitor.h"

#include "entity.h"

namespace {

int visit_entity(subdivision_visitor &, const subdivision_node &node)
{
    auto &ent = *reinterpret_cast<entity *>(const_cast<subdivision_node *>(&node));
    if ((ent.field_4 & 4) != 0 && ent.field_5C != entity::visit_key)
        ent.field_5C = entity::visit_key;
    return 0;
}
}

camera_teleport_update_visitor_t::camera_teleport_update_visitor_t()
{
    static const native_vtable table{visit_entity, nullptr};
    m_vtbl = reinterpret_cast<std::intptr_t>(&table);
}
