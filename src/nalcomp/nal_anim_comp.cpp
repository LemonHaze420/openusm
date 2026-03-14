#include "nal_anim_comp.h"

#include "common.h"
#include "component.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"
#include "vtbl.h"

#include <cassert>

void * nalComp::nalCompAnim::_GetPerAnimDataFromComponentIx(int a2)
{
    if ( a2 == -1 ) {
        return nullptr;
    }

    auto CompPerAnimDataInt = this->GetCompPerAnimDataInt(a2);
    auto *Skeleton = this->GetSkeleton();
    auto *v6 = Skeleton->GetComponent(a2);
    auto *v4 = this->GetSkeleton();
    auto v5 = v4->GetName(a2);
    return v6->ApplyPublicPerAnimDataOffset(v5, CompPerAnimDataInt);
}

int nalComp::nalCompAnim::_GetPerAnimUserDataInt()
{
    return (int)this->field_44 + this->field_44[*this->field_44];
}

void nalComp::nalCompAnim::_UnMash(void *a2)
{
    TRACE("nalCompAnim::UnMash");

    this->field_40 = (int *)((char *)this->field_40 + (unsigned int)a2);
    this->field_44 = (int *)((char *)this->field_44 + (unsigned int)a2);
    this->field_48 += (int)a2;

    auto *Skeleton = this->GetSkeleton();

    auto NumComponents = Skeleton->GetNumComponents();
    for ( int i = 0; i < NumComponents; ++i )
    {
        auto CompPerAnimDataInt = this->GetCompPerAnimDataInt(i);
        auto CompAnimTrackData = this->GetCompAnimTrackData(i);

        if ( CompPerAnimDataInt != nullptr || CompAnimTrackData != nullptr )
        {
            auto *v3 = this->GetSkeleton();
            auto *v8 = v3->GetComponent(i);
            auto *CompPerSkelDataInt = v3->GetCompPerSkelDataInt(i);
            auto v6 = v3->GetName(i);

            v8->AnimProcess(
                v6,
                CompPerAnimDataInt,
                CompAnimTrackData,
                CompPerSkelDataInt);
        }
    }
}

void nalComp::nalCompAnim::UnMash(void *a2) {
    void (__fastcall *func)(void *, void *edx, void *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
    func(this, nullptr, a2);
}

void nalComp::nalCompAnim::_ReMash(void *a2)
{
    auto *Skeleton = this->GetSkeleton();
    auto NumComponents = Skeleton->GetNumComponents();
    for ( int a1 = 0; a1 < NumComponents; ++a1 )
    {
        auto CompPerAnimDataInt = this->GetCompPerAnimDataInt(a1);
        auto CompAnimTrackData = this->GetCompAnimTrackData(a1);
        if ( CompPerAnimDataInt != nullptr || CompAnimTrackData != nullptr )
        {
            auto *v3 = this->GetSkeleton();
            auto *v8 = v3->GetComponent(a1);
            auto v4 = this->GetSkeleton();
            auto *CompPerSkelDataInt = v4->GetCompPerSkelDataInt(a1);
            auto v5 = this->GetSkeleton();
            auto v6 = v5->GetName(a1);
            v8->AnimRelease(v6, CompPerAnimDataInt, CompAnimTrackData, CompPerSkelDataInt);
        }
    }

    this->field_40 = (int *)((char *)a2 - (char *)this->field_40);
    this->field_44 = (int *)((char *)a2 - (char *)this->field_44);
    this->field_48 = (int)a2 - this->field_48;
}

void nalComp::nalCompAnim::ReMash(void *a2) {
    void (__fastcall *func)(void *, void *edx, void *) = CAST(func, get_vfunc(m_vtbl, 0x20));
    func(this, nullptr, a2);
}

void * nalComp::nalCompAnim::GetCompPerAnimDataInt(int iCompIx)
{
    TRACE("nalCompAnim::GetCompPerAnimDataInt");

    assert(iCompIx < this->GetSkeleton()->GetNumComponents()
            && "Asked anim for a component that doesn't exist in skeleton.");

    if ( (this->field_40[iCompIx] & 2) == 0 ) {
        return nullptr;
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

    auto *result = (char *)this->field_44 + pPerAnimDataDir[iOffsetIx + 1];
    return result;
}

void * nalComp::nalCompAnim::GetCompAnimTrackData(int iCompIx)
{
    TRACE("nalCompAnim::GetCompAnimTrackData");

    assert(iCompIx < this->GetSkeleton()->GetNumComponents() &&
                "Asked anim for a component that doesn't exist in skeleton.");

    if ( (this->field_40[iCompIx] & 1) == 0 ) {
        return nullptr;
    }

    int iOffsetIx = 0;
    for ( int i = 0; i < iCompIx; ++i )
    {
        if ( (this->field_40[i] & 1) != 0 ) {
            ++iOffsetIx;
        }
    }

    auto *pTrackDataDir = (int *)this->field_48;
    assert(pTrackDataDir[0] > iOffsetIx && "Bad track data offset.");

    auto *result = bit_cast<void *>(pTrackDataDir[iOffsetIx + 1] + this->field_48);
    return result;
}

bool nalComp::nalCompAnim::DoesComponentAddToPose(int32_t iCompIx)
{
    TRACE("nalCompAnim::DoesComponentAddToPose");

    assert(iCompIx < this->GetSkeleton()->GetNumComponents() && "Asked anim for a component that doesn't exist in skeleton.");

    if ( (this->field_40[iCompIx] & 1) != 0 ) {
        return true;
    }

    if ( (this->field_40[iCompIx] & 2) == 0 ) {
        return false;
    }

    auto *skeleton = this->GetSkeleton();
    auto *component = skeleton->GetComponent(iCompIx);
    auto CompPerAnimDataInt = this->GetCompPerAnimDataInt(iCompIx);
    auto CompPerSkelDataInt = skeleton->GetCompPerSkelDataInt(iCompIx);

    auto v7 = skeleton->GetName(iCompIx);

    return component->DoesContributeToPose(
            v7,
            CompPerSkelDataInt,
            CompPerAnimDataInt);
}

void nalCompAnim_patch()
{
    {
        FUNC_ADDRESS(address, &nalComp::nalCompAnim::_UnMash);
        set_vfunc(0x00891FEC, address);
    }

    /*
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
