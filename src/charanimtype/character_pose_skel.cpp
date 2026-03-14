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

int & nalCharSkeleton::vtbl_ptr = var<int>(0x0096AB90);

#endif

nalCharPose::nalCharPose(
    const nalChar::nalCharSkeleton *a2) : nalCompPose(a2)
{
    if constexpr (1) {
        void * (nalCompPose::*GetComponentPoseData0)(uint32_t) = &nalCompPose::_GetComponentPoseData;
        void * (nalCompPose::*GetComponentPoseData1)(uint32_t) const = &nalCompPose::_GetComponentPoseData;

        static void * g_vtbl[] {
            func_address(GetComponentPoseData0),
            func_address(GetComponentPoseData1),
            func_address(&_GetPoseDataSize)
        };

        m_vtbl = CAST(m_vtbl, &g_vtbl);
    }  else {
        this->m_vtbl = 0x00891A3C;
    }

    this->field_C = 0;
    this->InitializePoseDataFromSkel();
}

nalCharPose::nalCharPose(const nalCharPose &a2, bool a3) : nalCompPose(a2.field_4)
{
    if ( a3 ) {
        *this = a2;
    }
}

nalCharPose::~nalCharPose()
{
    this->m_vtbl = 0x00891A3C;
    nalComp::nalCompPose::FreePoseData();
}

void * nalCharPose::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

void nalCharPose::operator delete(void *ptr)
{
    tlMemFree(ptr);
}

void nalCharPose::Blend(
        Float a2,
        const nalCharPose &src0,
        const nalCharPose &src1)
{
    TRACE("nalChar::nalCharPose::Blend");

    if constexpr (0)
    {
        if ( this->m_pTheData != nullptr ) {
            this->InitializePoseDataFromSkel();
        }

        if ( equal<float>(a2, 1.0f) )
        {
            (*this) = src1;
        }
        else if ( equal<float>(a2, 0.0f) )
        {
            (*this) = src0;
        }
        else
        {
            auto *v6 = src1.GetSkeleton();
            auto numComponents = v6->GetNumComponents();
            auto *v11 = v6;
            for ( int i = 0; i < numComponents; ++i )
            {
                if ( v6->ConvertCompIxToPoseIx(i) != -1 )
                {
                    auto *v8 = this->GetComponentPoseData(i);
                    auto *v9 = (unsigned int *)src0.GetComponentPoseData(i);
                    auto v12 = src1.GetComponentPoseData(i);
                    auto &v13 = v11->field_70[i];
                    v13.m_component->BlendPoseData(
                            v8,
                            v13.m_name,
                            a2,
                            v9,
                            v12);
                }
            }
        }
    }
    else
    {
        THISCALL(0x005F13B0, this, a2, &src0, &src1);
    }
}

void * nalCharPose::GetNamedPoseData(CharComponentBase::Names a2)
{
    TRACE("nalCharPose::GetNamedPoseData");

    auto *Skeleton = (const nalCharSkeleton *) this->GetSkeleton();
    int CompIxByName = Skeleton->GetCompIxByName(a2);
    if ( CompIxByName != -1 ) {
        return this->GetComponentPoseData(CompIxByName);
    } else {
        return nullptr;
    }
}

char * nalChar::nalCharSkeleton::GetCompPerSkelDataInt(int a2) const
{
    return nalComp::nalCompSkeleton::GetCompPerSkelDataInt(a2);
}

void nalCharPose::InitializePoseDataFromSkel()
{
    TRACE("nalCharPose::InitializePoseDataFromSkel");

    if constexpr (1)
    {
        auto *v2 = this->field_4->field_78;
        if ( v2 != nullptr )
        {
            this->AllocPoseData();
            this->DirectCopyPoseData(v2);
            auto numComponents = this->field_4->GetNumComponents();
            for ( int v3 = 0; v3 < numComponents; ++v3 )
            {
                if ( this->field_4->ConvertCompIxToPoseIx(v3) != -1 )
                {
                    auto *v5 = this->GetComponentPoseData(v3);
                    auto v6 = this->field_4->GetComponentPoseDataOffset(v3);
                    auto *v7 = &this->field_4->field_70[v3];

                    bit_cast<CharComponentBase *>(v7->m_component)->CopyPoseDataToNothing(
                        v5,
                        v7->m_name,
                        &v2[v6]
                    );
                }
            }
        }
    }
    else
    {
        THISCALL(0x005F1290, this);
    }
}

nalCharSkeleton::nalCharSkeleton()
{
    static void * g_vtbl[] {
        nullptr,
        nullptr,
        func_address(&_Process),
        nullptr,
        func_address(&_CheckVersion),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        func_address(&_UnMash)
    };
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    this->m_theDefaultPose = nullptr;
    this->Version = 0x10003;
}

