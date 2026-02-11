#pragma once

#include <cstdint>

struct FakerootPoseDesc {

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

    char field_0[0x1C];
    int field_1C;
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
    int field_28;
    int field_2C;
};

struct PerSkelData {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
};

void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

void CopyPoseDataToNothing(
        FakerootPoseDesc::StdPoseData *a1,
        uint32_t,
        const FakerootPoseDesc::StdPoseData *a3);

};
