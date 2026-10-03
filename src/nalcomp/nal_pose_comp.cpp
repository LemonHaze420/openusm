#include "nal_pose_comp.h"

#include "common.h"
#include "trace.h"
#include <variables.h>
#include <vtbl.h>

#include <nal_anim_comp.h>
#include <nal_skeleton.h>
#include <nal_system.h>

VALIDATE_SIZE(nalComp::nalCompSkeleton, 0x7C);

namespace nalComp {
#if !STANDALONE_SYSTEM
nalCompPose *&pTempStuff = var<nalComp::nalCompPose *>(0x0096F7BC);
#else
nalCompPose *&pTempStuff = []() -> auto & {
    static nalCompPose *g_pTempStuff{};
    return g_pTempStuff;
}();
#endif
}  // namespace nalComp

void nalComp::nalCompSkeleton::GetBoneMatrices(const nalComp::nalCompPose *a2, nalMatrix4x4 *a3) const
{
    for (int idx = 0; idx < this->m_iNumComponents; ++idx) {
        if (!this->m_components[idx].hasFlag(0)) {
            auto *component = this->GetComponent(idx);
            auto name = this->GetName(idx);
            auto *stdPoseData = a2->GetComponentPoseData(idx);
            auto *perSkelData = this->GetCompPerSkelDataInt(idx);

            component->BuildBoneMatrices(a3, name, perSkelData, stdPoseData);
        }
    }
}

int nalComp::nalCompSkeleton::GetBoneMatrixCount() const
{
    return this->m_boneMatrixCount;
}

void nalComp::nalCompSkeleton::GetTrajectoryUpdate(const nalComp::nalCompPose *a2, nalPositionOrientation *a3)
{
    if ((this->field_4C & 1) != 0) {
        for (int i = 0; i < this->GetNumComponents(); ++i) {
            if (this->m_components[i].hasFlag(1)) {
                auto *component = this->GetComponent(i);
                auto *v16 = a2->GetComponentPoseData(i);
                auto *v10 = this->GetCompPerSkelDataInt(i);
                auto name = this->GetName(i);

                nalPositionOrientation result{};
                component->GetTrajectoryData(&result, name, v10, v16);

                *a3 = result;
            }
        }
    } else {
        *a3 = nalPositionOrientation::Identity;
    }
}

int nalComp::nalCompSkeleton::_VirtualGetBoneMatrixCount() const
{
    TRACE("nalCompSkeleton::VirtualGetBoneMatrixCount");

    return this->GetBoneMatrixCount();
}

void nalComp::nalCompSkeleton::_VirtualGetBoneMatrices(const nalBasePose *a1, nalMatrix4x4 *a2) const
{
    TRACE("nalCompSkeleton::VirtualGetBoneMatrices");

    if constexpr (1) {
        if (a1 != nullptr) {
            this->GetBoneMatrices((const nalComp::nalCompPose *)&a1[-1], a2);
        } else {
            this->GetBoneMatrices(nullptr, a2);
        }
    } else {
        void(__fastcall * func)(const void *, void *edx, const nalBasePose *a1, nalMatrix4x4 *a2) =
            CAST(func, 0x005FB520);
        func(this, nullptr, a1, a2);
    }
}

void nalComp::nalCompSkeleton::_VirtualGetTrajectoryUpdate(const nalBasePose *a2, nalPositionOrientation *a3)
{
    if (a2 != nullptr) {
        this->GetTrajectoryUpdate((const nalCompPose *)&a2[-1], a3);
    } else {
        this->GetTrajectoryUpdate(nullptr, a3);
    }
}

void nalComp::nalCompSkeleton::_VirtualGetPose(nalBasePose &, const nalMatrix4x4 *, nalMatrix4x4 *, const nalBasePose &)
{
    ;
}

void nalComp::nalCompSkeleton::CopyPose(nalComp::nalCompPose &a1, const nalComp::nalCompPose &a2)
{
    a1 = a2;
}

