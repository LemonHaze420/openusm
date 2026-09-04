#pragma once

#include "conglomerate_interface.h"

struct entity_base;
struct po;

#if defined(TARGET_XBOX) || defined(OPENUSM_XBPACK_MODE)
struct mash_info_struct;
#endif

struct skeleton_interface : conglomerate_interface {
    po *abs_po;
    int po_count;

    skeleton_interface(conglomerate *a1);

#if defined(TARGET_XBOX) || defined(OPENUSM_XBPACK_MODE)
    void unmash(mash_info_struct *a2, void *a3);
#endif

    void release_ifc();

    void connect_bone_abs_po(int bone_idx, entity_base *new_bone);
};
