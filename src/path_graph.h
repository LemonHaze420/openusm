#pragma once

#include "mvector.h"
#include "mVectorBasic.h"
#include "resource_key.h"

struct mash_info_struct;
struct nugget;
struct from_mash_in_place_constructor;

struct path_graph_node {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    nugget *field_14;
    mVectorBasic<short> field_18;

    path_graph_node(from_mash_in_place_constructor *a2);
};

struct path_graph_edge {

    int field_0;
    int field_4;
    float field_8;
    short field_C;
    short field_E;
    short field_10;
    short field_12;

    path_graph_edge(from_mash_in_place_constructor *);

    void initialize(mash::allocation_scope a2);

    void clear();
};

struct path_graph {
    resource_key id;
    mVector<path_graph_node> field_8;
    mVector<path_graph_edge> field_1C;
    int field_30;

    //0x005DE080
    path_graph(from_mash_in_place_constructor *a2);

    //0x005DC3A0
    void destruct_mashed_class();

    //0x005DC3E0
    void unmash(mash_info_struct *a1, void *a3);
};
