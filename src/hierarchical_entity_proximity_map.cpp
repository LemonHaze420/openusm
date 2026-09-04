#include "hierarchical_entity_proximity_map.h"

#include "common.h"
#include "entity_proximity_map_data.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "proximity_map.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(hierarchical_entity_proximity_map, 0x418u);

static Var<fixed_pool> hash_entry_pool{0x00922240};

static uint32_t BYTEn(int x, int n)
{
    return (*(bit_cast<BYTE *>(&x) + n));
}

static uint32_t BYTE1(int x)
{
    return BYTEn(x, 1);
}

int hierarchical_entity_proximity_map::traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4)
{
    TRACE("hierarchical_entity_proximity_map::traverse_sphere");

    for (auto i = 0; i < this->number_of_levels; ++i) {
        auto result = this->maps[i]->traverse_sphere(a2, a3, a4);
        if (result != 0) {
            return result;
        }
    }

    return 0;
}

bool hierarchical_entity_proximity_map::sub_55E9B0(entity **a2, entity_proximity_map_data **a3)
{
    if constexpr (1) {
        uint32_t idx = (*((uint8_t *)a2 + 1));
        auto **v3 = (entity_proximity_map_data **)&this->entity_data_lookup.entries[idx];
        for (auto *i = *v3; i != nullptr; i = i->field_8) {
            if (i->field_4 == *a2) {
                break;
            }

            v3 = &i->field_8;
        }

        if (!*v3) {
            return false;
        }

        *a3 = *v3;
        *v3 = (*v3)->field_8;
        return true;
    } else {
        return (bool)THISCALL(0x0055E9B0, this, a2, a3);
    }
}

void hierarchical_entity_proximity_map::remove_entity(entity *a2, entity_proximity_map_data *data)
{
    THISCALL(0x00511A90, this, a2, data);
}

bool hierarchical_entity_proximity_map::remove_entity(entity *ent)
{
    TRACE("hierarchical_entity_proximity_map::remove_entity");

    if constexpr (1) {
        auto *data = [](auto *a1, entity *ent) -> entity_proximity_map_data * {
            auto idx = BYTE1(int(ent));
            auto *data = static_cast<entity_proximity_map_data *>(a1->entries[idx]);
            for (; data != nullptr && data->field_4 != ent; data = data->field_8) {
                ;
            }

            return data;
        }(&this->entity_data_lookup, ent);

        if (data == nullptr) {
            return false;
        }

        this->remove_entity(ent, data);

        entity_proximity_map_data *result = nullptr;
        auto removed = this->sub_55E9B0(&ent, &result);
        assert(result == data);

        assert(removed);

        [](fixed_pool &pool, entity_proximity_map_data *a1) {
            assert(pool.get_entry_size() == sizeof(entity_proximity_map_data));

            pool.remove(a1);
        }(hash_entry_pool(), result);

        return true;
    } else {
        return (bool)THISCALL(0x00522CA0, this, ent);
    }
}

void hierarchical_entity_proximity_map::update_entity(entity *ent)
{
    TRACE("hierarchical_entity_proximity_map::update_entity");

    if constexpr (0) {
    } else {
        THISCALL(0x00511B90, this, ent);
    }
}

void hierarchical_entity_proximity_map_patch()
{
    bool (hierarchical_entity_proximity_map::*func)(entity *a2) = &hierarchical_entity_proximity_map::remove_entity;
    FUNC_ADDRESS(address, func);
    REDIRECT(0x004CB8A4, address);
}
