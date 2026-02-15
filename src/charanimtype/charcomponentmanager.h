#pragma once

#include <cstdint>

struct BaseComponent;
struct CharComponentBase;

struct CharComponentManager {
    static int & iCurrNumComponents;

    static BaseComponent **& pCompArray;

    static void RegisterComponent(CharComponentBase *a1);

    static void InitComponentArray();

    static BaseComponent * GetComponentByType(uint32_t a1);

    static inline constexpr auto iMaxNumComponents = 50;
};
