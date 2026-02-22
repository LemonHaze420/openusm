#include "item_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "item.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "worldly_pack_slot.h"

#include <cassert>

VALIDATE_SIZE(item_resource_handler, 0x10);

item_resource_handler::item_resource_handler(worldly_pack_slot *a2)
{
    if constexpr (1) {
        static void * g_vtbl[] = {
            func_address(&finalize),
            func_address(&_handle),
            func_address(&_get_num_resources),
            func_address(&_handle_resource),
            func_address(&_post_handle_resources),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00888A5C;
    }

    this->my_slot = a2;
}

void item_resource_handler::finalize(bool a2)
{
    this->~item_resource_handler();
    if (a2) {
        delete(this);
    }
}

int item_resource_handler::_get_num_resources() {
    return this->my_slot->item_instances->size();
}

bool item_resource_handler::_handle(worldly_resource_handler::eBehavior behavior, limited_timer *a5)
{
    TRACE("item_resource_handler::handle");

    return base_entity_resource_handler::_handle(behavior, a5);
}

bool item_resource_handler::_handle_resource([[maybe_unused]] eBehavior behavior)
{
    TRACE("item_resource_handler::handle_resource");
    bool result;

    assert(behavior == UNLOAD);

    if constexpr (1)
    {
        if (g_world_ptr->ent_mgr.items.get_vector_index(this->my_slot->item_instances) > 0)
        {
            auto *v4 = this->my_slot->item_instances->at(this->field_C);
            if (v4 != nullptr)
            {
                auto *v5 = this->my_slot->item_instances->at(this->field_C);
                if ( v4->is_conglom_member() )
                {
                    g_world_ptr->ent_mgr.remove_entity_from_misc_lists(v4);
                    if ( v5->is_dynamic() ) {
                        v5->~item();
                    } else {
                        v5->release_mem();
                    }

                    this->my_slot->item_instances->at(this->field_C) = nullptr;
                }
            }
            ++this->field_C;
            result = false;
        } else {
            this->field_C = this->get_num_resources();
            result = true;
        }

    } else {
        result = THISCALL(0x0056BF00, this, behavior);
    }

    return result;
}

void item_resource_handler::_post_handle_resources(eBehavior) {
    TRACE("item_resource_handler::post_handle_resources");

    if (this->my_slot->item_instances != nullptr) {
        g_world_ptr->ent_mgr.items.sub_572FB0(this->my_slot->item_instances);
        this->my_slot->item_instances = nullptr;
    }
}

void item_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &item_resource_handler::_handle);
        set_vfunc(0x00888A60, address);
    }

    {
        FUNC_ADDRESS(address, &item_resource_handler::_handle_resource);
        set_vfunc(0x00888A68, address);
    }
}
