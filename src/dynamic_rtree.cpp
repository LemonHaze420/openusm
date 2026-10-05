#include "dynamic_rtree.h"

#include "common.h"
#include "fixed_pool.h"
#include "float.hpp"
#include "func_wrapper.h"
#include "memory.h"
#include "subdivision_visitor.h"
#include "rtree.h"
#include "actor.h"
#include "region.h"
#include "subdivision_obb.h"
#include "terrain.h"
#include "wds.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"

#include <algorithm>
#include <limits>
#include <new>

VALIDATE_SIZE(dynamic_rtree_root_state, 0x5F0u);
VALIDATE_OFFSET(dynamic_rtree_root_state, field_110, 0x110);
VALIDATE_OFFSET(dynamic_rtree_root_state, half_extents, 0x540);
VALIDATE_OFFSET(dynamic_rtree_root_state, level_capacities, 0x550);
VALIDATE_OFFSET(dynamic_rtree_root_state, level_counts, 0x57C);
VALIDATE_OFFSET(dynamic_rtree_root_state, levels, 0x5A8);
VALIDATE_OFFSET(dynamic_rtree_root_state, field_5D4, 0x5D4);

VALIDATE_SIZE(rtree_hash_entry, 0xC);

Var<dynamic_rtree_root_t> collision_dynamic_rtree{0x0095D418};

void dynamic_rtree_root_t::traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor &a4)
{
    THISCALL(0x00521500, this, &a2, a3, &a4);
}

void dynamic_rtree_root_t::init(int root_count, int depth)
{
    state = new (arch_memalign(16, sizeof(dynamic_rtree_root_state))) dynamic_rtree_root_state{};
    state->init(root_count, depth);
}

void dynamic_rtree_root_state::init(int root_count, int depth)
{
    root_capacity = root_count;
    level_count = depth;
    bottom_level_index = depth - 1;
    BOTTOM_LEVEL_SIZE = root_count;
    for (int level = 1; level < depth; ++level)
        BOTTOM_LEVEL_SIZE *= 4;
    field_5D4 = nullptr;
    bottom_level = nullptr;
    level_capacities.m_size = 0;
    level_counts.m_size = 0;
    levels.m_size = 0;
    for (auto &bucket : field_140.field_0) {
        while (bucket) {
            auto *next = bucket->field_8;
            delete bucket;
            bucket = next;
        }
    }
    occupied.clear();

    vector3d minimum{std::numeric_limits<float>::max()};
    vector3d maximum{-std::numeric_limits<float>::max()};
    auto *terrain = g_world_ptr->the_terrain;
    for (int i = 0; i < terrain->total_regions; ++i) {
        vector3d lo, hi;
        terrain->regions[i]->obb->get_extents(&lo, &hi);
        for (int axis = 0; axis != 3; ++axis) {
            minimum[axis] = lo[axis] < minimum[axis] ? lo[axis] : minimum[axis];
            maximum[axis] = hi[axis] > maximum[axis] ? hi[axis] : maximum[axis];
        }
    }

    int capacity = root_count;
    int total_nodes = 0;
    for (int level = 0; level < depth; ++level) {
        level_capacities.push_back(capacity);
        level_counts.push_back(0);
        total_nodes += capacity;
        capacity *= 4;
    }
    auto *nodes = static_cast<rtree_node_t *>(arch_memalign(64, total_nodes * sizeof(rtree_node_t)));
    field_5D4 = nodes;
    int offset = 0;
    for (int level = 0; level < depth; ++level) {
        levels.push_back(nodes + offset);
        for (int index = 0; index < level_capacities.m_data[level]; ++index)
            nodes[offset + index].clear();
        offset += level_capacities.m_data[level];
    }
    bottom_level = levels.m_data[bottom_level_index];
    for (int level = 0; level < bottom_level_index; ++level) {
        for (int index = 0; index < level_capacities.m_data[level]; ++index) {
            const auto child_offset = levels.m_data[level + 1] + 4 * index - nodes;
            levels.m_data[level][index].field_C.field_0 = child_offset * sizeof(rtree_node_t);
        }
    }
    for (int axis = 0; axis != 3; ++axis) {
        half_extents[axis] = (maximum[axis] - minimum[axis]) * 0.5f + 20.0f;
        field_110.field_0[axis] = (maximum[axis] + minimum[axis]) * 0.5f;
        field_110.field_10[axis] = 32767.0f / half_extents[axis];
    }
    half_extents.w = 1.0f;
    field_110.field_0.w = 1.0f;
    field_110.field_10.w = 1.0f;
    field_110.field_20 = nodes;
    field_110.field_24 = 0;
    field_110.field_28 = 0;
    field_110.field_2C = depth;
}

void dynamic_rtree_root_t::term()
{
    mem_freealign(this->state->field_5D4);
    this->state->field_5D4 = nullptr;
    state->bottom_level = nullptr;

    mem_freealign(this->state);
    this->state = nullptr;
}

void dynamic_rtree_root_t::sort()
{
    if (this->state != nullptr) {
        this->state->sort();
    }
}

