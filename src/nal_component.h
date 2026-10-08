#pragma once

#include <cstdint>
#include <type_traits>
#include <nal_generic.h>
#include "utility.h"
#include "vtbl.h"

struct nalComponentEntropyQuat;
struct nalComponentPacked16EntropyQuat;
struct nalComponentEntropyFloat1;
struct nalComponentEntropyFloat3;
struct nalComponentPO;
struct nalComponentEntropyPO;
struct nalComponentEntropyTrajectoryPO;
struct nalComponentRLE8Int1;
struct USMEventComp;
struct spideySignal;
struct nalComponentMorphSlider;

enum class nalNativeStreamEncoding {
    RLE8,
    Signal,
    Event,
    EntropyFloat1,
    EntropyFloat3,
    EntropyQuat,
    Packed16EntropyQuat,
    PO,
    EntropyPO,
    EntropyTrajectoryPO,
    MorphSlider
};

std::intptr_t nalNativeStreamTable(nalNativeStreamEncoding encoding);
void nalDecodeEntropyScalar(const void *input, void *output, unsigned frames, int stride, float scale);
void nalDecodeEntropyQuaternion(const void *input, void *output, unsigned frames, int stride, float scale);

template <class T0, class T1, class T2>
struct nalComponent : T0, T1 {
    static constexpr nalNativeStreamEncoding Encoding = [] {
        if constexpr (std::is_same_v<T2, nalComponentRLE8Int1>)
            return nalNativeStreamEncoding::RLE8;
        else if constexpr (std::is_same_v<T2, spideySignal>)
            return nalNativeStreamEncoding::Signal;
        else if constexpr (std::is_same_v<T2, USMEventComp>)
            return nalNativeStreamEncoding::Event;
        else if constexpr (std::is_same_v<T2, nalComponentEntropyFloat1>)
            return nalNativeStreamEncoding::EntropyFloat1;
        else if constexpr (std::is_same_v<T2, nalComponentEntropyFloat3>)
            return nalNativeStreamEncoding::EntropyFloat3;
        else if constexpr (std::is_same_v<T2, nalComponentEntropyQuat>)
            return nalNativeStreamEncoding::EntropyQuat;
        else if constexpr (std::is_same_v<T2, nalComponentPacked16EntropyQuat>)
            return nalNativeStreamEncoding::Packed16EntropyQuat;
        else if constexpr (std::is_same_v<T2, nalComponentPO>)
            return nalNativeStreamEncoding::PO;
        else if constexpr (std::is_same_v<T2, nalComponentEntropyPO>)
            return nalNativeStreamEncoding::EntropyPO;
        else if constexpr (std::is_same_v<T2, nalComponentMorphSlider>)
            return nalNativeStreamEncoding::MorphSlider;
        else {
            static_assert(std::is_same_v<T2, nalComponentEntropyTrajectoryPO>);
            return nalNativeStreamEncoding::EntropyTrajectoryPO;
        }
    }();

    nalComponent()
    {
        T0::m_vtbl = nalNativeStreamTable(Encoding);
    }

    void _Process(const nalGeneric::nalComponentInfo *info, void *&skel, void *&anim)
    {
        void(__fastcall * func)(void *, void *, const nalGeneric::nalComponentInfo *, void **, void **) =
            CAST(func, get_vfunc(T0::m_vtbl, 0x10));
        func(this, nullptr, info, &skel, &anim);
    }
};
