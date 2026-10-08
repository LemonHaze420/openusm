#include "quad_path_cell_astar_search_record.h"
#include "ai_quad_path_cell.h"
#include "common.h"
#include <array>
#include "ai_quad_path.h"
#include "ai_quad_path_exit.h"
#include "ai_region_paths.h"
#include "region.h"

namespace {
void *__fastcall finalize(quad_path_cell_astar_search_record *self, void *, unsigned int flags)
{
    self->clean_up();
    self->~quad_path_cell_astar_search_record();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
int __fastcall get_handle(quad_path_cell_astar_search_record *, void *, ai_quad_path_cell *cell)
{
    return cell->field_44;
}
void *__fastcall reset_neighbors(quad_path_cell_astar_search_record *, void *, ai_quad_path_cell *cell)
{
    cell->field_48 = 0;
    return nullptr;
}
ai_quad_path_cell *__fastcall next_neighbor(quad_path_cell_astar_search_record *, void *, ai_quad_path_cell *cell,
                                            void *)
{
    auto edge = static_cast<unsigned int>(cell->field_48) & 0xFFFFu;
    auto index = static_cast<unsigned int>(cell->field_48) >> 16;
    while (edge < 4 && index >= cell->neighbor_counts[edge]) {
        ++edge;
        index = 0;
    }
    cell->field_48 = static_cast<int>(edge | (index << 16));
    if (edge == 4)
        return nullptr;
    auto *neighbor = cell->neighbors[edge][index++];
    cell->field_48 = static_cast<int>(edge | (index << 16));
    return neighbor;
}
float __fastcall distance(quad_path_cell_astar_search_record *, void *, ai_quad_path_cell *first,
                          ai_quad_path_cell *second)
{
    return first->fast_distance_check(*second);
}
bool __fastcall set_handle(quad_path_cell_astar_search_record *, void *, ai_quad_path_cell *cell, unsigned int handle)
{
    cell->field_44 = handle;
    return true;
}
}

quad_path_cell_astar_search_record::quad_path_cell_astar_search_record()
{
    static const std::array<std::uintptr_t, 7> table = {reinterpret_cast<std::uintptr_t>(&set_handle),
                                                        reinterpret_cast<std::uintptr_t>(&get_handle),
                                                        reinterpret_cast<std::uintptr_t>(&reset_neighbors),
                                                        reinterpret_cast<std::uintptr_t>(&next_neighbor),
                                                        reinterpret_cast<std::uintptr_t>(&distance),
                                                        reinterpret_cast<std::uintptr_t>(&distance),
                                                        reinterpret_cast<std::uintptr_t>(&finalize)};
    this->m_vtbl = reinterpret_cast<int>(table.data());
    this->field_4 = nullptr;
    this->m_node_pool = nullptr;
    this->field_1C = false;
    this->goal_found = false;
    this->path_goal_to_start = &field_24;
}

void quad_path_cell_astar_search_record::setup(void *a2, void *a3)
{
    astar_search_record::setup(a2, a3, &field_24, nullptr);
}

namespace {
bool __fastcall quad_assign_handle(quad_path_cell_astar_search_record *, void *, ai_quad_path *node,
                                   unsigned int handle)
{
    node->field_2C = handle;
    return true;
}
int __fastcall quad_get_handle(quad_path_cell_astar_search_record *, void *, ai_quad_path *node)
{
    return node->field_2C;
}
void *__fastcall quad_reset_neighbors(quad_path_cell_astar_search_record *, void *, ai_quad_path *node)
{
    node->field_30 = 0;
    return &node->field_30;
}
ai_quad_path *__fastcall quad_next_neighbor(quad_path_cell_astar_search_record *, void *, ai_quad_path *node, void *)
{
    const auto *exits = reinterpret_cast<const ai_quad_path_exit *>(node->field_20);
    while (node->field_30 < node->field_28) {
        const auto &exit = exits[node->field_30++];
        if (exit.district != node->field_1C || (exit.flags & 2) != 0)
            continue;
        auto *graph = node->field_18->get_region_path_graph();
        for (int i = 0; i < graph->quad_path_table_count; ++i) {
            auto *candidate = graph->get_quad_path(i);
            if ((static_cast<unsigned int>(candidate->field_1E) << 16 | candidate->field_1C) ==
                (static_cast<unsigned int>(exit.path) << 16 | exit.district))
                return candidate;
        }
        return nullptr;
    }
    return nullptr;
}
float __fastcall quad_distance(quad_path_cell_astar_search_record *, void *, ai_quad_path *first, ai_quad_path *second)
{
    if (first == second)
        return 0.0f;
    const auto *a = reinterpret_cast<const float *>(first->field_0);
    const auto *b = reinterpret_cast<const float *>(second->field_0);
    const vector3d delta{
        (b[0] + b[3] - a[0] - a[3]) * 0.5f, (b[1] + b[4] - a[1] - a[4]) * 0.5f, (b[2] + b[5] - a[2] - a[5]) * 0.5f};
    return std::max(delta.length(), 1.0f);
}
float __fastcall district_travel(quad_path_cell_astar_search_record *, void *, ai_quad_path *, void *)
{
    return 1.0f;
}
float __fastcall district_estimate(quad_path_cell_astar_search_record *, void *, ai_quad_path *node, void *district)
{
    const auto *exits = reinterpret_cast<const ai_quad_path_exit *>(node->field_20);
    for (unsigned int i = 0; i < node->field_28; ++i)
        if (exits[i].district == reinterpret_cast<std::uintptr_t>(district))
            return 0.0f;
    return 1.0f;
}
}

quad_path_astar_search_record::quad_path_astar_search_record(bool district_goal)
{
    static const std::array<std::uintptr_t, 7> path_table = {reinterpret_cast<std::uintptr_t>(&quad_assign_handle),
                                                             reinterpret_cast<std::uintptr_t>(&quad_get_handle),
                                                             reinterpret_cast<std::uintptr_t>(&quad_reset_neighbors),
                                                             reinterpret_cast<std::uintptr_t>(&quad_next_neighbor),
                                                             reinterpret_cast<std::uintptr_t>(&quad_distance),
                                                             reinterpret_cast<std::uintptr_t>(&quad_distance),
                                                             reinterpret_cast<std::uintptr_t>(&finalize)};
    static const std::array<std::uintptr_t, 7> district_table = {
        reinterpret_cast<std::uintptr_t>(&quad_assign_handle),
        reinterpret_cast<std::uintptr_t>(&quad_get_handle),
        reinterpret_cast<std::uintptr_t>(&quad_reset_neighbors),
        reinterpret_cast<std::uintptr_t>(&quad_next_neighbor),
        reinterpret_cast<std::uintptr_t>(&district_travel),
        reinterpret_cast<std::uintptr_t>(&district_estimate),
        reinterpret_cast<std::uintptr_t>(&finalize)};
    m_vtbl = reinterpret_cast<int>((district_goal ? district_table : path_table).data());
}
