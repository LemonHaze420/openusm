#include "rtree.h"

#include "func_wrapper.h"
#include "rtree_root.h"
#include "subdivision_visitor.h"

void traverse_rtree(const vector3d &start, const vector3d &end, const rtree_root_t &root, subdivision_visitor &visitor)
{
    int16_t minimum[3], negative_maximum[3];
    for (int axis = 0; axis != 3; ++axis) {
        float a = (start[axis] - root.field_0[axis]) * root.field_10[axis];
        float b = (end[axis] - root.field_0[axis]) * root.field_10[axis];
        a = a < -32766.0f ? -32766.0f : a;
        b = b < -32766.0f ? -32766.0f : b;
        a = a > 32766.0f ? 32766.0f : a;
        b = b > 32766.0f ? 32766.0f : b;
        minimum[axis] = static_cast<int16_t>(a < b ? a : b);
        negative_maximum[axis] = static_cast<int16_t>(-(a > b ? a : b));
    }
    struct stack_entry {
        uint16_t offset;
        uint16_t level;
    };
    stack_entry stack[100];
    unsigned size = 0;
    for (unsigned i = static_cast<uint16_t>(root.field_28); i != 0; --i)
        stack[size++] = {static_cast<uint16_t>((i - 1) * sizeof(rtree_node_t)), 1};
    while (size) {
        const auto entry = stack[--size];
        const auto &node = root.field_20[entry.offset / sizeof(rtree_node_t)];

        if (minimum[0] > node.maxx || negative_maximum[0] > node.minx || minimum[1] > node.maxy ||
            negative_maximum[1] > node.miny || minimum[2] > node.maxz || negative_maximum[2] > node.minz)
            continue;
        if (entry.level < root.field_2C) {
            for (int child = 3; child >= 0; --child)
                stack[size++] = {static_cast<uint16_t>(node.field_C.field_0 + child * sizeof(rtree_node_t)),
                                 static_cast<uint16_t>(entry.level + 1)};
        } else {
            auto *leaf = reinterpret_cast<const subdivision_node *>(root.field_24 + node.field_C.field_0);
            visitor.visit(*leaf);
        }
    }
}

rtree_construction_node_t::rtree_construction_node_t(entity_base_vhandle a2, const vector3d &a1, const vector3d &a4)
    : field_0(a1), field_C(a4), field_18(a2)
{}

void rtree_node_t::init(const rtree_construction_node_t &a2, const math::VecClass<4, -1> &a3,
                        const math::VecClass<4, -1> &a4)
{
    const auto pack = [](float value) {
        value = value < -32766.0f ? -32766.0f : value;
        value = value > 32766.0f ? 32766.0f : value;
        return static_cast<int16_t>(value);
    };
    minx = pack(-(a2.field_0.x - a3[0]) * a4[0] + 1.0f);
    miny = pack(-(a2.field_0.y - a3[1]) * a4[1] + 1.0f);
    minz = pack(-(a2.field_0.z - a3[2]) * a4[2] + 1.0f);
    maxx = pack((a2.field_C.x - a3[0]) * a4[0] + 1.0f);
    maxy = pack((a2.field_C.y - a3[1]) * a4[1] + 1.0f);
    maxz = pack((a2.field_C.z - a3[2]) * a4[2] + 1.0f);
    field_C = a2.field_18;
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