void nalComp::nalCompSkeleton::VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2)
{
    if constexpr (0) {
        const nalComp::nalCompPose *v2 = nullptr;
        if (a2 != nullptr) {
            v2 = (const nalComp::nalCompPose *)&a2[-1];
        }

        nalComp::nalCompPose *v3 = nullptr;
        if (a1 != nullptr) {
            v3 = (nalComp::nalCompPose *)&a1[-1];
        }

        this->CopyPose(*v3, *v2);
    } else {
        void *(__fastcall * func)(void *, void *, nalBasePose *a1, const nalBasePose *) =
            CAST(func, get_vfunc(m_vtbl, 0x30));
        func(this, nullptr, a1, a2);
    }
}

void nalComp::nalCompSkeleton::VirtualBlend(nalBasePose *a1, Float a2, const nalBasePose *a3, const nalBasePose *a4)
{
    const nalCompPose *v8 = nullptr;
    if (a4 != nullptr) {
        v8 = (const nalCompPose *)&a4[-1];
    }

    const nalCompPose *v7 = nullptr;
    if (a3) {
        v7 = (const nalCompPose *)&a3[-1];
    }

    nalCompPose *v6 = nullptr;
    if (a1 != nullptr) {
        v6 = (nalCompPose *)&a1[-1];
    }

    nalComp::Blend(*v6, a2, *v7, *v8);
}

int nalComp::nalCompSkeleton::ConvertCompIxToPoseIx(uint32_t iCompIx) const
{
    TRACE("nalCompSkeleton::ConvertCompIxToPoseIx");

    assert(int(iCompIx) < m_iNumComponents && "Asked for a component index that doesn't exist.");

    if (this->m_components[iCompIx].hasFlag(0)) {
        return -1;
    }

    int iPoseIx = -1;
    for (uint32_t i{0}; i < uint32_t(this->m_iNumComponents); ++i) {
        if (!this->m_components[i].hasFlag(0)) {
            ++iPoseIx;
        }

        if (i == iCompIx) {
            break;
        }
    }

    assert(iPoseIx != -1 && "A skeleton has pose data, but no component takes responsibility for this.");

    return iPoseIx;
}

int nalComp::nalCompSkeleton::GetComponentPoseDataOffset(uint32_t iCompIx) const
{
    TRACE("nalCompSkeleton::GetComponentPoseDataOffset");

    auto iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);

    auto *pDirectory = bit_cast<int *>(this->m_pDirectory);
    assert(pDirectory != nullptr && "Cannot ask for a component pose data offset if there is no default pose data.");

    assert(pDirectory[0] > iPoseIx);

    return pDirectory[iPoseIx + 1];
}

char *nalComp::nalCompSkeleton::GetCompDefaultPoseData(int iCompIx) const
{
    TRACE("nalCompSkeleton::GetCompDefaultPoseData");

    assert(iCompIx < this->m_iNumComponents && "Asked for a component index that doesn't exist.");

    int iPoseIx = this->ConvertCompIxToPoseIx(iCompIx);
    if (iPoseIx == -1) {
        return nullptr;
    }

    int *pDirectory = bit_cast<int *>(this->m_pDirectory);
    assert(pDirectory[0] > iPoseIx && "Offset to PoseData directory was too big.");

    return this->m_pDirectory + pDirectory[iPoseIx + 1];
}

char *nalComp::nalCompSkeleton::GetCompPerSkelDataInt(int iCompIx) const
{
    TRACE("nalCompSkeleton::GetCompPerSkelDataInt");

    assert(iCompIx < this->m_iNumComponents && "Asked for a component index that doesn't exist.");

    if (!this->m_components[iCompIx].hasFlag(2)) {
        return nullptr;
    }

    uint32_t iOffsetIx = 0;
    for (int i = 0; i < iCompIx; ++i) {
        if (this->m_components[i].hasFlag(2)) {
            ++iOffsetIx;
        }
    }

    uint32_t *pPerSkelDir = bit_cast<uint32_t *>(this->m_pPerSkelDir);

    assert(pPerSkelDir[0] > iOffsetIx && "Offset to PerSkel directory was too big.");

    auto *result = this->m_pPerSkelDir + pPerSkelDir[iOffsetIx + 1];
    return result;
}

