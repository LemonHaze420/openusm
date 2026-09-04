#pragma once

#include "variable.h"

using subdivision_node_type_t = uint8_t;

#pragma pack(push, 1)
struct subdivision_node {
    enum type_t {
        UNDEFINED_NODE,
        PROXIMITY_MAP_NODE,
        STATIC_LEAF_LIST_NODE,
        DYNAMIC_LEAF_LIST_NODE,
        AABB_LEAF_NODE,
        AABB_LARGE_LEAF_NODE,
        OBB_LEAF_NODE,
        OBB_LARGE_LEAF_NODE,
        AUDIO_OBB_LEAF_NODE,
        LEGO_LEAF_NODE,
        STATIC_LEGO_LIST_NODE,
        STATIC_REGION_LIST_NODE,
        DYNAMIC_ENTITY_LIST_NODE,
        AABSP_NODE,
        BUILDING_LEAF_NODE,
        TOTAL_TYPES,
        VISITED_FLAG = 0x80,
    };

    type_t get_type() const
    {
        return type_t(m_type & ~VISITED_FLAG);
    }

    void set_type(type_t type)
    {
        m_type = static_cast<subdivision_node_type_t>(type);
    }

    static inline auto &methods = var<void *[13]>(0x00960980);

private:
    subdivision_node_type_t m_type;
};
#pragma pack(pop)
