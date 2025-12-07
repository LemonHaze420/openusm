#include "nal_anim_comp.h"

#include "common.h"
#include "component.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"
#include "vtbl.h"

#include <cassert>

VALIDATE_SIZE(nalComp::nalCompSkeleton, 0x7C);

void nalComp::nalCompSkeleton::CopyPose(nalComp::nalCompPose &a1, const nalComp::nalCompPose &a2)
{
    a1 = a2;
}

void nalComp::nalCompSkeleton::VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2)
{
    if constexpr (0)
    {
        const nalComp::nalCompPose *v2 = nullptr;
        if ( a2 != nullptr ) {
            v2 = (const nalComp::nalCompPose *)&a2[-1];
        }

        nalComp::nalCompPose *v3 = nullptr;
        if ( a1 != nullptr ) {
            v3 = (nalComp::nalCompPose *)&a1[-1];
        }

        this->CopyPose(*v3, *v2);
    }
    else
    {
        void * (__fastcall *func)(void *, void *, nalBasePose *a1, const nalBasePose *) = CAST(func, get_vfunc(m_vtbl, 0x30));
        func(this, nullptr, a1, a2);
    }
}

void nalComp::nalCompSkeleton::VirtualBlend(
        nalBasePose *a1,
        Float a2,
        const nalBasePose *a3,
        const nalBasePose *a4)
{
    const nalCompPose *v8 = nullptr;
    if ( a4 != nullptr ) {
        v8 = (const nalCompPose *)&a4[-1];
    }

    const nalCompPose *v7 = nullptr;
    if ( a3 ) {
        v7 = (const nalCompPose *)&a3[-1];
    }

    nalCompPose *v6 = nullptr;
    if ( a1 != nullptr ) {
        v6 = (nalCompPose *)&a1[-1];
    }

    nalComp::Blend(*v6, a2, *v7, *v8);
}

int nalComp::nalCompSkeleton::ConvertCompIxToPoseIx(uint32_t a2) const
{
    auto *v2 = this->field_70;
    if ( (v2[a2].field_8 & 1) != 0 ) {
        return -1;
    }

    uint32_t NumComponents = this->m_iNumComponents;
    int result = -1;
    auto *v6 = (char *)&v2->field_8;

    for ( uint32_t v5 {0}; v5 < NumComponents; ++v5 )
    {
        if ( (*v6 & 1) == 0 ) {
            ++result;
        }

        if ( v5 == a2 ) {
            break;
        }

        v6 += 12;
    }

    return result;
}

int nalComp::nalCompSkeleton::GetComponentPoseDataOffset(uint32_t iCompIx) const
{
    auto iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);

    auto *pDirectory = bit_cast<int *>(this->field_78);
    assert(pDirectory != nullptr && "Cannot ask for a component pose data offset if there is no default pose data.");

    assert(pDirectory[0] > iPoseIx);

    return pDirectory[iPoseIx + 1];
}

char * nalComp::nalCompSkeleton::GetCompDefaultPoseData(int iCompIx) const
{
    assert(iCompIx < this->m_iNumComponents && "Asked for a component index that doesn't exist.");

    int iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);
    if ( iPoseIx == -1 ) {
        return nullptr;
    }

    int *pDirectory = bit_cast<int *>(this->field_78);
    assert(pDirectory[0] > iPoseIx && "Offset to PoseData directory was too big.");

    return &this->field_78[pDirectory[iPoseIx + 1]];
}

char * nalComp::nalCompSkeleton::GetCompPerSkelDataInt(int iCompIx) const
{
    TRACE("nalCompSkeleton::GetCompPerSkelDataInt");

    assert(iCompIx < this->m_iNumComponents && "Asked for a component index that doesn't exist.");

    if ( !this->field_70[iCompIx].sub_671D5F(2) ) {
        return nullptr;
    }

    uint32_t iOffsetIx = 0;
    for ( int i = 0; i < iCompIx; ++i )
    {
        if ( this->field_70[i].sub_671D5F(2) ) {
            ++iOffsetIx;
        }
    }

    uint32_t *pPerSkelDir = bit_cast<uint32_t *>(this->field_74);

    assert(pPerSkelDir[0] > iOffsetIx && "Offset to PerSkel directory was too big.");

    auto result = &this->field_74[pPerSkelDir[iOffsetIx + 1]];
    return result;
}

bool nalComp::nalCompSkeleton::DoesComponentHavePoseTrackData(int a2) const
{
  return this->ConvertCompIxToPoseIx(a2) != -1;
}

void nalComp::nalCompSkeleton::UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents)
{
    TRACE("nalCompSkeleton::UnMash");

    if constexpr (0)
    {
        this->field_70 = CAST(field_70, int(this->field_70) + int(a2));
        this->field_74 += int(a2);

        if (this->field_6C != 0) {
            this->field_78 += int(a2);
        } else {
            this->field_78 = 0;
        }

        for (auto iCompIx = 0u; iCompIx < this->m_iNumComponents; ++iCompIx)
        {
            auto *v9 = this->field_70;
            int v10 = (int) v9[iCompIx].m_component;
            uint32_t v23 = v9[iCompIx].m_name;

            uint32_t iArrayIx;
            for ( iArrayIx = 0; iArrayIx < iNumComponents; ++iArrayIx )
            {
                if ( a3[iArrayIx]->GetType() == v10 )
                {
                    this->field_70[iCompIx].m_component = CAST(this->field_70[iCompIx].m_component, a3[iArrayIx]);
                    auto *CompDefaultPoseData = this->GetCompDefaultPoseData(iCompIx);
                    auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(iCompIx);
                    this->field_70[iCompIx].m_component->SkelPoseProcess(
                        v23,
                        CompPerSkelDataInt,
                        CompDefaultPoseData);

                    break;
                }
            }

            assert(iArrayIx != iNumComponents && "Could not find a component name/type for one of the skel's component name/types");
        }
    }
    else
    {
        THISCALL(0x007378A0, this, a2, a3, iNumComponents);
    }
}

