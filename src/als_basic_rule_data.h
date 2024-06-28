#pragma once

#include "func_wrapper.h"
#include "mash.h"
#include "mvector.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace als
{
    struct als_data;
    struct filter_data;
    struct dest_weight_data;
    struct post_kill_rule;
    struct post_layer_alter;
    struct request_data;

    struct basic_rule_data {

        enum {
            TRANSITION = 0,
            TRANSITION_CATEGORY = 1,
        };

        struct rule_action {
            int the_action;
            mVector<dest_weight_data> *destination_states;
            string_hash field_8;

            rule_action(from_mash_in_place_constructor *a2);

            void initialize(mash::allocation_scope a2);

            void unmash(mash_info_struct *, void *);

            string_hash get_dest() const;

            void process_action(request_data &a2) const;
        };

        struct post_action_rule_set
        {
            mVector<als::post_kill_rule> field_0;
            mVector<als::post_layer_alter> field_14;

            post_action_rule_set(from_mash_in_place_constructor *a2);

            void unmash(mash_info_struct *, void *);
        };

        mVector<filter_data> field_0;
        rule_action field_14;
        post_action_rule_set *field_20;

        basic_rule_data(from_mash_in_place_constructor *a2);

        void unmash(mash_info_struct *a1, void *);

        bool can_transition(als_data &a2) const;

        void do_post_action(als_data &a2);

        bool has_post_action() const;
    };
}

extern void als_basic_rule_data_patch();
