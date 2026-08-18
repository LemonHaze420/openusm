#include "path_graph.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "mash_virtual_base.h"
#include "vtbl.h"

#include <cstdint>

#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
namespace
{
struct path_index_vector
{
    int field_0;
    int size;
    uint16_t *data;
    int capacity;
    bool field_10;
    uint8_t padding[3];
};

struct path_node
{
    uint8_t field_0[0x14];
    mash_virtual_base *field_14;
    path_index_vector field_18;
    uint32_t field_2C;
};

struct path_edge
{
    uint8_t fields[0x14];
};

static_assert(sizeof(path_index_vector) == 0x14);
static_assert(sizeof(path_node) == 0x30);
static_assert(sizeof(path_edge) == 0x14);

template<typename T>
void finish_vector(mVector<T> &vector, mash_info_struct &info)
{
    vector.field_0 = reinterpret_cast<int>(
        &info.mash_image_ptr[mash::NORMAL_BUFFER][
            info.buffer_size_used[mash::NORMAL_BUFFER] -
            reinterpret_cast<uint32_t>(&vector)]);
}

void unmash_indices(path_index_vector &indices, mash_info_struct &info)
{
    if (indices.data != nullptr) {
        indices.data = reinterpret_cast<uint16_t *>(info.read_from_buffer(
            mash::NORMAL_BUFFER, 2 * indices.size, 2));
    }

    indices.field_0 = reinterpret_cast<int>(
        &info.mash_image_ptr[mash::NORMAL_BUFFER][
            info.buffer_size_used[mash::NORMAL_BUFFER] -
            reinterpret_cast<uint32_t>(&indices)]);
}

void unmash_node(path_node &node, mash_info_struct &info)
{
    unmash_indices(node.field_18, info);

    if (node.field_14 == nullptr) {
        return;
    }

    node.field_14 = reinterpret_cast<mash_virtual_base *>(
        info.read_from_buffer(mash::NORMAL_BUFFER, 8, 0));
    mash_virtual_base::fixup_vtable(node.field_14);

    const auto get_size = reinterpret_cast<int (__fastcall *)(void *, int)>(
        get_vfunc(node.field_14->m_vtbl, 0x28));
    info.advance_buffer(mash::NORMAL_BUFFER,
                        get_size(node.field_14, 0) - 8);
    node.field_14->unmash(&info, &node);
}

void unmash_nodes(mVector<path_graph_node> &vector, mash_info_struct &info)
{
    if (vector.m_data != nullptr) {
        vector.m_data = reinterpret_cast<path_graph_node **>(
            info.read_from_buffer(
                mash::NORMAL_BUFFER, 4 * vector.m_size, 4));

        for (int i = 0; i < vector.m_size; ++i) {
            auto *node = reinterpret_cast<path_node *>(info.read_from_buffer(
                mash::NORMAL_BUFFER, sizeof(path_node), 4));
            vector.m_data[i] = reinterpret_cast<path_graph_node *>(node);
            unmash_node(*node, info);
        }
    }

    finish_vector(vector, info);
}

void unmash_edges(mVector<path_graph_edge> &vector, mash_info_struct &info)
{
    if (vector.m_data != nullptr) {
        vector.m_data = reinterpret_cast<path_graph_edge **>(
            info.read_from_buffer(
                mash::NORMAL_BUFFER, 4 * vector.m_size, 4));

        for (int i = 0; i < vector.m_size; ++i) {
            vector.m_data[i] = reinterpret_cast<path_graph_edge *>(
                info.read_from_buffer(
                    mash::NORMAL_BUFFER, sizeof(path_edge), 4));
        }
    }

    finish_vector(vector, info);
}
}
#endif

VALIDATE_SIZE(path_graph, 0x34);
VALIDATE_ALIGNMENT(path_graph, 4);

path_graph::path_graph(from_mash_in_place_constructor *a2) {
    THISCALL(0x005DE080, this, a2);
}

int path_graph::destruct_mashed_class() {
    return THISCALL(0x005DC3A0, this);
}

int path_graph::unmash(mash_info_struct *a1, void *a3) {
#if defined(OPENUSM_XBPACK_MODE) && !defined(TARGET_XBOX)
    a1->unmash_class_in_place(this->id, this);
    unmash_nodes(this->field_8, *a1);
    unmash_edges(this->field_1C, *a1);
    return 0;
#else
    return THISCALL(0x005DC3E0, this, a1, a3);
#endif
}
