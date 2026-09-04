#include "texture_array.h"

#include "func_wrapper.h"
#include "trace.h"

texture_array *sub_5207E0(nglTexture *a1)
{
    return (texture_array *)CDECL_CALL(0x005207E0, a1);
}

void texture_array::load_map_internal(texture_to_frame_map *a2, const char *a3)
{
    TRACE("texture_array::load_map_internal");

    THISCALL(0x00510360, this, a2, a3);
}

void texture_array::load_map(texture_to_frame_map *a2, nglTexture *a1, const char *a3)
{
    auto *v3 = sub_5207E0(a1);
    return v3->load_map_internal(a2, a3);
}

void texture_array::unload_map(texture_to_frame_map *a1, nglTexture *a2)
{
    CDECL_CALL(0x0052AE50, a1, a2);
}
