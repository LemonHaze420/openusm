#pragma once

#include "charcomponentbase.h"

#include "float.hpp"

struct vector3d;

struct ArbitraryPOCharComp : CharComponentBase {

    struct PerSkelData {
        char field_0[0x10];
        float *field_10;
        vector3d *field_14;
    };

    struct BoneData {
        char field_0[0x20];
        uint16_t field_20;
        uint16_t field_22;
        int field_24;
        uint16_t field_28;
        uint16_t field_2A;
    };

    struct StdPoseData {
        int field_0;
        int field_4;
        int field_8;
        int field_C;
        float field_10[4][1];
    };

    //0x005F6130
    void BlendPoseData(
            void *a1,
            uint32_t a2,
            Float a3,
            const void *a4,
            const void *a5,
            uint32_t a6,
            uint32_t a7);

    //virtual
    //0x005EF480
    nalPositionOrientation * GetTrajectoryData(
        nalPositionOrientation *,
        uint32_t,
        const void *,
        const void *);

    //virtual
    //0x005F5E60
    void BuildBoneMatrices(nalMatrix4x4 *, uint32_t , void const *, void const *);


    //virtula
    //0x008920D4
    int GetSizeOfPerInstData(
            uint32_t a1,
            const void *a2,
            const void *a3,
            const void *a4,
            const void *a5,
            const void *a6,
            bool a7);


    //virtual
    //0x005EF5A0
    void DestroyPerInstData(void *a1, uint32_t a2, const void *a3, const void *a4);

    //virtual
    //0x005EF5F0
    bool WillMapToComponentData(uint32_t a2, uint32_t a3, uint32_t a4);

    //virtual
    //0x005F98E0
    void CalcPoseDataDirect(
            void *a1,
            uint32_t a2,
            Float a3,
            Float a4,
            const nalComp::nalCompAnim *a5,
            const void *a6,
            const void *a7,
            const void *a8,
            void *a9);

    //virtual
    //0x005EF710
    void CalcPoseDataRemapped(
            void *a1,
            uint32_t a2,
            Float a3,
            Float a4,
            const nalComp::nalCompAnim *a5,
            const void *a6,
            uint32_t a7,
            uint32_t a8,
            const void *a9,
            const void *a10,
            void *a11);

    //virtual
    //0x005F9A20
    void BlendPoseData(
        void *a1,
        uint32_t a2,
        Float a3,
        const void *a4,
        const void *a5);

    //virtual
    //0x005EF750
    void SkelPoseProcess(uint32_t a1, void *a2, void *a3);

    //virtual
    //0x00439F80
    int GetDomain() const;

    //virtual
    //0x005FB910
    uint32_t GetPoseTypeID() const;

};

extern void ArbitraryPOCharComp_patch();
