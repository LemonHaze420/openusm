#pragma once

#include "charcomponentbase.h"

#include <float.hpp>


struct GenericCharComp : CharComponentBase {
    GenericCharComp();

    GenericCharComp *_DestroyComponent(unsigned char flags);
    void *_ApplyPublicPerSkelDataOffset(uint32_t, void *data) { return data; }
    void *_ApplyPublicPerAnimDataOffset(uint32_t, const void *data) { return const_cast<void *>(data); }
    nalPositionOrientation *_GetTrajectoryData(nalPositionOrientation *, uint32_t, const void *, const void *);
    int _DoesContributeToPose(uint32_t, const void *, const void *) { return 1; }
    int _GetSizeOfPerInstData(uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    { return 0x2C; }
    int _GetAlignOfPerInstData(uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    { return 4; }
    void _BuildPerInstData(void *, uint32_t, const void *, const void *, const void *, const void *, const void *, bool);
    void _DestroyPerInstData(void *, uint32_t, const void *, const void *);
    bool _WillMapToComponentData(uint32_t, uint32_t, uint32_t type) { return type == GetType(); }
    void _CalcPoseDataRemapped(void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *,
        uint32_t, uint32_t, const void *, const void *, void *);
    void _BlendPoseData(void *, uint32_t, Float, const void *, const void *);
    void _SkelPoseProcess(uint32_t, void *, void *);
    void _SkelPoseRelease(uint32_t, void *, void *);
    void _AnimProcess(uint32_t, void *, void *, const void *);
    void _AnimRelease(uint32_t, void *, void *, const void *);
    void _CopyPoseExtraData(void *, uint32_t, const void *);
    void _PoseDataFree(uint32_t, void *);
    int _GetDomain() const { return 0; }
    uint32_t _GetPoseTypeID() { return GetType(); }
    void _CopyPoseDataToNothing(void *, uint32_t, const void *);

    //virtual
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const void *a3, const void *a4);

    //virtual
    void CalcPoseDataDirect(void *a3, uint32_t _24, Float arg8, Float a5, const nalComp::nalCompAnim *a6,
                            const void *a7, const void *a8, const void *a9, void *a10);
};

extern void GenericCharComp_patch();
