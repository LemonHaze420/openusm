#include "path_graph.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(path_graph_node, 0x28);

VALIDATE_SIZE(path_graph, 0x34);
VALIDATE_ALIGNMENT(path_graph, 4);

path_graph_node::path_graph_node(from_mash_in_place_constructor *a2) : field_18(a2)
{
    if (this->field_14 != nullptr) {
        mash_info_struct::construct_class(this->field_14);
    }
}

path_graph_edge::path_graph_edge(from_mash_in_place_constructor *)
{
    this->initialize(mash::FROM_MASH);
}

void path_graph_edge::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->clear();
    }
}

void path_graph_edge::clear()
{
    this->field_E = -1;
    this->field_C = -1;
    this->field_0 = 0;
    this->field_4 = 0;
    this->field_8 = 1.0;
    this->field_10 = -1;
}

path_graph::path_graph(from_mash_in_place_constructor *a2)
{
    THISCALL(0x005DE080, this, a2);
}

void path_graph::destruct_mashed_class()
{
    THISCALL(0x005DC3A0, this);
}

void path_graph::unmash(mash_info_struct *a1, void *a3)
{
    THISCALL(0x005DC3E0, this, a1, a3);
}
