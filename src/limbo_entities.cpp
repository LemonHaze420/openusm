#include "limbo_entities.h"

#include "moved_entities.h"
#include "func_wrapper.h"

#include <algorithm>

#if STANDALONE_SYSTEM
namespace {
void update_limbo_bucket(int slot)
{
    auto **link = &limbo_entities().field_0[slot];
    while (*link != nullptr) {
        auto *entry = *link;
        if (entry->field_0.get_volatile_ptr() == nullptr) {
            *link = entry->field_4;
            limbo_hash_entry::pool().remove(entry);
        } else {
            link = &entry->field_4;
        }
    }
    for (auto *entry = limbo_entities().field_0[slot]; entry != nullptr; entry = entry->field_4)
        moved_entities::add_moved(entry->field_0);
}
}  // namespace
#endif

limbo_hash_entry *limbo_hash_table_t::get(const vhandle_type<entity> &a2)
{
    limbo_hash_entry *i = nullptr;

    auto sub_6727DC = [](const vhandle_type<entity> &a1) -> int {
        return static_cast<uint8_t>(a1.field_0.field_0);
    };

    auto sub_695E9E = [](limbo_hash_entry *self, const entity_base_vhandle &a2) -> bool {
        return a2 == self->field_0.field_0;
    };

    for (i = this->field_0[sub_6727DC(a2)]; i != nullptr && !sub_695E9E(i, a2.field_0); i = i->field_4) {
        ;
    }

    return i;
}

bool limbo_hash_table_t::add(const vhandle_type<entity> &key, limbo_hash_entry *a3)
{
    assert(!this->get(key) && "Entity is already in this proximity map - cannot add twice.");

    auto v3 = static_cast<uint8_t>(key.field_0.field_0);
    a3->field_4 = this->field_0[v3];
    this->field_0[v3] = a3;
    return true;
}

void add_to_limbo_list(vhandle_type<entity> vent)
{
    if constexpr (STANDALONE_SYSTEM) {
        assert(!limbo_entities().get(vent));

#if STANDALONE_SYSTEM
        auto &pool = limbo_hash_entry::pool();
        if (!pool.m_initialized)
            pool.init(sizeof(limbo_hash_entry), 128, 4, 1, 0, nullptr);
#endif

        auto *v2 = limbo_hash_entry::pool().allocate_new_block();
        auto *v1 = new (v2) limbo_hash_entry{vent};

        limbo_entities().add(vent, v1);
        if (update_started()) {
            auto v3 = static_cast<uint8_t>(vent.field_0.field_0);
            if (v3 < last_update_slot()) {
                hash_update_bitvector()[v3 >> 5] |= 1u << (v3 & 31);
            }
        }

    } else {
        CDECL_CALL(0x0052DF60, vent);
    }
}

void update_limbo_list()
{
#if STANDALONE_SYSTEM
    const int end = std::min(last_update_slot() + 20, 256);
    for (int slot = last_update_slot(); slot < end; ++slot)
        update_limbo_bucket(slot);
    last_update_slot() = end;
    if (end == 256) {
        for (int slot = 0; slot < 256; ++slot) {
            if ((hash_update_bitvector()[slot >> 5] & (1u << (slot & 31))) != 0)
                update_limbo_bucket(slot);
        }
        last_update_slot() = 0;
        update_started() = false;
    }
#else
    CDECL_CALL(0x005346E0);
#endif
}

void update_limbo_list_if_needed()
{
    if (update_started()) {
        update_limbo_list();
    }
}

void remove_from_limbo_list(vhandle_type<entity> vent)
{
#if STANDALONE_SYSTEM
    auto **link = &limbo_entities().field_0[static_cast<uint8_t>(vent.field_0.field_0)];
    while (*link != nullptr && (*link)->field_0.field_0 != vent.field_0)
        link = &(*link)->field_4;
    if (*link != nullptr) {
        auto *entry = *link;
        *link = entry->field_4;
        limbo_hash_entry::pool().remove(entry);
    }
#else
    CDECL_CALL(0x00523CD0, vent);
#endif
}
