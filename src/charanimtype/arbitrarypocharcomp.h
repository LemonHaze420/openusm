#pragma once

#include "charcomponentbase.h"

#include "float.hpp"

struct ArbitraryPOCharComp : CharComponentBase {

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
            const void *a7,
            const void *a8,
            void *a9);
};

extern void ArbitraryPOCharComp_patch();
