#pragma once

#include "als_category.h"
#include "string_hash.h"
#include "mvector.h"
#include "force_transitions.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace als {
struct alter_conditions;
struct implicit_transition_rule;
struct explicit_transition_rule;
struct layer_transition_rule;
struct incoming_transition_rule;

struct scripted_category : category {
    string_hash field_10;
    force_transitions field_14;
    mVectorBasic<int> field_2C;
    mVector<als::implicit_transition_rule> field_3C;
    mVector<als::explicit_transition_rule> field_50;
    mVector<als::incoming_transition_rule> field_64;
    mVector<als::layer_transition_rule> *field_78;

    scripted_category();

    scripted_category(from_mash_in_place_constructor *a2);

    static void *native_vtable();

    //0x004AC850
    void _unmash(mash_info_struct *a1, void *);

    //virtual


    void _destruct_mashed_class();
    void *_scalar_deleting_destructor(unsigned int flags);


    int _get_virtual_type_enum() const;

    //virtual
    request_data do_implicit_trans(animation_logic_system *a4, state_machine *a5);

    //0x004A7420
    //virtual
    request_data _do_explicit_trans(animation_logic_system *a4, state_machine *a5, string_hash a6);

    //0x004A7550
    //virtual
    request_data _do_layer_trans(animation_logic_system *a4, state_machine *a5);


    request_data _do_incoming_trans(animation_logic_system *a4, state_machine *a5);

    //0x004A7660
    //virtual
    void _do_post_trans(animation_logic_system *a1, state_machine *a2, transition_post_handle a3);

    //0x00493ED0
    //virtual
    string_hash _get_default_state() const;

    //virtual
    int _get_mash_sizeof() const;
};
}  // namespace als

extern void als_scripted_category_patch();