void *nalComp::nalCompSkeleton::_GetPerSkelDataFromComponent(nalComp::ComponentId a2)
{
    int CompIxFromName = this->GetCompIxFromName(a2);
    if (CompIxFromName == -1) {
        return nullptr;
    }

    auto *perSkelData = this->GetCompPerSkelDataInt(CompIxFromName);
    auto *component = this->GetComponent(CompIxFromName);
    return component->ApplyPublicPerSkelDataOffset(a2.field_0, perSkelData);
}

bool nalComp::nalCompSkeleton::_DoesComponentHavePoseTrackData(int a2) const
{
    TRACE("nalCompSkeleton::DoesComponentHavePoseTrackData");

    return this->ConvertCompIxToPoseIx(a2) != -1;
}

bool nalComp::nalCompSkeleton::DoesComponentHavePoseTrackData(int a2) const
{
    bool(__fastcall * func)(const void *, void *edx, int) = CAST(func, get_vfunc(m_vtbl, 0x3C));
    return func(this, nullptr, a2);
}

void nalComp::nalCompSkeleton::_UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents)
{
    TRACE("nalCompSkeleton::UnMash");

    if constexpr (1) {
        this->m_components = CAST(m_components, int(this->m_components) + int(a2));
        this->m_pPerSkelDir += int(a2);

        if (this->m_poseDataSize != 0) {
            this->m_pDirectory += int(a2);
        } else {
            this->m_pDirectory = nullptr;
        }

        for (auto iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx) {
            auto *v9 = this->m_components;
            uint32_t type = (int)v9[iCompIx].m_component;
            uint32_t name = v9[iCompIx].m_name;

            auto begin = a3;
            auto end = a3 + iNumComponents;

            auto it = std::find_if(begin, end, [type](BaseComponent *comp) { return comp->GetType() == type; });

            assert(it != end && "Could not find a component name/type for one of the skel's component name/types");

            this->m_components[iCompIx].m_component = (*it);
            auto *CompDefaultPoseData = this->GetCompDefaultPoseData(iCompIx);
            auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(iCompIx);
            this->m_components[iCompIx].m_component->SkelPoseProcess(name, CompPerSkelDataInt, CompDefaultPoseData);
        }

    } else {
        THISCALL(0x007378A0, this, a2, a3, iNumComponents);
    }
}

void nalComp::nalCompSkeleton::UnMash(void *a2, BaseComponent **a3, unsigned int iNumComponents)
{
    void(__fastcall * func)(const void *, void *edx, void *, BaseComponent **, unsigned int) =
        CAST(func, get_vfunc(m_vtbl, 0x40));
    func(this, nullptr, a2, a3, iNumComponents);
}


void nalComp::nalCompSkeleton::_ReMash(void *a2)
{
    TRACE("nalComp::nalCompSkeleton::ReMash");

    if constexpr (0) {
        for (auto iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx) {
            auto *CompDefaultPoseData = this->GetCompDefaultPoseData(iCompIx);
            auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(iCompIx);

            auto *v6 = this->m_components;
            v6[iCompIx].m_component->SkelPoseRelease(v6[iCompIx].m_name, CompPerSkelDataInt, CompDefaultPoseData);

            auto *v13 = this->m_components[iCompIx].m_component;
            this->m_components[iCompIx].m_component = (CharComponentBase *)v13->GetType();
        }

        if (this->m_poseDataSize != 0) {
            this->m_pDirectory -= (int)a2;
        }

        auto v14 = (char *)(this->m_components - (uint32_t)a2);
        this->m_pPerSkelDir -= (unsigned int)a2;
        this->m_components = CAST(this->m_components, v14);
    } else {
        THISCALL(0x007379E0, this, a2);
    }
}

