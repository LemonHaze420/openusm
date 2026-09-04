#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "fixed_pool.h"
#include "variable.h"

extern void add_to_limbo_list(vhandle_type<entity> vent);

extern void update_limbo_list();

extern void update_limbo_list_if_needed();

extern void remove_from_limbo_list(vhandle_type<entity> vent);

inline Var<bool> update_started{0x0095C8C0};

inline Var<int> last_update_slot{0x0095C8C4};

struct limbo_hash_entry {
    vhandle_type<entity> field_0;
    limbo_hash_entry *field_4;

    limbo_hash_entry(vhandle_type<entity> vent)
    {
        this->field_0 = vent;
    }

    static inline Var<fixed_pool> pool{0x00922288};
};


struct limbo_hash_table_t {
    limbo_hash_entry *field_0[256];

    limbo_hash_entry *get(const vhandle_type<entity> &a2);

    bool add(const vhandle_type<entity> &key, limbo_hash_entry *a3);
};

inline Var<limbo_hash_table_t> limbo_entities{0x00960508};
