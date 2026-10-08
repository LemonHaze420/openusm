#include "entity_tracker_manager.h"

#include "common.h"
#include "fe_mini_map_dot.h"
#include "entity_tracker.h"
#include "femanager.h"
#include "igofrontend.h"
#include "thug_health.h"

VALIDATE_SIZE(entity_tracker_manager, 0x50u);

entity_tracker_manager::entity_tracker_manager() : tracker_slot_pool(128)
{
    this->field_48 = {};
    this->field_4C = false;
}

entity_tracker_manager::~entity_tracker_manager()
{
    for (const auto &entry : field_0) {
        auto *tracker = id_to_ptr(entry.second);
        if (tracker != nullptr) {
            delete tracker->field_4;
            delete tracker;
        }
    }
}

entity_tracker *entity_tracker_manager::id_to_ptr(uint32_t a2)
{
    slot_pool<entity_tracker *, unsigned int>::slot_t *v2;
    entity_tracker *result = nullptr;

    if (a2 != 0 && (v2 = &this->tracker_slot_pool.slots[a2 & this->tracker_slot_pool.field_0], a2 == v2->id)) {
        result = v2->field_4;
    }

    return result;
}

// 0x00641500
uint32_t entity_tracker_manager::create_entity_tracker(entity_base_vhandle handle)
{
    auto existing = field_0.find(handle);
    if (existing != field_0.end())
        return existing->second;

    auto &pool = tracker_slot_pool;
    if (pool.field_38 == 0) {
        for (int i = 0; i < pool.MAX_SLOTS && pool.field_38 < 8; ++i) {
            if ((pool.slots[i].id & pool.field_8) == 0)
                pool.field_18[pool.field_38++] = i;
        }
    }
    if (pool.field_38 == 0)
        return 0;

    const int slot_index = pool.field_18[--pool.field_38];
    auto &slot = pool.slots[slot_index];
    const uint32_t id = pool.field_4 + (slot.id | pool.field_8);
    slot.field_4 = new entity_tracker{handle};
    slot.id = id;
    ++pool.field_10;
    field_0[handle] = id;
    return id;
}

// 0x0063A270
void entity_tracker_manager::destroy_entity_tracker(uint32_t id)
{
    entity_tracker *tracker = id_to_ptr(id);
    if (tracker == nullptr)
        return;
    auto *health = g_femanager.IGO->m_thug_health;
    if (tracker->field_C != health->field_0)
        health->destroy(tracker->field_C);

    field_0.erase(field_0.find(tracker->field_0));
    auto &pool = tracker_slot_pool;
    const int slot_index = id & pool.field_0;
    auto &slot = pool.slots[slot_index];
    slot.id &= ~static_cast<uint32_t>(pool.field_8);
    if (pool.field_38 < 8)
        pool.field_18[pool.field_38++] = slot_index;
    --pool.field_10;
    delete tracker->field_4;
    tracker->field_4 = nullptr;
    delete tracker;
    slot.field_4 = nullptr;
}

void entity_tracker_manager::set_entity(uint32_t id, entity *owner)
{
    const auto handle = owner->get_my_handle();
    if (auto *tracker = id_to_ptr(id); tracker != nullptr) {
        field_0.erase(field_0.find(tracker->field_0));
        field_0[handle] = id;
        tracker->field_0 = handle;
    }
}

bool entity_tracker_manager::get_the_arrow_target_pos(vector3d *a2)
{
    for (const auto &entry : field_0) {
        auto *tracker = id_to_ptr(entry.second);
        if (!tracker || !tracker->field_8)
            continue;
        if (auto *owner = tracker->field_0.get_volatile_ptr()) {
            *a2 = owner->get_abs_position();
            return true;
        }
    }
    return false;
}

void entity_tracker_manager::place_poi_reticles()
{
    for (const auto &entry : field_0) {
        auto *tracker = id_to_ptr(entry.second);
        if (tracker != nullptr && tracker->field_0.get_volatile_ptr() != nullptr && tracker->field_8 != 0)
            field_4C = true;
    }
}
