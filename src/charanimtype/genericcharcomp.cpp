#include "genericcharcomp.h"

#include "charcomponentmanager.h"
#include "common.h"
#include "nal_generic.h"

GenericCharComp::GenericCharComp()
{
    this->m_vtbl = 0x00892140;
    this->m_strTypeString = "Generic";
    CharComponentManager::RegisterComponent(this);
}

void GenericCharComp::BuildBoneMatrices(
    nalMatrix4x4 *a1,
    uint32_t ,
    const void *a3,
    const void *a4)
{
    bit_cast<const nalGeneric::nalGenericSkeleton *>(a3)->GetBoneMatrices(bit_cast<const nalGeneric::nalGenericPose *>(a4), a1);
}
