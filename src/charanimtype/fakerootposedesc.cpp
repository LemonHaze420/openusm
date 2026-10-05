#include "fakerootposedesc.h"

#include "common.h"
#include "utility.h"

#include <cassert>

VALIDATE_SIZE(FakerootPoseDesc::StdPoseData, 0x30u);

VALIDATE_SIZE(FakerootPoseDesc::PerAnimData::EventIterator, 0xC);
VALIDATE_SIZE(FakerootPoseDesc::PerAnimData::Signal, 0xC);


void FakerootPoseDesc::CopyPoseDataToNothing(FakerootPoseDesc::StdPoseData *a1, uint32_t,
                                             const FakerootPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

FakerootPoseDesc::PerAnimData::EventIterator FakerootPoseDesc::PerAnimData::GetStartIterator() const
{
    return EventIterator{this};
}

FakerootPoseDesc::PerAnimData::EventIterator::EventIterator(const FakerootPoseDesc::PerAnimData *a2)
{
    this->m_pAnimData = a2;
    this->m_pLoc = a2->numTotalSignals ? &a2->first_signal : nullptr;

    this->field_8 = false;
}

void FakerootPoseDesc::PerAnimData::EventIterator::PositionToSignalIx(int32_t iSignalIx)
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");
    assert((0 <= iSignalIx) && (iSignalIx < this->m_pAnimData->numTotalSignals));
    this->m_pLoc = &this->m_pAnimData->first_signal;
    for (int32_t i = 0; i < iSignalIx; ++i) {
        ++*this;
    }
}

uint32_t FakerootPoseDesc::PerAnimData::EventIterator::GetNumArguments() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    return this->m_pLoc->num_arguments;
}

int FakerootPoseDesc::PerAnimData::EventIterator::GetArgument(int iArgIx) const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    assert(uint32_t(iArgIx) < this->GetNumArguments());

    return reinterpret_cast<const uint32_t *>(this->m_pLoc + 1)[iArgIx];
}

int FakerootPoseDesc::PerAnimData::EventIterator::GetNameOfSignal() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    return this->m_pLoc->name;
}

int FakerootPoseDesc::PerAnimData::EventIterator::GetNameOfBone() const
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    return this->m_pLoc->bone;
}

uint16_t FakerootPoseDesc::PerAnimData::EventIterator::GetSignalFrame() const
{
    return this->m_pLoc->frame;
}

void FakerootPoseDesc::PerAnimData::EventIterator::operator++()
{
    assert(this->m_pLoc != nullptr && "Attempting to use an EventIterator that is invalid.");

    this->field_8 = this->m_pLoc->index == this->m_pAnimData->numTotalSignals - 1;
    if (this->field_8) {
        this->m_pLoc = &this->m_pAnimData->first_signal;
    } else {
        const auto *arguments = reinterpret_cast<const uint32_t *>(this->m_pLoc + 1);
        this->m_pLoc = reinterpret_cast<const Signal *>(arguments + this->m_pLoc->num_arguments);
    }
}
