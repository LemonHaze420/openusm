#pragma once

#include "mAvlTree.h"
#include "mstring.h"
#include "mvector.h"
#include "resource_pack_standalone.h"

struct from_mash_in_place_constructor;
struct entity_class_entry;
struct resource_pack_slot;
struct cut_scene_segment;

struct cut_scene {
    mAvlTree<entity_class_entry> field_0;
    mVector<cut_scene_segment> segments;
    bool field_24;
    bool field_25;
    bool field_26;
    bool field_27;
    bool field_28;
    int field_2C;
    bool field_30;
    resource_key field_34;
    mString field_3C;
    int field_4C;
    resource_pack_slot *field_50;

    //0x00742890
    cut_scene(from_mash_in_place_constructor *a2);

    //0x00742770
    void destruct_mashed_class();

    //0x00742930
    void unmash(mash_info_struct *a1, void *a3);

    //@bug
    //0x00732C80
    static void init_stream_scene_anims();

    //0x0096FB90
    static resource_pack_standalone &stream_anim_pack;

    //0x0096FB80
    static mString &scene_anim_packfile_id;
};
