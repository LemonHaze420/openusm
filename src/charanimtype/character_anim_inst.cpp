#include "character_anim_inst.h"

#include "character_pose_skel.h"
#include "common.h"

#include <cassert>

void * nalChar::nalCharAnim::GetPerAnimDataByName(CharComponentBase::Names a2)
{
    auto *v3 = (nalCharSkeleton *) this->GetSkeleton();
    int CompIxByName = v3->GetCompIxByName(a2);
    if ( CompIxByName == -1 ) {
        return nullptr;
    }

    auto *CompPerAnimDataInt = (const void *) this->GetCompPerAnimDataInt(CompIxByName);
    auto *v9 = this->field_30->field_70[CompIxByName].field_4;
    return v9->ApplyPublicPerAnimDataOffset(a2, CompPerAnimDataInt);
}
