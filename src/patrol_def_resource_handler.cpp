#include "patrol_def_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "patrol_def_set.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(patrol_def_resource_handler, 0x14);

patrol_def_resource_handler::patrol_def_resource_handler(worldly_pack_slot *a2)
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
        this->m_vtbl = 0x00888AB4;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_PATROL_DEF;
}

bool patrol_def_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("patrol_def_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

bool patrol_def_resource_handler::_handle_resource(worldly_resource_handler::eBehavior behavior, resource_location *loc)
{
    TRACE("patrol_def_resource_handler::handle_resource");

    if constexpr (1) {
        auto &dir = this->my_slot->get_resource_directory();
        auto *resource = dir.get_resource(loc, nullptr);
        assert(resource != nullptr);

        if (behavior == worldly_resource_handler::UNLOAD) {
            assert(g_world_ptr != nullptr);

            auto *the_set = bit_cast<patrol_def_set *>(resource);
            assert(the_set != nullptr);

            g_world_ptr->field_178.remove_patrol_defs_from_set(the_set);
            the_set->destruct_mashed_class();
        } else {
#ifdef TARGET_XBOX
            mash_info_struct v5{2, resource, loc->m_size, 1};
#else
            mash_info_struct v5{resource, loc->m_size};
#endif

            patrol_def_set *v6 = nullptr;

            v5.unmash_class(v6,
                            nullptr
#ifdef TARGET_XBOX
                            ,
                            mash::NORMAL_BUFFER
#endif
            );

            mash_info_struct::construct_class(v6);

#ifdef TARGET_XBOX
            loc->m_offset += v5.sub_6655AF();
#endif

            assert(g_world_ptr != nullptr);

            g_world_ptr->field_178.add_patrol_defs_from_set(v6);
        }

        ++this->field_C;
        return false;
    } else {
        bool(__fastcall * func)(void *, void *edx, worldly_resource_handler::eBehavior, resource_location *) =
            CAST(func, 0x00568BD0);
        return func(this, nullptr, behavior, loc);
    }
}

void patrol_def_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &patrol_def_resource_handler::_handle);
        set_vfunc(0x00888AB8, address);
    }

    {
        FUNC_ADDRESS(address, &patrol_def_resource_handler::_handle_resource);
        set_vfunc(0x00888AC0, address);
    }
}
