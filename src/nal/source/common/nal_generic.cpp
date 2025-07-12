#include "nal_generic.h"

#include "func_wrapper.h"
#include "common.h"
#include "nal_system.h"
#include "tl_instance_bank.h"
#include "vtbl.h"

namespace nalGeneric {

VALIDATE_SIZE(nalGenericSkeleton, 0xE4);
VALIDATE_SIZE(nalComponentInfo, 0x30);
VALIDATE_OFFSET(nalGenericSkeleton, field_64, 0x64);

VALIDATE_SIZE(nalGenericPose, 0xC);

int &nalGeneric::nalGenericPose::PoseSP = var<int>(0x0097DA08);

int &nalGeneric::nalGenericPose::PoseStack = var<int>(0x00977204);

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
    vtbl_ptr() = (int) std::addressof(bit_cast<int *>(this)[0]);
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

void nalGenericSkeleton::Process()
{
    if constexpr (1) {
        auto v2 = this->field_64;
        auto v3 = this->field_6C;
        this->field_68 = (int) &this->field_E0;
        auto *v4 = (char *) &this->field_E0 + v2;
        auto v5 = 48 * this->field_74;
        this->field_70 = (int) v4;
        auto v6 = (uint32_t) &v4[v3 + 3] & 0xFFFFFFFC;
        this->field_78 = v6;
        auto v7 = v5 + v6 + 3;
        auto v8 = 5 * this->field_7C;
        auto v9 = this->field_94;
        v7 &= 0xFFFFFFFC;
        this->field_84 = v7;
        auto v10 = (v7 + 8 * v8 + 3) & 0xFFFFFFFC;
        auto v11 = v10 + 48 * this->field_88;
        this->field_8C = CAST(field_8C, v10);
        auto v12 = v11 + v9 - 1;
        auto v13 = this->field_9C;
        auto v14 = ~(v9 - 1) & v12;
        auto v15 = (this->field_90 + v14 + 3) & 0xFFFFFFFC;
        this->field_A0 = v15;
        auto v16 = v13 + v15 + 3;
        auto v17 = this->field_B0;
        v16 &= 0xFFFFFFFC;
        auto v18 = (char *) (v16 + 0x30 * this->field_A4);
        this->field_A8 = CAST(field_A8, v16);
        auto v19 = (int) &v18[v17 - 1];
        auto v20 = this->field_B8;
        auto v21 = ~(v17 - 1) & v19;
        auto v22 = this->field_AC;
        this->field_B4 = v21;
        auto v23 = v22 + v21 + 3;
        auto v24 = this->field_C4;
        v23 &= 0xFFFFFFFC;
        this->field_BC = v23;
        this->field_98 = v14;
        this->field_C8 = ~(v24 - 1) & (v23 + v20 + v24 - 1);
        this->field_CC = (int) this;
        this->field_D0 = v14;

        //sp_log("%s", this->field_8C[0].field_0.c_str());

        for (int i = 0; i < this->field_88; ++i) {
            this->field_8C[i].field_20 = static_cast<decltype(this->field_8C[i].field_20)>(
                nalComponentInstanceBank.Search(this->field_8C[i].field_0)->field_20);
        }

        for (int i = 0; i < this->field_A4; ++i) {
            this->field_A8[i].field_20 = static_cast<decltype(this->field_A8[i].field_20)>(
                nalComponentInstanceBank.Search(this->field_A8[i].field_0)->field_20);
        }

        auto v29 = this->field_A0;

        auto *v30 = this->field_8C;
        auto v38 = this->field_98;

        auto v37 = v29;
        if (this->field_88 > 0) {
            for (int i = 0; i < this->field_88; ++i) {
                void (__fastcall *Process)(void *, void *, void *, void *, void *) = CAST(Process, get_vfunc(v30->field_20->m_vtbl, 0x10));
                Process(v30->field_20, nullptr, v30, &v38, &v37);

                ++v30;
            }
        }

        auto v34 = this->field_BC;
        auto *v35 = this->field_A8;
        v38 = this->field_B4;

        v37 = v34;
        if (this->field_A4 > 0) {
            for (int i = 0; i < this->field_A4; ++i) {
                void (__fastcall *Process)(void *, void *, void *, void *, void *) = CAST(Process, get_vfunc(v35->field_20->m_vtbl, 0x10));

                Process(v35->field_20, nullptr, v35, &v38, &v37);

                ++v35;
            }
        }

    } else {
        THISCALL(0x00793610, this);
    }
}

void nalGenericSkeleton::Release() {
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