rtree_hash_entry::rtree_hash_entry(entity_base_vhandle a2, rtree_node_t *a3) : field_0(a2), entity_aabb(a3) {}

rtree_hash_entry *rtree_hash_table::get(const entity_base_vhandle &key)
{
    rtree_hash_entry *i = nullptr;
    for (i = this->field_0[(key.field_0 >> 8) & 0xFF]; i != nullptr && i->field_0 != key; i = i->field_8)
        ;

    return i;
}

bool rtree_hash_table::insert(const entity_base_vhandle &key, rtree_hash_entry *a3)
{
    assert(get(key) == nullptr && "Entity is already in this proximity map - cannot add twice.");

    auto v3 = (key.field_0 >> 8) & 0xFF;
    a3->field_8 = this->field_0[v3];
    this->field_0[v3] = a3;
    return true;
}

bool rtree_hash_table::find(const entity_base_vhandle &key, rtree_hash_entry **a3)
{
    auto *v3 = &this->field_0[(key.field_0 >> 8) & 0xFF];
    for (auto *i = *v3; i; i = i->field_8) {
        if (i->field_0 == key) {
            break;
        }

        v3 = &i->field_8;
    }

    if (*v3 == nullptr) {
        return false;
    }

    *a3 = *v3;
    *v3 = (*v3)->field_8;
    return true;
}

static Var<fixed_pool> rtree_hash_entry_pool{0x00922130};

static fixed_pool &collision_entry_pool()
{
    auto &pool = rtree_hash_entry_pool();
    if (!pool.m_initialized)
        pool.init(sizeof(rtree_hash_entry), 128, 4, 1, 0, nullptr);
    return pool;
}


static void expand_bounds(rtree_node_t &node, const rtree_node_t &other)
{
    node.minx = std::max(node.minx, other.minx);
    node.maxx = std::max(node.maxx, other.maxx);
    node.miny = std::max(node.miny, other.miny);
    node.maxy = std::max(node.maxy, other.maxy);
    node.minz = std::max(node.minz, other.minz);
    node.maxz = std::max(node.maxz, other.maxz);
}

bool dynamic_rtree_root_t::remove_entity(entity *a2)
{
    auto *s = this->state;

    auto key = entity_base_vhandle{uint32_t(a2)};
    auto *data = s->field_140.get(key);
    if (data == nullptr) {
        return false;
    }

    rtree_hash_entry *result;
    auto removed = s->field_140.find(key, &result);

    assert(result == data);
    assert(removed);

    auto *v5 = s->bottom_level;
    auto *v6 = result;
    auto slot_index = result->entity_aabb - v5;
    assert(&s->bottom_level[slot_index] == result->entity_aabb);
    assert(slot_index >= 0 && slot_index < s->BOTTOM_LEVEL_SIZE);

    auto *v8 = &v5[slot_index];
    v8->clear();

    s->occupied.field_4[slot_index >> 5] &= ~(1u << (slot_index & 0x1F));
    collision_entry_pool().remove(v6);
    return true;
}

void dynamic_rtree_root_t::update_entity(entity *ent)
{
    const auto center = ent->get_colgeom_center();
    const auto radius = vector3d{ent->get_colgeom_radius()};
    const auto key = entity_base_vhandle{uint32_t(ent)};
    const rtree_construction_node_t bounds{key, center - radius, center + radius};
    auto &s = *state;
    if (auto *entry = s.field_140.get(key)) {
        entry->entity_aabb->init(bounds, s.field_110.field_0, s.field_110.field_10);
    } else {
        const int index = s.find_free_slot();
        assert(index >= 0 && index < s.BOTTOM_LEVEL_SIZE);
        auto *node = &s.bottom_level[index];
        node->init(bounds, s.field_110.field_0, s.field_110.field_10);
        s.occupy(index);
        auto *storage = allocate_new_block<rtree_hash_entry>(collision_entry_pool());
        auto *new_entry = new (storage) rtree_hash_entry{key, node};
        s.field_140.insert(key, new_entry);
        s.extend_ancestors(index);
    }
}

void dynamic_rtree_root_state::extend_ancestors(int index)
{
    auto &leaf_count = level_counts.m_data[bottom_level_index];
    leaf_count = std::max(leaf_count, index + 1);
    const auto &leaf = bottom_level[index];
    for (int level = bottom_level_index - 1; level >= 0; --level) {
        index >>= 2;
        expand_bounds(levels.m_data[level][index], leaf);
        level_counts.m_data[level] = std::max(level_counts.m_data[level], index + 1);
    }
    field_110.field_28 = level_counts.m_data[0];
}