void nalComp::nalCompSkeleton::ReMash(void *a2)
{
    TRACE("nalComp::nalCompSkeleton::ReMash");

    if constexpr (0)
    {
        for ( auto iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx )
        {
            auto *CompDefaultPoseData = this->GetCompDefaultPoseData(iCompIx);
            auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(iCompIx);

            auto *v6 = this->field_70;
            v6[iCompIx].m_component->SkelPoseRelease(
                v6[iCompIx].m_name,
                CompPerSkelDataInt,
                CompDefaultPoseData
            );

            auto *v13 = this->field_70[iCompIx].m_component;
            this->field_70[iCompIx].m_component = (CharComponentBase *) v13->GetType();
        }

        if ( this->field_6C ) {
            this->field_78 -= (int)a2;
        }

        auto v14 = (char *)(this->field_70 - (uint32_t)a2);
        this->field_74 -= (unsigned int)a2;
        this->field_70 = CAST(this->field_70, v14);
    } else {
        THISCALL(0x007379E0, this, a2);
    }
}

int nalComp::nalCompAnim::GetCompPerAnimDataInt(int iCompIx)
{
    TRACE("nalCompAnim::GetCompPerAnimDataInt");

    assert(iCompIx < this->GetSkeleton()->GetNumComponents()
            && "Asked anim for a component that doesn't exist in skeleton.");

    auto *v2 = this->field_40;
    if ( (v2[iCompIx] & 2) == 0 ) {
        return 0;
    }

    int iOffsetIx = 0;
    for ( int i = 0; i < iCompIx; ++i )
    {
        if ( (this->field_40[i] & 2) != 0 ) {
            ++iOffsetIx;
        }
    }

    auto *pPerAnimDataDir = this->field_44;

    assert(*pPerAnimDataDir > iOffsetIx
            && "Bad per-anim data offset.");

    auto result = (int)this->field_44 + pPerAnimDataDir[iOffsetIx + 1];
    return result;
}

int nalComp::nalCompAnim::GetCompAnimTrackData(int iCompIx)
{
    TRACE("nalCompAnim::GetCompAnimTrackData");

    assert(iCompIx < this->GetSkeleton()->GetNumComponents() &&
                "Asked anim for a component that doesn't exist in skeleton.");

    if ( (this->field_40[iCompIx] & 1) == 0 ) {
        return 0;
    }

    int iOffsetIx = 0;
    for ( int i = 0; i < iCompIx; ++i )
    {
        if ( (this->field_40[i] & 1) != 0 ) {
            ++iOffsetIx;
        }
    }

    auto *pTrackDataDir = (int *)this->field_48;
    assert(*pTrackDataDir > iOffsetIx && "Bad track data offset.");

    auto result = pTrackDataDir[iOffsetIx + 1] + this->field_48;
    return result;
}

nalComp::ComponentId nalComp::nalCompSkeleton::GetComponentId(int iCompIx)
{
    auto component = this->GetComponent(iCompIx);
    auto type = component->GetType();
    auto v3 = this->GetName(iCompIx);
    ComponentId result {v3, type};
    return result;
}

CharComponentBase * nalComp::nalCompSkeleton::GetComponent(int iCompIx)
{
    assert(iCompIx < this->m_iNumComponents &&
            "Invalid CompIx. Exceeds m_iNumComponents");

    return bit_cast<CharComponentBase *>(this->field_70[iCompIx].m_component);
}


CharComponentBase * nalComp::nalCompSkeleton::GetComponent(int iCompIx) const
{
    assert(iCompIx < this->m_iNumComponents &&
            "Invalid CompIx. Exceeds m_iNumComponents");
    return bit_cast<CharComponentBase *>(this->field_70[iCompIx].m_component);
}

int nalComp::nalCompSkeleton::GetName(int iCompIx) const
{
    assert(iCompIx < m_iNumComponents &&
            "Invalid CompIx. Exceeds m_iNumComponents");

    return this->field_70[iCompIx].m_name;
}

void nalCompAnim_patch()
{
    {
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::UnMash);
        set_vfunc(0x00891FC8, address);
        set_vfunc(0x008AA300, address);
    }


    {
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::ReMash);
        set_vfunc(0x00891FCC, address);
        set_vfunc(0x008AA304, address);
    }

    /*
    {
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::GetCompPerSkelDataInt);
        REDIRECT(0x00734007, address);
        REDIRECT(0x00733F66, address);
    }

    {
        FUNC_ADDRESS(address, &nalComp::nalCompAnim::GetCompPerAnimDataInt);
        REDIRECT(0x00733FF3, address);
        REDIRECT(0x00733F58, address);
    }

    {
        FUNC_ADDRESS(address, &nalComp::nalCompAnim::GetCompAnimTrackData);
        REDIRECT(0x00733FE6, address);
        REDIRECT(0x00733F4F, address);
    }
    */
}
