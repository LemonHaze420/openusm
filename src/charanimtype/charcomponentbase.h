#pragma once

#include "component.h"

namespace nalComp {
struct nalCompAnim;
}

struct CharComponentBase : BaseComponent {
    enum Names {
        Generic = 0,
        ArbitraryPO = 1,
        TorsoHeadOneNeckEntropyCompressed = 2,
        ArmsAndHandsEntropyCompressed = 3,
        FiveFingerTopTwoKnuckleCurlEntropyCompressed = 4,
        LegsAndFeetIKEntropyCompressed = 5,
        FakerootEntropyCompressed = 6,
        TentaclesCompressed = 7
    };

    BaseComponent *m_pSubComponent;
    uint32_t m_TheType;
    const char *m_strTypeString;
    int field_10;

    CharComponentBase();

    //virtual
    uint32_t _GetType();

    //virtual
    //0x005EC5D0
    int _DoesContributeToPose(uint32_t a2, const void *a3, const void *a4);

    //virtual
    //0x005EC4E0
    int _GetSizeOfPerInstData(uint32_t a2, const void *a3, const void *a4, const void *a5, const void *a6,
                              const void *a7, bool a8);

    //virtual
    //0x005EC510
    int _GetAlignOfPerInstData(uint32_t a2, const void *a3, const void *a4, const void *a5, const void *a6,
                               const void *a7, bool a8);

    //virtual
    //0x005EC540
    void _BuildPerInstData(void *a2, uint32_t a3, const void *a4, const void *a5, const void *a6, const void *a7,
                           const void *a8, bool a9);

    //virtual
    void *_CalcPoseDataDirect(void *a2, uint32_t a3, Float a4, Float a5, const nalComp::nalCompAnim *a6, const void *a7,
                              const void *a8, const void *a9, void *a10);

    //virtual
    void _CalcPoseDataRemapped(void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *, const void *,
                               const void *, void *)
    {}

    //virtual
    int GetDomain();

    int _GetRemapSizeOfPerInstData(uint32_t a1, uint32_t a2, const CharComponentBase *a3, const void *a4,
                                   const void *a5, const void *a6, const void *a7, const void *a8);

    //virtual
    //0x005EC590
    int GetRemapSizeOfPerInstData(uint32_t, uint32_t, const CharComponentBase *, const void *, const void *,
                                  const void *, const void *, const void *);

    int _GetRemapAlignOfPerInstData(uint32_t, uint32_t, const CharComponentBase *, const void *, const void *,
                                    const void *, const void *, const void *);

    //virtual
    //0x0037C7CC
    int GetRemapAlignOfPerInstData(uint32_t, uint32_t, const CharComponentBase *, const void *, const void *,
                                   const void *, const void *, const void *);

    void _BuildRemapPerInstData(void *, uint32_t, uint32_t, const CharComponentBase *, const void *, const void *,
                                const void *, const void *, const void *)
    {}

    //virtual
    void BuildRemapPerInstData(void *, uint32_t, uint32_t, const CharComponentBase *, const void *, const void *,
                               const void *, const void *, const void *);

    void _DestroyRemapPerInstData(void *, uint32_t, uint32_t, const CharComponentBase *, const void *, const void *) {}

    //virtual
    void DestroyRemapPerInstData(void *a2, uint32_t a3, uint32_t a4, const CharComponentBase *a5, const void *a6,
                                 const void *a7);

    //virtual
    void _AnimRelease(uint32_t, void *, void *);

    void _CopyPoseDataToNothing(void *, unsigned int, const void *) {}

    //virtual
    void CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3);

    void *_AllocTempPoseData(uint32_t, const void *, const void *);

    //virtual
    void *AllocTempPoseData(uint32_t, const void *, const void *);

    void _DeleteTempPoseData(uint32_t, void *) {}

    //virtual
    void DeleteTempPoseData(uint32_t a1, void *a2);
};

extern void CharComponentBase_patch();
