#include "combo_system_move.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "base_ai_core.h"
#include <cmath>
#include <cstdlib>
#include <iterator>

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

namespace {
bool satisfies_move_link(const combo_system_move::requirements &requirements, string_hash category, float eta)
{
    const auto &links = requirements.field_30;
    if (links.m_size == 0)
        return true;
    const auto time = -eta;
    for (int index = 0; index < links.m_size; ++index) {
        const auto *link = links.m_data[index];
        if (link->field_4 == category &&
            (link->field_8 == 0 ||
             (link->field_8 == 3 && bit_cast<float>(link->field_C) <= time && bit_cast<float>(link->field_10) >= time)))
            return true;
    }
    return false;
}

uint32_t converted_trigger_buttons(uint32_t buttons)
{
    constexpr uint32_t masks[]{4,
                               8,
                               0x10,
                               0x20,
                               2,
                               1,
                               0x40000,
                               0x80000,
                               0x100000,
                               0x200000,
                               0x20000,
                               0x10000,
                               0x100000,
                               0x80000,
                               0x40004,
                               0x40008,
                               0x20000,
                               0x10000};
    uint32_t converted = 0;
    for (unsigned bit = 0; bit != std::size(masks); ++bit)
        if ((buttons & (1u << bit)) != 0)
            converted |= masks[bit];
    return converted;
}

int trigger_satisfaction(const combo_system_move::trigger_info &trigger, uint32_t input)
{
    const auto required = static_cast<uint32_t>(trigger.field_4);
    if (trigger.field_4 < 0)
        return input == required ? 5 : -100;
    int score = 1;
    if (required != 0x7FA00000) {
        const auto matched = input & required;
        if ((matched & 0x200000) != 0 || (required & 0x55000000) != 0)
            score = 3;
        else if ((required & 0x2A800000) != 0)
            score = 2;
        if (matched == 0)
            return -100;
    }
    const auto converted = converted_trigger_buttons(static_cast<uint32_t>(trigger.field_8));
    if ((input & converted) != converted)
        return -100;
    return score + ((input & ~converted & 0x805FFFFF) != 0 ? 1 : 2);
}

int target_satisfaction(const combo_system_move::target_info &requirements, vhandle_type<actor> target, bool has_target)
{
    auto *actor = target.get_volatile_ptr();
    if (actor != nullptr && requirements.field_4 != 4 && requirements.field_4 != 5) {
        auto *core = actor->get_ai_core();
        if (core == nullptr)
            return 0;
        static const string_hash size_hash{"character_size"};
        static const string_hash power_hash{"power_level"};
        const auto size = core->field_50.get_optional_pb_int(size_hash, 1, nullptr);
        const auto power = core->field_50.get_optional_pb_int(power_hash, 0, nullptr);
        const auto size_score = 4 - std::abs(size - requirements.field_8);
        const auto power_score = 7 - std::abs(power - requirements.field_4);
        const auto inverse_seven = bit_cast<float>(0x3E124925u);
        return static_cast<int>(
            std::floor(10.0 * (size_score * 0.25) - (1.0 - power_score * double(inverse_seven)) * 2.5 + 0.5));
    }
    if (requirements.field_4 == 4)
        return actor == nullptr && !has_target ? 10 : 0;
    return requirements.field_4 == 5 && has_target ? 1 : -100;
}
}  // namespace

int combo_system_move::requirements_satisfaction(vhandle_type<actor> target, vector3d displacement, uint32_t input,
                                                 string_hash previous_category, float eta, bool has_target,
                                                 float combat_level) const
{
    const auto &range = field_80.field_1C;
    const auto horizontal =
        std::sqrt(double(displacement.x) * displacement.x + double(displacement.z) * displacement.z);
    const auto vertical = std::fabs(double(displacement.y));
    const auto epsilon = bit_cast<float>(0x38D1B717u);
    if (!satisfies_move_link(field_80, previous_category, eta) ||
        !(horizontal <= bit_cast<float>(range.field_8) && horizontal >= bit_cast<float>(range.field_4) &&
          vertical <= bit_cast<float>(range.field_10) && vertical >= bit_cast<float>(range.field_C)) ||
        double(combat_level) + epsilon < field_80.field_44)
        return -100;
    return target_satisfaction(field_80.field_10, target, has_target) + trigger_satisfaction(field_80.field_4, input);
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
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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

combo_system_move::results::results(from_mash_in_place_constructor *tag)
    : field_4(tag), field_8(tag), field_C(tag), field_10(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[99]);
    field_40.m_vtbl = field_54.m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[95]);
}

combo_system_move::results::results(const results &a2)
    : field_4(a2.field_4), field_8(a2.field_8), field_C(a2.field_C), field_10(a2.field_10), field_20(a2.field_20),
      field_24(a2.field_24), field_28(a2.field_28), field_2C(a2.field_2C), field_30(a2.field_30), field_34(a2.field_34),
      field_38(a2.field_38), field_3C(a2.field_3C), field_40(a2.field_40), field_54(a2.field_54), field_68(a2.field_68),
      field_6C(a2.field_6C), field_70(a2.field_70), field_74(a2.field_74), field_78(a2.field_78)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[99]);
    field_40.m_vtbl = field_54.m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[95]);
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
    if (!a2) {
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

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_4, this);
        a1->unmash_class_in_place(this->field_8, this);
        a1->unmash_class_in_place(this->field_C, this);
        a1->unmash_class_in_place(this->field_10, this);

        a1->unmash_class_in_place(this->field_40, this);

        a1->unmash_class_in_place(this->field_54, this);
    } else {
        THISCALL(0x00471B30, this, a1, a3);
    }
}

int combo_system_move::results::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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
    if (!a2) {
        this->field_44 = 0.0f;
    }
}

void combo_system_move::requirements::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system_move::requirements::unmash");

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_4, this);

        a1->unmash_class_in_place(this->field_10, this);

        a1->unmash_class_in_place(this->field_1C, this);

        a1->unmash_class_in_place(this->field_30, this);
    } else {
        THISCALL(0x00481470, this, a1, a3);
    }
}

int combo_system_move::requirements::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

void combo_system_move::_unmash(mash_info_struct *a2, void *)
{
    TRACE("combo_system_move::unmash");

    mash_virtual_base::fixup_vtable(&this->field_4);
    this->field_4.unmash(a2, this);

    mash_virtual_base::fixup_vtable(&this->field_80);
    this->field_80.unmash(a2, this);
}

int combo_system_move::_get_mash_sizeof() const
{
    return sizeof(*this);
}

int combo_system_move::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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

void combo_system_move::trigger_info::_unmash(mash_info_struct *a2, void *a3)
{
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::trigger_info::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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

void combo_system_move::target_info::_unmash(mash_info_struct *a2, void *a3)
{
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::target_info::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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

void combo_system_move::range_info::_unmash(mash_info_struct *a2, void *a3)
{
    mash_virtual_base::_unmash(a2, a3);
}

int combo_system_move::range_info::get_mash_sizeof() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
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
