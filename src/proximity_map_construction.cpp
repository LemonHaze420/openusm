#include "proximity_map_construction.h"

#include "common.h"
#include "func_wrapper.h"
#include "proximity_map.h"
#include "stack_allocator.h"
#include "subdivision_static_region_list.h"

#include <cmath>
#include <memory>

VALIDATE_SIZE(proximity_map, 0x60);
VALIDATE_SIZE(subdivision_node_builder, 0x8);

#if STANDALONE_SYSTEM
namespace {

void init_static_proximity_map(
    proximity_map &map, int cell_count, const vector3d &min_extent, const vector3d &max_extent)
{
    map.map_type = static_cast<proximity_map::proximity_map_type_t>(0);
    map.field_0 = subdivision_node::PROXIMITY_MAP_NODE;
    map.initialized = true;
    map.number_of_cells = cell_count;
    map.log_number_of_cells = 0;
    for (auto remaining = cell_count; remaining > 1; remaining >>= 1) {
        ++map.log_number_of_cells;
    }
    map.field_C = min_extent;
    map.field_18 = max_extent;
    map.field_24 = max_extent - min_extent;
    map.field_3C = vector3d{
        1.0f / map.field_24.x, 1.0f / map.field_24.y, 1.0f / map.field_24.z};
    const auto cell_scale = 1.0 / static_cast<double>(cell_count);
    map.field_30 = vector3d{
        static_cast<float>(cell_scale * map.field_24.x),
        static_cast<float>(cell_scale * map.field_24.y),
        static_cast<float>(cell_scale * map.field_24.z)};
    map.field_48 = vector3d{
        map.field_24.x - map.field_30.x * 0.5f,
        map.field_24.y - map.field_30.y * 0.5f,
        map.field_24.z - map.field_30.z * 0.5f};
}

}
#endif

proximity_map *create_static_proximity_map_on_the_stack(stack_allocator &a1,
                                                        _std::vector<proximity_map_construction_leaf> &a2,
                                                        subdivision_node_builder &a3, const vector3d &a4,
                                                        const vector3d &a5, int a6)
{
#if STANDALONE_SYSTEM
    auto root_count = static_cast<uint32_t>(std::sqrt(static_cast<double>(a2.size())));
    unsigned int shift = 0;
    while (root_count > 1) {
        root_count >>= 1;
        ++shift;
    }
    int cell_count = 1 << shift;
    if (cell_count < 4) {
        cell_count = 4;
    } else if (cell_count > 16) {
        cell_count = 16;
    }
    if (a6 != 0) {
        cell_count = a6;
    }

    auto *map = new (a1.push(sizeof(proximity_map))) proximity_map{};
    init_static_proximity_map(*map, cell_count, a4, a5);
    auto *grid = static_cast<uint16_t *>(a1.push(
        cell_count * cell_count * sizeof(uint16_t)));
    map->field_8 = static_cast<uint16_t>(
        (reinterpret_cast<char *>(grid) - reinterpret_cast<char *>(map)) / 4);

    std::unique_ptr<_std::vector<proximity_map_construction_leaf>[]> cell_leaves{
        new _std::vector<proximity_map_construction_leaf>[cell_count * cell_count]};
    a3.m_vtbl->build_mirror(&a3, nullptr, a1, a2);
    for (const auto &leaf : a2) {
        cell_index first;
        cell_index last;
        map->map_vector3d_to_cell_index(leaf.field_4, &first);
        map->map_vector3d_to_cell_index(leaf.field_10, &last);
        for (int y = first.y; y <= last.y; ++y) {
            for (int x = first.x; x <= last.x; ++x) {
                cell_leaves[x + y * cell_count].push_back(leaf);
            }
        }
    }
    for (int y = 0; y < cell_count; ++y) {
        for (int x = 0; x < cell_count; ++x) {
            const auto index = x + y * cell_count;
            auto &leaves = cell_leaves[index];
            if (leaves.size() == 0) {
                grid[index] = 0;
            } else {
                auto *node = a3.m_vtbl->build(&a3, nullptr, a1, leaves);
                grid[index] = static_cast<uint16_t>(
                    (reinterpret_cast<char *>(node) - reinterpret_cast<char *>(map)) / 4);
            }
        }
    }
    return map;
#else
    return (proximity_map *)CDECL_CALL(0x0054E720, &a1, &a2, &a3, &a4, &a5, a6);
#endif
}
