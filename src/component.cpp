#include "component.h"

#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"


void * BaseComponent::ApplyPublicPerSkelDataOffset(uint32_t a1, void *a2) const
{
    void * (__fastcall *func)(const void *, void *, uint32_t, void *) = CAST(func, get_vfunc(this->m_vtbl, 0x8));
    return func(this, nullptr, a1, a2);
}

void * BaseComponent::ApplyPublicPerAnimDataOffset(uint32_t a1, const void *a2) const
{
    void * (__fastcall *func)(const void *, void *, uint32_t, const void *) = CAST(func, get_vfunc(this->m_vtbl, 0xC));
    return func(this, nullptr, a1, a2);
}

nalPositionOrientation * BaseComponent::GetTrajectoryData(
        nalPositionOrientation *out,
        uint32_t a2,
        const void *a3,
        const void *a4)
{

    nalPositionOrientation * (__fastcall *func)(
        void *self,
        void *edx,
        nalPositionOrientation *out,
        uint32_t a2,
        const void *a3,
        const void *a4) = CAST(func, get_vfunc(this->m_vtbl, 0x10));
    return func(this, nullptr, out, a2, a3, a4);
}

void BaseComponent::BuildBoneMatrices(
        nalMatrix4x4 *a1,
        uint32_t a2,
        const void *a3,
        const void *a4)
{
    void (__fastcall *func)(
        void *self,
        void *edx,
        nalMatrix4x4 *,
        uint32_t a2,
        const void *a3,
        const void *a4) = CAST(func, get_vfunc(this->m_vtbl, 0x14));
    return func(this, nullptr, a1, a2, a3, a4);

}

void * BaseComponent::GetSizeOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool a8)
{

    void * (__fastcall *func)(
        void *self,
        void *,
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool a8) = CAST(func, get_vfunc(this->m_vtbl, 0x1C));
    return func(this, nullptr, a2, a3, a4, a5, a6, a7, a8);
}

void BaseComponent::BlendPoseData(void *a1, uint32_t a2, Float a3,const void *a4, const void *a5)
{
    void (__fastcall *func)(void *, void *, void *, uint32_t, Float, const void *, const void *) = CAST(func, get_vfunc(this->m_vtbl, 0x38));
    func(this, nullptr, a1, a2, a3, a4, a5);
}


void BaseComponent::SkelPoseProcess(uint32_t a1, void *a2, void *a3) const
{
    void (__fastcall *func)(const void *, void *, uint32_t, void *, void *) = CAST(func, get_vfunc(this->m_vtbl, 0x3C));
    func(this, nullptr, a1, a2, a3);
}

void BaseComponent::SkelPoseRelease(uint32_t a1, void *a2, void *a3) const
{
    void (__fastcall *func)(const void *, void *, uint32_t, void *, void *) = CAST(func, get_vfunc(this->m_vtbl, 0x40));
    func(this, nullptr, a1, a2, a3);
}

void BaseComponent::PoseDataFree(uint32_t a2, void *a3) const
{
    void (__fastcall *func)(const void *, void *, uint32_t, void *) = CAST(func, get_vfunc(this->m_vtbl, 0x50));
    func(this, nullptr, a2, a3);
}

