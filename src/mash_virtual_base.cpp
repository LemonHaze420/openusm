#include "mash_virtual_base.h"

#include "anim_record.h"
#include "enum_anim_key.h"
#include "func_wrapper.h"
#include "als_scripted_category.h"
#include "als_scripted_state.h"
#include "als_transition_group_base.h"
#include "layer_state_machine_shared.h"
#include "log.h"
#include "memory.h"
#include "meta_anim_interact.h"
#include "panelquad.h"
#include "fefloatingtext.h"
#include "femultilinetext.h"
#include "spidey_base_state.h"
#include "std_puppet_trans_state.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

mash_virtual_base::mash_virtual_base() {}

void *mash_virtual_base::operator new(size_t sz)
{
    return mem_alloc(sz);
}

void mash_virtual_base::operator delete(void *ptr, size_t sz)
{
    mem_dealloc(ptr, sz);
}

void *mash_virtual_base::create_subclass_by_enum(mash::virtual_types_enum a1)
{
    TRACE("mash_virtual_base::create_subclass_by_enum");

    return (void *)CDECL_CALL(0x0042AB60, a1);
}

void *mash_virtual_base::create_subclass_by_enum_in_place(mash::virtual_types_enum a1, mash_virtual_base *a2, int a3)
{
    TRACE("mash_virtual_base::create_subclass_by_enum_in_place");

    return (void *)CDECL_CALL(0x004227E0, a1, a2, a3);
}

void mash_virtual_base::destruct_mashed_class()
{
    ;
}

void mash_virtual_base::_unmash(mash_info_struct *, void *) {}

void mash_virtual_base::unmash(mash_info_struct *a2, void *a3)
{
    void(__fastcall * func)(void *, int, mash_info_struct *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this, 0, a2, a3);
}

uint32_t mash_virtual_base::_get_virtual_type_enum() const
{
    return 573;
}

uint32_t mash_virtual_base::get_virtual_type_enum() const
{
    uint32_t(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this);
}

bool mash_virtual_base::is_subclass_of(mash::virtual_types_enum) const
{
    return false;
}

bool mash_virtual_base::_is_or_is_subclass_of(mash::virtual_types_enum a2) const
{
    return this->get_virtual_type_enum() == a2 || this->is_subclass_of(a2);
}

bool mash_virtual_base::is_or_is_subclass_of(mash::virtual_types_enum a2) const
{
    bool(__fastcall * func)(const void *, void *edx, mash::virtual_types_enum) = CAST(func, get_vfunc(m_vtbl, 0x14));
    return func(this, nullptr, a2);
}

void mash_virtual_base::generate_vtable()
{
    CDECL_CALL(0x00432B60);

#ifdef TARGET_XBOX
    {
        auto *v1 = new PanelQuad{};
        map_vtable.insert_or_assign(to_hash("PanelQuad"), v1);
    }

    {
        auto *v1 = new FEText{};
        map_vtable.insert_or_assign(to_hash("FEText"), v1);
    }

    {
        auto *v1 = new FEMultiLineText{};
        map_vtable.insert_or_assign(to_hash("FEMultiLineText"), v1);
    }

    {
        auto *v1 = new FEFloatingText{};
        map_vtable.insert_or_assign(to_hash("FEFloatingText"), v1);
    }

    {
        auto *v1 = new als::state_machine_shared{};
        map_vtable.insert_or_assign(to_hash("als::state_machine_shared"), v1);
    }

    {
        auto *v1 = new als::layer_state_machine_shared{};
        map_vtable.insert_or_assign(to_hash("als::layer_state_machine_shared"), v1);
    }

    {
        auto *v1 = new als::scripted_state{};
        map_vtable.insert_or_assign(to_hash("als::scripted_state"), v1);
    }

    {
        auto *v1 = new als::base_layer_scripted_state{};
        map_vtable.insert_or_assign(to_hash("als::base_layer_scripted_state"), v1);
    }

    {
        auto *v1 = new als::scripted_category{};
        map_vtable.insert_or_assign(to_hash("als::scripted_category"), v1);
    }

    {
        auto *v1 = new als::scripted_trans_group{};
        map_vtable.insert_or_assign(to_hash("als::scripted_trans_group"), v1);
    }

    {
        auto *v1 = new ai::meta_anim_interact{};
        map_vtable.insert_or_assign(to_hash("als::meta_anim_interact"), v1);
    }

    {
        auto *v1 = new ai::meta_anim_strength_test{};
        map_vtable.insert_or_assign(to_hash("als::meta_anim_strength_test"), v1);
    }

    {
        auto *v1 = new als::als_meta_linear_blend{};
        map_vtable.insert_or_assign(to_hash("als::als_meta_linear_blend"), v1);
    }

    {
        auto *v1 = new als::als_meta_anim_swing{};
        map_vtable.insert_or_assign(to_hash("als::als_meta_anim_swing"), v1);
    }

    {
        auto *v1 = new ai::spidey_base_state{};
        map_vtable.insert_or_assign(to_hash("spidey_base_state"), v1);
    }

    {
        auto *v1 = new ai::std_puppet_trans_state{};
        map_vtable.insert_or_assign(to_hash("std_puppet_trans_state"), v1);
    }

    {
        auto *v1 = new anim_key{};
        map_vtable.insert_or_assign(to_hash("anim_key"), v1);
    }

    {
        auto *v1 = new anim_record{};
        map_vtable.insert_or_assign(to_hash("anim_record"), v1);
    }
#endif
}

void *mash_virtual_base::construct_class_helper(void *a1)
{
    if constexpr (0) {
        auto *v1 = static_cast<mash_virtual_base *>(a1);

        auto v2 = v1->get_virtual_type_enum();

        sp_log("mash::virtual_types_enum = %u", v2);

        return mash_virtual_base::create_subclass_by_enum_in_place(
            static_cast<mash::virtual_types_enum>(v2), v1, 0x7FFFFFFF);
    } else {
        auto *v1 = static_cast<mash_virtual_base *>(a1);
        auto v2 = v1->get_virtual_type_enum();

        sp_log("mash::virtual_types_enum = %u", v2);
        return (void *)CDECL_CALL(0x0042A7C0, a1);
    }
}

void mash_virtual_base::fixup_vtable(void *a1)
{
    TRACE("mash_virtual_base::fixup_vtable");

#ifdef TARGET_XBOX
    const auto hash = static_cast<uint32_t *>(a1)[0];

    sp_log("0x%08X", hash);

    struct {
        int m_vtbl;
    } *tmp = bit_cast<decltype(tmp)>(map_vtable.at(hash));
    static_cast<uint32_t *>(a1)[0] = map_vtable.at(hash)->m_vtbl;

#else

    const auto idx = static_cast<uint32_t *>(a1)[0];

    struct {
        int m_vtbl;
    } *tmp = static_cast<decltype(tmp)>(a1);
    tmp->m_vtbl = bit_cast<uint32_t>(vtable()[idx]);
#endif

    sp_log("0x%08X", tmp->m_vtbl);
}

void mash_virtual_base_patch()
{
    REDIRECT(0x00555726, mash_virtual_base::generate_vtable);

    REDIRECT(0x004B157A, mash_virtual_base::construct_class_helper);

    return;
}
