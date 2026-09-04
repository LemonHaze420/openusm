#include "limbo_entities.h"

#include "bitvector.h"
#include "func_wrapper.h"

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
    if constexpr (1) {
        assert(!limbo_entities().get(vent));

        auto *v2 = limbo_hash_entry::pool().allocate_new_block();
        ;
        auto *v1 = new (v2) limbo_hash_entry{vent};

        limbo_entities().add(vent, v1);
        if (update_started()) {
            auto v3 = static_cast<uint8_t>(vent.field_0.field_0);
            if (v3 < last_update_slot()) {
                hash_update_bitvector().set(v3);
            }
        }

    } else {
        CDECL_CALL(0x0052DF60, vent);
    }
}

void update_limbo_list()
{
    CDECL_CALL(0x005346E0);
}

void update_limbo_list_if_needed()
{
    if (update_started()) {
        update_limbo_list();
    }
}

void remove_from_limbo_list(vhandle_type<entity> vent)
{
    if constexpr (0) {
    } else {
        CDECL_CALL(0x00523CD0, vent);
    }
}
