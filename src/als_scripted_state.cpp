#include "als_scripted_state.h"

#include "als_basic_rule_data.h"
#include "als_data.h"
#include "als_filter_data.h"
#include "als_transition_rule.h"
#include "func_wrapper.h"
#include "layer_state_machine_shared.h"
#include "mash_info_struct.h"
#include "scripted_trans_group.h"
#include "state_machine.h"
#include "utility.h"
#include "trace.h"
#include "common.h"
#include "mash_config.h"


#include "param_block.h"

namespace als {
VALIDATE_SIZE(scripted_state, 0x54u);
VALIDATE_SIZE(base_layer_scripted_state, 0x58u);

scripted_state::scripted_state()
{
    if constexpr (1) {
        static void *g_vtbl[] = {func_address(&scripted_state::_destruct_mashed_class),
                                 func_address(&scripted_state::_unmash),
                                 func_address(&scripted_state::_scalar_deleting_destructor),
                                 func_address(&scripted_state::_get_virtual_type_enum),
                                 nullptr,
                                 func_address(&mash_virtual_base::_is_or_is_subclass_of),
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 func_address(&scripted_state::_get_mash_sizeof)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x004ACA80, this);
    }
}

scripted_state::scripted_state(from_mash_in_place_constructor *a2)
    : state(a2), field_14(a2), field_18(a2), field_28(a2), field_3C(a2)
{
    this->m_vtbl = 0x0087E1D8;

    if (this->field_50 != nullptr) {
        mash_info_struct::construct_class(this->field_50);
    }
}

void scripted_state::_destruct_mashed_class()
{
    field_14.destruct_mashed_class();
    field_18.destruct_mashed_class();
    field_28.clear();
    field_28.mContainer_base::destruct_mashed_class();
    field_3C.clear();
    field_3C.mContainer_base::destruct_mashed_class();
    if (field_50 != nullptr) {
        field_50->clear();
        field_50->mContainer_base::destruct_mashed_class();
        field_50 = nullptr;
    }
    m_state_id.destruct_mashed_class();
    m_cat_id.destruct_mashed_class();
    if (field_10 != nullptr) {
        field_10->destruct_mashed_class();
        field_10 = nullptr;
    }
}

void *scripted_state::_scalar_deleting_destructor(unsigned int flags)
{
    field_3C.clear();
    field_28.clear();
    field_18.destruct_mashed_class();
    if ((flags & 1) != 0)
        ::operator delete(this);
    return this;
}

void scripted_state::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("als::scripted_state::unmash");

    state::_unmash(a1, a3);

    a1->unmash_class_in_place(this->field_14, this);

    a1->unmash_class_in_place(this->field_18, this);

    a1->unmash_class_in_place(this->field_28, this);

    a1->unmash_class_in_place(this->field_3C, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if (this->field_50 != nullptr) {
        a1->unmash_class(this->field_50,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
        );
    }
}

int scripted_state::_get_virtual_type_enum() const
{
    return 532;
}

bool test_all_trans_groups(request_data &a1, const mVectorBasic<int> &a2,
                           scripted_trans_group::transition_type trans_type, als_data a4, string_hash a5)
{
    TRACE("als::test_all_trans_groups");

    if constexpr (1) {
        auto begin = a2.m_data;
        auto end = begin + a2.size();
        auto it = std::find_if(begin, end, [&](int v6) {
            auto *trans_group = a4.field_4->get_trans_group(v6);
            return trans_group->check_transition(a1, trans_type, a4, a5);
        });

        return (it != end);

    } else {
        return (bool)CDECL_CALL(0x004A6EA0, &a1, &a2, trans_type, a4, a5);
    }
}

int scripted_state::get_filter(int out, animation_logic_system *, state_machine *, int)
{
    TRACE("als::scripted_state::get_filter");

    return out;
}

int scripted_state::_get_mocomp_type()
{
    return 573;
}

request_data scripted_state::_do_implicit_trans(animation_logic_system *a4, state_machine *a5)
{
    TRACE("als::scripted_state::do_implicit_trans");

    if constexpr (STANDALONE_SYSTEM) {
        request_data data{};
        als_data context{a4, a5};
        string_hash explicit_state{};
        if (!test_all_trans_groups(data, this->field_18, scripted_trans_group::IMPLICIT, context, explicit_state)) {
            auto begin = this->field_28.m_data;
            auto end = begin + this->field_28.size();
            auto it =
                std::find_if(begin, end, [&context](auto *trans_rule) { return trans_rule->can_transition(context); });

            if (it != end) {
                auto &trans_rule = *it;
                trans_rule->field_0.field_14.process_action(data);
                if (trans_rule->field_0.has_post_action()) {
                    data.field_C.field_4 = scripted_trans_group::IMPLICIT;
                    data.field_C.implicit_rule = &trans_rule;
                }
            }
        }

        if (!data.did_rule_pass()) {
            if (this->is_flag_set(static_cast<state_flags>(8))) {
                data.ignore_no_transition = false;
            } else {
                data.ignore_no_transition = true;
            }
        }

        return data;
    } else {
        request_data data;
        THISCALL(0x004A6F10, this, &data, a4, a5);
        return data;
    }
}

request_data scripted_state::_do_explicit_trans(animation_logic_system *a4, state_machine *a5, string_hash a6)
{
    request_data data{};
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x004A7040, this, &data, a4, a5, a6);
    } else {
        als_data context{a4, a5};
        if (!test_all_trans_groups(data, field_18, scripted_trans_group::EXPLICIT, context, a6)) {
            for (int i = 0; i < field_3C.size(); ++i) {
                auto *rule = field_3C.m_data[i];
                if (!rule->can_transition(context, a6))
                    continue;
                rule->field_0.field_14.process_action(data);
                if (rule->field_0.has_post_action()) {
                    data.field_C.field_4 = scripted_trans_group::EXPLICIT;
                    data.field_C.explicit_rule = &field_3C.m_data[i];
                }
                break;
            }
        }
        if (!data.did_rule_pass())
            data.ignore_no_transition = !is_flag_set(static_cast<state_flags>(8));
    }
    return data;
}

