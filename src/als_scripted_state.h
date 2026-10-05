#pragma once

#include "als_state.h"
#include "mvector.h"

struct mash_info_struct;

namespace als {
struct dest_weight_data {
    string_hash field_0;
    int field_4;
};

struct implicit_transition_rule;

struct explicit_transition_rule;

struct layer_transition_rule;

struct request_data;
struct animation_logic_system;
struct state_machine;

struct scripted_state : state {
    string_hash field_14;
    mVectorBasic<int> field_18;
    mVector<als::implicit_transition_rule> field_28;
    mVector<als::explicit_transition_rule> field_3C;
    mVector<als::layer_transition_rule> *field_50;

    scripted_state();

    scripted_state(from_mash_in_place_constructor *a2);

    //virtual
    void _unmash(mash_info_struct *a1, void *a3);


    void _destruct_mashed_class();
    void *_scalar_deleting_destructor(unsigned int flags);

    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    int get_filter(int out, animation_logic_system *a2, state_machine *a3, int a4);

    //virtual
    int _get_mocomp_type();

    //virtual
    request_data _do_implicit_trans(animation_logic_system *a4, state_machine *a5);

    //0x004A7040
    //virtual
    request_data _do_explicit_trans(animation_logic_system *a4, state_machine *a5, string_hash a6);

    //0x004A7180
    //virtual
    request_data _do_layer_trans(animation_logic_system *a4, state_machine *a5);

    //004A72B0
    //virtual
    void _do_post_trans(animation_logic_system *a1, state_machine *a2, transition_post_handle a4);

    //virtual
    int _get_mash_sizeof() const;

    string_hash get_nal_anim_name() const;
};

struct base_layer_scripted_state : scripted_state {
    int field_54;

    base_layer_scripted_state();

    base_layer_scripted_state(from_mash_in_place_constructor *a2);

    //virtual
    void _unmash(mash_info_struct *a1, void *a3);

    //virtual
    int _get_virtual_type_enum() const;

    //virtual
    int _get_mash_sizeof() const;
};
}  // namespace als

extern void als_scripted_state_patch();
