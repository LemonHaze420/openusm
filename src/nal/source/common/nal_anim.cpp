#include "nal_anim.h"

#include "common.h"
#include "nal_anim_comp.h"
#include "nal_skeleton.h"
#include "nal_system.h"
#include "vtbl.h"

VALIDATE_SIZE(nalAnimClass<nalAnyPose>::nalInstanceClass, 0x14);

template<>
nalAnimClass<nalAnyPose>::nalInstanceClass::nalInstanceClass(
        nalAnimClass<nalAnyPose> *a2,
        nalBaseSkeleton *a3)
{
    this->m_vtbl = 0x0087E688;

    float v3 = a2->field_38;
    this->field_4 = a2->field_38;
    float v4;
    if ( equal(v3, 0.0f) ) {
        v4 = 0.0f;
    } else {
        v4 = 1.0f / v3;
    }

    auto *Skeleton = a3;
    this->field_8 = v4;
    if ( a3 == nullptr ) {
        Skeleton = CAST(Skeleton, a2->Skeleton);
    }

    this->field_C = Skeleton;
    this->field_10 = a2;
    ++a2->InstanceCount;
}

template<>
void * nalAnimClass<nalAnyPose>::nalInstanceClass::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

template<>
void nalAnimClass<nalAnyPose>::nalInstanceClass::operator delete(void *ptr)
{
    tlMemFree(ptr);
}

template<>
nalAnimClass<nalAnyPose>::nalInstanceClass::~nalInstanceClass()
{
    this->m_vtbl = 0x0087E688;
    --this->field_10->InstanceCount;
}

template<>
void nalAnimClass<nalAnyPose>::nalInstanceClass::finalize(
        bool a2)
{
    this->~nalInstanceClass();

    if ( (a2 & 1) != 0 ) {
        delete(this);
    }
}

nalAnyPose::nalAnyPose(nalBaseSkeleton *a2)
{
    this->field_0 = a2->VirtualCreatePose();
}

nalAnyPose::nalAnyPose(const nalBasePose *a2, bool a3)
{
    auto *skel = a2->GetSkeleton();
    this->field_0 = skel->VirtualCreatePose();
    if ( a3 )
    {
        auto *v4 = this->GetSkeleton();
        v4->VirtualCopyPose(this->field_0, a2);
    }
}

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

nalBaseInstance::nalBaseInstance(nalAnimClass<nalAnyPose> *a2, nalBaseSkeleton *a3) :
    nalAnimClass<nalAnyPose>::nalInstanceClass(a2, a3)
{
    this->m_vtbl = 0x008AA28C;
}

void nalBaseInstance::VirtualGetPose(
        Float a1,
        Float a2,
        nalBasePose *a3,
        const nalBasePose *a4)
{
    void (__fastcall *func)(void *, void *edx,
            Float,
            Float,
            nalBasePose *,
            const nalBasePose *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this, nullptr, a1, a2, a3, a4);
}

