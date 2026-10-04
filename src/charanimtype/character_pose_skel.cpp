#include "character_pose_skel.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "tl_system.h"
#include "trace.h"
#include "variables.h"

namespace nalChar {

VALIDATE_SIZE(nalCharSkeleton, 0x84);
VALIDATE_SIZE(nalCharPose, 0x10);

#ifndef STANDALONE_SYSTEM
#error "Not define macro STANDALONE_SYSTEM"
#endif

#if !STANDALONE_SYSTEM
int &nalCharSkeleton::vtbl_ptr = var<int>(0x0096AB90);
#else
int &nalCharSkeleton::vtbl_ptr = []() -> auto & {
    static nalCharSkeleton skel{};
    return skel.m_vtbl;
}();
#endif

nalCharPose::nalCharPose(const nalChar::nalCharSkeleton *a2) : nalCompPose(a2)
{
    if constexpr (0) {
        void *(nalCompPose::*GetComponentPoseData0)(uint32_t) = &nalCompPose::_GetComponentPoseData;
        void *(nalCompPose::*GetComponentPoseData1)(uint32_t) const = &nalCompPose::_GetComponentPoseData;

        static void *g_vtbl[]{func_address(GetComponentPoseData0),
                              func_address(GetComponentPoseData1),
                              func_address(&nalCompPose::_GetPoseDataSize),
                              func_address(&nalCompPose::_GetPoseDataAlign),
                              func_address(&nalCompPose::_AllocPoseData),
                              func_address(&nalCharPose::_CopyPoseData),
                              func_address(&nalCompPose::_DirectCopyPoseData),
                              func_address(&nalCompPose::_FreePoseData),
                              func_address(&nalCharPose::_InitializePoseDataFromSkel),
                              func_address(&nalCompPose::_ComponentFreePoseData)};

        m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00891A3C;
    }

    this->field_C = 0;
    this->InitializePoseDataFromSkel();
}

nalCharPose::nalCharPose(const nalCharPose &a2, bool a3) : nalCompPose(a2.GetSkeleton())
{
    if (a3) {
        *this = a2;
    }
}

nalCharPose::~nalCharPose()
{
    this->m_vtbl = 0x00891A3C;
    nalComp::nalCompPose::FreePoseData();
}

void *nalCharPose::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

void nalCharPose::operator delete(void *ptr)
{
    tlMemFree(ptr);
}

void nalCharPose::Blend(Float a2, const nalCharPose &src0, const nalCharPose &src1)
{
    TRACE("nalChar::nalCharPose::Blend");

    if constexpr (0) {
        if (this->m_pTheData != nullptr) {
            this->InitializePoseDataFromSkel();
        }

        if (equal<float>(a2, 1.0f)) {
            (*this) = src1;
        } else if (equal<float>(a2, 0.0f)) {
            (*this) = src0;
        } else {
            auto *v6 = src1.GetSkeleton();
            auto numComponents = v6->GetNumComponents();
            auto *v11 = v6;
            for (int i = 0; i < numComponents; ++i) {
                if (v6->ConvertCompIxToPoseIx(i) != -1) {
                    auto *v8 = this->GetComponentPoseData(i);
                    auto *v9 = (unsigned int *)src0.GetComponentPoseData(i);
                    auto *v12 = src1.GetComponentPoseData(i);
                    v11->GetComponent(i)->BlendPoseData(v8, v11->GetName(i), a2, v9, v12);
                }
            }
        }
    } else {
        THISCALL(0x005F13B0, this, a2, &src0, &src1);
    }
}

void *nalCharPose::GetNamedPoseData(CharComponentBase::Names a2)
{
    TRACE("nalCharPose::GetNamedPoseData");

    auto *Skeleton = (const nalCharSkeleton *)this->GetSkeleton();
    int CompIxByName = Skeleton->GetCompIxByName(a2);
    if (CompIxByName != -1) {
        return this->GetComponentPoseData(CompIxByName);
    } else {
        return nullptr;
    }
}

void *nalCharPose::GetNamedPoseData(CharComponentBase::Names a2) const
{
    TRACE("nalCharPose::GetNamedPoseData");

    auto *Skeleton = this->GetSkeleton();
    int CompIxByName = Skeleton->GetCompIxByName(a2);
    if (CompIxByName != -1) {
        return this->GetComponentPoseData(CompIxByName);
    } else {
        return nullptr;
    }
}

char *nalChar::nalCharSkeleton::GetCompPerSkelDataInt(int a2) const
{
    return nalComp::nalCompSkeleton::GetCompPerSkelDataInt(a2);
}

char *nalChar::nalCharSkeleton::GetCompDefaultPoseData(int iCompIx) const
{
    TRACE("nalChar::nalCharSkeleton::GetCompDefaultPoseData");

    return nalComp::nalCompSkeleton::GetCompDefaultPoseData(iCompIx);
}

void nalCharPose::_CopyPoseData(const void *a2)
{
    auto v9 = this->GetSkeleton()->GetNumComponents();
    for (int v4 = 0; v4 < v9; ++v4) {
        if (this->GetSkeleton()->ConvertCompIxToPoseIx(v4) != -1) {
            auto *v5 = this->GetComponentPoseData(v4);
            auto v6 = this->GetSkeleton()->GetComponentPoseDataOffset(v4);
            auto *extraData = static_cast<const char *>(a2) + v6;
            auto name = this->GetSkeleton()->GetName(v4);
            auto *component = this->GetSkeleton()->GetComponent(v4);
            component->CopyPoseExtraData(v5, name, extraData);
        }
    }
}

void nalCharPose::_InitializePoseDataFromSkel()
{
    TRACE("nalCharPose::InitializePoseDataFromSkel");

    if constexpr (1) {
        auto *pDirectory = this->GetSkeleton()->m_pDirectory;
        if (pDirectory != nullptr) {
            this->AllocPoseData();
            this->DirectCopyPoseData(pDirectory);
            auto numComponents = this->GetSkeleton()->GetNumComponents();
            for (int v3 = 0; v3 < numComponents; ++v3) {
                if (this->GetSkeleton()->ConvertCompIxToPoseIx(v3) != -1) {
                    auto *v5 = this->GetComponentPoseData(v3);
                    auto v6 = this->GetSkeleton()->GetComponentPoseDataOffset(v3);
                    auto *v7 = this->GetSkeleton();
                    auto name = v7->GetName(v3);
                    v7->GetComponent(v3)->CopyPoseDataToNothing(v5, name, &pDirectory[v6]);
                }
            }
        }
    } else {
        THISCALL(0x005F1290, this);
    }
}

nalCharSkeleton::nalCharSkeleton()
{
    if (0) {
        static void *g_vtbl[]{nullptr,
                              nullptr,
                              func_address(&nalCharSkeleton::_Process),
                              nullptr,
                              func_address(&nalCharSkeleton::_CheckVersion),
                              func_address(&nalCompSkeleton::_VirtualGetBoneMatrixCount),
                              func_address(&nalCompSkeleton::_VirtualGetBoneMatrices),
                              func_address(&nalCompSkeleton::_VirtualGetTrajectoryUpdate),
                              func_address(&nalCompSkeleton::_VirtualGetPose),
                              func_address(&nalCharSkeleton::_VirtualGetDefaultPose),
                              func_address(&nalCharSkeleton::_VirtualCreatePose),
                              func_address(&nalCharSkeleton::_VirtualDestroyPose),
                              func_address(&nalCharSkeleton::_VirtualCopyPose),
                              func_address(&nalCharSkeleton::_VirtualBlend),
                              func_address(&nalCompSkeleton::_GetPerSkelDataFromComponent),
                              func_address(&nalCompSkeleton::_DoesComponentHavePoseTrackData),
                              func_address(&nalCompSkeleton::_UnMash),
                              func_address(&nalCompSkeleton::_ReMash)};
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00891F88;
    }

    this->m_theDefaultPose = nullptr;
    this->Version = 0x10003;
}

int nalCharSkeleton::GetCompIxByName(CharComponentBase::Names a2) const
{
    for (int iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx) {
        if (this->GetName(iCompIx) == a2) {
            return iCompIx;
        }
    }

    return -1;
}

char *nalCharSkeleton::GetNamedPerSkelData(CharComponentBase::Names a2) const
{
    int CompIxByName = this->GetCompIxByName(a2);
    if (CompIxByName == -1) {
        return nullptr;
    }

    auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(CompIxByName);
    auto *v8 = this->GetComponent(CompIxByName);
    return bit_cast<char *>(v8->ApplyPublicPerSkelDataOffset(a2, CompPerSkelDataInt));
}

nalCharPose *nalCharSkeleton::GetDefaultPose() const
{
    assert(this->m_theDefaultPose != nullptr && "Should have made one in Process");
    return this->m_theDefaultPose;
}

nalCharPose *nalCharSkeleton::CreatePose() const
{
    nalCharPose v5{this};
    auto *v3 = new nalCharPose{v5, false};

    return v3;
}

void nalCharSkeleton::DestroyPose(nalCharPose *a1)
{
    if (a1 != nullptr) {
        delete a1;
    }
}

void nalCharSkeleton::CopyPose(nalCharPose *a1, const nalCharPose *a2)
{
    *a1 = *a2;
}

void nalChar::nalCharSkeleton::_finalize(bool a2)
{
    this->~nalCharSkeleton();
    if (a2) {
        tlMemFree(this);
    }
}

void nalCharSkeleton::_Process()
{
    TRACE("nalCharSkeleton::Process");

    if constexpr (1) {
        auto v1 = CharComponentManager::iCurrNumComponents;
        auto **v3 = (BaseComponent **)tlMemAlloc(4 * v1, 8u, 0);
        for (int i = 0; i < v1; ++i) {
            v3[i] = CharComponentManager::pCompArray[i];
        }

        this->UnMash(this, v3, v1);
        tlMemFree(v3);

        this->m_theDefaultPose = new nalCharPose{this};
    } else {
        THISCALL(0x005F28C0, this);
    }
}

void nalCharSkeleton::_Release()
{
    if (this->m_theDefaultPose != nullptr) {
        delete this->m_theDefaultPose;
    }

    this->m_theDefaultPose = nullptr;
    this->ReMash(this);
}

const nalBasePose *nalCharSkeleton::_VirtualGetDefaultPose() const
{
    TRACE("nalCharSkeleton::VirtualGetDefaultPose");

    auto *v1 = this->GetDefaultPose();
    if (v1 != nullptr) {
        return &v1->field_4;
    }

    return nullptr;
}

const nalBasePose *nalCharSkeleton::_VirtualCreatePose() const
{
    TRACE("nalCharSkeleton::VirtualCreatePose");

    auto *v1 = this->CreatePose();
    if (v1 != nullptr) {
        return &v1->field_4;
    }

    return nullptr;
}

void nalCharSkeleton::_VirtualDestroyPose(nalBasePose *a2)
{
    TRACE("nalCharSkeleton::VirtualDestroyPose");

    if constexpr (0) {
        if (a2 != nullptr) {
            this->DestroyPose((nalCharPose *)&a2[-1]);
        } else {
            this->DestroyPose(nullptr);
        }
    } else {
        void(__fastcall * func)(void *, void *edx, nalBasePose *a2) = CAST(func, 0x005FCB10);
        func(this, nullptr, a2);
    }
}

void nalCharSkeleton::_VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2)
{
    TRACE("nalCharSkeleton::VirtualCopyPose");

    if constexpr (1) {
        const nalCharPose *v3 = nullptr;
        if (a2 != nullptr) {
            v3 = (const nalCharPose *)&a2[-1];
        }

        nalCharPose *v1 = nullptr;
        if (a1 != nullptr) {
            v1 = (nalCharPose *)&a1[-1];
        }

        this->CopyPose(v1, v3);
    } else {
        void(__fastcall * func)(void *, void *edx, nalBasePose *a1, const nalBasePose *a2) = CAST(func, 0x005FB560);
        func(this, nullptr, a1, a2);
    }
}

