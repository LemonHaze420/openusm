#pragma once

#include <nal_anim.h>

#include "float.hpp"
#include "nal_system.h"

#include <cstdint>

struct nalBaseSkeleton;

struct linear_interpolator {
    float field_0;
    float field_4;

    //0x00492940
    bool update(Float a2);

    void setup(Float a2, Float a3);
};

namespace nalComp {
struct nalCompInstance;
}

template <typename T, uint32_t I>
struct usm_anim_player {
    enum class usm_anim_player_modifier_type : uint8_t {};

    struct nalAnimState;

    struct nalAnimCallback {
        int m_vtbl;

        //virtual
        void Invoke(usm_anim_player<T, I> *a1);

        //virtual
        void Release();

        //virtual
        void Reference();
    };

    struct nalPlayMethod {
        int m_vtbl;

        //virtual
        void Advance(nalAnimState *a1, Float a2);

        //virtual
        void Compose(nalAnimState *a2, nalAnyPose &a3, nalAnyPose &a4, const nalAnyPose &a5);

        //virtual
        nalComp::nalCompInstance *CreateInstance(nalAnimClass<nalAnyPose> *a1, nalBaseSkeleton *a2, void *a3);

        //virtual
        void Reference(nalAnimState *a2);

        //virtual
        void Release();
    };

    struct nalAnimState {
        nalComp::nalCompInstance *field_0;
        float field_4;
        float field_8;
        nalAnimCallback *field_C;
        nalPlayMethod *field_10;
        usm_anim_player<T, I> *field_14;
        float field_18;
        float field_1C;
        linear_interpolator field_20;
        uint32_t field_28;
        int field_2C;
        int field_30;
        int field_34;
        int field_38;
        int field_3C;
        nalAnimState *field_40;
        int field_44;
        float field_48;
        float field_4C;
        int field_50;

        void sub_854140(nalAnyPose &a2, nalAnyPose &a3);

        void sub_853CF0(nalAnyPose &a2, nalAnyPose &a3);

        void sub_853C80(nalAnyPose &a2, nalAnyPose &a3, const nalAnyPose &a4);

        bool sub_4B0020(usm_anim_player<T, I> *a2, Float a3);

        void sub_4AD850(nalAnimClass<nalAnyPose> *a2, nalBaseSkeleton *a3, Float a4,
                        usm_anim_player<T, I>::nalPlayMethod *a5, Float a6, usm_anim_player<T, I>::nalAnimCallback *a7,
                        Float a8, usm_anim_player<T, I> *a9, void *a10, bool a11, Float a12);
    };

    nalBaseSkeleton *field_0;
    nalAnyPose field_4;
    nalAnyPose field_8;
    nalAnyPose field_C;
    int field_10;
    nalAnimState *field_14[3];
    nalAnimState *field_20;
    nalAnimState *field_24;
    int field_28;

    usm_anim_player(nalBaseSkeleton *a2);

    void PlayModifier(T *a2, usm_anim_player_modifier_type a3, Float a4, nalPlayMethod *a5, Float a6, int a7, Float a8,
                      void *a9, bool a10, void *a11);

    bool IsAnimActive(Float a1)
    {
        return this->Advance(a1) != nullptr;
    }

    nalAnimState *Advance(Float a2);

    void sub_4B06A0(Float a2);

    void sub_4B0860(nalAnyPose &a2);

    void sub_4AE5F0(nalAnimState *a2);

    void sub_4AE210();
};

extern void usm_anim_player_patch();
