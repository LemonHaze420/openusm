#include "collision_event.h"

#include "func_wrapper.h"

collision_event::collision_event(entity_base_vhandle arg0, const subdivision_node *a3, const vector3d &a4,
                                 const vector3d &a5)
    : event(collision_event::type_id)
{
    THISCALL(0x005B1870, this, arg0, a3, &a4, &a5);
}
