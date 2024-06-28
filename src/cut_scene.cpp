#include "cut_scene.h"

#include "nfl_system.h"
#include "nlPlatformEnum.h"

#include "common.h"
#include "cut_scene_segment.h"
#include "entity_class_entry.h"
#include "func_wrapper.h"
#include "log.h"
#include "resource_manager.h"
#include "variables.h"

#include <cassert>

VALIDATE_SIZE(cut_scene, 0x54);
VALIDATE_OFFSET(cut_scene, segments, 0x10);
VALIDATE_ALIGNMENT(cut_scene, 4);

Var<resource_pack_standalone> cut_scene::stream_anim_pack = (0x0096FB90);

Var<mString> cut_scene::scene_anim_packfile_id = (0x0096FB80);

cut_scene::cut_scene(from_mash_in_place_constructor *a2) : field_0(a2), segments(a2)
{
    if constexpr (1)
    {
        if (resource_manager::resource_context_stack.size()) {
            this->field_50 = resource_manager::resource_context_stack.back();
        } else {
            this->field_50 = nullptr;
        }

    }
    else
    {
        THISCALL(0x00742890, this, a2);
    }
}

void cut_scene::init_stream_scene_anims() {
    if constexpr (1) {
        if ((!g_is_the_packer()) &&
            stream_anim_pack().get_nfl_file_handle() == NFL_FILE_ID_INVALID) {
            mString v2 = mString{scene_anim_packfile_id().c_str()};

            mString v3 = mString::get_standalone_filename(v2, g_platform);

            mString v1;
            v1 += v3;

            if (cut_scene::stream_anim_pack().load(v1)) {
                auto my_file = cut_scene::stream_anim_pack().get_nfl_file_handle();
                assert(my_file != NFL_FILE_ID_INVALID);
            }
        }

    } else {
        CDECL_CALL(0x00732C80);
    }
}

int cut_scene::destruct_mashed_class() {
    return THISCALL(0x00742770, this);
}

void cut_scene::unmash(mash_info_struct *a1, void *a3) {
    THISCALL(0x00742930, this, a1, a3);
}
