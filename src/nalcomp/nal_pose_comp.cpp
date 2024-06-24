#include "nal_pose_comp.h"

#include "common.h"
#include "trace.h"
#include <vtbl.h>

#include <nal_skeleton.h>
#include <nal_system.h>

namespace nalComp {

VALIDATE_SIZE(nalCompSkeleton, 0x7C);

int nalCompSkeleton::ConvertCompIxToPoseIx(uint32_t a2) const
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

int nalCompSkeleton::GetComponentPoseDataOffset(uint32_t iCompIx) const
{
    auto iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);

    auto *pDirectory = bit_cast<int *>(this->field_78);
    assert(pDirectory != nullptr && "Cannot ask for a component pose data offset if there is no default pose data.");

    assert(pDirectory[0] > iPoseIx);

    return pDirectory[iPoseIx + 1];
}

char * nalCompSkeleton::GetCompDefaultPoseData(uint32_t iCompIx) const
{
    assert(iCompIx < m_iNumComponents && "Asked for a component index that doesn't exist.");

    int iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);
    if ( iPoseIx == -1 ) {
        return nullptr;
    }

    int *pDirectory = bit_cast<int *>(this->field_78);
    assert(pDirectory[0] > iPoseIx && "Offset to PoseData directory was too big.");

    return &this->field_78[pDirectory[iPoseIx + 1]];
}

char * nalCompSkeleton::GetCompPerSkelDataInt(uint32_t iCompIx) const
{
    assert(iCompIx < m_iNumComponents && "Asked for a component index that doesn't exist.");

    if ( !this->field_70[iCompIx].sub_671D5F(2) ) {
        return nullptr;
    }

    uint32_t iOffsetIx = 0;
    for ( uint32_t i = 0; i < iCompIx; ++i )
    {
        if ( this->field_70[i].sub_671D5F(2) ) {
            ++iOffsetIx;
        }
    }

    uint32_t *pPerSkelDir = bit_cast<uint32_t *>(this->field_74);

    assert(pPerSkelDir[0] > iOffsetIx && "Offset to PerSkel directory was too big.");

    return &this->field_74[pPerSkelDir[iOffsetIx + 1]];
}

void nalCompSkeleton::UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents)
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

void nalCompSkeleton::ReMash(void *a2)
{
    THISCALL(0x007379E0, this, a2);
}

nalCompPose::nalCompPose(const nalComp::nalCompSkeleton *a2)
{
    m_vtbl = 0x008AA1E4;
    field_4 = a2;
    field_8 = nullptr;
}

void * nalCompPose::GetComponentPoseData(uint32_t a2)
{
    return (void *) THISCALL(0x00737870, this, a2);
}

void * nalCompPose::GetComponentPoseData(uint32_t a2) const
{
    return (void *) THISCALL(0x00737840, this, a2);
}

void nalCompPose::FreePoseData()
{
    this->ComponentFreePoseData();
    if ( this->field_8 != nullptr )
    {
        tlMemFree(this->field_8);
        this->field_8 = nullptr;
    }
}

void nalCompPose::ComponentFreePoseData()
{
    if ( this->field_8 != nullptr )
    {
        auto NumComponents = this->field_4->GetNumComponents();

        for ( uint32_t v3 {0}; v3 < NumComponents; ++v3 )
        {
            if ( this->field_4->ConvertCompIxToPoseIx(v3) != -1 )
            {
                void *v5 = this->GetComponentPoseData(v3);
                this->field_4->field_70[v3].field_4->PoseDataFree(
                    this->field_4->field_70[v3].field_0,
                    v5);
            }
        }
    }
}

} // namespace nalComp
