#include "als_transition_rule.h"


#include "als_animation_logic_system.h"
#include "als_data.h"
#include "als_state.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

#include "common.h"
#include "trace.h"

namespace als {
VALIDATE_SIZE(implicit_transition_rule, 0x24u);
VALIDATE_SIZE(explicit_transition_rule, 0x28u);
VALIDATE_SIZE(incoming_transition_rule, 0x2Cu);
VALIDATE_SIZE(layer_transition_rule, 0x18u);

layer_transition_rule::layer_transition_rule(from_mash_in_place_constructor *a2) : state_or_category_id(a2), field_8(a2)
{}

void layer_transition_rule::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_8, this);
}

bool layer_transition_rule::can_transition(als_data &a2) const
{
    TRACE("als::layer_transition_rule::can_transition");

    auto *layer = a2.field_0->get_als_layer_internal(this->layer_id);
    auto *state = this->use_previous_state ? layer->m_prev_state : layer->m_curr_state;
    if (state == nullptr) {
        return false;
    }
    return this->state_or_category_id == (this->match_category ? state->get_category_id() : state->get_state_id());
}

explicit_transition_rule::explicit_transition_rule(from_mash_in_place_constructor *a2) : field_0(a2), field_24(a2) {}

bool explicit_transition_rule::can_transition(als_data &a1, string_hash a3) const
{
    if (a3 == this->field_24) {
        return this->field_0.can_transition(a1);
    } else {
        return false;
    }
}

void explicit_transition_rule::unmash(mash_info_struct *a1, void *a3)
{
    this->field_0.unmash(a1, a3);
    a1->unmash_class_in_place(this->field_24, this);
}

implicit_transition_rule::implicit_transition_rule(from_mash_in_place_constructor *a2) : field_0(a2) {}

bool implicit_transition_rule::can_transition(als_data &a1) const
{
    return this->field_0.can_transition(a1);
}

void implicit_transition_rule::unmash(mash_info_struct *a1, void *a3)
{
    this->field_0.unmash(a1, a3);
}

incoming_transition_rule::incoming_transition_rule(from_mash_in_place_constructor *a2) : field_0(a2), field_24(a2) {}

bool incoming_transition_rule::can_transition(als_data &a1) const
{
    if (this->field_24 != string_hash{0}) {
        auto *state = a1.field_4->m_curr_state;
        auto id = static_cast<uint8_t>(this->field_28) != 0 ? state->get_category_id() : state->get_state_id();
        if (this->field_24 != id) {
            return false;
        }
    }
    return this->field_0.can_transition(a1);
}

void incoming_transition_rule::unmash(mash_info_struct *a1, void *a3)
{
    this->field_0.unmash(a1, a3);

#ifdef OPENUSM_XBPACK_V10
    this->field_28 = static_cast<uint8_t>(this->field_28);
#endif
}

}  // namespace als
