#pragma once

#include <cstdint>

struct subdivision_node;

struct subdivision_visitor {
    std::intptr_t m_vtbl;

    struct native_vtable {
        int (*visit)(subdivision_visitor &, const subdivision_node &);
        int (*visit_index)(subdivision_visitor &, int);
    };

    int visit(const subdivision_node &node)
    {
        return reinterpret_cast<const native_vtable *>(m_vtbl)->visit(*this, node);
    }

    int visit_index(int index)
    {
        return reinterpret_cast<const native_vtable *>(m_vtbl)->visit_index(*this, index);
    }
};
