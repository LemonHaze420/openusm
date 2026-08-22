#pragma once

#include "charcomponentbase.h"

#include "charcomponentmanager.h"
#include "string_hash.h"
#include "utility.h"

template <typename T0, typename T1>
struct FlexibleCharComp : CharComponentBase {
    T0 field_14;
    T1 field_15;
    int field_1C;
    T0::StdPoseData field_20;

    FlexibleCharComp(int a2, const char *a1)
    {
        if constexpr (1) {
            static void *g_vtbl[]{nullptr,
                                  func_address(&_GetType),
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  func_address(&_SkelPoseProcess),
                                  nullptr,
                                  func_address(&_AnimProcess),
                                  func_address(&_AnimRelease),
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  func_address(&_CopyPoseDataToNothing)};
            this->m_vtbl = CAST(m_vtbl, &g_vtbl);
        } else {
            this->m_vtbl = 0x008921C0;
        }

        this->m_strTypeString = a1;
        this->m_TheType = to_hash(a1);
        this->field_10 = a2;
        CharComponentManager::RegisterComponent(this);
    }

    //0x005FE940
    void _CalcPoseDataDirect(void *a2, uint32_t a3, Float a4, Float a5, const nalComp::nalCompAnim *a6, const void *a7,
                             const void *a8, const void *a9, void *a10);

    void _CalcPoseDataRemapped(void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *, const void *,
                               const void *, void *);

    void _CalcPoseDataRemapped(void *a2, uint32_t a3, Float a4, Float a5, const nalComp::nalCompAnim *a6,
                               const void *a7, uint32_t a8, uint32_t a9, const void *a10, const void *a11,
                               const void *a12, const void *a13, void *a14);

    //virtual
    void _SkelPoseProcess(uint32_t a1, void *a2, void *a3)
    {
        this->field_14.SkelPoseProcess(a1, static_cast<T0::PerSkelData *>(a2), static_cast<T0::StdPoseData *>(a3));
    }

    //virtual
    void _AnimProcess(uint32_t, void *, void *, const void *) {}

    //virtual
    void _AnimRelease(uint32_t, void *, void *, const void *) {}

    //virtual
    void _CopyPoseDataToNothing(void *a1, unsigned int a2, const void *a3)
    {
        this->field_14.CopyPoseDataToNothing(
            static_cast<T0::StdPoseData *>(a1), a2, static_cast<const T0::StdPoseData *>(a3));
    }


    //virtual
    int GetDomain() const;
};

extern void FlexibleCharComp_patch();
