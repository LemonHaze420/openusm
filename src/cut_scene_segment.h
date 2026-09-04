#pragma once

#include "mAvlTree.h"
#include "mstring.h"
#include "mvector.h"
#include "mVectorBasic.h"
#include "string_hash.h"

struct entity_class_entry;
struct camera_setup_entry;
struct from_mash_in_place_constructor;
struct nalSceneAnim;
struct replay_info;
struct resource_key;
struct tracking_panel;

struct cut_scene_segment {
    bool field_0;
    bool field_1;
    bool field_2;
    bool field_3;
    bool field_4;
    bool field_5;
    bool field_6;
    int field_8;
    replay_info *field_C;
    mVectorBasic<nalSceneAnim *> field_10;
    mVector<resource_key> field_20;
    mVector<mString> field_34;
    mVector<mVector<mString>> field_48;
    mVector<camera_setup_entry> field_5C;
    mVector<tracking_panel> field_70;
    mAvlTree<entity_class_entry> field_84;
    string_hash field_94;
    mString field_98;
    bool field_A8;
    int field_AC;

    //0x007425B0
    cut_scene_segment(from_mash_in_place_constructor *a2);
};
