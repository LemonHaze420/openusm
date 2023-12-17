#include "als_scripted_state.h"

#include "als_basic_rule_data.h"
#include "als_data.h"
#include "als_transition_rule.h"
#include "func_wrapper.h"
#include "layer_state_machine_shared.h"
#include "mash_info_struct.h"
#include "scripted_trans_group.h"
#include "state_machine.h"
#include "utility.h"
#include "trace.h"
#include "common.h"

namespace als
{
    VALIDATE_SIZE(scripted_state, 0x54u);
    VALIDATE_SIZE(base_layer_scripted_state, 0x58u);

    scripted_state::scripted_state()
    {
        THISCALL(0x004ACA80, this);
    }

    void scripted_state::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("als::scripted_state::unmash");

        state::_unmash(a1, a3);

        a1->unmash_class_in_place(this->field_14, this);

        a1->unmash_class_in_place(this->field_18, this);
        a1->unmash_class_in_place(this->field_28, this);
        a1->unmash_class_in_place(this->field_3C, this);

#ifdef TARGET_XBOX
        {
            uint8_t class_mashed = -1;
            class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
            assert(class_mashed == 0xAF || class_mashed == 0);
        }
#endif

        if ( this->field_50 != nullptr )
        {
            a1->unmash_class(this->field_50, this
#ifdef TARGET_XBOX
                    , mash::NORMAL_BUFFER
#endif
                    );
        }
    }

    bool test_all_trans_groups(
        request_data &a1,
        mVectorBasic<int> *a2,
        scripted_trans_group::transition_type a3,
        als_data a4,
        string_hash a5)
    {
        TRACE("als::test_all_trans_groups");

        if constexpr (1) {
#if 0
            for ( int i = 0; i < a2->size(); ++i )
            {
                auto v6 = a2->at(i);
                auto *v15 = bit_cast<scripted_trans_group *>(a4.field_4->get_trans_group(v6));
                if ( bit_cast<scripted_trans_group *>(v15)->check_transition(a1, a3, a4, a5) )
                {
                    return true;
                }
            }

            return false;
#else
            auto begin = a2->m_data;
            auto end = begin + a2->size();
            auto it = std::find_if(begin, end, [&](int v6)
            {
                auto *v15 = bit_cast<scripted_trans_group *>(a4.field_4->get_trans_group(v6));
                return ( v15->check_transition(a1, a3, a4, a5) );
            });

            return (it != end);
#endif

        } else {
            return (bool) CDECL_CALL(0x004A6EA0, &a1, a2, a3, a4, a5);
        }
    }

    int scripted_state::get_filter(
        int out,
        animation_logic_system *,
        state_machine *,
        int)
    {
        TRACE("als::scripted_state::get_filter");

        return out;
    }

    request_data scripted_state::do_implicit_trans(
        animation_logic_system *a4,
        state_machine *a5)
    {
        TRACE("als::scripted_state::do_implicit_trans");

        {
            std::for_each(this->field_28.begin(), this->field_28.end(), [](auto *the_rule){ 
                printf("%s\n", the_rule->field_14.field_8.to_string());
            });
        }
     
        if constexpr (0) {
            request_data data {};
            als_data a2 {a4, a5};
            string_hash v14 {};
            if ( !als::test_all_trans_groups(
                    data,
                    &this->field_18,
                    static_cast<scripted_trans_group::transition_type>(0),
                    a2,
                    v14) )
            {
                for ( int i = 0; i < this->field_28.size(); ++i)
                {
                    auto *trans_rule = this->field_28.at(i);
                    if ( bit_cast<basic_rule_data *>(trans_rule)->can_transition(
                        a2) )
                    {
                        trans_rule->field_14.process_action(data);
                        if ( trans_rule->field_20 != nullptr )
                        {
                            data.field_10 = static_cast<scripted_trans_group::transition_type>(0);
                            data.field_C = int(&this->field_28.m_data[i]);
                        }
                    }
                }
            }

            if ( !(data.did_transition_occur || data.field_1) ) {
                if ( (this->field_C & 8) != 0 ) {
                    data.field_3 = false;
                } else {
                    data.field_3 = true;
                }

            }

            return data;
        } else {
            request_data data;
            THISCALL(0x004A6F10, this, &data, a4, a5);
            return data;
        }
    }

    void scripted_state::do_post_trans(
            animation_logic_system *a1,
            state_machine *a2,
            transition_post_handle a4)
    {
        TRACE("als::scripted_state::do_post_trans");

        als::als_data v1 {a1, a2};
        if ( a4.field_4 <= 1 ) {
            a4.field_0->do_post_action(v1);
        }
    }

    string_hash scripted_state::get_nal_anim_name() const
    {
        sp_log("0x%08X", m_vtbl);
        return this->field_14;
    }

    base_layer_scripted_state::base_layer_scripted_state()
    {
        THISCALL(0x00444000, this);
    }

    void base_layer_scripted_state::_unmash(mash_info_struct *a1, void *a3)
    {
        TRACE("base_layer_scripted_state::unmash");

        scripted_state::_unmash(a1, a3);
    }

}

als::request_data * __fastcall scripted_state__do_implicit_trans(
    als::scripted_state *self, void *,
    als::request_data *out,
    als::animation_logic_system *a4,
    als::state_machine *a5)
{
    *out = self->do_implicit_trans(a4, a5);
    return out;
}

string_hash * __fastcall scripted_state__get_nal_anim_name(als::scripted_state *self, void *, string_hash *a2)
{
    *a2 = self->get_nal_anim_name();
    return a2;
}

void als_scripted_state_patch()
{
    {
        FUNC_ADDRESS(address, &als::scripted_state::_unmash);
        set_vfunc(0x0087E1DC, address);
    }

    {
        FUNC_ADDRESS(address, &als::scripted_state::get_filter);
        SET_JUMP(0x00493E80, address);
    }

    SET_JUMP(0x004A6EA0, &als::test_all_trans_groups);

    {
        auto address = int(&scripted_state__do_implicit_trans);
        set_vfunc(0x0087E1FC, address);
        set_vfunc(0x0087E238, address);
    }

    {
        FUNC_ADDRESS(address, &als::scripted_state::do_post_trans);
        SET_JUMP(0x004A72B0, address);
    }

    {
        auto address = &scripted_state__get_nal_anim_name;
        SET_JUMP(0x00493E90, address);
    }

    {
        FUNC_ADDRESS(address, &als::base_layer_scripted_state::_unmash);
        set_vfunc(0x0087E218, address);
    }
}
