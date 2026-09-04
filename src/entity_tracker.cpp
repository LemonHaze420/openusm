#include "entity_tracker.h"

#include "func_wrapper.h"

entity_tracker::entity_tracker() {}

entity *entity_tracker::get_entity()
{
    return (entity *)this->field_0.get_volatile_ptr();
}
