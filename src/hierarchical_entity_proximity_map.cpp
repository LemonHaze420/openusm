#include "hierarchical_entity_proximity_map.h"

#include "common.h"
#include "entity.h"
#include "entity_proximity_map_data.h"
#include "fixed_pool.h"
#include "light_source.h"
#include "memory.h"
#include "proximity_map.h"
#include "subdivision_visitor.h"
#include "utility.h"
#include "vtbl.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <new>

VALIDATE_SIZE(hierarchical_entity_proximity_map, 0x418u);
VALIDATE_SIZE(dynamic_proximity_map, 0x74u);

static Var<fixed_pool> hash_entry_pool{0x00922240};

namespace {
unsigned entity_bucket(const entity *ent)
{
    return (reinterpret_cast<uintptr_t>(ent) >> 8) & 0xFF;
}

void *pool_entry(fixed_pool &pool)
{
    if (pool.field_4 == nullptr) {
        assert(pool.field_18 || pool.field_14 <= 0);
        pool.sub_4368C0();
    }
    void *entry = pool.field_4;
    pool.field_4 = *static_cast<void **>(entry);
    return entry;
}

dynamic_entity_list_node &cell_node(dynamic_proximity_map &map, int x, int y)
{
    auto *words = reinterpret_cast<uint32_t *>(&map);
    const auto *directory = reinterpret_cast<const uint16_t *>(words + map.field_8);
    const auto offset = static_cast<int16_t>(directory[x + (y << map.log_number_of_cells)]);
    return *reinterpret_cast<dynamic_entity_list_node *>(words + offset);
}
}


dynamic_proximity_map_stack *acquire_district_proximity_map_stack()
{
    auto &stacks = var<dynamic_proximity_map_stack *[8]>(0x0095C928);
    auto &used = var<uint32_t>(0x0095C948);
    auto &count = var<int>(0x00921E40);
    if (stacks[0] == nullptr) {
        count = 8;
        for (int i = 0; i < count; ++i)
            stacks[i] = new dynamic_proximity_map_stack;
    }
    for (int i = 0; i < count; ++i) {
        if ((used & (1u << i)) == 0) {
            used |= 1u << i;
            return stacks[i];
        }
    }
    return nullptr;
}

void release_district_proximity_map_stack(dynamic_proximity_map_stack *stack)
{
    stack->storage.reset();
    const auto &stacks = var<dynamic_proximity_map_stack *[8]>(0x0095C928);
    auto &used = var<uint32_t>(0x0095C948);
    for (int i = 0; i < var<int>(0x00921E40); ++i) {
        if (stacks[i] == stack) {
            used &= ~(1u << i);
            return;
        }
    }
}



void dynamic_proximity_map::init(dynamic_proximity_map_stack *allocator, fixed_pool *pool,
    int cells, const vector3d &min, const vector3d &max, subdivision_node::type_t node_type)
{
    field_0 = 1;
    map_type = DYNAMIC;
    initialized = true;
    number_of_cells = cells;
    log_number_of_cells = 0;
    for (unsigned n = cells; n > 1; n >>= 1)
        ++log_number_of_cells;
    field_C = min;
    field_18 = max;
    field_24 = max - min;
    field_3C = vector3d{1.0f / field_24.x, 1.0f / field_24.y, 1.0f / field_24.z};
    field_30 = field_24 * (1.0f / float(cells));
    field_48 = field_24 - field_30 * 0.5f;
    auto *directory = static_cast<uint16_t *>(allocator->alloc(2 * cells * cells));
    const auto directory_offset = reinterpret_cast<char *>(directory) - reinterpret_cast<char *>(this);
    field_8 = static_cast<uint16_t>(directory_offset / 4);
    auto *nodes = static_cast<dynamic_entity_list_node *>(allocator->alloc(sizeof(dynamic_entity_list_node) * cells * cells));
    for (int i = 0; i < cells * cells; ++i) {
        const auto offset = reinterpret_cast<char *>(&nodes[i]) - reinterpret_cast<char *>(this);
        directory[i] = static_cast<uint16_t>(offset / 4);
        nodes[i].set_type(node_type);
        nodes[i].head = nullptr;
    }
    entry_pool = pool;
    if (entry_pool == nullptr) {
        entry_pool = new (allocator->alloc(sizeof(fixed_pool))) fixed_pool{8, 128, 8, 1, 0, allocator};
    }
}

void hierarchical_entity_proximity_map::init(dynamic_proximity_map_stack &allocator,
    int sphere_kind, const vector3d &min, const vector3d &max, const _std::vector<int> &levels)
{
    static Var<fixed_pool> list_pool{0x0092221C};
    if (!list_pool().m_initialized)
        list_pool().init(8, 1280, 4, 1, 0, nullptr);
    if (!hash_entry_pool().m_initialized)
        hash_entry_pool().init(16, 640, 4, 1, 0, nullptr);
    number_of_levels = levels.size();
    assert(number_of_levels <= 5);
    for (int i = 0; i < number_of_levels; ++i) {
        auto *map = new (allocator.alloc(sizeof(dynamic_proximity_map))) dynamic_proximity_map{};
        maps[i] = map;
        map->init(&allocator, &list_pool(), levels[i], min, max, subdivision_node::DYNAMIC_ENTITY_LIST_NODE);
        map->entity_sphere_kind = sphere_kind;
        map->entity_count = map->cell_entry_count = 0;
        map->hierarchy_level = i;
    }
}


