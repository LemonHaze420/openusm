#pragma once

#include "float.hpp"

#include <cstdint>

struct nalAnyPose;

struct linear_interpolator {
    float field_0;
    float field_4;
};

namespace nalComp {
struct nalCompInstance;
}

template<typename T, uint32_t I>
struct usm_anim_player {
    enum class usm_anim_player_modifier_type : uint8_t {};

    struct nalPlayMethod {
        int m_vtbl;
    };

    struct nalAnimState {
        nalComp::nalCompInstance *field_0;
        float field_4;
        float field_8;
        struct {
            int m_vtbl;
        } *field_C;
        nalPlayMethod *field_10;
        usm_anim_player<T, I> *field_14;
        float field_18;
        float field_1C;
        linear_interpolator field_20;
        uint32_t field_28;
    };

    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    nalAnimState *field_14[1];
    int field_18;
    int field_1C;
    linear_interpolator field_20;
    int field_28;

    void PlayModifier(
        T *a2,
        usm_anim_player_modifier_type a3,
        Float a4,
        nalPlayMethod *a5,
        Float a6,
        int a7,
        Float a8,
        void *a9,
        bool a10,
        void *a11);

    bool IsAnimActive(Float a1) {
        return this->Advance(a1) != nullptr;
    }

    nalAnimState *Advance(Float a2);

    void sub_4B06A0(Float a2);

    void sub_4B0860(nalAnyPose &a2);
};

extern void usm_anim_player_patch();
