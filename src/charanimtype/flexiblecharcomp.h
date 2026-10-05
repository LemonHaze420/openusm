#pragma once

#include "charcomponentbase.h"
#include "charcomponentmanager.h"
#include "charcompressor.h"
#include "func_wrapper.h"
#include "nal_system.h"
#include "nal_math.h"
#include "string_hash.h"
#include "utility.h"
#include "vector4d.h"

#include <cstring>
#include <new>
#include <type_traits>

struct FakerootPoseDesc;
struct TorsoHeadStdPoseDesc;
struct LegsStdPoseDesc;
struct LegsIKPoseDesc;
struct ArmStdPoseDesc;
struct ArmIKPoseDesc;
struct TentaclesPoseDesc;
struct Fing52KnuckCurlPoseDesc;
struct Fing5CurlPoseDesc;
struct Fing5ReducedPoseDesc;
struct Finger5StdPoseDesc;
namespace nalChar { struct nalCharAnim; }



template <typename T0, typename T1>
struct FlexibleCharComp : CharComponentBase {
    T0 field_14;
    T1 field_15;
    typename T0::StdPoseData field_18;

    static constexpr int PoseKind = [] {
        if constexpr (std::is_same_v<T0, FakerootPoseDesc>) return 0;
        else if constexpr (std::is_same_v<T0, TorsoHeadStdPoseDesc>) return 1;
        else if constexpr (std::is_same_v<T0, LegsStdPoseDesc>) return 2;
        else if constexpr (std::is_same_v<T0, LegsIKPoseDesc>) return 3;
        else if constexpr (std::is_same_v<T0, ArmStdPoseDesc>) return 4;
        else if constexpr (std::is_same_v<T0, ArmIKPoseDesc>) return 5;
        else if constexpr (std::is_same_v<T0, TentaclesPoseDesc>) return 6;
        else if constexpr (std::is_same_v<T0, Fing52KnuckCurlPoseDesc>) return 7;
        else if constexpr (std::is_same_v<T0, Fing5CurlPoseDesc>) return 8;
        else if constexpr (std::is_same_v<T0, Fing5ReducedPoseDesc>) return 9;
        else return 10;
    }();
    static constexpr int DecoderOffset = PoseKind == 0 ? 0x70 : 2 * sizeof(typename T0::StdPoseData);
    static constexpr int InstanceSize = PoseKind == 0 ? 0x90 : DecoderOffset + 0x10;

    FlexibleCharComp(int flags, const char *name)
    {
        using DirectRemap = void (FlexibleCharComp::*)(void *, uint32_t, Float, Float,
            const nalComp::nalCompAnim *, const void *, uint32_t, uint32_t, const void *, const void *, void *);
        using ComponentRemap = void (FlexibleCharComp::*)(void *, uint32_t, Float, Float,
            const nalComp::nalCompAnim *, const void *, uint32_t, uint32_t, const void *, const void *,
            const void *, const void *, void *);
        static void *table[]{
            func_address(&FlexibleCharComp::_DestroyComponent),
            func_address(&CharComponentBase::_GetType),
            func_address(&FlexibleCharComp::_ApplyPublicPerSkelDataOffset),
            func_address(&FlexibleCharComp::_ApplyPublicPerAnimDataOffset),
            func_address(&FlexibleCharComp::_GetTrajectoryData),
            func_address(&FlexibleCharComp::_BuildBoneMatrices),
            func_address(&FlexibleCharComp::_DoesContributeToPose),
            func_address(&FlexibleCharComp::_GetSizeOfPerInstData),
            func_address(&FlexibleCharComp::_GetAlignOfPerInstData),
            func_address(&FlexibleCharComp::_BuildPerInstData),
            func_address(&FlexibleCharComp::_DestroyPerInstData),
            func_address(&FlexibleCharComp::_WillMapToComponentData),
            func_address(&FlexibleCharComp::_CalcPoseDataDirect),
            func_address(static_cast<DirectRemap>(&FlexibleCharComp::_CalcPoseDataRemapped)),
            func_address(&FlexibleCharComp::_BlendPoseData),
            func_address(&FlexibleCharComp::_SkelPoseProcess),
            func_address(&FlexibleCharComp::_SkelPoseRelease),
            func_address(&FlexibleCharComp::_AnimProcess),
            func_address(&FlexibleCharComp::_AnimRelease),
            func_address(&FlexibleCharComp::_CopyPoseDataToNothing),
            func_address(&FlexibleCharComp::_PoseDataFree),
            func_address(&FlexibleCharComp::_GetDomain),
            func_address(&FlexibleCharComp::_GetPoseTypeID),
            func_address(&FlexibleCharComp::_GetRemapSizeOfPerInstData),
            func_address(&FlexibleCharComp::_GetRemapAlignOfPerInstData),
            func_address(&FlexibleCharComp::_BuildRemapPerInstData),
            func_address(&FlexibleCharComp::_DestroyRemapPerInstData),
            func_address(static_cast<ComponentRemap>(&FlexibleCharComp::_CalcPoseDataRemapped)),
            func_address(&CharComponentBase::_AnimRelease),
            func_address(&FlexibleCharComp::_CopyPoseDataToNothing),
            func_address(&FlexibleCharComp::_AllocTempPoseData),
            func_address(&CharComponentBase::_DeleteTempPoseData)};
        m_vtbl = CAST(m_vtbl, table);
        m_strTypeString = name;
        m_TheType = to_hash(name);
        field_10 = flags;
        if constexpr (PoseKind == 1) field_14.field_0 = (flags & 0x800000) != 0;
        CharComponentManager::RegisterComponent(this);
    }

