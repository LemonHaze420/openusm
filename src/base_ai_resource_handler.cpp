#include "base_ai_resource_handler.h"

#include "common.h"
#include "core_ai_resource.h"
#include "mash_info_struct.h"
#include "resource_directory.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(base_ai_resource_handler, 0x14);

base_ai_resource_handler::base_ai_resource_handler(worldly_pack_slot *a2)
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
        this->m_vtbl = 0x00888A08;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_BASE_AI;
}

void base_ai_resource_handler::finalize(bool a2)
{
    this->~base_ai_resource_handler();
    if (a2) {
        delete(this);
    }
}

bool base_ai_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("base_ai_resource_handler::handle");

#ifdef TARGET_XBOX
    this->field_10 = RESOURCE_KEY_TYPE_BASE_AI;
#endif

    return base_engine_resource_handler::_handle(a2, a3);
}

bool base_ai_resource_handler::_handle_resource(worldly_resource_handler::eBehavior behavior,
                                               resource_location *a3)
{
    TRACE("base_ai_resource_handler::handle_resource", a3->field_0.get_platform_string(g_platform).c_str());

    if constexpr (1)
    {
        auto &res_dir = this->my_slot->get_resource_directory();
        auto *resource = res_dir.get_resource(a3, nullptr);
        assert(resource != nullptr);

        if ( behavior == UNLOAD )
        {
            bit_cast<ai::core_ai_resource *>(resource)->destruct_mashed_class();
        }
        else
        {
            auto *new_ai_resource = bit_cast<ai::core_ai_resource *>(resource);
            assert(new_ai_resource != nullptr);

#ifndef TARGET_XBOX
            mash_info_struct info_struct {resource, a3->m_size};
#else
            mash_info_struct info_struct {mash::UNMASH_MODE, resource, a3->m_size, true};
#endif

            info_struct.unmash_class(new_ai_resource, nullptr
#ifdef TARGET_XBOX
                , mash::NORMAL_BUFFER
#endif 
                    );
            mash_info_struct::construct_class(new_ai_resource);

#ifdef TARGET_XBOX
            a3->m_offset += info_struct.get_header_size();
#endif
        }

        ++this->field_C;
        return false;
    }
    else
    {
        bool (__fastcall *func)(void *, void *, worldly_resource_handler::eBehavior ,
                                               resource_location *) = CAST(func, 0x00568A10);
        return func(this, nullptr, behavior, a3); 
    }
}

void base_ai_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &base_ai_resource_handler::_handle);
        set_vfunc(0x00888A0C, address);
    }

    {
        FUNC_ADDRESS(address, &base_ai_resource_handler::_handle_resource);
        set_vfunc(0x00888A14, address);
    }
}
