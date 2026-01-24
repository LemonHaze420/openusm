#include "nal_pose_comp.h"

#include "common.h"
#include "trace.h"
#include <vtbl.h>

#include <nal_anim_comp.h>
#include <nal_skeleton.h>
#include <nal_system.h>

VALIDATE_SIZE(nalComp::nalCompSkeleton, 0x7C);

namespace nalComp {
    nalCompPose *&pTempStuff = var<nalComp::nalCompPose *>(0x0096F7BC);
}

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

int nalComp::nalCompSkeleton::ConvertCompIxToPoseIx(uint32_t iCompIx) const
{
    assert(int(iCompIx) < m_iNumComponents && "Asked for a component index that doesn't exist.");

    if ( this->field_70[iCompIx].sub_671D5F(0) ) {
        return -1;
    }

    int iPoseIx = -1;
    for ( uint32_t i {0}; i < uint32_t(this->m_iNumComponents); ++i )
    {
        if ( !this->field_70[i].sub_671D5F(0) ) {
            ++iPoseIx;
        }

        if ( i == iCompIx ) {
            break;
        }
    }

    assert(iPoseIx != -1 && "A skeleton has pose data, but no component takes responsibility for this.");

    return iPoseIx;
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

bool nalComp::nalCompSkeleton::_DoesComponentHavePoseTrackData(int a2) const
{
    TRACE("nalCompSkeleton::DoesComponentHavePoseTrackData");

    return this->ConvertCompIxToPoseIx(a2) != -1;
}

bool nalComp::nalCompSkeleton::DoesComponentHavePoseTrackData(int a2) const
{
    bool (__fastcall *func)(const void *, void *edx, int) = CAST(func, get_vfunc(m_vtbl, 0x3C));
    return func(this, nullptr, a2);
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

nalComp::ComponentId nalComp::nalCompSkeleton::GetComponentId(int iCompIx)
{
    auto component = this->GetComponent(iCompIx);
    auto type = component->GetType();
    auto v3 = this->GetName(iCompIx);
    ComponentId result {v3, type};
    return result;
}

int nalComp::nalCompSkeleton::GetCompIxFromName(nalComp::ComponentId a2) const
{
    if ( !this->m_iNumComponents ) {
        return -1;
    }

    for ( int i = 0; i < this->m_iNumComponents; ++i )
    {
        auto &v5 = this->field_70[i];
        if ( v5.m_component->GetType() == int(a2.field_4) && a2.field_0 == v5.m_name ) {
            return i;
        }
    }

    return -1;
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

void nalComp::Blend(nalComp::nalCompPose &a1, Float a2, const nalComp::nalCompPose &src0, const nalComp::nalCompPose &src1)
{
    assert(src0.GetSkeleton() == src1.GetSkeleton()
        && "Cannot blend between poses of two different skeletons.");

    auto *v8 = src0.GetSkeleton();
    for ( int a1a = 0; a1a < v8->m_iNumComponents; ++a1a )
    {
        if ( v8->DoesComponentHavePoseTrackData(a1a) )
        {
            auto *v6 = v8->field_70[a1a].m_component;
            auto v9 = src1.GetComponentPoseData(a1a);
            auto v10 = src0.GetComponentPoseData(a1a);
            auto v7 = v8->field_70[a1a].m_name;
            auto v5 = a1.GetComponentPoseData(a1a);
            v6->BlendPoseData(v5, v7, a2, v10, v9);
        }
    }
}

nalComp::nalCompPose::nalCompPose(const nalComp::nalCompSkeleton *a2)
{
    this->m_vtbl = 0x008AA1E4;
    this->field_4 = a2;
    this->m_pTheData = nullptr;
}

nalComp::nalCompPose & nalComp::nalCompPose::operator=(const nalComp::nalCompPose *a2)
{
    auto *v2 = a2->m_pTheData;
    if ( v2 != nullptr )
    {
        if ( this->m_pTheData == nullptr ) {
            this->InitializePoseDataFromSkel();
        }

        this->CopyPoseData(v2);
    }
    else
    {
        this->FreePoseData();
    }

    return (*this);
}

void nalComp::nalCompPose::CopyPoseDataNoFree(const void *a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x00737710, this, a2);
    }
}

void * nalComp::nalCompPose::GetComponentPoseData(uint32_t a2)
{
    return (void *) THISCALL(0x00737870, this, a2);
}

void * nalComp::nalCompPose::GetComponentPoseData(uint32_t a2) const
{
    return (void *) THISCALL(0x00737840, this, a2);
}

int nalComp::nalCompPose::GetPoseDataSize()
{
    return this->field_4->field_6C;
}

int nalComp::nalCompPose::GetPoseDataAlign()
{
    return this->field_4->field_68;
}

void nalComp::nalCompPose::AllocPoseData()
{
    TRACE("nalCompPose::AllocPoseData");

    assert(this->m_pTheData == nullptr
            && "Must free old data before allocating data");

    pTempStuff = this;
    auto v3 = this->GetPoseDataSize();
    auto v4 = pTempStuff->GetPoseDataAlign();
    this->m_pTheData = tlMemAlloc(v3, v4, 0);
}

void nalComp::nalCompPose::CopyPoseData(void *a2)
{
    this->ComponentFreePoseData();
    this->CopyPoseDataNoFree(a2);
}

void nalComp::nalCompPose::DirectCopyPoseData(const void *a2)
{
    std::memcpy(this->m_pTheData, a2, this->GetPoseDataSize());
}

void nalComp::nalCompPose::FreePoseData()
{
    this->ComponentFreePoseData();
    if ( this->m_pTheData != nullptr )
    {
        tlMemFree(this->m_pTheData);
        this->m_pTheData = nullptr;
    }
}

void nalComp::nalCompPose::InitializePoseDataFromSkel()
{
    auto *v2 = this->field_4->field_78;
    if ( v2 != nullptr )
    {
        this->AllocPoseData();
        this->CopyPoseDataNoFree(v2);
    }
}

void nalComp::nalCompPose::ComponentFreePoseData()
{
    if ( this->m_pTheData != nullptr )
    {
        uint32_t NumComponents = this->field_4->GetNumComponents();

        for ( uint32_t v3 {0}; v3 < NumComponents; ++v3 )
        {
            if ( this->field_4->ConvertCompIxToPoseIx(v3) != -1 )
            {
                void *v5 = this->GetComponentPoseData(v3);
                auto &v6 = this->field_4->field_70[v3];

                v6.m_component->PoseDataFree(
                    v6.m_name,
                    v5
                );
            }
        }
    }
}

void nalCompSkeleton_patch()
{
    {
        set_vfunc(0x00732000, func_address(&nalComp::nalCompSkeleton::_DoesComponentHavePoseTrackData));
    }

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
    */
}
