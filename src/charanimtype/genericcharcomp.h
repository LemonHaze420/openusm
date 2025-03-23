#pragma once

#include "charcomponentbase.h"

struct GenericCharComp : CharComponentBase {
    GenericCharComp();

    //virtual
    void BuildBoneMatrices(
        nalMatrix4x4 *a1,
        uint32_t a2,
        const void *a3,
        const void *a4);
};
