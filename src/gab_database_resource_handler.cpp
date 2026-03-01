#include "gab_database_resource_handler.h"

#include "common.h"
#include "gab_manager.h"
#include "mash_info_struct.h"
#include "resource_directory.h"
#include "worldly_pack_slot.h"
#include "trace.h"

VALIDATE_SIZE(gab_database_resource_handler, 0x14);

gab_database_resource_handler::gab_database_resource_handler(worldly_pack_slot *a2)
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
        this->m_vtbl = 0x00888AF4;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_GAB_DATABASE;
}

bool gab_database_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                    resource_location *a3)
{
    TRACE("gab_database_resource_handler::handle_resource");

    assert(my_slot->get_resource_directory().get_resource_count(RESOURCE_KEY_TYPE_GAB_DATABASE) ==
           1);

    auto *resource = this->my_slot->get_resource_directory().get_resource(a3, nullptr);
    assert(resource != nullptr);

    if (a2 == worldly_resource_handler::UNLOAD) {
        gab_database *the_gab_database = CAST(the_gab_database, resource);

        assert(the_gab_database == gab_manager::get_gab_database());
        assert(the_gab_database != nullptr);

        gab_manager::set_gab_database(nullptr);
        the_gab_database->destruct_mashed_class();
    } else {

#ifndef TARGET_XBOX
        mash_info_struct info_struct{resource, a3->m_size};
#else
        mash_info_struct info_struct {mash::UNMASH_MODE, resource, a3->m_size, true};
#endif

        gab_database *the_gab_database = nullptr;
        info_struct.unmash_class(the_gab_database, nullptr
#ifdef TARGET_XBOX
            , mash::NORMAL_BUFFER
#endif 
                );

        mash_info_struct::construct_class(the_gab_database);
        gab_manager::set_gab_database(the_gab_database);
    }

    ++this->field_C;
    return false;
}

void gab_database_resource_handler_patch()
{
    FUNC_ADDRESS(address, &gab_database_resource_handler::_handle_resource);
    set_vfunc(0x00888B00, address);
}
