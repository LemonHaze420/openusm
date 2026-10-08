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
#include <new>

VALIDATE_SIZE(cut_scene, 0x54);
VALIDATE_OFFSET(cut_scene, segments, 0x10);
VALIDATE_ALIGNMENT(cut_scene, 4);

resource_pack_standalone &cut_scene::stream_anim_pack = var<resource_pack_standalone>(0x0096FB90);

mString &cut_scene::scene_anim_packfile_id = var<mString>(0x0096FB80);

cut_scene::cut_scene(from_mash_in_place_constructor *a2) : field_0(a2), segments(a2)
{
    if constexpr (1) {
        if (resource_manager::resource_context_stack.size()) {
            this->field_50 = resource_manager::resource_context_stack.back();
        } else {
            this->field_50 = nullptr;
        }

    } else {
        THISCALL(0x00742890, this, a2);
    }
}

void cut_scene::init_stream_scene_anims()
{
#if STANDALONE_SYSTEM
    static const bool initialized = [] {
        new (&scene_anim_packfile_id) mString{"scnanims"};
        new (&stream_anim_pack) resource_pack_standalone{};
        return true;
    }();
    (void)initialized;
#endif
    if constexpr (1) {
        if (!g_is_the_packer && stream_anim_pack.get_nfl_file_handle() == NFL_FILE_ID_INVALID) {
            mString v2{scene_anim_packfile_id.c_str()};
            mString v3 = mString::get_standalone_filename(v2, g_platform);
            mString v1;
            v1 += v3;
            if (stream_anim_pack.load(v1)) {
                auto my_file = stream_anim_pack.get_nfl_file_handle();
                assert(my_file != NFL_FILE_ID_INVALID);
            }
        }
    } else {
        CDECL_CALL(0x00732C80);
    }
}

void cut_scene::destruct_mashed_class()
{
    if constexpr (STANDALONE_SYSTEM) {
        field_0.destruct_mashed_class();
        segments.destruct_mashed_class();
        sync_camera.destruct_mashed_class();
        field_3C.destruct_mashed_class();
    } else {
        void(__fastcall * func)(cut_scene *) = CAST(func, 0x00742770);
        func(this);
    }
}

void cut_scene::unmash(mash_info_struct *a1, [[maybe_unused]] void *a3)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->segments, this);
    a1->unmash_class_in_place(this->sync_camera, this);
    a1->unmash_class_in_place(this->field_3C, this);
}
