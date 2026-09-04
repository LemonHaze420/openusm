#include "panelanimobject.h"

#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"

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

void PanelAnimObject::Update(Float a2)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x18));
    func(this, nullptr, a2);
}

void PanelAnimObject::StartAnim(bool a1)
{
    void(__fastcall * func)(void *, void *, bool) = CAST(func, get_vfunc(m_vtbl, 0x20));
    func(this, nullptr, a1);
}

double PanelAnimObject::GetZvalue()
{
    float(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x3C));
    return func(this);

    //return this->field_8;
}

void PanelAnimObject::SetZvalue(Float a1, panel_layer a2)
{
    float(__fastcall * func)(void *, int, Float, panel_layer) = CAST(func, get_vfunc(m_vtbl, 0x30));
    func(this, 0, a1, a2);
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
    void(__fastcall * func)(void *, void *, bool) = CAST(func, get_vfunc(m_vtbl, 0x64));

    func(this, nullptr, a2);
}
