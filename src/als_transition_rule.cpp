#include "als_transition_rule.h"

#include "func_wrapper.h"
#include "mash_info_struct.h"

#include "common.h"
#include "trace.h"

namespace als
{
    VALIDATE_SIZE(implicit_transition_rule, 0x24u);
    VALIDATE_SIZE(explicit_transition_rule, 0x28u);
    VALIDATE_SIZE(incoming_transition_rule, 0x2Cu);
    VALIDATE_SIZE(layer_transition_rule, 0x18u);

    void layer_transition_rule::unmash(mash_info_struct *a1, void *)
    {
        a1->unmash_class_in_place(this->field_8, this);
    }

    bool layer_transition_rule::can_transition(als_data &a2) const
    {
        TRACE("als::layer_transition_rule::can_transition");

        return (bool) THISCALL(0x0049FF50, this, &a2);
    }

    bool explicit_transition_rule::can_transition(
        als_data &a1,
        string_hash a3) const
    {
        if ( a3 == this->field_24 )
        {
            auto can_transition = bit_cast<basic_rule_data *>(this)->can_transition(a1);
            return can_transition;
        }
        else
        {
            return false;
        }
    }

    void explicit_transition_rule::unmash(mash_info_struct *a1, void *a3)
    {
        this->field_0.unmash(a1, a3);
        a1->unmash_class_in_place(this->field_24, this);
    }

    bool implicit_transition_rule::can_transition(als_data &a1) const
    {
        this->field_0.can_transition(a1);
    }

    void implicit_transition_rule::unmash(mash_info_struct *a1, void *a3)
    {
        this->field_0.unmash(a1, a3);
    }

    void incoming_transition_rule::unmash(mash_info_struct *a1, void *a3)
    {
        this->field_0.unmash(a1, a3);
    }

}
