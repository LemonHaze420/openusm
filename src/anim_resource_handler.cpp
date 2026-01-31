#include "anim_resource_handler.h"

#include <character_anim_inst.h>
#include "common.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cassert>

VALIDATE_SIZE(anim_resource_handler, 0x14);

anim_resource_handler::anim_resource_handler(worldly_pack_slot *a2)
{
    this->m_vtbl = 0x008889D8;
    this->my_slot = a2;
    this->field_10 = TLRESOURCE_TYPE_ANIM_FILE;
}

bool anim_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("anim_resource_handler::handle");

    return base_tl_resource_handler::_handle(a2, a3);
}

bool anim_resource_handler::_handle_resource(worldly_resource_handler::eBehavior behavior,
                                            tlresource_location *a3)
{
    TRACE("anim_resource_handler::handle_resource");

    if constexpr (0)
    {
        auto *anim_file = (nalAnimFile *) a3->get_data();
        assert(anim_file != nullptr && "Could not load the animation file.");

        if (behavior == UNLOAD) {
            auto *v7 = (nalChar::nalCharAnim *) anim_file->field_34;
            while (v7 != nullptr)
            {
                auto *v8 = (nalChar::nalCharAnim *) v7->field_4;

                void (__fastcall *Release)(nalChar::nalCharAnim *) = CAST(Release, get_vfunc(v7->m_vtbl, 0x8));
                Release(v7);

                v7 = v8;
            }

        } else {
            //sp_log("%s", anim_file->field_48.to_string());

            anim_file->field_44 = 1;
            anim_file->field_4 |= 4u;
            auto result = nalLoadAnimFileInternal(anim_file);
            assert(result);
        }

        ++this->field_C;
        return false;
    }
    else
    {
        return (bool) THISCALL(0x0055F930, this, behavior, a3);
    }
}

void anim_resource_handler_patch()
{
    {
        FUNC_ADDRESS(address, &anim_resource_handler::_handle);
        set_vfunc(0x008889DC, address);
    }

    {
        FUNC_ADDRESS(address, &anim_resource_handler::_handle_resource);
        set_vfunc(0x008889E4, address);
    }
}
