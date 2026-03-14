#pragma once

#include "als_category.h"
#include "string_hash.h"
#include "mvector.h"
#include "force_transitions.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace als
{
    struct alter_conditions;
    struct implicit_transition_rule;
    struct explicit_transition_rule;
    struct layer_transition_rule;
    struct incoming_transition_rule;

    struct scripted_category : category
    {
        string_hash field_10;
        force_transitions field_14;
        mVectorBasic<int> field_2C;
        mVector<als::implicit_transition_rule> field_3C;
        mVector<als::explicit_transition_rule> field_50;
        mVector<als::incoming_transition_rule> field_64;
        mVector<als::layer_transition_rule> *field_78;

        scripted_category();

        scripted_category(from_mash_in_place_constructor *a2);

        //0x004AC850
        void _unmash(mash_info_struct *a1, void *);

        //virtual
        int _get_virtual_type_enum() const;

        //virtual
        request_data do_implicit_trans(
            animation_logic_system *a4,
            state_machine *a5);

        //virtual
        int _get_mash_sizeof() const;
    };
}

extern void als_scripted_category_patch();