    FlexibleCharComp *_DestroyComponent(unsigned char flags)
    {
        this->~FlexibleCharComp();
        if (flags & 1) operator delete(this);
        return this;
    }
    void *_ApplyPublicPerSkelDataOffset(uint32_t, void *) const { return nullptr; }
    void *_ApplyPublicPerAnimDataOffset(uint32_t, const void *data) const
    {
        if constexpr (PoseKind == 0) return const_cast<void *>(data);
        else return nullptr;
    }
    nalPositionOrientation *_GetTrajectoryData(nalPositionOrientation *out, uint32_t, const void *, const void *pose)
    {

        if constexpr (PoseKind == 0) std::memcpy(out, pose, 0x1C);
        return out;
    }
    void _BuildBoneMatrices(nalMatrix4x4 *out, uint32_t index, const void *skel, const void *pose)
    {

        if constexpr (PoseKind != 0 && PoseKind != 6)
            field_14.BuildBoneMatrices(out, index, static_cast<const typename T0::PerSkelData *>(skel),
                static_cast<const typename T0::StdPoseData *>(pose));
    }
    int _DoesContributeToPose(uint32_t, const void *, const void *)
    {
        return PoseKind != 3 && PoseKind != 5;
    }
    static int TrackCount(const void *data)
    {
        const auto mask = static_cast<const uint32_t *>(data)[PoseKind == 0 ? 7 : 0];
        int count = 0;
        const int bits = PoseKind == 0 ? 2 : PoseKind == 1 ? 6 : PoseKind == 2 || PoseKind == 4 ? 8 :
            PoseKind == 3 || PoseKind == 5 ? 4 : PoseKind == 6 ? 15 : PoseKind == 8 ? 10 : 30;
        for (int i = 0; i < bits; ++i) {
            if (!(mask & (uint32_t(1) << i))) continue;
            if constexpr (PoseKind == 0) count += i == 0 ? 6 : 1;
            else if constexpr (PoseKind == 1) count += i == 5 ? 6 : 3;
            else if constexpr (PoseKind == 3 || PoseKind == 5) count += i < 2 ? 3 : 7;
            else if constexpr (PoseKind == 6) ++count;
            else if constexpr (PoseKind == 7 || PoseKind == 9) count += i < 2 ? 3 : i < 10 ? 2 : 1;
            else if constexpr (PoseKind == 8) count += i < 2 ? 4 : 2;
            else count += 3;
        }
        return count;
    }
    int _GetSizeOfPerInstData(uint32_t, const void *, const void *, const void *, const void *mask, const void *, bool)
    {
        return InstanceSize + 16 * TrackCount(mask);
    }
    int _GetAlignOfPerInstData(uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    {
        return 16;
    }
    void _BuildPerInstData(void *out, uint32_t, const void *, const void *, const void *, const void *mask,
        const void *tracks, bool)
    {
        if (out) {
            auto *decoder = reinterpret_cast<CharEntropyDecoder::CharChannelDecoder *>(
                static_cast<char *>(out) + DecoderOffset);
            new (decoder) CharEntropyDecoder::CharChannelDecoder(tracks, false);
            auto *state = reinterpret_cast<int *>(decoder + 1);
            state[0] = TrackCount(mask);
            state[1] = -1;
            if constexpr (PoseKind == 0) *reinterpret_cast<float *>(static_cast<char *>(out) + 0x8C) = -1.0f;
        }
    }

    void _DestroyPerInstData(void *, uint32_t, const void *, const void *) {}
    void _SkelPoseProcess(uint32_t, void *, void *) {}
    void _SkelPoseRelease(uint32_t, void *, void *) {}
    void _AnimProcess(uint32_t, void *, void *, const void *) {}
    void _AnimRelease(uint32_t, void *, void *, const void *) {}
    void _PoseDataFree(uint32_t, void *) {}

    bool _WillMapToComponentData(uint32_t, uint32_t, uint32_t type)
    {
        auto *generic = CharComponentManager::GetComponentByType(to_hash("generic"));
        if (generic->GetType() == type) return false;
        auto *source = static_cast<CharComponentBase *>(CharComponentManager::GetComponentByType(type));
        return source->GetDomain() == _GetDomain() && source->GetPoseTypeID() == _GetPoseTypeID();
    }
    void _CalcPoseDataDirect(void *out, uint32_t index, Float time, Float weight, const nalComp::nalCompAnim *anim,
        const void *skel, const void *animData, const void *tracks, void *instance)
    {
        field_15.GetPose(static_cast<typename T0::StdPoseData *>(out), index, time, weight,
            bit_cast<const nalChar::nalCharAnim *>(anim), static_cast<const typename T0::PerSkelData *>(skel),
            static_cast<const typename T0::PerAnimData *>(animData), tracks,
            static_cast<typename T1::PerInstData *>(instance), field_14);
    }

    void _CalcPoseDataRemapped(void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *,
        uint32_t, uint32_t, const void *, const void *, void *) {}
    void _CalcPoseDataRemapped(void *out, uint32_t, Float time, Float weight, const nalComp::nalCompAnim *anim,
        const void *, uint32_t index, uint32_t type, const void *skel, const void *pose,
        const void *animData, const void *tracks, void *instance)
    {
        auto *source = static_cast<CharComponentBase *>(CharComponentManager::GetComponentByType(type));
        if (source->GetPoseTypeID() == _GetPoseTypeID()) {
            source->CalcPoseDataDirect(out, index, time, weight, anim, skel, animData, tracks, instance);
        } else {
            auto *temp = source->AllocTempPoseData(index, skel, pose);
            source->CalcPoseDataDirect(temp, index, time, weight, anim, skel, animData, tracks, instance);
            source->DeleteTempPoseData(index, temp);
        }
    }
    void _BlendPoseData(void *out, uint32_t, Float blend, const void *a, const void *b)
    {
        auto *dst = static_cast<float *>(out);
        const auto *left = static_cast<const float *>(a);
        const auto *right = static_cast<const float *>(b);
        constexpr int quats = PoseKind == 0 ? 1 : PoseKind == 1 ? 6 : PoseKind == 2 || PoseKind == 4 ? 8 :
            PoseKind == 3 || PoseKind == 5 ? 4 : PoseKind == 6 ? 0 : PoseKind == 10 ? 30 : 2;
        for (int i = 0; i < quats; ++i)
            reinterpret_cast<vector4d *>(dst)[i] = math::Slerp(blend,
                reinterpret_cast<const vector4d *>(left)[i], reinterpret_cast<const vector4d *>(right)[i]);
        constexpr int end = PoseKind == 0 ? 8 : PoseKind == 1 ? 27 : PoseKind == 3 || PoseKind == 5 ? 24 :
            PoseKind == 6 ? 15 : PoseKind == 7 ? 34 : PoseKind == 8 ? 26 : PoseKind == 9 ? 44 : 4 * quats;
        for (int i = 4 * quats; i < end; ++i) {
            if constexpr (PoseKind == 0 || PoseKind == 7 || PoseKind == 8 || PoseKind == 9)
                dst[i] = (1.0f - blend) * left[i] + blend * right[i];
            else dst[i] = (right[i] - left[i]) * blend + left[i];
        }
        if constexpr (PoseKind == 0) std::memcpy(dst + 8, right + 8, 8);
    }
    int _GetDomain() const
    {
        return PoseKind == 0 ? 6 : PoseKind == 1 ? 2 : PoseKind == 2 || PoseKind == 3 ? 5 :
            PoseKind == 4 || PoseKind == 5 ? 3 : PoseKind == 6 ? 7 : 4;
    }
    uint32_t _GetPoseTypeID() const
    {
        constexpr const char *names[]{"Fakeroot", "StandardTorsoHead", "StandardLegsFeet", "IKLegsFeet",
            "StandardArmsHands", "IKArmsHand", "", "2KnuckCurlFingers5", "CurlFingers5",
            "ReducedFingers5", "StandardFingers5"};
        return to_hash(names[PoseKind]);
    }
    int _GetRemapSizeOfPerInstData(uint32_t, uint32_t index, const CharComponentBase *source,
        const void *skel, const void *pose, const void *anim, const void *mask, const void *tracks)
    {
        return const_cast<CharComponentBase *>(source)->GetSizeOfPerInstData(index, skel, pose, anim, mask, tracks, false);
    }
    int _GetRemapAlignOfPerInstData(uint32_t, uint32_t index, const CharComponentBase *source,
        const void *skel, const void *pose, const void *anim, const void *mask, const void *tracks)
    {
        return const_cast<CharComponentBase *>(source)->GetAlignOfPerInstData(index, skel, pose, anim, mask, tracks, false);
    }
    void _BuildRemapPerInstData(void *out, uint32_t, uint32_t index, const CharComponentBase *source,
        const void *skel, const void *pose, const void *anim, const void *mask, const void *tracks)
    {
        const_cast<CharComponentBase *>(source)->BuildPerInstData(out, index, skel, pose, anim, mask, tracks, false);
    }
    void _DestroyRemapPerInstData(void *out, uint32_t, uint32_t index, const CharComponentBase *source,
        const void *skel, const void *pose)
    {
        const_cast<CharComponentBase *>(source)->DestroyPerInstData(out, index, skel, pose);
    }
    void _CopyPoseDataToNothing(void *out, uint32_t, const void *pose)
    {
        std::memcpy(out, pose, sizeof(typename T0::StdPoseData));
    }
    void *_AllocTempPoseData(uint32_t, const void *, const void *pose)
    {
        std::memcpy(&field_18, pose, sizeof(field_18));
        return &field_18;
    }
};

extern void FlexibleCharComp_patch();
