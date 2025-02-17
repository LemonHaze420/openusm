#include "nal_anim.h"

#include "common.h"
#include "nal_anim_comp.h"
#include "nal_skeleton.h"
#include "nal_system.h"
#include "vtbl.h"

VALIDATE_SIZE(nalAnimClass<nalAnyPose>::nalInstanceClass, 0x14);

nalComp::nalCompSkeleton * nalAnyPose::GetSkeleton()
{
    return this->field_0->GetSkeleton();
}

void nalAnyPose::operator=(const nalAnyPose &a2)
{
    this->GetSkeleton()->VirtualCopyPose(this->field_0, a2.field_0);
}

template<>
void *nalAnimClass<nalAnyPose>::VirtualCreateInstance(nalBaseSkeleton *Skel)
{
    void * (__fastcall *func)(void *, void *, nalBaseSkeleton *) = CAST(func, get_vfunc(m_vtbl, 0x10));
    return func(this, nullptr, Skel);
}

template<>
void * nalAnimClass<nalAnyPose>::CreateInstance(nalBaseSkeleton *skeleton)
{
    if ( skeleton != nullptr )
    {
        assert((skeleton == nullptr || Compatible(GetSkeleton(),skeleton)) && "attempt to create an instance without a compatible skeleton");
    }

    return this->VirtualCreateInstance(skeleton);
}

bool Compatible(nalBaseSkeleton *a1, nalBaseSkeleton *a2)
{
    return (a2->m_vtbl == a1->m_vtbl);
}

void sub_826140(nalAnyPose &a1, Float a2, nalAnyPose &a3, nalAnyPose &a4)
{
    auto &v6 = *a4.field_0;
    auto &v5 = *a3.field_0;
    auto &v4 = *a1.field_0;
    sub_826190(v4, a2, v5, v6);
}
