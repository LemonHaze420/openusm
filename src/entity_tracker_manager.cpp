#include "entity_tracker_manager.h"

#include "common.h"
#include "entity_tracker.h"

VALIDATE_SIZE(entity_tracker_manager, 0x50u);

entity_tracker_manager::entity_tracker_manager() : tracker_slot_pool(128)
{
    this->field_48 = {};
    this->field_4C = false;
}

entity_tracker *entity_tracker_manager::id_to_ptr(uint32_t a2) {
    slot_pool<entity_tracker*, unsigned int>::slot_t *v2;
    entity_tracker *result = nullptr;

    if (a2 != 0 && (v2 = &this->tracker_slot_pool.slots[a2 & this->tracker_slot_pool.field_0], a2 == v2->id)) {
        result = v2->field_4;
    }

    return result;
}

bool entity_tracker_manager::get_the_arrow_target_pos(vector3d *a2)
{
    return (bool) THISCALL(0x0062EE10, this, a2);
}

void entity_tracker_manager::place_poi_reticles()
{
    THISCALL(0x0062EEB0, this);
}
