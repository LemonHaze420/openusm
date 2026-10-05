#include "entity_tracker.h"
#include "entity.h"
#include "fe_mini_map_dot.h"
#include "mini_map_dot_type.h"

#include "func_wrapper.h"

entity_tracker::entity_tracker() : field_0(), field_4(nullptr), field_8(0), field_C(30) {}

entity_tracker::entity_tracker(entity_base_vhandle handle) : field_0(handle), field_4(nullptr), field_8(0), field_C(30)
{}

entity *entity_tracker::get_entity()
{
    return (entity *)this->field_0.get_volatile_ptr();
}

// 0x00641120
void entity_tracker::set_poi_icon(mini_map_dot_type type)
{
    if (field_4 != nullptr && static_cast<int>(field_4->field_20) != static_cast<int>(type)) {
        delete field_4;
        field_4 = nullptr;
    }
    if (field_4 == nullptr) {
        const vector3d position = get_entity() != nullptr ? get_entity()->get_abs_position() : vector3d{};
        field_4 = new fe_mini_map_dot{type, position};
    }
}
