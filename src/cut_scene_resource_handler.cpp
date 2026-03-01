#include "cut_scene_resource_handler.h"

#include "common.h"
#include "cut_scene.h"
#include "cut_scene_player.h"
#include "mash_info_struct.h"
#include "resource_directory.h"
#include "trace.h"
#include "utility.h"
#include "worldly_pack_slot.h"

VALIDATE_SIZE(cut_scene_resource_handler, 0x14);

cut_scene_resource_handler::cut_scene_resource_handler(worldly_pack_slot *a2)
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
        this->m_vtbl = 0x00888AD4;
    }

    this->my_slot = a2;
    this->field_10 = RESOURCE_KEY_TYPE_CUT_SCENE;
}

bool cut_scene_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("cut_scene_resource_handler::handle");

#ifdef TARGET_XBOX
    this->field_10 = RESOURCE_KEY_TYPE_CUT_SCENE;
#endif

    return base_engine_resource_handler::_handle(a2, a3);
}

bool cut_scene_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                 resource_location *a3) {
    auto &res_dir = this->my_slot->get_resource_directory();
    auto *resource = res_dir.get_resource(a3, nullptr);
    assert(resource != nullptr);

    if (a2 == UNLOAD) {
        auto *v6 = g_cut_scene_player();

        auto *scene = (cut_scene *) resource;
        v6->stop(scene);
        scene->destruct_mashed_class();
    } else {

        cut_scene *v1 = nullptr;

#ifndef TARGET_XBOX
        mash_info_struct info_struct{resource, a3->m_size};

        info_struct.unmash_class(v1, nullptr);
#else
        mash_info_struct info_struct {mash::UNMASH_MODE, resource, a3->m_size, true};

        info_struct.unmash_class(v1, nullptr, mash::NORMAL_BUFFER);

#endif

        mash_info_struct::construct_class(v1);

#ifdef TARGET_XBOX
        a3->m_offset += info_struct.get_header_size();
#endif
    }

    ++this->field_C;
    return false;
}

void cut_scene_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &cut_scene_resource_handler::_handle);
        set_vfunc(0x00888AD8, address);
    }

    {
        FUNC_ADDRESS(address, &cut_scene_resource_handler::_handle_resource);
        set_vfunc(0x00888AE0, address);
    }
}