request_data scripted_state::_do_layer_trans(animation_logic_system *a4, state_machine *a5)
{
    request_data data{};
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x004A7180, this, &data, a4, a5);
    } else {
        als_data context{a4, a5};
        if (!test_all_trans_groups(data, field_18, scripted_trans_group::LAYER, context, string_hash{}) &&
            field_50 != nullptr) {
            for (int i = 0; i < field_50->size(); ++i) {
                auto *rule = field_50->m_data[i];
                if (rule->can_transition(context))
                    rule->field_8.process_action(data);
            }
        }
        if (!data.did_rule_pass())
            data.ignore_no_transition = !is_flag_set(static_cast<state_flags>(8));
    }
    return data;
}

void scripted_state::_do_post_trans(animation_logic_system *a1, state_machine *a2, transition_post_handle a4)
{
    TRACE("als::scripted_state::do_post_trans");

    als_data context{a1, a2};
    if (a4.field_4 <= scripted_trans_group::EXPLICIT) {
        a4.rule_data().do_post_action(context);
    }
}

int scripted_state::_get_mash_sizeof() const
{
    return sizeof(scripted_state);
}

string_hash scripted_state::get_nal_anim_name() const
{
    return this->field_14;
}

base_layer_scripted_state::base_layer_scripted_state()
{
    if constexpr (1) {
        static void *g_vtbl[] = {func_address(&base_layer_scripted_state::_destruct_mashed_class),
                                 func_address(&base_layer_scripted_state::_unmash),
                                 func_address(&base_layer_scripted_state::_scalar_deleting_destructor),
                                 func_address(&base_layer_scripted_state::_get_virtual_type_enum),
                                 nullptr,
                                 func_address(&mash_virtual_base::_is_or_is_subclass_of),
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 func_address(&base_layer_scripted_state::_get_mash_sizeof)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x00444000, this);
    }
}

base_layer_scripted_state::base_layer_scripted_state(from_mash_in_place_constructor *a2) : scripted_state(a2)
{
    this->m_vtbl = 0x0087E214;
}

void base_layer_scripted_state::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("base_layer_scripted_state::unmash");

    scripted_state::_unmash(a1, a3);
}

int base_layer_scripted_state::_get_virtual_type_enum() const
{
    return 530;
}

int base_layer_scripted_state::_get_mash_sizeof() const
{
    return sizeof(base_layer_scripted_state);
}

}  // namespace als

als::request_data *__fastcall scripted_state__do_implicit_trans(als::scripted_state *self, void *,
                                                                als::request_data *out, als::animation_logic_system *a4,
                                                                als::state_machine *a5)
{
    *out = self->_do_implicit_trans(a4, a5);
    return out;
}

string_hash *__fastcall scripted_state__get_nal_anim_name(als::scripted_state *self, void *, string_hash *a2)
{
    *a2 = self->get_nal_anim_name();
    return a2;
}

als::scripted_state *__fastcall als_base_layer_scripted_state__constructor0(als::scripted_state *self)
{
    TRACE("als::base_layer_scripted_state::base_layer_scripted_state");
    THISCALL(0x00444000, self);
    return self;
}

als::base_layer_scripted_state *__fastcall als_base_layer_scripted_state__constructor1(
    als::base_layer_scripted_state *self, void *, from_mash_in_place_constructor *a2)
{
    TRACE("als::base_layer_scripted_state::base_layer_scripted_state(from_mash_in_place_constructor *)");

    THISCALL(0x004ACBD0, self, a2);
    return self;
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
        FUNC_ADDRESS(address, &als::scripted_state::_do_post_trans);
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

    REDIRECT(0x0043192E, als_base_layer_scripted_state__constructor0);

    REDIRECT(0x00429A04, als_base_layer_scripted_state__constructor1);
}
