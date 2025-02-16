#include "nal_anim_comp.h"

#include "common.h"
#include "component.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(nalComp::nalCompSkeleton, 0x7C);

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

    return &this->field_74[pPerSkelDir[iOffsetIx + 1]];
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
            int v10 = (int) v9[iCompIx].field_4;
            uint32_t v23 = v9[iCompIx].field_0;

            uint32_t iArrayIx;
            for ( iArrayIx = 0; iArrayIx < iNumComponents; ++iArrayIx )
            {
                if ( a3[iArrayIx]->GetType() == v10 )
                {
                    this->field_70[iCompIx].field_4 = CAST(this->field_70[iCompIx].field_4, a3[iArrayIx]);
                    auto *CompDefaultPoseData = this->GetCompDefaultPoseData(iCompIx);
                    auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(iCompIx);
                    this->field_70[iCompIx].field_4->SkelPoseProcess(
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
    THISCALL(0x007379E0, this, a2);
}

int nalComp::nalCompAnim::GetCompPerAnimDataInt(int iCompIx)
{
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

    return (int)this->field_44 + pPerAnimDataDir[iOffsetIx + 1];
}
