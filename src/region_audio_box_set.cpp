#include "region_audio_box_set.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(region_audio_box_set, 0x20);

region_audio_box_set::region_audio_box_set() {}

void region_audio_box_set::un_mash(char *a2, int *a3, region *a4)
{
    THISCALL(0x00520600, this, a2, a3, a4);
}
