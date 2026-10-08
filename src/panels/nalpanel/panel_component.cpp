#include "panel_component.h"
#include "panel_anim_inst.h"
#include "common.h"
#include "nal_component.h"
#include "nalcomp/nal_anim_comp.h"
#include "nalcomp/nal_pose_comp.h"
#include "nal_system.h"
#include "string_hash.h"
#include "utility.h"
#include "variables.h"
#include <algorithm>
#include <cstring>

VALIDATE_SIZE(PanelComponent, 0x8);
VALIDATE_SIZE(nalPanel::nalPanelAnim, 0x58);

#if STANDALONE_SYSTEM
namespace {
struct frame_cache {
    void *frames;
    uint32_t count;
    uint32_t stride;
};
void decode_words(const void *input, void *output, unsigned count, unsigned stride)
{
    auto *source = static_cast<const uint8_t *>(input);
    auto *target = static_cast<uint8_t *>(output);
    unsigned remaining = 0;
    uint32_t value = 0;
    for (unsigned frame = 0; frame < count; ++frame) {
        if (!remaining) {
            remaining = *source++;
            std::memcpy(&value, source, sizeof(value));
            source += sizeof(value);
        }
        std::memcpy(target, &value, sizeof(value));
        target += stride;
        --remaining;
    }
}
void decode_vector(const void *input, void *output, unsigned count, unsigned stride, unsigned dimensions)
{
    const auto *offsets = static_cast<const uint32_t *>(input);
    const auto *base = static_cast<const char *>(input);
    for (unsigned channel = 0; channel < dimensions; ++channel) {
        const void *source = channel ? base + offsets[channel - 1] : base + 4 * (dimensions - 1);
        nalDecodeEntropyScalar(source, static_cast<char *>(output) + 4 * channel, count, stride, 1.0f / 256.0f);
    }
}
template <unsigned Kind>
struct native_panel_component : PanelComponent {
    static constexpr unsigned sizes[]{48, 36, 20, 28, 28, 20, 16, 32};
    static constexpr const char *names[]{"PanelBase",
                                         "PanelScissor",
                                         "PanelGutter",
                                         "PanelTexture",
                                         "PanelCamera",
                                         "PanelColor",
                                         "PanelEffect",
                                         "PanelCharacter"};
    native_panel_component()
    {
        static void *table[]{func_address(&native_panel_component::Destroy),
                             func_address(&native_panel_component::Type),
                             func_address(&native_panel_component::PublicData),
                             func_address(&native_panel_component::PublicData),
                             func_address(&native_panel_component::Trajectory),
                             func_address(&native_panel_component::BoneMatrices),
                             func_address(&native_panel_component::Contributes),
                             func_address(&native_panel_component::InstanceSize),
                             func_address(&native_panel_component::InstanceAlign),
                             func_address(&native_panel_component::Build),
                             func_address(&native_panel_component::Release),
                             func_address(&native_panel_component::Maps),
                             func_address(&native_panel_component::Calculate),
                             func_address(&native_panel_component::Remapped),
                             func_address(&native_panel_component::Blend),
                             func_address(&native_panel_component::ProcessSkeleton),
                             func_address(&native_panel_component::ProcessSkeleton),
                             func_address(&native_panel_component::ProcessAnimation),
                             func_address(&native_panel_component::ProcessAnimation),
                             func_address(&native_panel_component::CopyExtra),
                             func_address(&native_panel_component::FreePose)};
        m_vtbl = reinterpret_cast<std::intptr_t>(table);
    }
    void *Destroy(unsigned flags)
    {
        if (flags & 1)
            delete this;
        return this;
    }
    uint32_t Type() const
    {
        return to_hash(names[Kind]);
    }
    void *PublicData(uint32_t, const void *data) const
    {
        return const_cast<void *>(data);
    }
    nalPositionOrientation *Trajectory(nalPositionOrientation *out, uint32_t, const void *, const void *)
    {
        *out = nalPositionOrientation{};
        out->field_0[3] = 1.0f;
        return out;
    }
    void BoneMatrices(nalMatrix4x4 *, uint32_t, const void *, const void *) {}
    int Contributes(uint32_t, const void *, const void *)
    {
        return 0;
    }
    int InstanceSize(uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    {
        return 12;
    }
    int InstanceAlign(uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    {
        return 4;
    }
    void Build(void *out, uint32_t, const void *, const void *, const void *, const void *, const void *, bool)
    {
        if (out)
            *static_cast<frame_cache *>(out) = {nullptr, 0, sizes[Kind]};
    }
    void Release(void *state, uint32_t, const void *, const void *)
    {
        tlMemFree(static_cast<frame_cache *>(state)->frames);
        static_cast<frame_cache *>(state)->frames = nullptr;
    }
    bool Maps(uint32_t, uint32_t, uint32_t)
    {
        return false;
    }
    void Calculate(void *out, uint32_t, Float time, Float, const nalComp::nalCompAnim *anim, const void *, const void *,
                   const void *track, void *state)
    {
        auto &cache = *static_cast<frame_cache *>(state);
        if (!cache.frames) {
            cache.count = static_cast<const nalPanel::nalPanelAnim *>(anim)->frame_count;
            cache.frames = tlMemAlloc(cache.count * cache.stride, Kind == 0 ? 16 : 4, 0);
            const auto *base = static_cast<const char *>(track);
            const auto *directory = static_cast<const uint32_t *>(track) + 1;
            unsigned channel = 0;
            auto input = [&]() {
                return base + directory[channel++];
            };
            auto output = [&](unsigned offset) {
                return static_cast<char *>(cache.frames) + offset;
            };
            auto words = [&](unsigned offset) {
                decode_words(input(), output(offset), cache.count, cache.stride);
            };
            auto scalar = [&](unsigned offset) {
                nalDecodeEntropyScalar(input(), output(offset), cache.count, cache.stride, 1.0f / 256.0f);
            };
            auto vector = [&](unsigned offset, unsigned dimensions) {
                decode_vector(input(), output(offset), cache.count, cache.stride, dimensions);
            };
            if constexpr (Kind == 0) {
                nalDecodeEntropyQuaternion(input(), output(0), cache.count, cache.stride, 1.0f / 256.0f);
                vector(16, 3);
                vector(28, 2);
                scalar(36);
                words(40);
            } else if constexpr (Kind == 1) {
                vector(0, 4);
                vector(16, 4);
                words(32);
            } else if constexpr (Kind == 2 || Kind == 5) {
                vector(0, 4);
                words(16);
            } else if constexpr (Kind == 3 || Kind == 4) {
                vector(0, 4);
                words(16);
                scalar(20);
                words(24);
            } else if constexpr (Kind == 6) {
                words(0);
                scalar(4);
                scalar(8);
                words(12);
            } else {
                vector(0, 3);
                vector(12, 3);
                scalar(24);
                words(28);
            }
        }
        const unsigned frame = static_cast<unsigned>((cache.count - 1) * double(std::clamp(time.value, 0.0f, 1.0f)));
        std::memcpy(out, static_cast<const char *>(cache.frames) + frame * cache.stride, cache.stride);
    }
    void Remapped(void *, uint32_t, Float, Float, const nalComp::nalCompAnim *, const void *, uint32_t, uint32_t,
                  const void *, const void *, void *)
    {}
    void Blend(void *, uint32_t, Float, const void *, const void *) {}
    void ProcessSkeleton(uint32_t, void *, void *) const {}
    void ProcessAnimation(uint32_t, void *, void *, const void *) const {}
    void CopyExtra(void *, uint32_t, const void *) {}
    void FreePose(uint32_t, void *) const {}
};
}
#endif

#if !STANDALONE_SYSTEM
PanelComponent *&PanelComponentMgr::comp_list = var<PanelComponent *>(0x0096F7DC);
#else
PanelComponent *&PanelComponentMgr::comp_list = []() -> auto & {
    static PanelComponent *list{};
    return list;
}();
static native_panel_component<0> base_component;
static native_panel_component<1> scissor_component;
static native_panel_component<2> gutter_component;
static native_panel_component<3> texture_component;
static native_panel_component<4> camera_component;
static native_panel_component<5> color_component;
static native_panel_component<6> effect_component;
static native_panel_component<7> character_component;
#endif

void PanelComponentMgr::Add(PanelComponent *value)
{
    value->m_prevComp = comp_list;
    comp_list = value;
}

PanelComponent::PanelComponent() : m_prevComp(nullptr)
{
    m_vtbl = 0;
    PanelComponentMgr::Add(this);
}