void dynamic_proximity_map::compute_entity_center_and_radius(entity &ent, vector3d &center, float &radius)
{
    if (entity_sphere_kind == 3) {
        center = ent.get_abs_position();
        radius = static_cast<light_source &>(ent).properties->cutoff_range;
        return;
    }
    const int center_slot = entity_sphere_kind == 1 ? 0x258 : 0x2C;
    auto center_callback = reinterpret_cast<vector3d *(__fastcall *)(entity *, void *, vector3d *)>(
        get_vfunc(ent.m_vtbl, center_slot));
    center_callback(&ent, nullptr, &center);
    if (entity_sphere_kind == 4) {
        radius = 0.0f;
        return;
    }
    auto radius_callback = reinterpret_cast<float(__fastcall *)(entity *, void *)>(
        get_vfunc(ent.m_vtbl, entity_sphere_kind == 1 ? 0x254 : 0x28));
    radius = radius_callback(&ent, nullptr);
    if (entity_sphere_kind == 2)
        radius = std::max(radius, 0.5f);
}

int hierarchical_entity_proximity_map::traverse_point(const vector3d &position, subdivision_visitor &visitor)
{
    for (int i = 0; i < number_of_levels; ++i) {
        const int result = maps[i]->traverse_point(position, visitor);
        if (result != 0)
            return result;
    }
    return 0;
}

int hierarchical_entity_proximity_map::traverse_sphere(const vector3d &center, Float radius, subdivision_visitor *visitor)
{
    for (int i = 0; i < number_of_levels; ++i) {
        const int result = maps[i]->traverse_sphere(center, radius, visitor);
        if (result != 0)
            return result;
    }
    return 0;
}


int hierarchical_entity_proximity_map::traverse_convex_hull_raster(
    const fixed_vector<vector2d, 14> &points, subdivision_visitor &visitor)
{
    for (int i = 0; i < number_of_levels; ++i)
        maps[i]->traverse_convex_hull_raster(points, visitor);
    return number_of_levels;
}

bool hierarchical_entity_proximity_map::sub_55E9B0(entity **ent, entity_proximity_map_data **removed)
{
    auto **link = &entity_data_lookup.entries[entity_bucket(*ent)];
    while (*link != nullptr && (*link)->ent != *ent)
        link = &(*link)->next;
    if (*link == nullptr)
        return false;
    *removed = *link;
    *link = (*link)->next;
    return true;
}


void hierarchical_entity_proximity_map::remove_entity(entity *ent, entity_proximity_map_data *data)
{
    auto &map = *maps[data->map_level];
    for (int y = data->min_y; y <= data->max_y; ++y) {
        for (int x = data->min_x; x <= data->max_x; ++x) {
            auto **link = &cell_node(map, x, y).head;
            while (*link != nullptr && (*link)->ent != ent)
                link = &(*link)->next;
            if (*link != nullptr) {
                auto *entry = *link;
                *link = entry->next;
                map.entry_pool->remove(entry);
            }
            --map.cell_entry_count;
        }
    }
    --map.entity_count;
}

bool hierarchical_entity_proximity_map::remove_entity(entity *ent)
{
    auto *data = entity_data_lookup.entries[entity_bucket(ent)];
    while (data != nullptr && data->ent != ent)
        data = data->next;
    if (data == nullptr)
        return false;
    remove_entity(ent, data);
    entity_proximity_map_data *removed = nullptr;
    const bool found = sub_55E9B0(&ent, &removed);
    assert(found && removed == data);
    hash_entry_pool().remove(removed);
    return true;
}


void hierarchical_entity_proximity_map::update_entity(entity *ent)
{
    auto *data = entity_data_lookup.entries[entity_bucket(ent)];
    while (data != nullptr && data->ent != ent)
        data = data->next;
    vector3d center;
    float radius;
    maps[0]->compute_entity_center_and_radius(*ent, center, radius);
    const vector3d min = center - vector3d{radius, radius, radius};
    const vector3d max = center + vector3d{radius, radius, radius};
    cell_index first, last;
    int level = number_of_levels - 1;
    for (; level >= 0; --level) {
        maps[level]->map_vector3d_to_cell_index(min, &first);
        maps[level]->map_vector3d_to_cell_index(max, &last);
        if ((last.x - first.x + 1) * (last.y - first.y + 1) <= 4 || level == 0)
            break;
    }
    if (level < 0)
        return;
    if (data != nullptr) {
        if (data->map_level == level && data->min_x == first.x && data->min_y == first.y
            && data->max_x == last.x && data->max_y == last.y)
            return;
        remove_entity(ent, data);
    } else {
        data = static_cast<entity_proximity_map_data *>(pool_entry(hash_entry_pool()));
        auto &bucket = entity_data_lookup.entries[entity_bucket(ent)];
        data->next = bucket;
        bucket = data;
    }
    data->ent = ent;
    data->map_level = level;
    data->min_x = static_cast<uint8_t>(first.x);
    data->min_y = static_cast<uint8_t>(first.y);
    data->max_x = static_cast<uint8_t>(last.x);
    data->max_y = static_cast<uint8_t>(last.y);
    auto &map = *maps[level];
    ++map.entity_count;
    for (int y = first.y; y <= last.y; ++y) {
        for (int x = first.x; x <= last.x; ++x) {
            ++map.cell_entry_count;
            auto &node = cell_node(map, x, y);
            auto *entry = static_cast<dynamic_entity_list_entry *>(pool_entry(*map.entry_pool));
            entry->ent = ent;
            entry->next = node.head;
            node.head = entry;
        }
    }
}

void hierarchical_entity_proximity_map_patch()
{
    bool (hierarchical_entity_proximity_map::*func)(entity *) = &hierarchical_entity_proximity_map::remove_entity;
    FUNC_ADDRESS(address, func);
    REDIRECT(0x004CB8A4, address);
}
