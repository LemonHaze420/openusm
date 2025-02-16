#include "character_pose_skel.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "tl_system.h"
#include "trace.h"

namespace nalChar {

VALIDATE_SIZE(nalCharSkeleton, 0x84);
VALIDATE_SIZE(nalCharPose, 0x10);

nalCharPose::nalCharPose(
    const nalChar::nalCharSkeleton *a2) : nalCompPose(a2)
{
    this->m_vtbl = 0x00891A3C;
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
        nalCharPose *a3,
        nalCharPose *a4)
{
    THISCALL(0x005F13B0, this, a2, a3, a4);
}

void * nalCharPose::GetNamedPoseData(CharComponentBase::Names a2)
{
    auto *Skeleton = (const nalCharSkeleton *) this->GetSkeleton();
    int CompIxByName = Skeleton->GetCompIxByName(a2);
    if ( CompIxByName != -1 ) {
        return this->GetComponentPoseData(CompIxByName);
    } else {
        return nullptr;
    }
}


void nalCharPose::InitializePoseDataFromSkel()
{
    TRACE("nalCharPose::InitializePoseDataFromSkel");

    THISCALL(0x005F1290, this);
}

int nalCharSkeleton::GetCompIxByName(CharComponentBase::Names a2) const
{
    for ( int iCompIx = 0; iCompIx < this->m_iNumComponents; ++iCompIx )
    {
        if ( this->field_70[iCompIx].field_0 == a2 ) {
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
    auto *v8 = this->field_70[CompIxByName].field_4;
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

void nalCharSkeleton::Process()
{
    TRACE("nalCharSkeleton::Process");

    auto v1 = CharComponentManager::iCurrNumComponents();
    auto **v3 = (BaseComponent **)tlMemAlloc(4 * v1, 8u, 0);
    for ( int i = 0; i < v1; ++i ) {
        v3[i] = CharComponentManager::pCompArray()[i];
    }

    this->UnMash(this, v3, v1);
    tlMemFree(v3);

    this->m_theDefaultPose = new nalCharPose {this};
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

void nalCharSkeleton::VirtualBlend(
        nalBasePose *a2,
        Float a3,
        nalBasePose *a4,
        nalBasePose *a5)
{
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

    v7->Blend(a3, v6, v5);
}

} // nalChar

void nalChar_patch()
{
    {
        FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::VirtualGetDefaultPose);
        set_vfunc(0x00891FAC, address);
    }

    FUNC_ADDRESS(address, &nalChar::nalCharSkeleton::Process);
    SET_JUMP(0x005F28C0, address);
}
