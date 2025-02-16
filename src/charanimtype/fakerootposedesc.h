#pragma once

#include <cstdint>

namespace FakerootPoseDesc {

struct PerAnimData {
    struct EventIterator {
        const PerAnimData *m_pAnimData;
        const struct {
        } *m_pLoc;
        bool field_8;

        EventIterator(const PerAnimData *a2);

        void PositionToSignalIx(int32_t iSignalIx);

        uint32_t GetNumArguments() const;

        int GetArgument(int iArgIx) const;

        int GetNameOfSignal() const;

        int GetNameOfBone() const;

        void operator++();

        bool IsIteratorValid() const
        {
            return this->m_pLoc != nullptr;
        }
    };

    char field_0[0x20];
    int numTotalSignals;
    int field_24;
    int field_28;
    int field_2C;
    uint16_t field_30;

    EventIterator GetStartIterator() const;
};

struct StdPoseData {
    float field_0[4];
    float field_10[3];
    float field_1C;
    int field_20;
    int field_24;
};

}
