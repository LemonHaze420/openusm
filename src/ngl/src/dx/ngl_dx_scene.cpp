#include "ngl_dx_scene.h"

#include "common.h"
#include "ngl.h"
#include "nglrendernode.h"
#include "nglshader.h"

#include <func_wrapper.h>
#include <trace.h>
#include <utility.h>
#include <vtbl.h>
#include <algorithm>
#include <cstdint>

nglRenderNode *g_CurrentRenderNode = nullptr;

namespace nglRenderList {

struct nglRenderTextureNode {
    nglRenderNode *m_node;
    nglTexture *m_tex;
};

VALIDATE_SIZE(nglRenderTextureNode, 0x8u);

// 0x0077DFB0
void sort_opaque_nodes(nglRenderTextureNode *begin, nglRenderTextureNode *end)
{
    std::sort(begin, end, [](const auto &left, const auto &right) { return left.m_tex < right.m_tex; });
}

static void render_node(nglRenderNode *node)
{
#if STANDALONE_SYSTEM
    if (node->m_vtbl == 0x008B9FB4) {
        static_cast<nglQuadNode *>(node)->Render();
        return;
    }
    if (node->m_vtbl == 0x0088EBB4) {
        reinterpret_cast<nglStringNode *>(node)->Render();
        return;
    }
#endif
    node->Render();
}

template <>
void nglOpaqueCompare<nglRenderNode>(nglRenderNode *node, int count, int a3)
{
    TRACE("nglRenderList::nglOpaqueCompare<nglRenderNode>");

    if constexpr (1) {
        nglRenderTextureNode *v1 = static_cast<decltype(v1)>(nglListAlloc(sizeof(nglRenderTextureNode) * count, 16));

        [](nglRenderTextureNode *a1, nglRenderNode *a2) -> void {
            for (; a2 != nullptr; ++a1, a2 = a2->m_next_node) {
                a1->m_node = a2;
                a1->m_tex = a2->m_tex;
            }
        }(v1, node);

        sort_opaque_nodes(v1, v1 + count);

        [](auto *begin, nglRenderNode *&a2, int count) -> void {
            auto end = begin + count;

            nglRenderNode *v3 = nullptr;
            std::for_each(std::make_reverse_iterator(end), std::make_reverse_iterator(begin), [&v3](auto &n) {
                n.m_node->m_next_node = v3;
                v3 = n.m_node;
            });

            a2 = v3;
        }(v1, node, count);

        static Var<nglRenderNode *> nglPrevNode{0x00971F18};

        for (auto *v9 = node; v9 != nullptr; v9 = v9->m_next_node) {
            g_CurrentRenderNode = v9;
            render_node(v9);

            nglPrevNode() = v9;
            g_CurrentRenderNode = nullptr;
        }
    } else {
        CDECL_CALL(0x0077E190, node, count, a3);
    }
}

// 0x0077E220
void nglTransCompare(nglRenderNode *node, int count, int)
{
    nglRenderTextureNode *nodes =
        static_cast<nglRenderTextureNode *>(nglListAlloc(sizeof(nglRenderTextureNode) * count, 16));
    auto *entry = nodes;
    for (auto *current = node; current != nullptr; current = current->m_next_node) {
        entry->m_node = current;
        entry->m_tex = current->m_tex;
        ++entry;
    }

    std::sort(nodes, nodes + count, [](const auto &left, const auto &right) {
        const auto left_key = bit_cast<float>(bit_cast<uint32_t>(left.m_tex));
        const auto right_key = bit_cast<float>(bit_cast<uint32_t>(right.m_tex));
        if (left_key > right_key) {
            return true;
        }
        if (right_key > left_key) {
            return false;
        }
        return left.m_node < right.m_node;
    });

    nglRenderNode *sorted = nullptr;
    for (auto *current = nodes + count; current != nodes;) {
        --current;
        current->m_node->m_next_node = sorted;
        sorted = current->m_node;
    }

    static Var<nglRenderNode *> nglPrevNode{0x00971F18};
    for (auto *current = sorted; current != nullptr; current = current->m_next_node) {
        g_CurrentRenderNode = current;
        render_node(current);
        nglPrevNode() = current;
        g_CurrentRenderNode = nullptr;
    }
}

}  // namespace nglRenderList

void *nglListAlloc(int size, int align)
{
    TRACE("nglListAlloc");

    if constexpr (STANDALONE_SYSTEM) {
        assert(size >= 0);
        assert(align > 0 && (align & (align - 1)) == 0);

        auto *list_begin = var<std::uint8_t *>(0x00971F08);
        const auto list_capacity = var<std::uint32_t>(0x00971F10);
        auto *position = nglListWorkPos();
        assert(list_begin != nullptr && position != nullptr);

        const auto aligned =
            (reinterpret_cast<std::uintptr_t>(position) + align - 1) & ~static_cast<std::uintptr_t>(align - 1);
        auto *result = reinterpret_cast<std::uint8_t *>(aligned);
        assert(result >= list_begin);
        assert(static_cast<std::size_t>(result - list_begin) + static_cast<std::size_t>(size) <= list_capacity);
        nglListWorkPos() = result + size;
        return result;
    } else {
        return (void *)CDECL_CALL(0x00401A20, size, align);
    }
}

void nglRenderList_patch()
{
    auto *address = &nglRenderList::nglOpaqueCompare<nglRenderNode>;

    REDIRECT(0x0077D162, address);
}
