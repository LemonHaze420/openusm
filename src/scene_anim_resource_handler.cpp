#include "scene_anim_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "nal_system.h"
#include "scene_anim.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(scene_anim_resource_handler, 0x14);

scene_anim_resource_handler::scene_anim_resource_handler(worldly_pack_slot *a2)
{
    this->m_vtbl = 0x008889E8;
    this->my_slot = a2;
    this->field_10 = TLRESOURCE_TYPE_SCENE_ANIM;
}

bool scene_anim_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("scene_anim_resource_handler::handle");

    return base_tl_resource_handler::_handle(a2, a3);
}

bool scene_anim_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                  tlresource_location *loc)
{
    TRACE("scene_anim_resource_handler::handle_resource");

    if constexpr (1)
    {
        auto *scene_anim = (nalSceneAnim *) loc->get_data();
        assert(scene_anim != nullptr && "Scene anim didn't load.");

        if (scene_anim->field_10.m_hash != loc->get_name().source_hash_code) {
            auto *v4 = scene_anim->field_10.to_string();
            sp_log("Scene animation name mismatch with %s", v4);
            assert(0);
        }

        if (a2 == UNLOAD) {
            nalReleaseSceneAnimInternal(scene_anim);
        } else {
            scene_anim->field_4C = 1;
            scene_anim->field_4 |= 4;
            nalLoadSceneAnimInternal(scene_anim);
        }

        ++this->field_C;
        return false;
    }
    else
    {
        return (bool) THISCALL(0x0055F990, this, a2, loc);
    }
}

void scene_anim_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &scene_anim_resource_handler::_handle);
        set_vfunc(0x008889EC, address);
    }

    {
        FUNC_ADDRESS(address, &scene_anim_resource_handler::_handle_resource);
        set_vfunc(0x008889F4, address);
    }
}
