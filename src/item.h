#pragma once

#include "actor.h"
#include "float.hpp"
#include "mstring.h"

struct item : actor {
    using base_type = vhandle_type<actor>;

    item *field_C0;
    item *field_C4;
    bool field_C8;
    bool field_C9;
    bool field_CA;
    bool field_CB;
    bool field_CC;
    int field_D0;
    mString field_D4;
    int field_E4;
    int field_E8;
    float field_EC;
    int field_F0;
    float field_F4;
    int field_F8;
    int field_FC;

    //0x004FAA30
    item(const string_hash &a2, uint32_t a3);

    ~item();

    //0x004CDA40
    bool is_same_item(const item &a2);

    //0x004C3B30
    void remove_from_list();

    //0x004F7590
    //virtual
    bool give_to_entity(actor *a2);

    //0x004FAB10
    //virtual
    void release_mem();

    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    void change_list_status();
    void spawn_item_script();
    void set_quantity(int);
    void apply_effects(actor *);
    void frame_advance(Float);
    static void install_weapon_callbacks(void **table);

    //0x004CD9D0
    static void frame_advance_all_items(Float a3);

    static Var<item *> inactive_items;

    static Var<item *> active_items;
};

template <>
item *vhandle_type<item>::get_volatile_ptr() const;