void nalComp::nalCompSkeleton::ReMash(void *a2)
{
    void(__fastcall * func)(const void *, void *edx, void *) = CAST(func, get_vfunc(m_vtbl, 0x44));
    func(this, nullptr, a2);
}

nalComp::ComponentId nalComp::nalCompSkeleton::GetComponentId(int iCompIx)
{
    auto component = this->GetComponent(iCompIx);
    auto type = component->GetType();
    auto v3 = this->GetName(iCompIx);
    ComponentId result{v3, type};
    return result;
}

int nalComp::nalCompSkeleton::GetCompIxFromName(nalComp::ComponentId a2) const
{
    if (!this->m_iNumComponents) {
        return -1;
    }

    for (int i = 0; i < this->m_iNumComponents; ++i) {
        if (this->GetComponent(i)->GetType() == a2.field_4 && a2.field_0 == this->GetName(i)) {
            return i;
        }
    }

    return -1;
}

BaseComponent *nalComp::nalCompSkeleton::GetComponent(int iCompIx)
{
    assert(iCompIx < this->m_iNumComponents && "Invalid CompIx. Exceeds m_iNumComponents");

    return this->m_components[iCompIx].m_component;
}

BaseComponent *nalComp::nalCompSkeleton::GetComponent(int iCompIx) const
{
    assert(iCompIx < this->m_iNumComponents && "Invalid CompIx. Exceeds m_iNumComponents");
    return this->m_components[iCompIx].m_component;
}

int nalComp::nalCompSkeleton::GetName(int iCompIx) const
{
    assert(iCompIx < m_iNumComponents && "Invalid CompIx. Exceeds m_iNumComponents");

    return this->m_components[iCompIx].m_name;
}

void nalComp::Blend(nalComp::nalCompPose &dst, Float blend, const nalComp::nalCompPose &src0,
                    const nalComp::nalCompPose &src1)
{
    assert(src0.GetSkeleton() == src1.GetSkeleton() && "Cannot blend between poses of two different skeletons.");

    auto *v8 = src0.GetSkeleton();
    for (int a1a = 0; a1a < v8->m_iNumComponents; ++a1a) {
        if (v8->DoesComponentHavePoseTrackData(a1a)) {
            auto *v6 = v8->GetComponent(a1a);
            auto *v9 = src1.GetComponentPoseData(a1a);
            auto *v10 = src0.GetComponentPoseData(a1a);
            auto v7 = v8->GetName(a1a);
            auto *v5 = dst.GetComponentPoseData(a1a);
            v6->BlendPoseData(v5, v7, blend, v10, v9);
        }
    }
}

nalComp::nalCompPose::nalCompPose(const nalComp::nalCompSkeleton *a2)
{
    this->m_vtbl = 0x008AA1E4;
    this->field_4 = a2;
    this->m_pTheData = nullptr;
}

nalComp::nalCompPose &nalComp::nalCompPose::operator=(const nalComp::nalCompPose *a2)
{
    auto *v2 = a2->m_pTheData;
    if (v2 != nullptr) {
        if (this->m_pTheData == nullptr) {
            this->InitializePoseDataFromSkel();
        }

        this->CopyPoseData(v2);
    } else {
        this->FreePoseData();
    }

    return (*this);
}

void nalComp::nalCompPose::CopyPoseDataNoFree(const void *a2)
{
    TRACE("nalCompPose::CopyPoseDataNoFree");

    if constexpr (1) {
        this->DirectCopyPoseData(a2);

        auto numComponents = this->GetSkeleton()->GetNumComponents();
        for (int iCompIx = 0; iCompIx < numComponents; ++iCompIx) {
            auto *skel = this->GetSkeleton();
            if (skel->ConvertCompIxToPoseIx(iCompIx) != -1) {
                auto *v11 = bit_cast<void *>(this->GetComponentPoseData(iCompIx));
                auto *component = skel->GetComponent(iCompIx);

                auto name = skel->GetName(iCompIx);
                auto *v16 = static_cast<const char *>(a2) + skel->GetComponentPoseDataOffset(iCompIx);
                component->CopyPoseExtraData(v11, name, v16);
            }
        }
    } else {
        THISCALL(0x00737710, this, a2);
    }
}