int nalCharSkeleton::GetCompIxByName(CharComponentBase::Names a2) const
{
    for ( int iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx )
    {
        if ( this->field_70[iCompIx].m_name == a2 ) {
            return iCompIx;
        }
    }

    return -1;
}

char * nalCharSkeleton::GetNamedPerSkelData(CharComponentBase::Names a2) const
{
    int CompIxByName = this->GetCompIxByName(a2);
    if ( CompIxByName == -1 ) {
        return nullptr;
    }

    auto *CompPerSkelDataInt = this->GetCompPerSkelDataInt(CompIxByName);
    auto *v8 = this->field_70[CompIxByName].m_component;
    return bit_cast<char *>(v8->ApplyPublicPerSkelDataOffset(a2, CompPerSkelDataInt));
}

nalCharPose *nalCharSkeleton::GetDefaultPose() const
{
    assert( this->m_theDefaultPose != nullptr && "Should have made one in Process");
    return this->m_theDefaultPose;
}

nalCharPose * nalCharSkeleton::CreatePose() const
{
    nalCharPose v5 {this};
    auto *v3 = new nalCharPose {v5, false};

    return v3;
}

void nalCharSkeleton::_Process()
{
    TRACE("nalCharSkeleton::Process");

    if constexpr (1) {
        auto v1 = CharComponentManager::iCurrNumComponents;
        auto **v3 = (BaseComponent **)tlMemAlloc(4 * v1, 8u, 0);
        for ( int i = 0; i < v1; ++i ) {
            v3[i] = CharComponentManager::pCompArray[i];
        }

        this->UnMash(this, v3, v1);
        tlMemFree(v3);

        this->m_theDefaultPose = new nalCharPose {this};
    } else {
        THISCALL(0x005F28C0, this);
    }
}

void nalCharSkeleton::Release()
{
    if ( this->m_theDefaultPose != nullptr )
    {
        delete this->m_theDefaultPose;
    }

    this->m_theDefaultPose = nullptr;
    this->ReMash(this);
}

const nalComp::nalCompSkeleton ** nalCharSkeleton::VirtualGetDefaultPose() const
{
    TRACE("nalCharSkeleton::VirtualGetDefaultPose");
    
    const nalComp::nalCompSkeleton **result = nullptr;

    auto *v1 = this->GetDefaultPose();
    if ( v1 != nullptr ) {
        result = &v1->field_4;
    }

    if (result != nullptr)
    {
        
    }

    return result;
}

const nalComp::nalCompSkeleton ** nalCharSkeleton::VirtualCreatePose() const
{
    auto *v1 = this->CreatePose();
    if ( v1 != nullptr ) {
        return &v1->field_4;
    }

    return nullptr;
}

void nalChar::nalCharSkeleton::VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2)
{
    TRACE("nalChar::nalCharSkeleton::VirtualCopyPose");

    const nalComp::nalCompPose *v3 = nullptr;
    if ( a2 != nullptr ) {
        v3 = (const nalComp::nalCompPose *)&a2[-1];
    }

    nalComp::nalCompPose *v1 = nullptr;
    if ( a1 != nullptr ) {
        v1 = (nalComp::nalCompPose *)&a1[-1];
    }

    (*v1) = (*v3);
}

void nalCharSkeleton::VirtualBlend(
        nalBasePose *a2,
        Float a3,
        nalBasePose *a4,
        nalBasePose *a5)
{
    TRACE("nalCharSkeleton::VirtualBlend");

    nalCharPose *v5 = nullptr;
    if ( a5 != nullptr ) {
        v5 = (nalCharPose *)&a5[-1];
    }

    nalCharPose *v6 = nullptr;
    if ( a4 != nullptr ) {
        v6 = (nalCharPose *)&a4[-1];
    }

    nalCharPose *v7 = nullptr;
    if ( a4 != nullptr ) {
        v7 = (nalCharPose *)&a2[-1];
    }

    v7->Blend(a3, *v6, *v5);
}

} // nalChar

void nalChar_patch()
{
    {
        FUNC_ADDRESS(address, &nalChar::nalCharPose::GetNamedPoseData);
        SET_JUMP(0x005F1330, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::_Process);
        set_vfunc(0x00891F90, address);
        //SET_JUMP(0x005F28C0, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::VirtualBlend);
        set_vfunc(0x00891FBC, address);
    }

    {
        set_vfunc(0x00891FC4, func_address(&nalComp::nalCompSkeleton::_DoesComponentHavePoseTrackData));
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharPose::InitializePoseDataFromSkel);
        set_vfunc(0x00891A5C, address);
    }

    return;
    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::VirtualGetDefaultPose);
        set_vfunc(0x00891FAC, address);
    }

    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::VirtualCopyPose);
        set_vfunc(0x00891FB8, address);
    }
}
