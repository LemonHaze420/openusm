#include "nglrendernode.h"

#include "common.h"
#include "vtbl.h"

VALIDATE_SIZE(nglRenderNode, 0xC);

void nglRenderNode::Render()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    func(this);
}

void nglRenderNode::GetSortInfo(nglSortInfo &a1)
{
    void(__fastcall * func)(void *, void *, nglSortInfo *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this, nullptr, &a1);
}
