#include "cut_scene_segment.h"

#include "camera_setup_entry.h"
#include "common.h"
#include "func_wrapper.h"
#include "tracking_panel.h"

VALIDATE_SIZE(cut_scene_segment, 0xB0u);

cut_scene_segment::cut_scene_segment(from_mash_in_place_constructor *a2) {
    THISCALL(0x007425B0, this, a2);
}
