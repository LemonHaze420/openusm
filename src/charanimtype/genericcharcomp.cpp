#include "genericcharcomp.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "nal_generic.h"
#include "nal_list.h"
#include "nal_system.h"
#include "tl_instance_bank.h"
#include "utility.h"
#include "vtbl.h"

#include <cstring>
#include <new>

GenericCharComp::GenericCharComp()
{

    static void *table[]{
        func_address(&GenericCharComp::_DestroyComponent),
        func_address(&CharComponentBase::_GetType),
        func_address(&GenericCharComp::_ApplyPublicPerSkelDataOffset),
        func_address(&GenericCharComp::_ApplyPublicPerAnimDataOffset),
        func_address(&GenericCharComp::_GetTrajectoryData),
        func_address(&GenericCharComp::BuildBoneMatrices),
        func_address(&GenericCharComp::_DoesContributeToPose),
        func_address(&GenericCharComp::_GetSizeOfPerInstData),
        func_address(&GenericCharComp::_GetAlignOfPerInstData),
        func_address(&GenericCharComp::_BuildPerInstData),
        func_address(&GenericCharComp::_DestroyPerInstData),
        func_address(&GenericCharComp::_WillMapToComponentData),
        func_address(&GenericCharComp::CalcPoseDataDirect),
        func_address(&GenericCharComp::_CalcPoseDataRemapped),
        func_address(&GenericCharComp::_BlendPoseData),
        func_address(&GenericCharComp::_SkelPoseProcess),
        func_address(&GenericCharComp::_SkelPoseRelease),
        func_address(&GenericCharComp::_AnimProcess),
        func_address(&GenericCharComp::_AnimRelease),
        func_address(&GenericCharComp::_CopyPoseExtraData),
        func_address(&GenericCharComp::_PoseDataFree),
        func_address(&GenericCharComp::_GetDomain),
        func_address(&GenericCharComp::_GetPoseTypeID),
        func_address(&CharComponentBase::_GetRemapSizeOfPerInstData),
        func_address(&CharComponentBase::_GetRemapAlignOfPerInstData),
        func_address(&CharComponentBase::_BuildRemapPerInstData),
        func_address(&CharComponentBase::_DestroyRemapPerInstData),
        func_address(&CharComponentBase::_CalcPoseDataRemapped),
        func_address(&CharComponentBase::_AnimRelease),
        func_address(&GenericCharComp::_CopyPoseDataToNothing),
        func_address(&CharComponentBase::_AllocTempPoseData),
        func_address(&CharComponentBase::_DeleteTempPoseData)};
    m_vtbl = CAST(m_vtbl, table);
    m_strTypeString = "Generic";
    CharComponentManager::RegisterComponent(this);
}

GenericCharComp *GenericCharComp::_DestroyComponent(unsigned char flags)
{
    this->~GenericCharComp();
    if (flags & 1) operator delete(this);
    return this;
}

nalPositionOrientation *GenericCharComp::_GetTrajectoryData(nalPositionOrientation *out, uint32_t,
    const void *, const void *data)
{
    auto *pose = static_cast<const nalGeneric::nalGenericPose *>(data);
    pose->GetSkeleton()->GetTrajectoryData(pose, out);
    return out;
}

void GenericCharComp::BuildBoneMatrices(nalMatrix4x4 *out, uint32_t, const void *skel, const void *pose)
{
    static_cast<const nalGeneric::nalGenericSkeleton *>(skel)->GetBoneMatrices(
        static_cast<const nalGeneric::nalGenericPose *>(pose), out);
}

void GenericCharComp::_BuildPerInstData(void *out, uint32_t, const void *skel, const void *, const void *,
    const void *anim, const void *, bool)
{
    if (out) new (out) nalGeneric::nalGenericInstance(
        const_cast<nalGeneric::nalGenericAnim *>(static_cast<const nalGeneric::nalGenericAnim *>(anim)),
        const_cast<nalGeneric::nalGenericSkeleton *>(static_cast<const nalGeneric::nalGenericSkeleton *>(skel)));
}

void GenericCharComp::_DestroyPerInstData(void *data, uint32_t, const void *, const void *)
{
    static_cast<nalGeneric::nalGenericInstance *>(data)->~nalGenericInstance();
}

