#include "nal_anim_comp.h"

#include "common.h"
#include "component.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "trace.h"
#include "vtbl.h"

#include <cassert>

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
    assert(*pTrackDataDir > iOffsetIx && "Bad track data offset.");

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
