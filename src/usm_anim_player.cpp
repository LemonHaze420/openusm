#include "usm_anim_player.h"

#include "common.h"
#include "func_wrapper.h"
#include "nal_anim.h"
#include <nal_anim_comp.h>
#include <nal_instance.h>
#include "nal_system.h"
#include "ngl.h"
#include "utility.h"

using type = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

VALIDATE_SIZE(type, 0x2C);
VALIDATE_OFFSET(type::nalAnimState, field_28, 0x28);


template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_853C80(
        nalAnyPose &a2,
        nalAnyPose &a3,
        const nalAnyPose &a4)
{
    this->field_0->VirtualGetPose(this->field_18, this->field_1C, a3.field_0, a4.field_0);

    sub_826140(a2, this->field_20.field_0, a2, a3);
}


template<>
void usm_anim_player<nalAnimClass<nalAnyPose>,3>::nalPlayMethod::Compose(
        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2,
        nalAnyPose &a3,
        nalAnyPose &a4,
        const nalAnyPose &a5)
{

    a2->sub_853C80(a3, a4, a5);
}


template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_853CF0(
        nalAnyPose &a2,
        nalAnyPose &a3)
{
    auto &v3 = this->field_14->field_C;
    if ( this->field_10 != nullptr ) {
        this->field_10->Compose(
                this,
                a2,
                a3,
                v3);
    } else {
        this->sub_853C80(a2, a3, v3);
    }

    this->field_1C = this->field_18;
}


template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_854140(
        nalAnyPose &a2,
        nalAnyPose &a3)
{
    nalAnyPose *v3 = &a2;
    if ( this->field_50 == 2 ) {
        v3 = &this->field_14->field_C;
    }

    if ( this->field_10 != nullptr ) {
        this->field_10->Compose(
                this,
                a2,
                a3,
                *v3);
    } else {
        this->sub_853C80(a2, a3, *v3);
    }

    this->field_1C = this->field_18;
}

template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::PlayModifier(
        nalAnimClass<nalAnyPose> *a2,
        usm_anim_player_modifier_type a3,
        Float a4,
        nalPlayMethod *a5,
        Float a6,
        int a7,
        Float a8,
        void *a9,
        bool a10,
        void *a11)
{
    if constexpr (0) {
    } else {
        THISCALL(0x004B0530, this, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}

template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4B06A0(Float a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x004B06A0, this, a2);
    }
}

template<>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4B0860(nalAnyPose &pose)
{
    if constexpr (0)
    {
        auto perf_counter = query_perf_counter();

        pose = this->field_4;

        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *v2 = nullptr;
        for ( auto *i = this->field_20; i != nullptr; i = i->field_40 )
        {
            if ( i->field_50 == 2 && i->field_20.field_0 >= 1.0 ) {
                v2 = i;
            }
        }

        if ( v2 == nullptr )
        {
            for ( auto j = this->field_10 - 1; j >= 0; --j ) {
                this->field_14[j]->sub_853CF0(pose, this->field_8);
            }

            v2 = this->field_20;
        }

        for ( auto *k = v2; k != nullptr; k = k->field_40 ) {
            k->sub_854140(pose, this->field_8);
        }

        nalPlayerGetPoseTicks.QuadPart += query_perf_counter().QuadPart - perf_counter.QuadPart;
    }
    else
    {
        THISCALL(0x004B0860, this, &pose);
    }
}

template<>
usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *usm_anim_player<nalAnimClass<nalAnyPose>, 3>::Advance(
        Float a2)
{
    if constexpr (0) {
    } else {
        return (nalAnimState *) THISCALL(0x004AD7F0, this, a2);
    }
}

bool __fastcall sub_4B0020(void *self, void *, int a2, Float a3)
{
    struct {
        char field_0[0x10];
        struct {
            int m_vtbl;
        } *field_10;
    } *v1 = static_cast<decltype(v1)>(self);
    sp_log("0x%08X", v1->field_10->m_vtbl);

    return THISCALL(0x004B0020, self, a2, a3);
}

void __fastcall sub_4AD850(
        void *self,
        void *,
        nalAnimClass<nalAnyPose> *a2,
        int a3,
        Float a4,
        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod *a5,
        Float a6,
        void *a7,
        int a8,
        int a9,
        void *a10,
        bool a11,
        int a12)
{
    sp_log("0x%08X", a5->m_vtbl);
    sp_log("0x%08X", a2->m_vtbl);

    THISCALL(0x004AD850, self, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

void usm_anim_player_patch()
{
    REDIRECT(0x004B06D7, sub_4B0020);

    if constexpr (0)
    {
        REDIRECT(0x004AFF2E, sub_4AD850);
        REDIRECT(0x004B063A, sub_4AD850);
    }

    {
        using type = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;
        FUNC_ADDRESS(address, &type::sub_4B0860);
        REDIRECT(0x004A6172, address);
    }
}
