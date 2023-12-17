#include "als_basic_rule_data.h"

#include "als_dest_weight_data.h"
#include "als_request_data.h"
#include "mash_info_struct.h"
#include "common.h"
#include "trace.h"
#include "utility.h"

namespace als
{
    VALIDATE_SIZE(basic_rule_data::post_action_rule_set, 0x28u);

    bool basic_rule_data::can_transition(als_data &a2) const
    {
        return (bool) THISCALL(0x0049FEE0, this, &a2);
    }

    void basic_rule_data::do_post_action(als_data &a2)
    {
        TRACE("als::basic_rule_data::do_post_action");

        THISCALL(0x004A6CC0, this, &a2);
    }

    bool basic_rule_data::has_post_action() const
    {
        return this->field_20 != nullptr;
    }

    void basic_rule_data::rule_action::unmash(mash_info_struct *a1, void *)
    {
        a1->unmash_class_in_place(this->field_8, this);

#ifdef TARGET_XBOX
        {
            uint8_t class_mashed = -1;
            class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
            assert(class_mashed == 0xAF || class_mashed == 0);
        }
#endif

        if (this->destination_states != nullptr)
        {
            a1->unmash_class(this->destination_states, this
#ifdef TARGET_XBOX
                , mash::NORMAL_BUFFER
#endif
                    );
        }
    }

    string_hash basic_rule_data::rule_action::get_dest()
    {
        TRACE("als::basic_rule_data::rule_action::get_dest");

        assert((the_action == basic_rule_data::TRANSITION) || (the_action == basic_rule_data::TRANSITION_CATEGORY));

        if constexpr (1) {
            if ( this->destination_states != nullptr ) {
                assert(destination_states->size() > 0);

                auto sub_65DB3E = [](float a1, float a2) -> double
                {
                    return ((rand() * 0.000030518509) * (a2 - a1)) + a1;
                };

                auto v8 = sub_65DB3E(0.0, 1.0);
                float v7 = 0.0;

                {
                    std::for_each(this->destination_states->begin(), this->destination_states->end(), [](auto &state) {
                        printf("%s", string_hash {state->field_0}.to_string());
                    });
                }

                for ( int i = 0; i < this->destination_states->size(); ++i )
                {
                    v7 += this->destination_states->at(i)->field_4;
                    if ( v7 >= v8 )
                    {
                        auto *v3 = (string_hash *)this->destination_states->at(i);
                        return (*v3);
                    }
                }

                auto v4 = this->destination_states->size();
                auto *v5 = (string_hash *)this->destination_states->at(v4 - 1);
                return (*v5);
            }
            else
            {
                return this->field_8;
            }
        } else {
            string_hash result;
            THISCALL(0x00499730, this, &result);

            return result;
        }
    }

    void basic_rule_data::rule_action::process_action(request_data &a2)
    {
        TRACE("als::basic_rule_data::rule_action::process_action");

        if constexpr (1) {
            switch ( this->the_action )
            {
            case TRANSITION:
                a2.did_transition_occur = true;
                a2.field_1 = true;
                a2.field_2 = false;
                a2.field_8 = this->get_dest();
                break;
            case TRANSITION_CATEGORY:
                a2.did_transition_occur = true;
                a2.field_1 = true;
                a2.field_2 = true;
                a2.field_8 = this->get_dest();
                break;
            case 2:
                a2.did_transition_occur = false;
                a2.field_1 = true;
                break;
            case 3:
                a2.did_transition_occur = false;
                a2.field_1 = false;
                a2.field_3 = true;
                break;
            case 4:
                a2.did_transition_occur = false;
                a2.field_1 = false;
                a2.field_3 = false;
                break;
            default:
                return;
            }
        } else {
            THISCALL(0x004997D0, this, &a2);
        }
    }

    void basic_rule_data::post_action_rule_set::unmash(mash_info_struct *a1, void *)
    {
        TRACE("post_action_rule_set::unmash");

        a1->unmash_class_in_place(this->field_0, this);
        a1->unmash_class_in_place(this->field_14, this);
    }
        
}

void als_basic_rule_data_patch()
{
    FUNC_ADDRESS(address, &als::basic_rule_data::rule_action::process_action);
    SET_JUMP(0x004997D0, address);

    {
        FUNC_ADDRESS(address, &als::basic_rule_data::do_post_action);
        REDIRECT(0x004A72E9, address);
        REDIRECT(0x004A72E9, address);
        REDIRECT(0x004A768B, address);
        REDIRECT(0x004A769E, address);
        REDIRECT(0x004A76B1, address);
    }
}
