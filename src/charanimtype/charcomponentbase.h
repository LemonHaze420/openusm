#pragma once

#include "component.h"

struct CharComponentBase : BaseComponent {
    enum Names {};

    int m_pSubComponent;
    uint32_t m_TheType;
    const char *m_strTypeString;

    //virtual
    uint32_t GetType();

    //virtual
    void CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3);
};

extern void CharComponentBase_patch();
