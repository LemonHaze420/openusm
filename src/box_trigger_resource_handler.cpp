#include "box_trigger_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "slab_allocator.h"
#include "trace.h"
#include "trigger_manager.h"
#include "utility.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(box_trigger_resource_handler, 0x10u);

box_trigger_resource_handler::box_trigger_resource_handler(worldly_pack_slot *a2)
{
    if constexpr (1) {
        static void *g_vtbl[] = {
            func_address(&finalize),
            func_address(&_handle),
            func_address(&_get_num_resources),
            func_address(&_handle_resource),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00888A48;
    }

    this->my_slot = a2;
}


void box_trigger_resource_handler::finalize(bool a2)
{
    this->~box_trigger_resource_handler();
    if (a2) {
        delete (this);
    }
}

int box_trigger_resource_handler::_get_num_resources()
{
    return this->my_slot->box_trigger_instances->size();
}

bool box_trigger_resource_handler::_handle(worldly_resource_handler::eBehavior behavior, limited_timer *a5)
{
    TRACE("box_trigger_resource_handler::handle");

    return base_entity_resource_handler::_handle(behavior, a5);
}

bool box_trigger_resource_handler::_handle_resource([[maybe_unused]] worldly_resource_handler::eBehavior behavior)
{
    TRACE("box_trigger_resource_handler::handle_resource");

    assert(behavior == UNLOAD);

    assert(my_slot->box_trigger_instances != nullptr);

    auto *v3 = this->my_slot->box_trigger_instances->at(this->field_C);
    if (v3 != nullptr) {
        trigger_manager::instance->delete_trigger((trigger *)v3);
    }

    ++this->field_C;
    return false;
}

void sub_56FF50(_std::vector<box_trigger *> *a1)
{
    if (a1 != nullptr) {
        a1->clear();

        mem_dealloc(a1, sizeof(*a1));
    }
}

void box_trigger_resource_handler::post_handle_resources(worldly_resource_handler::eBehavior)
{
    if (this->my_slot->box_trigger_instances != nullptr) {
        sub_56FF50(this->my_slot->box_trigger_instances);
        this->my_slot->box_trigger_instances = nullptr;
    }
}

void box_trigger_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &box_trigger_resource_handler::_handle);
        set_vfunc(0x00888A4C, address);
    }

    {
        FUNC_ADDRESS(address, &box_trigger_resource_handler::_handle_resource);
        set_vfunc(0x00888A54, address);
    }
}
