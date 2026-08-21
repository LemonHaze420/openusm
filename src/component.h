#pragma once

#include <cstdint>

#include <float.hpp>

struct nalPositionOrientation;
struct nalMatrix4x4;

namespace nalComp {
struct nalCompAnim;
}

struct BaseComponent {
    std::intptr_t m_vtbl;

    //virtual ~BaseComponent() = default;

    //virtual
    uint32_t GetType();

    //virtual
    void *ApplyPublicPerSkelDataOffset(uint32_t a1, void *a2) const;

    //virtual
    void *ApplyPublicPerAnimDataOffset(uint32_t a1, const void *) const;

    //virtual
    nalPositionOrientation *GetTrajectoryData(nalPositionOrientation *out, uint32_t a2, const void *a3, const void *a4);

    //virtual
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const void *a3, const void *a4);

    //virtual
    int DoesContributeToPose(uint32_t a2, const void *a3, const void *a4);

    //virtual
    int GetSizeOfPerInstData(uint32_t a2, const void *a3, const void *a4, const void *a5, const void *a6,
                             const void *a7, bool a8);

    //virtual
    int GetAlignOfPerInstData(uint32_t a2, const void *a3, const void *a4, const void *a5, const void *a6,
                              const void *a7, bool bIsRemapped);

    //virtual
    void BuildPerInstData(void *a2, uint32_t, const void *a4, const void *a5, const void *a6, const void *,
                          const void *, bool bIsRemapped);

    //virtual
    void DestroyPerInstData(void *a2, uint32_t a3, const void *a4, const void *a5);

    //virtual
    bool WillMapToComponentData(uint32_t a2, uint32_t a3, uint32_t a4);

    //virtual
    void CalcPoseDataDirect(void *a1, uint32_t a2, Float a3, Float a4, const nalComp::nalCompAnim *a5, const void *a6,
                            const void *a7, const void *a8, void *a9);

    //virtual
    void CalcPoseDataRemapped(void *a1, uint32_t a2, Float a3, Float a4, const nalComp::nalCompAnim *a5, const void *a6,
                              const void *a7, const void *a8, void *a9);

    //virtual
    void BlendPoseData(void *a1, uint32_t a2, Float a3, const void *a4, const void *a5);

    void _SkelPoseProcess(uint32_t, void *, void *) const
    {
        ;
    }

    //virtual
    void SkelPoseProcess(uint32_t a1, void *a2, void *a3) const;

    //virtual
    void SkelPoseRelease(uint32_t a1, void *a2, void *a3) const;

    //virtual
    void AnimProcess(uint32_t a1, void *a2, void *a3, const void *a4) const;

    void _AnimRelease(uint32_t, void *, void *, const void *) const {}

    //virtual
    void AnimRelease(uint32_t a1, void *a2, void *a3, const void *a4) const;

    //virtual
    void PoseDataFree(uint32_t, void *) const;
};
