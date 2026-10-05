#include "als_category.h"

#include "common.h"

#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "param_block.h"
#include "trace.h"
#include "mash_config.h"
#include "utility.h"
#include "vtbl.h"

namespace als {
VALIDATE_SIZE(category, 0x10);

request_data category::do_implicit_trans(animation_logic_system *a3, state_machine *a4)
{
    request_data data;
    void(__fastcall * func)(void *, void *, request_data *, animation_logic_system *, state_machine *) =
        CAST(func, get_vfunc(m_vtbl, 0x18));
    func(this, nullptr, &data, a3, a4);
    return data;
}

request_data category::do_explicit_trans(animation_logic_system *a4, state_machine *a5, string_hash a6)
{
    request_data data;
    void(__fastcall * func)(void *, void *, request_data *, animation_logic_system *, state_machine *, string_hash) =
        CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this, nullptr, &data, a4, a5, a6);
    return data;
}

request_data category::do_layer_trans(animation_logic_system *a4, state_machine *a5)
{
    request_data data;
    void(__fastcall * func)(void *, void *, request_data *, animation_logic_system *, state_machine *) =
        CAST(func, get_vfunc(m_vtbl, 0x20));
    func(this, nullptr, &data, a4, a5);
    return data;
}

request_data category::do_incoming_trans(animation_logic_system *a3, state_machine *a4)
{
    request_data data;
    void(__fastcall * func)(void *, void *, request_data *, animation_logic_system *, state_machine *) =
        CAST(func, get_vfunc(m_vtbl, 0x24));
    func(this, nullptr, &data, a3, a4);
    return data;
}

void category::do_post_trans(animation_logic_system *a1, state_machine *a2, transition_post_handle a3)
{
    void(__fastcall * func)(void *, void *edx, animation_logic_system *, state_machine *, transition_post_handle) =
        CAST(func, get_vfunc(m_vtbl, 0x28));
    func(this, nullptr, a1, a2, a3);
}

category::category()
{
    if constexpr (1) {
        this->initialize(mash::ALLOCATED);
    } else {
        THISCALL(0x00493150, this);
    }
}

category::category(from_mash_in_place_constructor *a2) : field_4(a2)
{
    TRACE("als::category::category");

    this->m_vtbl = 0x00875704;

    if (this->field_C != nullptr) {
        mash_info_struct::construct_class(this->field_C);
    }
}

void category::initialize(mash::allocation_scope a2)
{
    if (a2 != mash::FROM_MASH) {
        this->field_C = nullptr;
    }
}

string_hash category::get_default_state() const
{
    string_hash result{};
    int(__fastcall * func)(const category *, void *edx, string_hash *) = CAST(func, get_vfunc(m_vtbl, 0x30));
    func(this, nullptr, &result);
    return result;
}

int category::get_mash_sizeof() const
{
    int(__fastcall * func)(const category *) = CAST(func, get_vfunc(m_vtbl, 0x34));
    return func(this);
}

void category::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("als::category::unmash");

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_4, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
        {
            uint8_t class_mashed = -1;
            class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
            assert(class_mashed == 0xAF || class_mashed == 0);
        }
#endif

        if (this->field_C != nullptr) {
            a1->unmash_class(this->field_C,
                             this
#if OPENUSM_XBOX_MASH_FORMAT
                             ,
                             mash::NORMAL_BUFFER
#endif
            );
        }
    } else {
        THISCALL(0x0049F110, this, a1, a3);
    }
}
}  // namespace als

void als_category_patch()
{
    {
        FUNC_ADDRESS(address, &als::category::_unmash);
        set_vfunc(0x00875708, address);
    }
}
