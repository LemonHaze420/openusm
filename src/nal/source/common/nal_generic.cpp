#include "nal_generic.h"

#include "func_wrapper.h"
#include "common.h"
#include "nal_system.h"
#include "tl_instance_bank.h"
#include "trace.h"
#include "variables.h"
#include "vtbl.h"

namespace nalGeneric {

VALIDATE_SIZE(nalGenericSkeleton, 0xE4);
VALIDATE_SIZE(nalComponentInfo, 0x30);
VALIDATE_OFFSET(nalGenericSkeleton, field_64, 0x64);

VALIDATE_SIZE(nalGenericPose, 0xC);

#if !STANDALONE_SYSTEM

int & nalGenericPose::PoseSP = var<int>(0x0097DA08);

int & nalGenericPose::PoseStack = var<int>(0x00977204);

int & nalGenericAnim::vtbl_ptr = var<int>(0x00977120);

int & nalGenericSkeleton::vtbl_ptr = var<int>(0x009770E0);

#else

int & nalGenericPose::PoseSP = []() -> auto & {
    static int g_PoseSP {};
    return g_PoseSP;
}();

int & nalGenericPose::PoseStack = []() -> auto & {
    static int g_PoseStack {};
    return g_PoseStack;
}();

int & nalGenericAnim::vtbl_ptr = []() -> auto & {
    static int g_vtbl_ptr {};
    return g_vtbl_ptr;
}();

int & nalGenericSkeleton::vtbl_ptr = []() -> auto & {
    static nalGenericSkeleton skel {};
    return skel.m_vtbl;
}();

#endif

void nalGenericInstance::GetPose(
        Float a2,
        Float a3,
        nalGeneric::nalGenericPose &a4,
        const nalGeneric::nalGenericPose &a5)
{
    THISCALL(0x007946A0, this, a2, a3, &a4, &a5);
}

nalGenericSkeleton::nalGenericSkeleton()
{
    if constexpr (1) {
        static void * g_vtbl[] {
            nullptr,
            nullptr,
            func_address(&_Process),
            func_address(&_Release),
            func_address(&_CheckVersion)
        };

        m_vtbl = CAST(m_vtbl, &g_vtbl);
    }
}

nalMatrix4x4 * nalGenericSkeleton::GetBoneMatrices(
        const nalGeneric::nalGenericPose *a2,
        nalMatrix4x4 *a3) const
{
    if constexpr (0) {
    } else {
        return (nalMatrix4x4 *) THISCALL(0x00794CF0, this, a2, a3);
    }
}

void nalGenericSkeleton::_Process()
{
    TRACE("nalGenericSkeleton::Process");

    if constexpr (1) {

        auto func = [](int a1, int a2) -> int {
              return ~(a2 - 1) & (a1 + a2 - 1);
        };

        this->field_68 = func((int)&this->field_E0, 1);
        this->field_70 = func(this->field_64 + this->field_68, 1);
        this->field_78 = func(this->field_6C + this->field_70, 4);
        this->field_84 = func(this->field_78 + 48 * this->field_74, 4);

        this->field_8C = CAST(field_8C, func(this->field_84 + 40 * this->field_7C, 4));

        this->field_98 = func((int)&this->field_8C[this->field_88], this->field_94);

        this->field_A0 = func(this->field_90 + this->field_98, 4);

        this->field_A8 = CAST(field_A8, func(this->field_9C + this->field_A0, 4));

        this->field_B4 = func(int(this->field_A8 + 48 * this->field_A4), this->field_B0);

        this->field_BC = func(this->field_AC + this->field_B4, 4);

        this->field_C8 = func(this->field_B8 + this->field_BC, this->field_C4);

        this->field_CC.field_0 = this;
        this->field_CC.field_4 = this->field_98;

        for (int i = 0; i < this->field_88; ++i) {
            auto *inst = nalComponentInstanceBank.Search(this->field_8C[i].field_0);
            assert(inst != nullptr && "could not find an instance of the encoding type of a pose component");
            this->field_8C[i].field_20 = static_cast<decltype(this->field_8C[i].field_20)>(
                inst->field_20);
        }

        for (int i = 0; i < this->field_A4; ++i) {
            auto *inst = nalComponentInstanceBank.Search(this->field_A8[i].field_0);
            assert(inst != nullptr && "could not find an instance of the encoding type of a const component");

            this->field_A8[i].field_20 = static_cast<decltype(this->field_A8[i].field_20)>(inst->field_20);
        }

        auto v29 = this->field_A0;

        auto *v30 = this->field_8C;
        auto v38 = (void *)this->field_98;
        auto v37 = (void *)v29;

        for (int i = 0; i < this->field_88; ++i) {
            v30->field_20->Process(v30, v38, v37);

            ++v30;
        }

        auto v34 = this->field_BC;
        auto *v35 = this->field_A8;
        v38 = (void *)this->field_B4;

        v37 = (void *)v34;
        for (int i = 0; i < this->field_A4; ++i) {
            v35->field_20->Process(v35, v38, v37);

            ++v35;
        }

    } else {
        THISCALL(0x00793610, this);
    }
}

void nalGenericSkeleton::_Release() {
    ;
}

template<>
void nalGenericSkeleton::GetComponentHandle<nalPositionOrientation>(
        nalGenericConstComponentHandle<nalPositionOrientation> &a2,
        tlFixedString &a3,
        tlFixedString &a4) const
{
    THISCALL(0x004AE7B0, this, &a2, &a3, &a4);
}

template<>
void nalGenericSkeleton::GetComponentHandle<float>(
        nalGenericComponentHandle<float> &a2,
        tlFixedString &a3,
        tlFixedString &a4)
{
    THISCALL(0x004AE9F0, this, &a2, &a3, &a4);
}

template<>
void nalGenericSkeleton::GetComponentHandle<float>(
        nalGenericConstComponentHandle<float> &a2,
        tlFixedString &a3,
        tlFixedString &a4) const
{
    THISCALL(0x004AEC90, this, &a2, &a3, &a4);
}

template<>
void nalGenericSkeleton::GetComponentHandle<nalPositionOrientation>(
        nalGenericComponentHandle<nalPositionOrientation> &a2,
        tlFixedString &a3,
        tlFixedString &a4)
{
    THISCALL(0x004AE660, this, &a2, &a3, &a4);
}

template<>
void nalGenericSkeleton::GetComponentHandle<unsigned char>(
        nalGenericComponentHandle<unsigned char> &a2,
        tlFixedString &a3,
        tlFixedString &a4)
{
    THISCALL(0x004AEB40, this, &a2, &a3, &a4);
}

template<>
void nalGenericSkeleton::GetComponentHandle<nalVector3>(
        nalGenericConstComponentHandle<nalVector3> &a2,
        uint32_t a3,
        tlFixedString &a4) const
{
    THISCALL(0x004AEF30, this, &a2, &a3, &a4);
}

int * sub_796F90(unsigned int a1)
{
    return (int *) CDECL_CALL(0x00796F90, a1);
}

nalGenericPose::nalGenericPose() {
    this->field_8 = false;
}

nalGeneric::nalGenericPose::nalGenericPose(const nalGeneric::nalGenericSkeleton *a3)
{
    auto v2 = a3;
    this->field_0 = bit_cast<nalGenericSkeleton *>(a3);
    this->field_4 = 0;
    this->field_8 = false;
    if ( sub_101BF70((uint32_t)this) )
    {
        auto v5 = ~(v2->field_94 - 1) & (nalGeneric::nalGenericPose::PoseSP + v2->field_94 - 1);
        this->field_4 = (int)&nalGeneric::nalGenericPose::PoseStack + v5;
        auto v6 = (v2->field_90 + v5 + 3) & 0xFFFFFFFC;
        auto v7 = (int *)((char *)&nalGeneric::nalGenericPose::PoseStack + v6);
        auto v8 = v6 + 4;
        if ( v8 <= 0x2800 )
        {
            *v7 = nalGeneric::nalGenericPose::PoseSP;
            nalGeneric::nalGenericPose::PoseSP = v8;
        }
        else
        {
            this->field_4 = 0;
        }
    }

    if ( !this->field_4 )
    {
        this->field_4 = (int)sub_796F90(this->field_0->field_90);
        this->field_8 = true;
    }

    auto v9 = this->field_0;
    auto v3 = (void *)this->field_4;
    auto v10 = v9->field_8C;
    for ( int v4 = 0; v4 < v9->field_88; ++v4 )
    {
        struct {
            char field_0[0x2C];
            void (__fastcall *Construct)(void *, void *edx, const nalGeneric::nalComponentInfo *, void *&);
        } *vtbl = CAST(vtbl, v10->field_20->m_vtbl);

        vtbl->Construct(v10->field_20, nullptr,
                v10,
                v3);
        ++v10;
    }
}

nalGeneric::nalGenericPose::nalGenericPose(
        const nalGeneric::nalGenericPose &a3,
        bool a4)
{
    auto v4 = a3.field_0;
    this->field_0 = a3.field_0;
    this->field_4 = 0;
    this->field_8 = 0;

    if ( sub_101BF70((uint32_t)this) )
    {
        auto v6 = ~(v4->field_94 - 1) & (nalGeneric::nalGenericPose::PoseSP + v4->field_94 - 1);
        this->field_4 = (int)&nalGeneric::nalGenericPose::PoseStack + v6;
        auto v7 = (v4->field_90 + v6 + 3) & 0xFFFFFFFC;
        auto v8 = (int *)((char *)&nalGeneric::nalGenericPose::PoseStack + v7);
        auto v9 = v7 + 4;
        if ( v9 <= 0x2800 )
        {
            *v8 = nalGeneric::nalGenericPose::PoseSP;
            nalGeneric::nalGenericPose::PoseSP = v9;
        }
        else
        {
            this->field_4 = 0;
        }
    }

    if ( !this->field_4 )
    {
        this->field_4 = (int) sub_796F90(this->field_0->field_90);
        this->field_8 = true;
    }

    auto *v10 = this->field_0;
    auto *v13 = bit_cast<void *>(this->field_4);
    auto *v11 = v10->field_8C;

    for ( int v5 = 0; v5 < v10->field_88; ++v5 )
    {
        struct {
            char field_0[0x2C];
            void (__fastcall *Construct)(void *, void *edx, const nalGeneric::nalComponentInfo *, void *&);
        } *vtbl = CAST(vtbl, v11->field_20->m_vtbl);

        vtbl->Construct(v11->field_20, nullptr, v11, v13);
        ++v11;
    }

    if ( a4 ) {
        (*this) = a3;
    }
}

} // namespace nalGeneric

void nalGeneric_patch()
{
}
