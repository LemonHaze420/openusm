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

uint32_t CharComponentBase::_GetType()
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

int CharComponentBase::_DoesContributeToPose(
        uint32_t ,
        const void *,
        const void *)
{
    TRACE("CharComponentBase::DoesContributeToPose");

    assert(this->m_pSubComponent != nullptr
            && "Must have a subcomponent in order to delegate to it.");

    return this->m_pSubComponent->GetType();
}

int CharComponentBase::_GetSizeOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool bIsRemapped)
{
    TRACE("CharComponentBase::GetSizeOfPerInstData");

    if constexpr (1) {
        assert(this->m_pSubComponent != nullptr
                && "Must have a subcomponent in order to delegate to it.");
      
        assert(!bIsRemapped && "Cannot delegate if remapped.");

        return int(this->m_pSubComponent->ApplyPublicPerAnimDataOffset((uint32_t)a3, a4));
    } else {
        int (__fastcall *func)(void *, void *edx, uint32_t, const void *, const void *, const void *, const void *, const void *, bool) = CAST(func, 0x005EC4E0);
        return func(this, nullptr, a2, a3, a4, a5, a6, a7, bIsRemapped);
    }
}

int CharComponentBase::_GetAlignOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool bIsRemapped)
{
    TRACE("CharComponentBase::GetAlignOfPerInstData");

    if constexpr (0) {
        assert(this->m_pSubComponent != nullptr
                && "Must have a subcomponent in order to delegate to it.");

        assert(!bIsRemapped && "Cannot delegate if remapped.");

        return int(this->m_pSubComponent->GetTrajectoryData(
            bit_cast<nalPositionOrientation *>(a3),
            (uint32_t)a4,
            a5,
            a6));
    } else {
        int (__fastcall *func)(
            void *,
            void *edx,
            uint32_t,
            const void *,
            const void *,
            const void *,
            const void *,
            const void *,
            bool) = CAST(func, 0x005EC510);
        return func(this, nullptr, a2, a3, a4, a5, a6, a7, bIsRemapped);
    }
}

void CharComponentBase::_BuildPerInstData(
        void *a2,
        uint32_t ,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *,
        const void *,
        bool bIsRemapped)
{
    TRACE("CharComponentBase::BuildPerInstData");

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

    return bit_cast<void *>(this->m_pSubComponent->GetSizeOfPerInstData(
            bit_cast<uint32_t>(a2),
            bit_cast<const void *>(a4),
            bit_cast<const void *>(a5),
            a6,
            a7,
            a8,
            (bool)a9));
}

int CharComponentBase::GetDomain()
{
    int (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x54));
    return func(this);
}

int CharComponentBase::_GetRemapSizeOfPerInstData(
        uint32_t,
        uint32_t,
        const CharComponentBase *,
        const void *,
        const void *,
        const void *,
        const void *,
        const void *)
{
    return 0;
}

int CharComponentBase::GetRemapSizeOfPerInstData(
        uint32_t a1,
        uint32_t a2,
        const CharComponentBase *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        const void *a8)
{
    int (__fastcall *func)(void *, void *edx, uint32_t, uint32_t a2,
        const CharComponentBase *,
        const void *,
        const void *,
        const void *,
        const void *,
        const void *) = CAST(func, get_vfunc(m_vtbl, 0x5C));
    return func(this, nullptr, a1, a2, a3, a4, a5, a6, a7, a8);
}

int CharComponentBase::_GetRemapAlignOfPerInstData(
        uint32_t,
        uint32_t,
        const CharComponentBase *,
        const void *,
        const void *,
        const void *,
        const void *,
        const void *)
{
    return 0;
}

int CharComponentBase::GetRemapAlignOfPerInstData(
        uint32_t a2,
        uint32_t a3,
        const CharComponentBase *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        const void *a8,
        const void *a9)
{
    int (__fastcall *func)(void *, void *edx,
        uint32_t,
        uint32_t,
        const CharComponentBase *,
        const void *,
        const void *,
        const void *,
        const void *,
        const void *) = CAST(func, get_vfunc(m_vtbl, 0x60));
    return func(this, nullptr, a2, a3, a4, a5, a6, a7, a8, a9);
}

void CharComponentBase::BuildRemapPerInstData(
        void *a2,
        uint32_t a3,
        uint32_t a4,
        const CharComponentBase *a5,
        const void *a6,
        const void *a7,
        const void *a8,
        const void *a9,
        const void *a10)
{
    void * (__fastcall *func)(
            void *, void *edx,
            void *,
            uint32_t,
            uint32_t,
            const CharComponentBase *,
            const void *,
            const void *,
            const void *,
            const void *,
            const void *) = CAST(func, get_vfunc(m_vtbl, 0x64));

    func(this, nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

void CharComponentBase::DestroyRemapPerInstData(
    void *a2,
    uint32_t a3,
    uint32_t a4,
    const CharComponentBase *a5,
    const void *a6,
    const void *a7)
{
    void * (__fastcall *func)(
            void *, void *edx,
            void *,
            uint32_t,
            uint32_t,
            const CharComponentBase *,
            const void *,
            const void *) = CAST(func, get_vfunc(m_vtbl, 0x68));

    func(this, nullptr, a2, a3, a4, a5, a6, a7);
}

void CharComponentBase::_AnimRelease(uint32_t, void *, void *)
{}

void CharComponentBase::CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3)
{
    void * (__fastcall *func)(void *, void *, void *, uint32_t, const void *) = CAST(func, get_vfunc(m_vtbl, 0x74));
    func(this, nullptr, a1, a2, a3);
}

void * CharComponentBase::_AllocTempPoseData(uint32_t ,
        const void *, const void *)
{
    return nullptr;
}

void * CharComponentBase::AllocTempPoseData(uint32_t a1, const void *a2, const void *a3)
{
    void * (__fastcall *func)(void *, void *, uint32_t, const void *, const void *) = CAST(func, get_vfunc(m_vtbl, 0x78));
    return func(this, nullptr, a1, a2, a3);
}

void CharComponentBase::DeleteTempPoseData(uint32_t a1, void *a2)
{
      void (__fastcall *func)(void *, void *, uint32_t, void *) = CAST(func, get_vfunc(m_vtbl, 0x7C));
      func(this, nullptr, a1, a2);
}

void CharComponentBase_patch()
{
    {
        auto constexpr address_vtbl = 0x00891A68;

        set_vfunc(address_vtbl + 0x18, func_address(&CharComponentBase::_DoesContributeToPose));
        set_vfunc(address_vtbl + 0x1C, func_address(&CharComponentBase::_GetSizeOfPerInstData));
    }

    {
        FUNC_ADDRESS(address, &CharComponentBase::_CalcPoseDataDirect);
        set_vfunc(0x00891A98, address);
    }
}