void nalCharSkeleton::_VirtualBlend(nalBasePose *a2, Float a3, const nalBasePose *a4, const nalBasePose *a5)
{
    TRACE("nalCharSkeleton::VirtualBlend");

    const nalCharPose *v5 = nullptr;
    if (a5 != nullptr) {
        v5 = (const nalCharPose *)&a5[-1];
    }

    const nalCharPose *v6 = nullptr;
    if (a4 != nullptr) {
        v6 = (const nalCharPose *)&a4[-1];
    }

    nalCharPose *v7 = nullptr;
    if (a4 != nullptr) {
        v7 = (nalCharPose *)&a2[-1];
    }

    v7->Blend(a3, *v6, *v5);
}

}  // namespace nalChar

void nalChar_patch()
{
    auto make_cb = [](uint32_t address_vtbl) {
        auto result = [address_vtbl](std::intptr_t offset, auto func) {
            set_vfunc(address_vtbl + offset, func_address(func));
        };

        return result;
    };

    {
        static constexpr auto address_vtbl = 0x00891F88;
        auto set_vfunc_local = make_cb(address_vtbl);

        {
            set_vfunc_local(0x4, &nalChar::nalCharSkeleton::_finalize);
            set_vfunc_local(0x8, &nalChar::nalCharSkeleton::_Process);
            set_vfunc_local(0xC, &nalChar::nalCharSkeleton::_Release);
            set_vfunc_local(0x10, &nalChar::nalCharSkeleton::_CheckVersion);
            set_vfunc_local(0x14, &nalComp::nalCompSkeleton::_VirtualGetBoneMatrixCount);
            set_vfunc_local(0x18, &nalComp::nalCompSkeleton::_VirtualGetBoneMatrices);
            set_vfunc_local(0x1C, &nalComp::nalCompSkeleton::_VirtualGetTrajectoryUpdate);
            set_vfunc_local(0x20, &nalComp::nalCompSkeleton::_VirtualGetPose);
            set_vfunc_local(0x24, &nalChar::nalCharSkeleton::_VirtualGetDefaultPose);
            set_vfunc_local(0x28, &nalChar::nalCharSkeleton::_VirtualCreatePose);
            set_vfunc_local(0x2C, &nalChar::nalCharSkeleton::_VirtualDestroyPose);
            set_vfunc_local(0x30, &nalChar::nalCharSkeleton::_VirtualCopyPose);
            set_vfunc_local(0x34, &nalChar::nalCharSkeleton::_VirtualBlend);
            set_vfunc_local(0x38, &nalComp::nalCompSkeleton::_GetPerSkelDataFromComponent);
            set_vfunc_local(0x3C, &nalComp::nalCompSkeleton::_DoesComponentHavePoseTrackData);
            set_vfunc_local(0x40, &nalComp::nalCompSkeleton::_UnMash);
            set_vfunc_local(0x44, &nalComp::nalCompSkeleton::_ReMash);
        }
    }

    {
        static constexpr auto address_vtbl = 0x00891A3C;
        auto set_vfunc_local = make_cb(address_vtbl);

        void *(nalComp::nalCompPose::*GetComponentPoseData0)(uint32_t) = &nalComp::nalCompPose::_GetComponentPoseData;
        void *(nalComp::nalCompPose::*GetComponentPoseData1)(uint32_t) const =
            &nalComp::nalCompPose::_GetComponentPoseData;

        set_vfunc_local(0x0, GetComponentPoseData0);
        set_vfunc_local(0x4, GetComponentPoseData1);
        set_vfunc_local(0x8, &nalComp::nalCompPose::_GetPoseDataSize);
        set_vfunc_local(0xC, &nalComp::nalCompPose::_GetPoseDataAlign);
        set_vfunc_local(0x10, &nalComp::nalCompPose::_AllocPoseData);
        set_vfunc_local(0x14, &nalChar::nalCharPose::_CopyPoseData);
        set_vfunc_local(0x18, &nalComp::nalCompPose::_DirectCopyPoseData);
        set_vfunc_local(0x1C, &nalComp::nalCompPose::_FreePoseData);
        set_vfunc_local(0x20, &nalChar::nalCharPose::_InitializePoseDataFromSkel);
        set_vfunc_local(0x24, &nalComp::nalCompPose::_ComponentFreePoseData);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::GetCompPerSkelDataInt);
        REDIRECT(0x005F0FC3, address);
        REDIRECT(0x005F0FD0, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::GetCompDefaultPoseData);
        REDIRECT(0x005F0FB4, address);
    }

    {
        void *(nalChar::nalCharPose::*GetNamedPoseData)(CharComponentBase::Names) =
            &nalChar::nalCharPose::GetNamedPoseData;

        FUNC_ADDRESS(address, GetNamedPoseData);
        SET_JUMP(0x005F1330, address);
    }
}
