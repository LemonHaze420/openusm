#include "path_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "path_graph.h"
#include "resource_directory.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "worldly_pack_slot.h"
#include "xbpack.h"

VALIDATE_SIZE(path_resource_handler, 0x14);

path_resource_handler::path_resource_handler(worldly_pack_slot *a2)
{
    this->m_vtbl = 0x00888AA4;
    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_PATH;
}

bool path_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("path_resource_handler::handle");

    return base_engine_resource_handler::_handle(a2, a3);
}

bool path_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                            resource_location *a3) {
    if constexpr (1)
    {
        auto &res_dir = this->my_slot->get_resource_directory();
        auto *resource = res_dir.get_resource(a3, nullptr);
        assert(resource != nullptr);

        if (a2 == UNLOAD) {
            assert(g_world_ptr != nullptr);

            auto *the_path = g_world_ptr->field_14.get_path_graph(a3->field_0);
            assert(the_path != nullptr);

            g_world_ptr->field_14.add_path_graph(the_path);
            the_path->destruct_mashed_class();
        } else {

#ifndef TARGET_XBOX
            mash_info_struct info_struct{resource, a3->m_size};
#else
            mash_info_struct info_struct {mash::UNMASH_MODE, resource, a3->m_size, true};
#endif

            path_graph *pg = nullptr;
            info_struct.unmash_class(pg, nullptr
#ifdef TARGET_XBOX
                , mash::NORMAL_BUFFER
#endif 
                    );

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
            if (pg->id.m_type != RESOURCE_KEY_TYPE_PATH) {
                pg->id.m_type = static_cast<resource_key_type>(
                    xbpack::pc_type(static_cast<int>(pg->id.m_type)));
            }
            assert(pg->id.m_type == RESOURCE_KEY_TYPE_PATH);
#endif

            mash_info_struct::construct_class(pg);

            void (__fastcall *push_back)(void *, void *, const path_graph *) = CAST(push_back, 0x00542220);

            push_back(&g_world_ptr->field_14.path_graph_list, nullptr, pg);
        }

        ++this->field_C;
        return false;
    } else {
        return (bool) THISCALL(0x0056FF90, this, a2, a3);
    }
}

void path_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &path_resource_handler::_handle);
        set_vfunc(0x00888AA8, address);
    }

    {
        FUNC_ADDRESS(address, &path_resource_handler::_handle_resource);
        set_vfunc(0x00888AB0, address);
    }
}
