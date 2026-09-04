#include "texture_to_frame_map.h"

#include "common.h"

#include <cassert>

VALIDATE_OFFSET(texture_to_frame_map, from_mash, 0x25);
VALIDATE_OFFSET(texture_to_frame_map, total_textures, 0x28);

void texture_to_frame_map::un_mash(char *image_ptr, int *a3)
{
    assert(*(bool *)image_ptr == this->from_mash);
    assert(this->from_mash);
    assert(*(int *)image_ptr == total_textures);

    *((int *)image_ptr + 11) = (int)(image_ptr + 0x30);
    *a3 = 0x24 * this->total_textures + 0x30;
}

mString texture_to_frame_map::get_ifl_name() const
{
    return mString{this->field_4};
}
