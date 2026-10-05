#include "als_basic_rule_data.h"


#include "als_animation_logic_system.h"
#include "als_category.h"
#include "als_data.h"
#include "als_state.h"
#include "alter_conditions.h"
#include "als_dest_weight_data.h"
#include "als_filter_data.h"
#include "als_post_layer_alter.h"
#include "als_post_kill_rule.h"
#include "als_request_data.h"
#include "mash_info_struct.h"
#include "mash_config.h"
#include "common.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"
#include "xbpack.h"

namespace als {
VALIDATE_SIZE(basic_rule_data, 0x24);
VALIDATE_SIZE(basic_rule_data::post_action_rule_set, 0x28u);

basic_rule_data::basic_rule_data(from_mash_in_place_constructor *a2) : field_0(a2), field_14(a2)
{
    if (this->field_20 != nullptr) {
        mash_info_struct::construct_class(this->field_20);
    }
}

void filter_data::unmash(mash_info_struct *, void *)
{
#ifdef OPENUSM_XBPACK_V10
    this->field_0 = xbpack::pc_als_param(this->field_0);
#endif
}

void basic_rule_data::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_14, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if (this->field_20 != nullptr) {
        a1->unmash_class(this->field_20,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
        );
    }
}


bool basic_rule_data::can_transition(als_data &a2) const
{
    TRACE("als::basic_rule_data::can_transition");

    for (int i = 0; i < this->field_0.size(); ++i) {
        auto *filter = this->field_0.m_data[i];
        const double param = a2.field_4->get_param(a2.field_0, filter->field_0);
        filter = this->field_0.m_data[i];
        if (param < filter->field_4 || param > filter->field_8) {
            return false;
        }
    }
    return true;
}

void basic_rule_data::do_post_action(als_data &a2)
{
    TRACE("als::basic_rule_data::do_post_action");

    if (this->field_20 == nullptr) {
        return;
    }

    for (int i = 0; i < this->field_20->field_0.size(); ++i) {
        auto *rule = this->field_20->field_0.m_data[i];
        if (static_cast<uint8_t>(rule->field_0) != 0) {
            a2.field_0->kill_animation_domain(static_cast<uint32_t>(rule->field_4));
        } else {
            a2.field_0->sub_4A6630(static_cast<layer_types>(rule->field_4));
        }
    }

    for (int i = 0; i < this->field_20->field_14.size(); ++i) {
        auto *alter = this->field_20->field_14.m_data[i];
        bool matched = false;
        string_hash destination;
        for (int j = 0; j < alter->field_4.size(); ++j) {
            auto *condition = alter->field_4.m_data[i];
            auto *layer = a2.field_0->get_als_layer_internal(static_cast<layer_types>(alter->field_0));
            const string_hash current = static_cast<uint8_t>(condition->field_0) != 0
                                            ? layer->m_curr_state->get_state_id()
                                            : layer->get_curr_category()->field_4;
            if (current == string_hash{condition->field_4}) {
                destination = condition->field_8;
                matched = true;
                break;
            }
            alter = this->field_20->field_14.m_data[i];
        }
        alter = this->field_20->field_14.m_data[i];
        if (!matched) {
            destination = alter->field_18;
            if (destination == string_hash{0}) {
                continue;
            }
        }
        a2.field_0->transition_layer(static_cast<layer_types>(alter->field_0), destination);
    }
}

bool basic_rule_data::has_post_action() const
{
    return this->field_20 != nullptr;
}

basic_rule_data::rule_action::rule_action(from_mash_in_place_constructor *a2) : field_8(a2)
{
    this->initialize(mash::FROM_MASH);

    if (this->destination_states != nullptr) {
        mash_info_struct::construct_class(this->destination_states);
    }
}

void basic_rule_data::rule_action::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->destination_states = nullptr;
    }
}

void basic_rule_data::rule_action::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_8, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if (this->destination_states != nullptr) {
        a1->unmash_class(this->destination_states,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
        );
    }
}

string_hash basic_rule_data::rule_action::get_dest() const
{
    TRACE("als::basic_rule_data::rule_action::get_dest");

    assert((the_action == basic_rule_data::TRANSITION) || (the_action == basic_rule_data::TRANSITION_CATEGORY));

    if (this->destination_states == nullptr) {
        return this->field_8;
    }

    assert(this->destination_states->size() > 0);
    const float threshold = static_cast<float>(rand() * (1.0 / 32768.0));
    long double cumulative_weight = 0.0;
    for (int i = 0; i < this->destination_states->size(); ++i) {
        auto *destination = this->destination_states->m_data[i];
        cumulative_weight += destination->field_4;
        if (threshold <= cumulative_weight) {
            return destination->field_0;
        }
    }
    return this->destination_states->m_data[this->destination_states->size() - 1]->field_0;
}

void basic_rule_data::rule_action::process_action(request_data &a2) const
{
    TRACE("als::basic_rule_data::rule_action::process_action");

    switch (this->the_action) {
    case TRANSITION:
        a2.did_transition_occur = true;
        a2.do_post_action = true;
        a2.is_trans_to_category = false;
        a2.field_8 = this->get_dest();
        break;
    case TRANSITION_CATEGORY:
        a2.did_transition_occur = true;
        a2.do_post_action = true;
        a2.is_trans_to_category = true;
        a2.field_8 = this->get_dest();
        break;
    case 2:
        a2.did_transition_occur = false;
        a2.do_post_action = true;
        break;
    case 3:
        a2.did_transition_occur = false;
        a2.do_post_action = false;
        a2.ignore_no_transition = true;
        break;
    case 4:
        a2.did_transition_occur = false;
        a2.do_post_action = false;
        a2.ignore_no_transition = false;
        break;
    default:
        return;
    }
}

basic_rule_data::post_action_rule_set::post_action_rule_set(from_mash_in_place_constructor *a2)
    : field_0(a2), field_14(a2)
{}

void basic_rule_data::post_action_rule_set::unmash(mash_info_struct *a1, void *)
{
    TRACE("post_action_rule_set::unmash");

    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_14, this);
}

}  // namespace als

void als_basic_rule_data_patch()
{
    FUNC_ADDRESS(address, &als::basic_rule_data::rule_action::process_action);
    SET_JUMP(0x004997D0, address);

    {
        FUNC_ADDRESS(address, &als::basic_rule_data::can_transition);
        REDIRECT(0x004A6F9F, address);
    }

    {
        FUNC_ADDRESS(address, &als::basic_rule_data::do_post_action);
        REDIRECT(0x004A72E9, address);
        REDIRECT(0x004A72E9, address);
        REDIRECT(0x004A768B, address);
        REDIRECT(0x004A769E, address);
        REDIRECT(0x004A76B1, address);
    }
}
