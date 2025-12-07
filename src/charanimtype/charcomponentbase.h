#pragma once

#include "component.h"

namespace nalComp {
struct nalCompAnim;
}

struct CharComponentBase : BaseComponent {
    enum Names {
        Generic                                      = 0,
        ArbitraryPO                                  = 1,
        TorsoHeadOneNeckEntropyCompressed            = 2,
        ArmsAndHandsEntropyCompressed                = 3,
        FiveFingerTopTwoKnuckleCurlEntropyCompressed = 4,
        LegsAndFeetIKEntropyCompressed               = 5,
        FakerootEntropyCompressed                    = 6,
        TentaclesCompressed                          = 7
    };

    BaseComponent *m_pSubComponent;
    uint32_t m_TheType;
    const char *m_strTypeString;
    int field_10;

    CharComponentBase();

    //virtual
    uint32_t GetType();
    
    //virtual
    int DoesContributeToPose(
        uint32_t a2,
        const void *a3,
        const void *a4);

    //virtual
    void * GetSizeOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool a8);

    //virtual
    void * GetAlignOfPerInstData(
        uint32_t a2,
        const void *a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        bool a8);

    //virtual
    void BuildPerInstData(
        void *a2,
        uint32_t a3,
        const void *a4,
        const void *a5,
        const void *a6,
        const void *a7,
        const void *a8,
        bool a9);

    //virtual
    void * _CalcPoseDataDirect(
        void *a2,
        uint32_t a3,
        Float a4,
        Float a5,
        const nalComp::nalCompAnim *a6,
        const void *a7,
        const void *a8,
        const void *a9,
        void *a10);

    //virtual
    void CalcPoseDataRemapped(
        void *a2,
        uint32_t ,
        Float a4,
        Float a5,
        const nalComp::nalCompAnim *a6,
        const void *a7,
        const void *a8,
        const void *a9,
        void *);

    //virtual
    void CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3);
};

extern void CharComponentBase_patch();
