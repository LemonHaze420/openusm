#include "skeleton_resource_handler.h"

#include "common.h"
#include "entity.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"
#include "string_hash_dictionary.h"
#include "utility.h"
#include "vtbl.h"

#include <nal_skeleton.h>

VALIDATE_SIZE(skeleton_resource_handler, 0x14u);

skeleton_resource_handler::skeleton_resource_handler(worldly_pack_slot *a1)
{
    this->m_vtbl = 0x008889C8;
    this->my_slot = a1;
    this->field_10 = TLRESOURCE_TYPE_SKELETON;
}

bool skeleton_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("skeleton_resource_handler::handle");

    return base_tl_resource_handler::_handle(a2, a3);
}

//FIXME
bool skeleton_resource_handler::_handle_resource(worldly_resource_handler::eBehavior a2,
                                                tlresource_location *a3)
{

    TRACE("skeleton_resource_handler::handle_resource", int(a2), a3->get_name().to_string(), int(static_cast<uint8_t>(a3->get_type())));

    if constexpr (0)
    {
        if (a2 == UNLOAD)
        {
            nalBaseSkeleton *skel = CAST(skel, a3->get_data());
            skel->Release();
        } else {
            auto *new_skel = static_cast<char *>(nalConstructSkeleton(a3->get_data()));
            a3->set_data(new_skel);
        }

        ++this->field_C;
        return false;
    } else {
        return (bool) THISCALL(0x0055F8E0, this, a2, a3);
    }
}

void skeleton_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &skeleton_resource_handler::_handle);
        set_vfunc(0x008889CC, address);
    }

    {
        FUNC_ADDRESS(address, &skeleton_resource_handler::_handle_resource);
        set_vfunc(0x008889D4, address);
    }
}
