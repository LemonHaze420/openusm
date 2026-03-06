#include "glass_house_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "glass_house.h"
#include "glass_house_manager.h"
#include "mash_info_struct.h"
#include "resource_directory.h"
#include "trace.h"
#include "utility.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(glass_house_resource_handler, 0x14);

glass_house_resource_handler::glass_house_resource_handler(worldly_pack_slot *a2)
{
    if constexpr (1) {
        static void * g_vtbl[] = {
            func_address(&finalize),
            func_address(&_handle),
            func_address(&_pre_handle_resources),
            func_address(&_handle_resource),
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00888B04;
    }

    this->my_slot = a2;
    this->field_10 = static_cast<resource_key_type>(39);
}

bool glass_house_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("glass_house_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

bool glass_house_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                   resource_location *a3)
{
    TRACE("glass_house_resource_handler::handle_resource");

    if constexpr (1) {
        auto *resource = my_slot->get_resource_directory().get_resource(a3, nullptr);
        assert(resource != nullptr);

        if ( a2 == worldly_resource_handler::UNLOAD )
        {
            auto *the_glass_house = bit_cast<glass_house *>(resource);
            assert(the_glass_house != nullptr);

            string_hash v6 = the_glass_house->get_name();
            glass_house_manager::remove_glass_house(v6);
            the_glass_house->destruct_mashed_class();
        }
        else
        {
            glass_house *v9 = nullptr;

#ifndef TARGET_XBOX
            mash_info_struct v8 {resource, a3->m_size};
#else
            mash_info_struct v8 {mash::UNMASH_MODE, resource, a1->m_size, true};
#endif

            v8.unmash_class(v9, nullptr
#ifdef TARGET_XBOX
            , mash::NORMAL_BUFFER
#endif
                    );
            mash_info_struct::construct_class(v9);

#ifdef TARGET_XBOX
            a3->field_8 += v8.get_header_size();
#endif
        }

        ++this->field_C;
        return false;
    } else {
        return (bool) THISCALL(0x005730A0, this, a2, a3);
    }
}

void glass_house_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &glass_house_resource_handler::_handle);
        set_vfunc(0x00888B08, address);
    }

    {
        FUNC_ADDRESS(address, &glass_house_resource_handler::_handle_resource);
        set_vfunc(0x00888B10, address);
    }
}
