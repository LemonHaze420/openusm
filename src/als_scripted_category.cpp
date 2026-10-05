#include "als_scripted_category.h"

#include "als_filter_data.h"
#include "als_transition_rule.h"
#include "mash_info_struct.h"
#include "utility.h"
#include "trace.h"
#include "func_wrapper.h"
#include "common.h"
#include "mash_config.h"


#include "param_block.h"

#include <cassert>


namespace {
als::request_data *__fastcall category_implicit_trans(als::scripted_category *self, void *, als::request_data *out,
                                                      als::animation_logic_system *system, als::state_machine *machine)
{
    *out = self->do_implicit_trans(system, machine);
    return out;
}

als::request_data *__fastcall category_explicit_trans(als::scripted_category *self, void *, als::request_data *out,
                                                      als::animation_logic_system *system, als::state_machine *machine,
                                                      string_hash state)
{
    *out = self->_do_explicit_trans(system, machine, state);
    return out;
}

als::request_data *__fastcall category_layer_trans(als::scripted_category *self, void *, als::request_data *out,
                                                   als::animation_logic_system *system, als::state_machine *machine)
{
    *out = self->_do_layer_trans(system, machine);
    return out;
}

als::request_data *__fastcall category_incoming_trans(als::scripted_category *self, void *, als::request_data *out,
                                                      als::animation_logic_system *system, als::state_machine *machine)
{
    *out = self->_do_incoming_trans(system, machine);
    return out;
}

string_hash *__fastcall category_default_state(const als::scripted_category *self, void *, string_hash *out)
{
    *out = self->_get_default_state();
    return out;
}
}  // namespace
namespace als {
VALIDATE_SIZE(scripted_category, 0x7C);

void *scripted_category::native_vtable()
{
    static void *table[] = {func_address(&scripted_category::_destruct_mashed_class),
                            func_address(&scripted_category::_unmash),
                            func_address(&scripted_category::_scalar_deleting_destructor),
                            func_address(&scripted_category::_get_virtual_type_enum),
                            nullptr,
                            func_address(&mash_virtual_base::_is_or_is_subclass_of),
                            reinterpret_cast<void *>(&category_implicit_trans),
                            reinterpret_cast<void *>(&category_explicit_trans),
                            reinterpret_cast<void *>(&category_layer_trans),
                            reinterpret_cast<void *>(&category_incoming_trans),
                            func_address(&scripted_category::_do_post_trans),
                            nullptr,
                            reinterpret_cast<void *>(&category_default_state),
                            func_address(&scripted_category::_get_mash_sizeof)};
    return table;
}

scripted_category::scripted_category() : field_78(nullptr)
{
    this->m_vtbl = CAST(m_vtbl, native_vtable());
}

scripted_category::scripted_category(from_mash_in_place_constructor *a2)
    : category(a2), field_10(a2), field_14(a2), field_2C(a2), field_3C(a2), field_50(a2), field_64(a2)
{
    TRACE("als::scripted_category::scripted_category");

    this->m_vtbl = CAST(m_vtbl, native_vtable());

    if (this->field_78 != nullptr) {
        mash_info_struct::construct_class(this->field_78);
    }
}

void scripted_category::_destruct_mashed_class()
{
    field_10.destruct_mashed_class();
    field_14.field_0.destruct_mashed_class();
    field_14.field_4.clear();
    field_14.field_4.mContainer_base::destruct_mashed_class();
    field_2C.destruct_mashed_class();
    field_3C.clear();
    field_3C.mContainer_base::destruct_mashed_class();
    field_50.clear();
    field_50.mContainer_base::destruct_mashed_class();
    field_64.clear();
    field_64.mContainer_base::destruct_mashed_class();
    if (field_78 != nullptr) {
        field_78->clear();
        field_78->mContainer_base::destruct_mashed_class();
        field_78 = nullptr;
    }
    field_4.destruct_mashed_class();
    if (field_C != nullptr) {
        field_C->destruct_mashed_class();
        field_C = nullptr;
    }
}

void *scripted_category::_scalar_deleting_destructor(unsigned int flags)
{
    field_64.clear();
    field_50.clear();
    field_3C.clear();
    field_2C.destruct_mashed_class();
    field_14.field_4.clear();
    if ((flags & 1) != 0)
        ::operator delete(this);
    return this;
}

void scripted_category::_unmash(mash_info_struct *a1, void *)
{
    TRACE("als::scripted_category::unmash");

    category::_unmash(a1, this);

    a1->unmash_class_in_place(this->field_10, this);

    a1->unmash_class_in_place(this->field_14, this);

    a1->unmash_class_in_place(this->field_2C, this);

    a1->unmash_class_in_place(this->field_3C, this);
    a1->unmash_class_in_place(this->field_50, this);
    a1->unmash_class_in_place(this->field_64, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if (this->field_78 != nullptr) {
        a1->unmash_class(this->field_78,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
        );
    }
}

int scripted_category::_get_virtual_type_enum() const
{
    return 531;
}

request_data scripted_category::do_implicit_trans(animation_logic_system *a4, state_machine *a5)
{
    request_data data;
    data.field_C.field_0 = nullptr;
    als_data context{a4, a5};
    if (!test_all_trans_groups(data, this->field_2C, scripted_trans_group::IMPLICIT, context, string_hash{})) {
        for (int i = 0; i < this->field_3C.size(); ++i) {
            const auto index = static_cast<uint16_t>(i);
            if (this->field_3C.m_data[index]->can_transition(context)) {
                this->field_3C.m_data[index]->field_0.field_14.process_action(data);
                if (this->field_3C.m_data[index]->field_0.has_post_action()) {
                    data.field_C.field_4 = scripted_trans_group::IMPLICIT;
                    data.field_C.implicit_rule = &this->field_3C.m_data[index];
                }
                break;
            }
        }
    }
    return data;
}

request_data scripted_category::_do_explicit_trans(animation_logic_system *a4, state_machine *a5, string_hash a6)
{
    request_data data;
    data.field_C.field_0 = nullptr;
    als_data context{a4, a5};
    if (!test_all_trans_groups(data, this->field_2C, scripted_trans_group::EXPLICIT, context, a6)) {
        for (int i = 0; i < this->field_50.size(); ++i) {
            const auto index = static_cast<uint16_t>(i);
            if (this->field_50.m_data[index]->can_transition(context, a6)) {
                this->field_50.m_data[index]->field_0.field_14.process_action(data);
                if (this->field_50.m_data[index]->field_0.has_post_action()) {
                    data.field_C.field_4 = scripted_trans_group::EXPLICIT;
                    data.field_C.explicit_rule = &this->field_50.m_data[index];
                }
                break;
            }
        }
    }
    return data;
}

request_data scripted_category::_do_layer_trans(animation_logic_system *a4, state_machine *a5)
{
    request_data data;
    data.field_C.field_0 = nullptr;
    als_data context{a4, a5};
    if (!test_all_trans_groups(data, this->field_2C, scripted_trans_group::LAYER, context, string_hash{})) {
        if (this->field_78 != nullptr) {
            for (int i = 0; i < this->field_78->size(); ++i) {
                const auto index = static_cast<uint16_t>(i);
                if (this->field_78->m_data[index]->can_transition(context)) {
                    this->field_78->m_data[index]->field_8.process_action(data);
                }
            }
        }
    }
    return data;
}

request_data scripted_category::_do_incoming_trans(animation_logic_system *a4, state_machine *a5)
{
    request_data data;
    data.field_C.field_0 = nullptr;
    als_data context{a4, a5};
    for (int i = 0; i < this->field_64.size(); ++i) {
        const auto index = static_cast<uint16_t>(i);
        if (this->field_64.m_data[index]->can_transition(context)) {
            this->field_64.m_data[index]->field_0.field_14.process_action(data);
            if (this->field_64.m_data[index]->field_0.has_post_action()) {
                data.field_C.field_4 = static_cast<scripted_trans_group::transition_type>(3);
                data.field_C.incoming_rule = &this->field_64.m_data[index];
            }
            break;
        }
    }
    return data;
}

void scripted_category::_do_post_trans(animation_logic_system *a1, state_machine *a2, transition_post_handle a3)
{
    als_data context{a1, a2};
    if (a3.field_4 == scripted_trans_group::IMPLICIT || a3.field_4 == scripted_trans_group::EXPLICIT ||
        a3.field_4 == static_cast<scripted_trans_group::transition_type>(3)) {
        a3.rule_data().do_post_action(context);
    }
}

string_hash scripted_category::_get_default_state() const
{
    return this->field_10;
}

int scripted_category::_get_mash_sizeof() const
{
    return sizeof(scripted_category);
}
}  // namespace als


void als_scripted_category_patch()
{
    {
        FUNC_ADDRESS(address, &als::scripted_category::_unmash);
        set_vfunc(0x0087E254, address);
    }

    {
        auto address = int(&category_implicit_trans);
        set_vfunc(0x0087E268, address);
    }
}
