#include "panelanimobject.h"

#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"

#include <algorithm>
#include <cmath>

VALIDATE_SIZE(PanelAnimObject, 0x14);

PanelAnimObject::PanelAnimObject()
{
    this->m_vtbl = 0x00873898;
    this->field_10 = 4;
    this->field_4 = 1.0;
    this->field_C = 0;
}

PanelAnimObject::PanelAnimObject(int)
{
    this->m_vtbl = 0x00873898;
    this->field_10 = 4;
    this->field_4 = 1.0;
}

void PanelAnimObject::Update(Float delta_time)
{
#if STANDALONE_SYSTEM
    if (!IsShown())
        return;
    if ((field_10 & 0x10) != 0) {
        field_4 = std::fpclassify(field_C) == FP_ZERO
            ? 1.0f : field_4 + delta_time / field_C;
        if (field_4 >= 1.0f) {
            field_4 = 1.0f;
            field_10 &= static_cast<char>(~0x10);
        }
    }
    if ((field_10 & 0x20) != 0) {
        field_4 = std::fpclassify(field_C) == FP_ZERO
            ? 0.0f : field_4 - delta_time / field_C;
        if (field_4 <= 0.0f) {
            field_4 = 0.0f;
            field_10 &= static_cast<char>(~0x24);
        }
    }
#else
    void(__fastcall *func)(void *, void *, Float) =
        CAST(func, get_vfunc(m_vtbl, 0x18));
    func(this, nullptr, delta_time);
#endif
}

void PanelAnimObject::StartAnim(bool a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (a1)
            field_10 |= 2;
        else
            field_10 &= static_cast<char>(~2);
    } else {
        void(__fastcall *func)(void *, void *, bool) =
            CAST(func, get_vfunc(m_vtbl, 0x20));
        func(this, nullptr, a1);
    }
}

void PanelAnimObject::ResetXform()
{
    field_10 &= static_cast<char>(~1);
}

double PanelAnimObject::GetZvalue()
{
    if constexpr (STANDALONE_SYSTEM)
        return field_8;

    auto func = bit_cast<float(__fastcall *)(void *)>(
        get_vfunc(m_vtbl, 0x3C));
    return func(this);
}

void PanelAnimObject::SetZvalue(Float a1, panel_layer a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        const auto local_z = std::clamp(float(a1) * 0.5f, 0.0f, 1000.0f);
        field_8 = (static_cast<int>(a2) * 1000.0f + local_z) * 0.125f;
    } else {
        auto func = bit_cast<void(__fastcall *)(void *, int, Float, panel_layer)>(
            get_vfunc(m_vtbl, 0x30));
        func(this, 0, a1, a2);
    }
}

int PanelAnimObject::get_mash_sizeof()
{
    int(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4C));
    return func(this);
}

bool PanelAnimObject::IsShown()
{
    return (this->field_10 & 4) != 0;
}

void PanelAnimObject::SetShown(bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (a2)
            field_10 |= 4;
        else
            field_10 &= static_cast<char>(~4);
    } else {
        void(__fastcall *func)(void *, void *, bool) =
            CAST(func, get_vfunc(m_vtbl, 0x64));
        func(this, nullptr, a2);
    }
}
