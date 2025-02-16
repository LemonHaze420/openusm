#include "fakerootposedesc.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

#include <cassert>

namespace FakerootPoseDesc {

VALIDATE_SIZE(PerAnimData::EventIterator, 0xC);

PerAnimData::EventIterator PerAnimData::GetStartIterator() const
{
    return EventIterator {this};
}

PerAnimData::EventIterator::EventIterator(const FakerootPoseDesc::PerAnimData *a2)
{
    this->m_pAnimData = a2;
    this->m_pLoc = CAST(this->m_pLoc, a2->numTotalSignals
                            ? &this->m_pAnimData->field_30
                            : nullptr
                        );

    this->field_8 = false;
}

void PerAnimData::EventIterator::PositionToSignalIx(int32_t iSignalIx)
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    assert((0 <= iSignalIx) && (iSignalIx < this->m_pAnimData->numTotalSignals));

    auto *m_pAnimData = this->m_pAnimData;
    const uint16_t *v3 = &this->m_pAnimData->field_30;
    this->m_pLoc = CAST(this->m_pLoc, v3);
    if ( iSignalIx != 0 )
    {
        auto v4 = iSignalIx;
        do
        {
            auto *pLoc = bit_cast<uint16_t *>(this->m_pLoc);
            const uint16_t *v6;
            if ( *((const uint8_t *)this->m_pLoc + 2) == m_pAnimData->numTotalSignals - 1 )
            {
                this->field_8 = true;
                v6 = v3;
            }
            else
            {
                this->field_8 = false;
                v6 = &pLoc[2 * this->GetNumArguments() + 6];
            }
            --v4;
            this->m_pLoc = CAST(this->m_pLoc, v6);
        }
        while ( v4 != 0 );
    }
}

uint32_t PerAnimData::EventIterator::GetNumArguments() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid."); 

    return *((const uint8_t *)this->m_pLoc + 3);
}

int PerAnimData::EventIterator::GetArgument(int iArgIx) const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    assert(uint32_t(iArgIx) < this->GetNumArguments());
    
    const uint16_t *pLoc = CAST(pLoc, this->m_pLoc);

    return *(const uint32_t *)&pLoc[2 * iArgIx + 6];
}

int PerAnimData::EventIterator::GetNameOfSignal() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    return *((const uint32_t *)this->m_pLoc + 1);
}

int PerAnimData::EventIterator::GetNameOfBone() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    return *((const uint32_t *)this->m_pLoc + 2);
}

void PerAnimData::EventIterator::operator++()
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    THISCALL(0x005F3E30, this);
}

} //
