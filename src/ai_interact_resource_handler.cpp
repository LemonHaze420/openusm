#include "ai_interact_resource_handler.h"

#include "common.h"
#include "trace.h"
#include "utility.h"
#include "ai_interaction_data.h"
#include "mash_info_struct.h"
#include "resource_directory.h"
#include "resource_location.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(ai_interact_resource_handler, 0x14);

ai_interact_resource_handler::ai_interact_resource_handler(worldly_pack_slot *a2)
{
    if constexpr (1) {
        static void *g_vtbl[] = {
            func_address(&finalize),
            func_address(&_handle),
            func_address(&_pre_handle_resources),
            func_address(&_handle_resource),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00888AE4;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_AI_INTERACTION;
}

bool ai_interact_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("ai_interact_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

bool ai_interact_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2, resource_location *a3)
{
    TRACE("ai_interact_resource_handler::handle_resource");

    auto &v3 = this->my_slot->get_resource_directory();
    auto *resource = v3.get_resource(a3, nullptr);
    assert(resource != nullptr);

    if (a2 == UNLOAD) {
        bit_cast<ai_interaction_data *>(resource)->destruct_mashed_class();
    } else {
        ai_interaction_data *new_interact = CAST(new_interact, resource);
        assert(new_interact != nullptr);

#ifdef TARGET_XBOX
        mash_info_struct info_struct{mash::UNMASH_MODE, resource, a3->m_size, true};
#else
        mash_info_struct info_struct{resource, a3->m_size};
#endif

        info_struct.unmash_class(new_interact,
                                 nullptr
#ifdef TARGET_XBOX
                                 ,
                                 mash::NORMAL_BUFFER
#endif
        );

        mash_info_struct::construct_class(new_interact);

#ifdef TARGET_XBOX
        a3->m_offset += info_struct.get_header_size();
#endif
    }

    ++this->field_C;
    return false;
}

void ai_interact_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &ai_interact_resource_handler::_handle);
        set_vfunc(0x00888AE8, address);
    }

    {
        FUNC_ADDRESS(address, &ai_interact_resource_handler::_handle_resource);
        set_vfunc(0x00888AF0, address);
    }
}
