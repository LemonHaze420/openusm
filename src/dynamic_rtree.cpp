#include "dynamic_rtree.h"

#include "common.h"
#include "fixed_pool.h"
#include "float.hpp"
#include "func_wrapper.h"
#include "memory.h"
#include "subdivision_visitor.h"
#include "rtree.h"
#include "actor.h"

VALIDATE_SIZE(dynamic_rtree_root_state, 0x5F0u);
VALIDATE_OFFSET(dynamic_rtree_root_state, field_110, 0x110);

VALIDATE_SIZE(rtree_hash_entry, 0xC);

Var<dynamic_rtree_root_t> collision_dynamic_rtree{0x0095D418};

void dynamic_rtree_root_t::traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor &a4)
{
    THISCALL(0x00521500, this, &a2, a3, &a4);
}

void dynamic_rtree_root_t::init(int a2, int a3)
{
    THISCALL(0x005386B0, this, a2, a3);
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
    this->state->sort();
}

rtree_hash_entry::rtree_hash_entry(entity_base_vhandle a2, rtree_node_t *a3) : field_0(a2), entity_aabb(a3) {}

rtree_hash_entry *rtree_hash_table::get(const entity_base_vhandle &key)
{
    rtree_hash_entry *i = nullptr;
    for (i = this->field_0[bit_cast<uint8_t *>(key.field_0)[1]]; i != nullptr && i->field_0 != key; i = i->field_8)
        ;

    return i;
}

bool rtree_hash_table::insert(const entity_base_vhandle &key, rtree_hash_entry *a3)
{
    assert(get(key) == nullptr && "Entity is already in this proximity map - cannot add twice.");

    auto v3 = bit_cast<uint8_t *>(key.field_0)[1];
    a3->field_8 = this->field_0[v3];
    this->field_0[v3] = a3;
    return true;
}

bool rtree_hash_table::find(const entity_base_vhandle &key, rtree_hash_entry **a3)
{
    auto *v3 = &this->field_0[*((uint8_t *)key.field_0 + 1)];
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

bool dynamic_rtree_root_t::remove_entity(entity *a2)
{
    auto *s = this->state;

    auto key = entity_base_vhandle{(int)a2};
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

    s->occupied.field_4[slot_index >> 5] &= ~(1 << (slot_index & 0x1F));
    v6->field_0 = (entity_base_vhandle)rtree_hash_entry_pool().field_0;
    rtree_hash_entry_pool().field_0 = CAST(rtree_hash_entry_pool().field_0, v6);
    return true;
}

void dynamic_rtree_root_t::update_entity(entity *ent)
{
    if constexpr (0) {
        assert(ent->is_an_actor());

        assert(ent->possibly_collide());

        auto *act = bit_cast<actor *>(ent);
        auto center = act->get_colgeom_center();
        float radius = act->get_colgeom_radius();
        auto vec_radius = vector3d{radius};

        rtree_construction_node_t a2{(int)ent, center - vec_radius, center + vec_radius};

        auto &s = *this->state;
        auto key = entity_base_vhandle{(int)ent};
        auto *data = s.field_140.get(key);
        if (data != nullptr) {
            assert(data->entity_aabb != nullptr);
            data->entity_aabb->init(a2, s.field_110.field_0, s.field_110.field_10);
        } else {
            auto free_slot_index = s.find_free_slot();

            assert(s.bottom_level != nullptr);

            auto *v7 = &s.bottom_level[free_slot_index];
            assert(free_slot_index >= 0 && free_slot_index < s.BOTTOM_LEVEL_SIZE);

            v7->init(a2, s.field_110.field_0, s.field_110.field_10);

            s.occupy(free_slot_index);

            auto *mem = allocate_new_block<rtree_hash_entry>(rtree_hash_entry_pool());
            data = new (mem) rtree_hash_entry{key, v7};

            assert(data != nullptr);
            s.field_140.insert(key, data);
            s.sub_563270(free_slot_index);
        }
    } else {
        THISCALL(0x0052AFC0, this, ent);
    }
}

void dynamic_rtree_root_state::sub_563270(int a2)
{
    THISCALL(0x00563270, this, a2);
}

int dynamic_rtree_root_state::find_free_slot() const
{
    auto v1 = 0;
    for (auto *i = this->occupied.field_4; *i == 0xFFFFFFFF; ++i) {
        if (++v1 >= 65) {
            return 0;
        }
    }

    auto v4 = ~this->occupied.field_4[v1];
    auto v5 = ~(v4 - 1) & v4;
    auto v6 = 31;
    if ((uint16_t)v5)
        v6 = 15;

    if ((v5 & 0xFF00FF) != 0)
        v6 -= 8;

    if ((v5 & 0xF0F0F0F) != 0)
        v6 -= 4;

    if ((v5 & 0x33333333) != 0)
        v6 -= 2;

    if ((v5 & 0x55555555) != 0)
        --v6;

    auto index = v6 + 32 * v1;

    assert(!(bottom_level + index)->is_valid());

    return index;
}

void dynamic_rtree_root_state::occupy(int index)
{
    assert((bottom_level + index)->is_valid());

    this->occupied.set(index);
}

void dynamic_rtree_root_state::sort()
{
    THISCALL(0x005709C0, this);
}
