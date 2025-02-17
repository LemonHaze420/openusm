#include "nal_pose_comp.h"

#include "common.h"
#include "trace.h"
#include <vtbl.h>

#include <nal_anim_comp.h>
#include <nal_skeleton.h>
#include <nal_system.h>

namespace nalComp {

nalCompPose *&pTempStuff = var<nalCompPose *>(0x0096F7BC);

void Blend(nalCompPose &a1, Float a2, const nalCompPose &src0, const nalCompPose &src1)
{
    assert(src0.GetSkeleton() == src1.GetSkeleton()
        && "Cannot blend between poses of two different skeletons.");

    auto *v8 = src0.GetSkeleton();
    for ( int a1a = 0; a1a < v8->m_iNumComponents; ++a1a )
    {
        if ( v8->DoesComponentHavePoseTrackData(a1a) )
        {
            auto *v6 = v8->field_70[a1a].field_4;
            auto v9 = src1.GetComponentPoseData(a1a);
            auto v10 = src0.GetComponentPoseData(a1a);
            auto v7 = v8->field_70[a1a].field_0;
            auto v5 = a1.GetComponentPoseData(a1a);
            v6->BlendPoseData(v5, v7, a2, v10, v9);
        }
    }
}

nalCompPose::nalCompPose(const nalComp::nalCompSkeleton *a2)
{
    this->m_vtbl = 0x008AA1E4;
    this->field_4 = a2;
    this->m_pTheData = nullptr;
}

void * nalCompPose::GetComponentPoseData(uint32_t a2)
{
    return (void *) THISCALL(0x00737870, this, a2);
}

void * nalCompPose::GetComponentPoseData(uint32_t a2) const
{
    return (void *) THISCALL(0x00737840, this, a2);
}

int nalCompPose::GetPoseDataSize()
{
    return this->field_4->field_6C;
}

int nalCompPose::GetPoseDataAlign()
{
    return this->field_4->field_68;
}

void nalComp::nalCompPose::AllocPoseData()
{
    TRACE("nalCompPose::AllocPoseData");

    assert(this->m_pTheData == nullptr
            && "Must free old data before allocating data");

    nalComp::pTempStuff = this;
    auto v3 = this->GetPoseDataSize();
    auto v4 = nalComp::pTempStuff->GetPoseDataAlign();
    this->m_pTheData = tlMemAlloc(v3, v4, 0);
}

void nalComp::nalCompPose::DirectCopyPoseData(const void *a2)
{
    std::memcpy(this->m_pTheData, a2, this->GetPoseDataSize());
}

void nalCompPose::FreePoseData()
{
    this->ComponentFreePoseData();
    if ( this->m_pTheData != nullptr )
    {
        tlMemFree(this->m_pTheData);
        this->m_pTheData = nullptr;
    }
}

void nalCompPose::ComponentFreePoseData()
{
    if ( this->m_pTheData != nullptr )
    {
        uint32_t NumComponents = this->field_4->GetNumComponents();

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