void GenericCharComp::CalcPoseDataDirect(void *out, uint32_t, Float time, Float weight,
    const nalComp::nalCompAnim *, const void *, const void *, const void *, void *instance)
{
    auto *pose = static_cast<nalGeneric::nalGenericPose *>(out);
    nalGeneric::nalGenericPose source{*pose, true};
    static_cast<nalGeneric::nalGenericInstance *>(instance)->GetPose(time, weight, *pose, source);
}

void GenericCharComp::_CalcPoseDataRemapped(void *out, uint32_t index, Float time, Float weight,
    const nalComp::nalCompAnim *anim, const void *skel, uint32_t, uint32_t,
    const void *animData, const void *tracks, void *instance)
{
    CalcPoseDataDirect(out, index, time, weight, anim, skel, animData, tracks, instance);
}

void GenericCharComp::_BlendPoseData(void *out, uint32_t, Float weight, const void *a, const void *b)
{
    nalGeneric::Blend(static_cast<nalGeneric::nalGenericPose *>(out), weight,
        static_cast<const nalGeneric::nalGenericPose *>(a), static_cast<const nalGeneric::nalGenericPose *>(b));
}

void GenericCharComp::_SkelPoseProcess(uint32_t, void *data, void *poseData)
{
    if (data) {
        auto *skel = static_cast<nalGeneric::nalGenericSkeleton *>(data);
        auto *entry = nalTypeInstanceBank.Search(skel->AnimTypeName);
        skel->m_vtbl = static_cast<nalInitListAnimType *>(entry->field_20)->skel_vtbl_ptr;
        skel->CheckVersion();
        skel->Process();
        *reinterpret_cast<std::intptr_t *>(&skel->field_50) = 0;
        auto *pose = new (poseData) nalGeneric::nalGenericPose;
        pose->field_0 = skel;
        pose->field_4 += reinterpret_cast<std::intptr_t>(pose);
    }
}

void GenericCharComp::_SkelPoseRelease(uint32_t, void *data, void *poseData)
{
    if (data) {
        auto *pose = static_cast<nalGeneric::nalGenericPose *>(poseData);
        const auto offset = pose->field_4 - reinterpret_cast<std::intptr_t>(pose);
        pose->~nalGenericPose();
        pose->field_4 = offset;
        static_cast<nalGeneric::nalGenericSkeleton *>(data)->Release();
    }
}

void GenericCharComp::_AnimProcess(uint32_t, void *data, void *, const void *skelData)
{
    auto *anim = static_cast<nalGeneric::nalGenericAnim *>(data);
    auto *skel = const_cast<nalGeneric::nalGenericSkeleton *>(static_cast<const nalGeneric::nalGenericSkeleton *>(skelData));
    anim->field_30 = skel;
    auto *entry = nalTypeInstanceBank.Search(skel->AnimTypeName);
    anim->m_vtbl = static_cast<nalInitListAnimType *>(entry->field_20)->anim_vtbl_ptr;
    anim->CheckVersion();
    anim->field_3C = 0;
    anim->Process();
}

void GenericCharComp::_AnimRelease(uint32_t, void *anim, void *, const void *)
{
    static_cast<nalGeneric::nalGenericAnim *>(anim)->Release();
}

void GenericCharComp::_CopyPoseExtraData(void *out, uint32_t, const void *pose)
{
    *static_cast<nalGeneric::nalGenericPose *>(out) = *static_cast<const nalGeneric::nalGenericPose *>(pose);
}

void GenericCharComp::_PoseDataFree(uint32_t, void *pose)
{
    static_cast<nalGeneric::nalGenericPose *>(pose)->~nalGenericPose();
}

void GenericCharComp::_CopyPoseDataToNothing(void *out, uint32_t, const void *data)
{
    auto *source = static_cast<const nalGeneric::nalGenericPose *>(data);
    auto *pose = new (out) nalGeneric::nalGenericPose;
    pose->field_0 = source->field_0;
    pose->field_4 = reinterpret_cast<std::intptr_t>(pose) + source->field_4
        - reinterpret_cast<std::intptr_t>(source);
    pose->ConstructEmptyData();
    *pose = *source;
}

void GenericCharComp_patch()
{
    FUNC_ADDRESS(address, &GenericCharComp::CalcPoseDataDirect);
    set_vfunc(0x00892170, address);
}