int dynamic_rtree_root_state::find_free_slot() const
{
    const int words = (BOTTOM_LEVEL_SIZE + 31) / 32;
    int word = 0;
    while (word < words && occupied.field_4[word] == 0xFFFFFFFFu)
        ++word;
    if (word == words)
        return BOTTOM_LEVEL_SIZE;
    const auto available = ~occupied.field_4[word];
    const auto lowest = available & (0u - available);
    int bit = 31;
    if ((lowest & 0xFFFFu) != 0)
        bit = 15;
    if ((lowest & 0x00FF00FFu) != 0)
        bit -= 8;
    if ((lowest & 0x0F0F0F0Fu) != 0)
        bit -= 4;
    if ((lowest & 0x33333333u) != 0)
        bit -= 2;
    if ((lowest & 0x55555555u) != 0)
        --bit;
    return std::min(word * 32 + bit, BOTTOM_LEVEL_SIZE);
}

void dynamic_rtree_root_state::occupy(int index)
{
    assert((bottom_level + index)->is_valid());

    this->occupied.set(index);
}

void dynamic_rtree_root_state::compact_leaves()
{
    int count = level_counts.m_data[bottom_level_index];
    int hole = find_free_slot();
    while (hole < count) {
        --count;
        while (count > hole && !bottom_level[count].is_valid())
            --count;
        if (count > hole) {
            bottom_level[hole] = bottom_level[count];
            bottom_level[count].clear();
            occupied.field_4[count >> 5] &= ~(1u << (count & 31));
            auto *entry = field_140.get(bottom_level[hole].field_C);
            assert(entry != nullptr);
            entry->entity_aabb = &bottom_level[hole];
            occupy(hole);
        }
        hole = find_free_slot();
    }
    level_counts.m_data[bottom_level_index] = count;
}

void dynamic_rtree_root_state::rebuild_bounds()
{
    for (int level = bottom_level_index; level > 0; --level) {
        const int count = level_counts.m_data[level];
        auto *children = levels.m_data[level];
        auto *parents = levels.m_data[level - 1];
        const int complete = count / 4;
        for (int parent = 0; parent < complete; ++parent) {
            const auto offset = parents[parent].field_C;
            parents[parent] = children[4 * parent];
            for (int child = 1; child < 4; ++child)
                expand_bounds(parents[parent], children[4 * parent + child]);
            parents[parent].field_C = offset;
        }
        if ((count & 3) != 0) {
            const auto offset = parents[complete].field_C;
            parents[complete] = children[4 * complete];

            for (int child = 1; child < count; ++child)
                expand_bounds(parents[complete], children[child]);
            parents[complete].field_C = offset;
        }
    }
}

namespace {
struct collision_sort_index {
    uint16_t index;
    uint16_t x;
    uint16_t z;
    uint16_t padding;
};

void sort_collision_partition(collision_sort_index *indices, int begin, int end, int split, bool x_axis, int rounds)
{
    const auto less = [x_axis](const collision_sort_index &a, const collision_sort_index &b) {
        return x_axis ? a.x < b.x : a.z < b.z;
    };
    std::sort(indices + begin, indices + split, less);
    std::sort(indices + split, indices + end, less);
    if (rounds > 1) {
        if (split - begin > 4)
            sort_collision_partition(indices, begin, split, begin + (split - begin) / 2, !x_axis, rounds - 1);
        if (end - split > 4)
            sort_collision_partition(indices, split, end, split + (end - split) / 2, !x_axis, rounds - 1);
    }
}
}  // namespace

void dynamic_rtree_root_state::sort()
{
    compact_leaves();
    const int count = level_counts.m_data[bottom_level_index];
    if (count == 0)
        return;
    stack_allocator saved;
    scratchpad_stack::save_state(&saved);
    auto *indices = static_cast<collision_sort_index *>(scratchpad_stack::alloc(count * sizeof(collision_sort_index)));
    for (int i = 0; i < count; ++i) {
        const auto &node = bottom_level[i];
        indices[i] = {static_cast<uint16_t>(i),
                      static_cast<uint16_t>((node.maxx - node.minx + 65535) >> 1),
                      static_cast<uint16_t>((node.maxz - node.minz + 65535) >> 1),
                      0};
    }
    std::sort(indices, indices + count, [](const collision_sort_index &a, const collision_sort_index &b) {
        return a.z < b.z;
    });
    if (count > 4) {
        int split = 4;
        while (split < count)
            split *= 4;
        if (split >= count)
            split = (count / 2 + 3) & ~3;
        if (split > count)
            split = count - 1;

        int rounds = 1;
        for (int first_size = split; first_size > 4; first_size /= 2)
            ++rounds;
        sort_collision_partition(indices, 0, count, split, true, rounds);
    }

    for (auto *bucket : field_140.field_0) {
        for (auto *entry = bucket; entry; entry = entry->field_8) {
            const int old_slot = entry->entity_aabb - bottom_level;
            entry->entity_aabb = bottom_level + indices[old_slot].index;
        }
    }
    for (int i = 0; i < count; ++i) {
        if (indices[i].index == 0xFFFF)
            continue;
        auto node = bottom_level[i];
        int current = i;
        do {
            const int destination = indices[current].index;
            indices[current].index = 0xFFFF;
            std::swap(node, bottom_level[destination]);
            current = destination;
        } while (indices[current].index != 0xFFFF);
    }
    scratchpad_stack::restore_state(saved);
    rebuild_bounds();
}
