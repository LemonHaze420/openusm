#include "scripted_trans_group.h"

#include "als_basic_rule_data.h"
#include "als_request_data.h"
#include "als_transition_rule.h"
#include "als_filter_data.h"
#include "common.h"
#include "func_wrapper.h"
#include "mash_config.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(scripted_trans_group, 0x40u);

const char *to_string(scripted_trans_group::transition_type trans_type)
{
    const char *str[] = {"IMPLICIT", "EXPLICIT", "LAYER"};
    return str[trans_type];
}

scripted_trans_group::scripted_trans_group()
{
    if constexpr (1) {
        static void *g_vtbl[] = {func_address(&scripted_trans_group::_destruct_mashed_class),
                                 func_address(&scripted_trans_group::_unmash),
                                 func_address(&scripted_trans_group::_scalar_deleting_destructor),
                                 func_address(&scripted_trans_group::_get_virtual_type_enum),
                                 nullptr,
                                 func_address(&mash_virtual_base::_is_or_is_subclass_of),
                                 nullptr,
                                 func_address(&scripted_trans_group::_get_mash_sizeof)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x004AC950, this);
    }
}


scripted_trans_group::scripted_trans_group(from_mash_in_place_constructor *a1) : field_4(a1), field_14(a1), field_28(a1)
{
    this->m_vtbl = 0x0087E1B8;

    if (this->field_3C != nullptr) {
        mash_info_struct::construct_class(this->field_3C);
    }
}

void scripted_trans_group::_destruct_mashed_class()
{
    field_4.destruct_mashed_class();
    field_14.clear();
    field_14.mContainer_base::destruct_mashed_class();
    field_28.clear();
    field_28.mContainer_base::destruct_mashed_class();
    if (field_3C != nullptr) {
        field_3C->clear();
        field_3C->mContainer_base::destruct_mashed_class();
        field_3C = nullptr;
    }
}

void *scripted_trans_group::_scalar_deleting_destructor(unsigned int flags)
{
    field_28.clear();
    field_14.clear();
    field_4.destruct_mashed_class();
    if ((flags & 1) != 0)
        ::operator delete(this);
    return this;
}

void scripted_trans_group::_unmash(mash_info_struct *a1, void *)
{
    TRACE("scripted_trans_group::unmash");

    a1->unmash_class_in_place(this->field_4, this);

    a1->unmash_class_in_place(this->field_14, this);

    a1->unmash_class_in_place(this->field_28, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    {
        uint8_t class_mashed = -1;
        class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
        assert(class_mashed == 0xAF || class_mashed == 0);
    }
#endif

    if (this->field_3C != nullptr) {
        a1->unmash_class(this->field_3C,
                         this
#if OPENUSM_XBOX_MASH_FORMAT
                         ,
                         mash::NORMAL_BUFFER
#endif
        );
    }
}

int scripted_trans_group::_get_virtual_type_enum() const
{
    return 533;
}

int scripted_trans_group::_get_mash_sizeof() const
{
    return sizeof(scripted_trans_group);
}

bool scripted_trans_group::check_transition(request_data &data, scripted_trans_group::transition_type trans_type,
                                            als_data a4, string_hash a5) const
{
    TRACE("scripted_trans_group::check_transition", to_string(trans_type));

    if (test_all_trans_groups(data, this->field_4, trans_type, a4, a5)) {
        return true;
    }

    switch (trans_type) {
    case IMPLICIT:
        for (int i = 0; i < this->field_14.size(); ++i) {
            auto *rule = this->field_14.m_data[i];
            if (rule->can_transition(a4)) {
                rule->field_0.field_14.process_action(data);
                if (rule->field_0.has_post_action()) {
                    data.field_C.field_4 = IMPLICIT;
                    data.field_C.implicit_rule = &this->field_14.m_data[i];
                }
                break;
            }
        }
        break;
    case EXPLICIT:
        for (int i = 0; i < this->field_28.size(); ++i) {
            auto *rule = this->field_28.m_data[i];
            if (rule->can_transition(a4, a5)) {
                rule->field_0.field_14.process_action(data);
                if (rule->field_0.has_post_action()) {
                    data.field_C.field_4 = EXPLICIT;
                    data.field_C.explicit_rule = &this->field_28.m_data[i];
                }
                break;
            }
        }
        break;
    case LAYER:
        if (this->field_3C != nullptr) {
            for (int i = 0; i < this->field_3C->size(); ++i) {
                auto *rule = this->field_3C->m_data[i];
                if (rule->can_transition(a4)) {
                    rule->field_8.process_action(data);
                }
            }
        }
        break;
    default:
        break;
    }
    return false;
}

}  // namespace als


void als_scripted_trans_group_patch()
{
    {
        FUNC_ADDRESS(address, &als::scripted_trans_group::_unmash);
        set_vfunc(0x0087E1BC, address);
    }
}
