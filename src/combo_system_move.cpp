#include "combo_system_move.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(combo_system_move::requirements, 0x48);
VALIDATE_SIZE(combo_system_move::results, 0x7C);
VALIDATE_SIZE(combo_system_move::dialation_info, 0x14);
VALIDATE_SIZE(combo_system_move::link_info, 0x14);
VALIDATE_SIZE(combo_system_move::trigger_info, 0xC);
VALIDATE_SIZE(combo_system_move::target_info, 0xC);
VALIDATE_SIZE(combo_system_move::range_info, 0x14);
VALIDATE_SIZE(combo_system_move::link_info, 0x14);

VALIDATE_SIZE(combo_system_move, 0xC8);

combo_system_move::combo_system_move()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087B8BC;
    }
}

combo_system_move::dialation_info::dialation_info()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00873734;
    }
}

void combo_system_move::dialation_info::_unmash(mash_info_struct *a2, void *a3)
{
    TRACE("combo_system_move::dialation_info::unmash");

    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::dialation_info::get_mash_sizeof() const
{
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_move::link_info::link_info()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x008737A4;
    }
}

void combo_system_move::link_info::_unmash(mash_info_struct *a1, void *)
{
    TRACE("combo_system_move::link_info::unmash");

    a1->unmash_class_in_place(this->field_4, this);
}

int combo_system_move::link_info::get_mash_sizeof() const
{
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_move::results::results()
{
    TRACE("combo_system_move::results::results");

    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00879FC0;
    }

    this->initialize(false);
}

combo_system_move::results::results(const results &a2) {
    THISCALL(0x0048CBB0, this, &a2);
}

combo_system_move::results::~results()
{
    if constexpr (1) {
    } else {
        THISCALL(0x0043C0A0, this);
    }
}

void combo_system_move::results::initialize(bool a2)
{
    if ( !a2 )
    {
        this->field_30 = 0;
        this->field_34 = 0;
        this->field_10 = {""};
        this->field_38 = 0;
        this->field_3C = 0.30000001;
        this->field_68 = 0;
        this->field_6C = 0;
        this->field_78 = 2.0;
    }
}

void combo_system_move::results::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system_move::results::unmash");

    if constexpr (1)
    {
        a1->unmash_class_in_place(this->field_4, this);
        a1->unmash_class_in_place(this->field_8, this);
        a1->unmash_class_in_place(this->field_C, this);
        a1->unmash_class_in_place(this->field_10, this);

        a1->unmash_class_in_place(this->field_40, this);

        a1->unmash_class_in_place(this->field_54, this);
    }
    else
    {
        THISCALL(0x00471B30, this, a1, a3);
    }
}

int combo_system_move::results::get_mash_sizeof() const
{
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_move::requirements::requirements()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087B8A0;
    }

    this->initialize(false);
}

void combo_system_move::requirements::initialize(bool a2)
{
    if ( !a2 ) {
        this->field_44 = nullptr;
    }
}

void combo_system_move::requirements::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system_move::requirements::unmash");

    if constexpr (1)
    {
        a1->unmash_class_in_place(this->field_4, this);

        a1->unmash_class_in_place(this->field_10, this);

        a1->unmash_class_in_place(this->field_1C, this);

        a1->unmash_class_in_place(this->field_30, this);
    }
    else
    {
        THISCALL(0x00481470, this, a1, a3);
    }
}

int combo_system_move::requirements::get_mash_sizeof() const
{
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

void combo_system_move::_unmash(mash_info_struct *a2, void *)
{
    TRACE("combo_system_move::unmash");

    mash_virtual_base::fixup_vtable(&this->field_4);
    this->field_4.unmash(a2,this);

    mash_virtual_base::fixup_vtable(&this->field_80);
    this->field_80.unmash(a2, this);
}

int combo_system_move::_get_mash_sizeof() const
{
    return sizeof(*this);
}

int combo_system_move::get_mash_sizeof() const
{
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}


combo_system_move::trigger_info::trigger_info()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00873750;
    }

    this->initialize(false);
}


void combo_system_move::trigger_info::initialize(bool a2)
{
    if (!a2) {
        this->field_4 = 0x80000000;
        this->field_8 = 0;
    }
}

void combo_system_move::trigger_info::_unmash(mash_info_struct *a2, void *a3) {
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::trigger_info::get_mash_sizeof() const {
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_move::target_info::target_info()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0087376C;
    }

    this->initialize(false);
}

void combo_system_move::target_info::initialize(bool a2)
{
    if (!a2) {
        this->field_4 = 0;
        this->field_8 = 1;
    }
}

void combo_system_move::target_info::_unmash(mash_info_struct *a2, void *a3) {
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::target_info::get_mash_sizeof() const {
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_move::range_info::range_info()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00873788;
    }

    this->initialize(false);
}

void combo_system_move::range_info::initialize(bool a2)
{
    if (!a2) {
        this->field_4 = 0xFF7FFFFF;
        this->field_8 = 0x7F7FFFFF;
        this->field_C = 0xFF7FFFFF;
        this->field_10 = 0x7F7FFFFF;
    }
}

void combo_system_move::range_info::_unmash(mash_info_struct *a2, void *a3) {
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::range_info::get_mash_sizeof() const {
    int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

void combo_system_move_patch()
{
    {
        FUNC_ADDRESS(address, &combo_system_move::dialation_info::_unmash);
        set_vfunc(0x00873738, address);
    }
    {
        FUNC_ADDRESS(address, &combo_system_move::link_info::_unmash);
        set_vfunc(0x008737A8, address);
    }

    {
        FUNC_ADDRESS(address, &combo_system_move::requirements::_unmash);
        set_vfunc(0x0087B8A4, address);
    }

    {
        FUNC_ADDRESS(address, &combo_system_move::results::_unmash);
        set_vfunc(0x00879FC4, address);
    }

    {
        FUNC_ADDRESS(address, &combo_system_move::_unmash);
        set_vfunc(0x0087B8C0, address);
    }
}
