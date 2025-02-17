#pragma once

#include "variable.h"

struct BaseComponent;
struct CharComponentBase;

struct CharComponentManager {
    static inline int & iCurrNumComponents = var<int>(0x0096A558);

    static inline BaseComponent **& pCompArray = var<BaseComponent **>(0x0096A55C);

    static void RegisterComponent(CharComponentBase *a1);
};