void *nalComp::nalCompPose::_GetComponentPoseData(uint32_t a2)
{
    TRACE("nalCompPose::GetComponentPoseData");

    if constexpr (1) {
        if (this->m_pTheData == nullptr) {
            return nullptr;
        }

        auto *data = static_cast<char *>(this->m_pTheData);
        auto *v4 = this->GetSkeleton();
        return data + v4->GetComponentPoseDataOffset(a2);
    } else {
        void *(__fastcall * func)(void *, void *edx, uint32_t) = CAST(func, 0x00737870);
        return func(this, nullptr, a2);
    }
}

void *nalComp::nalCompPose::GetComponentPoseData(uint32_t a2)
{
    void *(__fastcall * func)(void *, void *edx, uint32_t) = CAST(func, get_vfunc(m_vtbl, 0x0));
    return func(this, nullptr, a2);
}

void *nalComp::nalCompPose::_GetComponentPoseData(uint32_t a2) const
{
    TRACE("nalCompPose::GetComponentPoseData");

    if constexpr (1) {
        if (this->m_pTheData == nullptr) {
            return nullptr;
        }

        auto *data = static_cast<char *>(this->m_pTheData);
        auto *v4 = this->GetSkeleton();
        return data + v4->GetComponentPoseDataOffset(a2);
    } else {
        void *(__fastcall * func)(const void *, void *edx, uint32_t) = CAST(func, 0x00737840);
        return func(this, nullptr, a2);
    }
}

void *nalComp::nalCompPose::GetComponentPoseData(uint32_t a2) const
{
    void *(__fastcall * func)(const void *, void *edx, uint32_t) = CAST(func, get_vfunc(m_vtbl, 0x4));
    return func(this, nullptr, a2);
}

int nalComp::nalCompPose::_GetPoseDataSize() const
{
    return this->GetSkeleton()->m_poseDataSize;
}

int nalComp::nalCompPose::GetPoseDataSize() const
{
    int(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    return func(this);
}

int nalComp::nalCompPose::GetPoseDataAlign() const
{
    return this->GetSkeleton()->m_poseDataAlign;
}

void nalComp::nalCompPose::AllocPoseData()
{
    TRACE("nalCompPose::AllocPoseData");

    assert(this->m_pTheData == nullptr && "Must free old data before allocating data");

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
    if (this->m_pTheData != nullptr) {
        tlMemFree(this->m_pTheData);
        this->m_pTheData = nullptr;
    }
}

void nalComp::nalCompPose::InitializePoseDataFromSkel()
{
    auto *v2 = this->GetSkeleton()->m_pDirectory;
    if (v2 != nullptr) {
        this->AllocPoseData();
        this->CopyPoseDataNoFree(v2);
    }
}

void nalComp::nalCompPose::ComponentFreePoseData()
{
    if (this->m_pTheData != nullptr) {
        uint32_t NumComponents = this->GetSkeleton()->GetNumComponents();

        for (uint32_t v3{0}; v3 < NumComponents; ++v3) {
            if (this->GetSkeleton()->ConvertCompIxToPoseIx(v3) != -1) {
                void *v5 = this->GetComponentPoseData(v3);
                auto *v6 = this->GetSkeleton();

                v6->GetComponent(v3)->PoseDataFree(v6->GetName(v3), v5);
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
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::_UnMash);
        set_vfunc(0x00891FC8, address);
        set_vfunc(0x008AA300, address);
    }


    {
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::_ReMash);
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
