#include "region_audio_box_set.h"

#include "common.h"
#include "func_wrapper.h"
#include "oriented_bounding_box_root_node.h"

VALIDATE_SIZE(region_audio_box_set, 0x20);

region_audio_box_set::region_audio_box_set() {}

void region_audio_box_set::un_mash(char *image, int *image_size_used, region *reg)
{
#if STANDALONE_SYSTEM
    field_0[0] = reinterpret_cast<int>(reg);
    for (int i = 2; i < 8; ++i) {
        field_0[i] = 0;
    }

    auto *obb_root = reinterpret_cast<oriented_bounding_box_root_node *>((reinterpret_cast<uintptr_t>(image) + 63u) &
                                                                         ~uintptr_t{63u});
    field_0[1] = reinterpret_cast<int>(obb_root);
    obb_root->un_mash(image, image_size_used, reg);
    if (obb_root->field_30 == 0) {
        field_0[1] = 0;
    }

    auto *cursor = image + *image_size_used;
    const int box_count = *reinterpret_cast<int *>(cursor);
    cursor += sizeof(int);
    field_0[4] = box_count;
    field_0[5] = reinterpret_cast<int>(cursor);
    cursor += box_count * 68;
    *image_size_used += sizeof(int) + box_count * 68;

    const int event_count = *reinterpret_cast<int *>(cursor);
    cursor += sizeof(int);
    field_0[6] = event_count;
    field_0[7] = reinterpret_cast<int>(cursor);
    *image_size_used += sizeof(int) + event_count * 64;

    auto *boxes = reinterpret_cast<char *>(field_0[5]);
    auto *events = reinterpret_cast<char *>(field_0[7]);
    for (int i = 0; i < event_count; ++i) {
        auto *event = events + i * 64;
        const int box_index = *reinterpret_cast<int *>(event + 44);
        *reinterpret_cast<char **>(event + 44) = boxes + box_index * 68;
    }

    auto *nodes = reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(obb_root + 1) + 3u) & ~uintptr_t{3u});
    for (int i = 0; i < event_count; ++i) {
        auto *event = events + i * 64;
        auto *node = nodes + i * 76;
        *reinterpret_cast<char **>(node + 72) = event;
        *reinterpret_cast<char **>(event + 40) = node;
    }
#else
    THISCALL(0x00520600, this, image, image_size_used, reg);
#endif
}
