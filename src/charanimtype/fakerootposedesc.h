#pragma once

#include <cstdint>

struct FakerootPoseDesc {
    struct PerAnimData {
        struct Signal {
            uint16_t frame;
            uint8_t index;
            uint8_t num_arguments;
            uint32_t name;
            uint32_t bone;
        };

        struct EventIterator {
            const PerAnimData *m_pAnimData;
            const Signal *m_pLoc;
            bool field_8;

            EventIterator(const PerAnimData *a2);

            void PositionToSignalIx(int32_t iSignalIx);

            uint32_t GetNumArguments() const;

            int GetArgument(int iArgIx) const;


            uint16_t GetSignalFrame() const;

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
        Signal first_signal;

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

    void CopyPoseDataToNothing(FakerootPoseDesc::StdPoseData *a1, uint32_t, const FakerootPoseDesc::StdPoseData *a3);
};
