#include "sound_alias_database_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "resource_directory.h"
#include "utility.h"
#include "worldly_pack_slot.h"
#include "sound_alias_database.h"
#include "sound_manager.h"
#include "mash_info_struct.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(sound_alias_database_resource_handler, 0x14);

sound_alias_database_resource_handler::sound_alias_database_resource_handler(worldly_pack_slot *a2)
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
        this->m_vtbl = 0x00888B14;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_SOUND_ALIAS_DATABASE;
}

bool sound_alias_database_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("sound_alias_database_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

bool sound_alias_database_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                            resource_location *a3)
{
    TRACE("sound_alias_database_resource_handler::handle_resource");

    assert(my_slot->get_resource_directory().get_resource_count(
               RESOURCE_KEY_TYPE_SOUND_ALIAS_DATABASE) == 1);

    if constexpr (1)
    {
        auto &res_dir = my_slot->get_resource_directory();
        auto *resource = res_dir.get_resource(a3, nullptr);
        assert(resource != nullptr);

        sound_alias_database *the_sound_alias_database =
            CAST(the_sound_alias_database, resource);

        if (a2 == UNLOAD)
        {
            assert(the_sound_alias_database == sound_manager::get_sound_alias_database());
            assert(the_sound_alias_database != nullptr);

            sound_manager::set_sound_alias_database(nullptr);

            the_sound_alias_database->destruct_mashed_class();
        }
        else
        {
#ifndef TARGET_XBOX
            mash_info_struct v7 {resource, a3->m_size};
#else
            mash_info_struct v7 {mash::UNMASH_MODE, resource, a3->m_size, true};
#endif

            sound_alias_database *new_resource = nullptr;
            v7.unmash_class(new_resource, nullptr
#ifdef TARGET_XBOX
                , mash::NORMAL_BUFFER
#endif
                    );
            mash_info_struct::construct_class(new_resource);

#ifdef TARGET_XBOX
            a3->m_offset += v7.get_header_size();
#endif

            auto *v5 = new_resource;
            sound_manager::set_sound_alias_database(v5);
        }

        ++this->field_C;

        return false;

    }
    else
    {
        return (bool) THISCALL(0x00568FC0, this, a2, a3);
    }
}

void sound_alias_database_resource_handler_patch() {

    {
        FUNC_ADDRESS(address, &sound_alias_database_resource_handler::_handle);
        set_vfunc(0x00888B18, address);
    }

    FUNC_ADDRESS(address, &sound_alias_database_resource_handler::_handle_resource);
    set_vfunc(0x00888B20, address);
}
