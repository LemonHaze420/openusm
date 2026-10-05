#include "entity_tracker.h"
#include "entity.h"
#include "fe_mini_map_dot.h"
#include "mini_map_dot_type.h"

#include "femanager.h"
#include "igofrontend.h"
#include "sound_instance_id.h"
#include "thug_health.h"
#include "func_wrapper.h"

entity_tracker::entity_tracker() : field_0(), field_4(nullptr), field_8(0), field_C(-1) {}

entity_tracker::entity_tracker(entity_base_vhandle handle) : field_0(handle), field_4(nullptr), field_8(0), field_C(-1)
{
    if (auto *owner = handle.get_volatile_ptr(); owner != nullptr)
        owner->get_abs_position();
    field_4 = new fe_mini_map_dot{static_cast<mini_map_dot_type>(2), ZEROVEC};
}

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
    set_health_widget_active(true);
    if (static_cast<int>(type) == 1 || static_cast<int>(type) == 15 || static_cast<int>(type) == 16)
        (void)sub_60B960(string_hash{"FE_MINIMAP_BLIP"}, 1.f, 1.f);
}
void entity_tracker::set_health_widget_active(bool enabled)
{
    if (field_4 == nullptr || static_cast<int>(field_4->field_20) != 2)
        return;
    auto *health = g_femanager.IGO->m_thug_health;
    if (enabled) {
        if (field_C == health->field_0) {
            field_C = health->create();
            health->set_entity(field_C, field_0.get_volatile_ptr());
        }
        if (static_cast<unsigned>(field_C) < 30 && health->field_1C[field_C].field_0)
            health->field_1C[field_C].visible = true;
    } else if (field_C != health->field_0) {
        health->destroy(field_C);
        field_C = health->field_0;
    }
}
