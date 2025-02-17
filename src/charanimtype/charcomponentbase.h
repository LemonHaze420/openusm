#pragma once

#include "component.h"

struct CharComponentBase : BaseComponent {
    enum Names {};

    int field_4;
    uint32_t field_8;
    const char *field_C;

    //virtual
    uint32_t GetType();

    //virtual
    void CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3);
};

extern void CharComponentBase_patch();
