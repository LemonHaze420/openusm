#include "rtree.h"

#include "func_wrapper.h"
#include "rtree_client.h"

void traverse_rtree(const vector3d &a1, const vector3d &a2, const rtree_root_t &a3, subdivision_visitor &a4)
{
    if (!rtree_query_size_error_handler(a1, a2)) {
        return;
    }

    CDECL_CALL(0x00520FA0, &a1, &a2, &a3, &a4);
}

rtree_construction_node_t::rtree_construction_node_t(entity_base_vhandle a2, const vector3d &a1, const vector3d &a4)
    : field_0(a1), field_C(a4), field_18(a2)
{}

void rtree_node_t::init(const rtree_construction_node_t &a2, const math::VecClass<4, -1> &a3,
                        const math::VecClass<4, -1> &a4)
{
    THISCALL(0x0055E9F0, this, &a2, &a3, &a4);
}

void rtree_node_t::clear()
{
    this->minx = -32767;
    this->miny = -32767;
    this->minz = -32767;
    this->maxx = -32767;
    this->maxy = -32767;
    this->maxz = -32767;
    this->field_C.field_0 = -1;
}
