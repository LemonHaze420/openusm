#include "charcomponentbase.h"

#include "string_hash.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cassert>

uint32_t CharComponentBase::GetType()
{
    TRACE("CharComponentBase::GetType");

    if ( this->m_TheType == 0 ) {
        if ( this->m_strTypeString != nullptr ) {
            this->m_TheType = to_hash(this->m_strTypeString);
        } else {
            assert(0 && "You did not override GetType. You need to do this or set m_TheType or set m_strTypeString.");
        }
    }

    return this->m_TheType;
}

void CharComponentBase::CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3)
{
    void * (__fastcall *func)(void *, void *, void *, uint32_t, const void *) = CAST(func, get_vfunc(m_vtbl, 0x74));
    func(this, nullptr, a1, a2, a3);
}

void CharComponentBase_patch()
{
    FUNC_ADDRESS(address, &CharComponentBase::GetType);
    SET_JUMP(0x005EC4B0, address);
}
