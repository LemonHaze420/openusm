#include "entity_base_vhandle.h"

#include "entity_base.h"
#include "entity_handle_manager.h"
#include "item.h"
#include "oldmath_po.h"
#include "switch_obj.h"

#include "config.h"
#include "func_wrapper.h"

entity_base *entity_base_vhandle::get_volatile_ptr() const {
    auto func = [](const entity_base_vhandle *a1) -> void * {
        auto v2 = a1->field_0 & 0x3FFF;
        if (v2 < MAX_ENTITIES && entity_handle_manager::ent_slots[v2].my_id == a1->field_0) {
            auto *result = entity_handle_manager::ent_slots[v2].my_ptr;

            return result;
        }

        return nullptr;
    };

    auto *my_ent = static_cast<entity_base *>(func(this));

    assert(my_ent == nullptr || my_ent->is_an_entity_base());

    return my_ent;
}

//0x006022C0
template<>
switch_obj *vhandle_type<
    switch_obj,
    vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>>::
    get_volatile_ptr() const
{
    switch_obj *result = nullptr;

    uint32_t v2;

    auto v1 = this->field_0.field_0;
    if (v1 && (v2 = v1 & 0x3FFF, v2 < 0x2800) &&
        entity_handle_manager::ent_slots[v2].my_id == v1) {
        result = static_cast<switch_obj *>(entity_handle_manager::ent_slots[v2].my_ptr);
    }

    return result;
}

//0x00601AE0
template<>
item *vhandle_type<
    item,
    vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>>::
    get_volatile_ptr() const
{
    item *result;

    uint32_t v2;

    auto v1 = this->field_0.field_0;
    if (v1 && (v2 = v1 & 0x3FFF, v2 < MAX_ENTITIES) &&
        entity_handle_manager::ent_slots[v2].my_id == v1) {
        result = static_cast<item *>(entity_handle_manager::ent_slots[v2].my_ptr);
    } else {
        result = nullptr;
    }

    return result;
}

//0x00601660
#if 0
template<>
entity *vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>::get_volatile_ptr() {
    auto *result = static_cast<entity *>(this->field_0.get_volatile_ptr());

    return result;
}
#endif
