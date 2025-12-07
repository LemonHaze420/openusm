#include "charcomponentbase.h"

#include "common.h"
#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cassert>

VALIDATE_SIZE(CharComponentBase, 0x14);

CharComponentBase::CharComponentBase()
{
    this->m_vtbl = 0x00891A68;
    this->m_pSubComponent = nullptr;
    this->m_TheType = 0;
    this->m_strTypeString = nullptr;
    this->field_10 = 0;
}


uint32_t CharComponentBase::GetType()
{
    TRACE("CharComponentBase::GetType");

    if ( this->m_TheType == 0 ) {
        if ( this->m_strTypeString != nullptr ) {
            this->m_TheType = to_hash(this->m_strTypeString);
        } else {
            assert(0 && "You did not override GetType. You need to do this or set m_TheType or set m_strTypeString.");
        }
    }

    return this->m_TheType;
}

int CharComponentBase::DoesContributeToPose(
        uint32_t ,
        const void *,
        const void *)
{
    assert(this->m_pSubComponent != nullptr
            && "Must have a subcomponent in order to delegate to it.");

    return this->m_pSubComponent->GetType();
}

void * CharComponentBase::GetSizeOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool bIsRemapped)
{
    TRACE("CharComponentBase::GetSizeOfPerInstData");

    if constexpr (0) {
        assert(this->m_pSubComponent != nullptr
                && "Must have a subcomponent in order to delegate to it.");
      
        assert(!bIsRemapped && "Cannot delegate if remapped.");

        return this->m_pSubComponent->ApplyPublicPerAnimDataOffset((uint32_t)a3, a4);
    } else {
        return (void *) THISCALL(0x005EC4E0, this, a2, a3, a4, a5, a6, a7, bIsRemapped);
    }
}

void * CharComponentBase::GetAlignOfPerInstData(
        uint32_t ,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *,
        bool bIsRemapped)
{
    assert(this->m_pSubComponent != nullptr
            && "Must have a subcomponent in order to delegate to it.");

    assert(!bIsRemapped && "Cannot delegate if remapped.");

    return this->m_pSubComponent->GetTrajectoryData(
        bit_cast<nalPositionOrientation *>(a3),
        (uint32_t)a4,
        a5,
        a6);
}

void CharComponentBase::BuildPerInstData(
        void *a2,
        uint32_t ,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *,
        const void *,
        bool bIsRemapped)
{
    assert(this->m_pSubComponent != nullptr
            && "Must have a subcomponent in order to delegate to it.");

    assert(!bIsRemapped
            && "Cannot delegate if remapped.");

    this->m_pSubComponent->BuildBoneMatrices(
           (nalMatrix4x4 *)a2,
           (uint32_t)a4,
           a5,
           a6);
}

void * CharComponentBase::_CalcPoseDataDirect(
        void *a2,
        uint32_t ,
        Float a4,
        Float a5,
        const nalComp::nalCompAnim *a6,
        const void *a7,
        const void *a8,
        const void *a9,
        void *)
{
    TRACE("CharComponentBase::CalcPoseDataDirect");

    assert(this->m_pSubComponent != nullptr && "Must have a subcomponent in order to delegate to it.");

    return this->m_pSubComponent->GetSizeOfPerInstData(
            bit_cast<uint32_t>(a2),
            bit_cast<const void *>(a4),
            bit_cast<const void *>(a5),
            a6,
            a7,
            a8,
            (bool)a9);
}

void CharComponentBase::CalcPoseDataRemapped(
        void *,
        uint32_t ,
        Float ,
        Float ,
        const nalComp::nalCompAnim *,
        const void *,
        const void *,
        const void *,
        void *)
{
    TRACE("CharComponentBase::CalcPoseDataRemapped");
}

void CharComponentBase::CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3)
{
    void * (__fastcall *func)(void *, void *, void *, uint32_t, const void *) = CAST(func, get_vfunc(m_vtbl, 0x74));
    func(this, nullptr, a1, a2, a3);
}

void CharComponentBase_patch()
{
    FUNC_ADDRESS(address, &CharComponentBase::GetType);
    SET_JUMP(0x005EC4B0, address);

    {
        FUNC_ADDRESS(address, &CharComponentBase::GetSizeOfPerInstData);
        set_vfunc(0x00891A84, address);
    }

    {
        FUNC_ADDRESS(address, &CharComponentBase::_CalcPoseDataDirect);
        set_vfunc(0x00891A98, address);
    }
}
