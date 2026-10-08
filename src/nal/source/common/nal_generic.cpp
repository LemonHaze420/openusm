#include "nal_generic.h"

#include "common.h"
#include "memory.h"
#include "nal_system.h"
#include "nal_math.h"
#include "matrix4x4.h"
#include "tl_instance_bank.h"
#include "variables.h"
#include "vtbl.h"
#include "vector3d.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>
#include <functional>

namespace nalGeneric {

VALIDATE_SIZE(nalGenericSkeleton, 0xE4);
VALIDATE_SIZE(nalComponentInfo, 0x30);
VALIDATE_SIZE(nalGenericPose, 0xC);
VALIDATE_SIZE(nalGenericInstance, 0x2C);
VALIDATE_OFFSET(nalGenericAnim, field_80, 0x80);

namespace {
template <typename Result, typename... Args>
Result component_call(nalComponentBase *component, unsigned slot, Args... args)
{
    auto function = reinterpret_cast<Result(__fastcall *)(nalComponentBase *, void *, Args...)>(
        get_vfunc(component->m_vtbl, slot * 4));
    return function(component, nullptr, args...);
}

std::intptr_t aligned(std::intptr_t value, int alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

struct ComponentCursor {
    nalGenericAnim *anim;
    const nalComponentInfo *info;
    void **skeleton;
    void **animation;
};

void *pose_free_lists[3]{};
int pose_pool_counts[3]{};
constexpr int pose_pool_sizes[]{416, 768, 1280};
constexpr int pose_pool_limits[]{15, 8, 1};

void *allocate_pose(int size)
{
    int pool = size < 400 ? 0 : size < 752 ? 1 : size < 1264 ? 2 : 3;
    void *block = nullptr;
    if (pool != 3) {
        if (!pose_free_lists[pool] && pose_pool_counts[pool] < pose_pool_limits[pool]) {
            auto *page = static_cast<char *>(tlMemAlloc(15 * pose_pool_sizes[pool], 16, 0x2000004u));
            ++pose_pool_counts[pool];
            for (int i = 14; i >= 0; --i) {
                auto *entry = page + i * pose_pool_sizes[pool];
                *reinterpret_cast<void **>(entry) = pose_free_lists[pool];
                pose_free_lists[pool] = entry;
            }
        }
        block = pose_free_lists[pool];
        if (block)
            pose_free_lists[pool] = *static_cast<void **>(block);
    }
    if (!block) {
        pool = 3;
        block = tlMemAlloc(size + 16, 16, 0x2000000u);
    }
    *static_cast<int *>(block) = pool;
    return static_cast<char *>(block) + 16;
}

void free_pose(void *data)
{
    auto *block = static_cast<char *>(data) - 16;
    const int pool = *reinterpret_cast<int *>(block);
    if (pool == 3)
        tlMemFree(block);
    else {
        *reinterpret_cast<void **>(block) = pose_free_lists[pool];
        pose_free_lists[pool] = block;
    }
}

bool same_channel(const nalGenericSkeleton *a, int ai, const nalGenericSkeleton *b, int bi)
{
    const auto *ac = reinterpret_cast<const char *>(a->field_84 + ai * 40);
    const auto *bc = reinterpret_cast<const char *>(b->field_84 + bi * 40);
    const int an = *reinterpret_cast<const int *>(ac + 32);
    const int bn = *reinterpret_cast<const int *>(bc + 32);
    return std::memcmp(ac, bc, 32) == 0 && std::memcmp(reinterpret_cast<const void *>(a->field_78 + an * 48),
                                                       reinterpret_cast<const void *>(b->field_78 + bn * 48),
                                                       32) == 0;
}

void find_handle(const nalGenericSkeleton *skeleton, void *output, const void *name, const tlFixedString &channel,
                 const void *type, bool constant, bool hash_only)
{
    auto *handle = static_cast<int *>(output);
    handle[0] = 0;
    int channel_index = 0;
    for (int pass = 0; pass < (constant ? 2 : 1); ++pass) {
        auto *infos = pass ? skeleton->field_A8 : skeleton->field_8C;
        const int count = pass ? skeleton->field_A4 : skeleton->field_88;
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < infos[i].field_28; ++j, ++channel_index) {
                const auto *entry = reinterpret_cast<const char *>(skeleton->field_84 + channel_index * 40);
                const auto *node = reinterpret_cast<const void *>(skeleton->field_78 +
                                                                  48 * *reinterpret_cast<const int *>(entry + 32));
                if (std::memcmp(entry, &channel, 32) || std::memcmp(node, name, hash_only ? 4 : 32))
                    continue;
                if (component_call<const void *>(infos[i].field_20, 0) != type)
                    continue;
                handle[0] = reinterpret_cast<int>(skeleton);
                handle[1] = reinterpret_cast<int>(&infos[i]);
                handle[2] = j;
                handle[3] = pass;
                return;
            }
        }
    }
}
}  // namespace

struct OffsetMap {
    int references;
    OffsetMap *next;
    const nalGenericSkeleton *source;
    const nalGenericSkeleton *destination;
    int *offsets;
};
static OffsetMap *offset_maps[67]{};

#if !STANDALONE_SYSTEM
int &nalGenericPose::PoseSP = var<int>(0x0097DA08);
int &nalGenericPose::PoseStack = var<int>(0x00977204);
int &nalGenericAnim::vtbl_ptr = var<int>(0x00977120);
int &nalGenericSkeleton::vtbl_ptr = var<int>(0x009770E0);
#else
int &nalGenericPose::PoseSP = []() -> int & {
    static int value;
    return value;
}();
int &nalGenericPose::PoseStack = []() -> int & {
    alignas(16) static int storage[0x2800 / 4];
    return storage[0];
}();
int &nalGenericAnim::vtbl_ptr = []() -> int & {
    static void *table[]{func_address(&nalGenericSkeleton::_Release),
                         func_address(&Process),
                         func_address(&Release),
                         func_address(&CheckVersion),
                         func_address(&CreateInstance)};
    static int value = reinterpret_cast<int>(table);
    return value;
}();
int &nalGenericSkeleton::vtbl_ptr = []() -> int & {
    static nalGenericSkeleton skeleton;
    return skeleton.m_vtbl;
}();
#endif

void nalGenericAnim::Process()
{
    field_60 = reinterpret_cast<uint32_t *>(aligned(reinterpret_cast<int>(&field_80), 4));
    field_5C = reinterpret_cast<void *>(
        aligned(reinterpret_cast<int>(field_60) + 4 * ((field_30->field_80 + 31) / 32), field_58));
    field_6C = reinterpret_cast<void **>(aligned(reinterpret_cast<int>(field_5C) + field_54, 4));
    const auto base = reinterpret_cast<int>(field_6C + field_64);
    for (int i = 0; i < field_64; ++i)
        field_6C[i] = reinterpret_cast<void *>(reinterpret_cast<int>(field_6C[i]) + base);
    field_78 = static_cast<void **>(tlMemAlloc(4 * field_64, 8, 0x2000000u));
    std::memset(field_78, 0, 4 * field_64);
}

void nalGenericAnim::Release()
{
    for (int block = 0; block < field_64; ++block) {
        if (!field_78[block])
            continue;
        const int frames = std::min(field_68, field_44 - block * field_68);
        for (int frame = 0; frame < frames; ++frame) {
            void *data = static_cast<char *>(field_78[block]) + frame * field_70;
            void *skel = reinterpret_cast<void *>(field_30->field_C8);
            void *anim = field_5C;
            for (int i = 0; i < field_30->field_88; ++i) {
                auto *info = &field_30->field_8C[i];
                ComponentCursor cursor{this, info, &skel, &anim};
                component_call<void>(info->field_20, 14, &cursor, &data);
            }
        }
        nalCacheFree(field_78[block]);
    }
    tlMemFree(field_78);
    const int base = reinterpret_cast<int>(field_6C + field_64);
    for (int i = 0; i < field_64; ++i)
        field_6C[i] = reinterpret_cast<void *>(reinterpret_cast<int>(field_6C[i]) - base);
}

bool nalGenericAnim::CheckVersion() const
{
    return field_2C == 0x10200;
}

nalGenericInstance *nalGenericAnim::CreateInstance(nalGenericSkeleton *skeleton)
{
    auto *memory = tlMemAlloc(sizeof(nalGenericInstance), 8, 0);
    return memory ? new (memory) nalGenericInstance(this, skeleton ? skeleton : field_30) : nullptr;
}

nalGenericInstance::nalGenericInstance(nalGenericAnim *anim, nalGenericSkeleton *skeleton)
    : field_4(*reinterpret_cast<float *>(&anim->field_38)),
      field_8(std::equal_to<float>{}(field_4, 0.0f) ? 0.0f : 1.0f / field_4),
      field_C(skeleton ? skeleton : anim->field_30), field_10(anim), field_14(field_C), field_20(-1000000000.0f)
{
    static void *table[]{func_address(&nalGenericInstance::Finalize), func_address(&nalGenericInstance::GetPose)};
    m_vtbl = reinterpret_cast<int>(table);
    ++anim->field_3C;
    const auto *source = anim->field_30;
    const unsigned bucket =
        ((reinterpret_cast<unsigned>(source) ^ (reinterpret_cast<unsigned>(field_C) >> 3)) >> 3) % 67;
    for (auto *map = offset_maps[bucket]; map; map = map->next) {
        if (map->source == source && map->destination == field_C) {
            field_28 = map;
            ++map->references;
            return;
        }
    }
    field_28 = static_cast<OffsetMap *>(tlMemAlloc(sizeof(OffsetMap) + source->field_80 * 4, 8, 0x2000000u));
    *field_28 = {1, offset_maps[bucket], source, field_C, reinterpret_cast<int *>(field_28 + 1)};
    offset_maps[bucket] = field_28;
    auto *offsets = field_28->offsets;
    if (source == field_C) {
        int channel = 0;
        for (int i = 0; i < source->field_88; ++i) {
            const auto &info = source->field_8C[i];
            const int size = component_call<int>(info.field_20, 1);
            for (int j = 0; j < info.field_28; ++j)
                offsets[channel++] = info.field_2C + j * size;
        }
    } else {
        std::fill(offsets, offsets + source->field_80, -1);
        for (int i = 0; i < source->field_88; ++i) {
            const auto &a = source->field_8C[i];
            for (int j = 0; j < field_C->field_88; ++j) {
                const auto &b = field_C->field_8C[j];
                if (component_call<const void *>(a.field_20, 0) != component_call<const void *>(b.field_20, 0))
                    continue;
                const int size = component_call<int>(b.field_20, 1);
                for (int ac = 0; ac < a.field_28; ++ac)
                    for (int bc = 0; bc < b.field_28; ++bc)
                        if (same_channel(source, a.field_24 + ac, field_C, b.field_24 + bc)) {
                            offsets[a.field_24 + ac] = b.field_2C + bc * size;
                            break;
                        }
            }
        }
    }
}

nalGenericInstance::~nalGenericInstance()
{
    const unsigned bucket =
        ((reinterpret_cast<unsigned>(field_10->field_30) ^ (reinterpret_cast<unsigned>(field_C) >> 3)) >> 3) % 67;
    auto **link = &offset_maps[bucket];
    while (*link && *link != field_28)
        link = &(*link)->next;
    if (*link && --field_28->references == 0) {
        *link = field_28->next;
        tlMemFree(field_28);
    }
    field_14.~nalGenericPose();
    --field_10->field_3C;
}

void nalGenericInstance::Finalize(bool release)
{
    this->~nalGenericInstance();
    if (release)
        tlMemFree(this);
}

void nalGenericInstance::CacheBlock(int block)
{
    auto *anim = field_10;
    auto *skeleton = anim->field_30;
    const int count = std::min(anim->field_68, anim->field_44 - block * anim->field_68);
    void *out = nalCacheAllocate(count * anim->field_70, anim->field_74, &anim->field_78[block]);
    void *input = anim->field_6C[block];
    void *skel = reinterpret_cast<void *>(skeleton->field_C8);
    void *constant = anim->field_5C;
    if (anim->field_48) {
        alignas(16) static char scratch[0x4000];
        auto *work_base = reinterpret_cast<void *>(
            aligned(reinterpret_cast<int>(scratch) + anim->field_48 * anim->field_70 + 3, anim->field_50));
        void *work = work_base;
        void *temporary = static_cast<char *>(work_base) + anim->field_4C;
        for (int i = 0; i < skeleton->field_88; ++i) {
            auto *info = &skeleton->field_8C[i];
            ComponentCursor cursor{anim, info, &skel, &constant};
            component_call<void>(info->field_20, 5, &cursor, &work, &input, count);
        }
        for (int first = 0; first < count;) {
            const int frames = std::min(anim->field_48, count - first);
            work = work_base;
            void *decoded = scratch;
            skel = reinterpret_cast<void *>(skeleton->field_C8);
            constant = anim->field_5C;
            for (int i = 0; i < skeleton->field_88; ++i) {
                auto *info = &skeleton->field_8C[i];
                ComponentCursor cursor{anim, info, &skel, &constant};
                component_call<void>(
                    info->field_20, 6, &cursor, &decoded, &work, temporary, first, frames, anim->field_70);
            }
            const int bytes = frames * anim->field_70;
            std::memcpy(out, scratch, bytes);
            out = static_cast<char *>(out) + bytes;
            first += frames;
        }
    } else {
        for (int i = 0; i < skeleton->field_88; ++i) {
            auto *info = &skeleton->field_8C[i];
            ComponentCursor cursor{anim, info, &skel, &constant};
            component_call<void>(info->field_20, 7, &cursor, &out, &input, count, anim->field_70);
        }
    }
}

void nalGenericInstance::GetFrame(int frame, nalGenericPose &out, const nalGenericPose &reference)
{
    auto *anim = field_10;
    const int block = frame / anim->field_68;
    if (anim->field_78[block])
        nalCacheTouch(anim->field_78[block]);
    else
        CacheBlock(block);
    void *data = static_cast<char *>(anim->field_78[block]) + (frame % anim->field_68) * anim->field_70;
    if (field_C != anim->field_30)
        out = reference;
    void *skel = reinterpret_cast<void *>(anim->field_30->field_C8);
    void *constant = anim->field_5C;
    for (int i = 0; i < anim->field_30->field_88; ++i) {
        auto *info = &anim->field_30->field_8C[i];
        ComponentCursor cursor{anim, info, &skel, &constant};
        component_call<void>(info->field_20,
                             8,
                             &cursor,
                             reinterpret_cast<void *>(out.field_4),
                             &data,
                             reinterpret_cast<void *>(reference.field_4),
                             field_28->offsets);
    }
}

void nalGenericInstance::GetPose(Float time_value, Float previous_value, nalGenericPose &out,
                                 const nalGenericPose &reference)
{
    auto *anim = field_10;
    const bool loop = (anim->field_34 & 1) != 0;
    const bool relative = (anim->field_34 & 2) != 0;
    float time = time_value, previous = previous_value;
    int cycles = 0;
    if (loop) {
        const int now_cycle = time < 0.0f ? 1 - static_cast<int>(time) : static_cast<int>(time);
        const int old_cycle = previous < 0.0f ? 1 - static_cast<int>(previous) : static_cast<int>(previous);
        time = time < 0.0f ? time + now_cycle : time - now_cycle;
        previous = previous < 0.0f ? previous + old_cycle : previous - old_cycle;
        cycles = now_cycle - old_cycle;
    } else {
        time = std::max(0.0f, std::min(time, 1.0f));
        previous = std::max(0.0f, std::min(previous, 1.0f));
    }
    auto sample = [&](float t, bool current, nalGenericPose &a, nalGenericPose &b) {
        const double position = double(loop ? anim->field_44 : anim->field_44 - 1) * t;
        int first = static_cast<int>(position);
        if (!loop && first >= anim->field_44)
            first = anim->field_44 - 1;
        else if (loop && current && first >= anim->field_44)
            first = 0;
        int second = first + 1;
        if (second >= anim->field_44)
            second = loop ? 0 : anim->field_44 - 1;
        GetFrame(first, a, reference);
        GetFrame(second, b, reference);
        if (first > second) {
            void *skel = reinterpret_cast<void *>(anim->field_30->field_C8);
            void *constant = anim->field_5C;
            for (int i = 0; i < anim->field_30->field_88; ++i) {
                auto *info = &anim->field_30->field_8C[i];
                ComponentCursor cursor{anim, info, &skel, &constant};
                component_call<void>(
                    info->field_20, 9, &cursor, reinterpret_cast<void *>(b.field_4), relative, field_28->offsets);
            }
        }
        Blend(&out, static_cast<float>(position - first), &a, &b);
    };
    if (!std::equal_to<float>{}(previous, field_20)) {
        nalGenericPose a(field_C), b(field_C);
        sample(previous, false, a, b);
        field_14 = out;
        field_20 = previous;
    }
    nalGenericPose a(field_C), b(field_C);
    sample(time, true, a, b);
    nalGenericPose absolute(out, true);
    void *skel = reinterpret_cast<void *>(anim->field_30->field_C8);
    void *constant = anim->field_5C;
    for (int i = 0; i < anim->field_30->field_88; ++i) {
        auto *info = &anim->field_30->field_8C[i];
        ComponentCursor cursor{anim, info, &skel, &constant};
        component_call<void>(info->field_20,
                             10,
                             &cursor,
                             reinterpret_cast<void *>(out.field_4),
                             reinterpret_cast<void *>(field_14.field_4),
                             cycles,
                             relative,
                             field_28->offsets);
    }
    field_14 = absolute;
    field_20 = time;
}

nalGenericPose::nalGenericPose() : field_0(nullptr), field_8(false) {}

nalGenericPose::nalGenericPose(const nalGenericSkeleton *skeleton)
    : field_0(const_cast<nalGenericSkeleton *>(skeleton)), field_4(0), field_8(false)
{
    if (sub_101BF70(reinterpret_cast<int>(this))) {
        const int begin = aligned(PoseSP, skeleton->field_94);
        const int end = aligned(begin + skeleton->field_90, 4);
        if (end + 4 <= 0x2800) {
            field_4 = reinterpret_cast<int>(&PoseStack) + begin;
            *reinterpret_cast<int *>(reinterpret_cast<char *>(&PoseStack) + end) = PoseSP;
            PoseSP = end + 4;
        }
    }
    if (!field_4) {
        field_4 = reinterpret_cast<int>(allocate_pose(skeleton->field_90));
        field_8 = true;
    }
    ConstructEmptyData();
}

nalGenericPose::nalGenericPose(const nalGenericPose &source, bool copy) : nalGenericPose(source.field_0)
{
    if (copy)
        *this = source;
}

void nalGenericPose::ConstructEmptyData()
{
    void *data = reinterpret_cast<void *>(field_4);
    for (int i = 0; i < field_0->field_88; ++i) {
        auto *info = &field_0->field_8C[i];
        component_call<void>(info->field_20, 11, info, &data);
    }
}

nalGenericPose::~nalGenericPose()
{
    if (!field_0)
        return;
    void *data = reinterpret_cast<void *>(field_4);
    for (int i = 0; i < field_0->field_88; ++i) {
        auto *info = &field_0->field_8C[i];
        component_call<void>(info->field_20, 12, info, &data);
    }
    if (field_8)
        free_pose(reinterpret_cast<void *>(field_4));
    else if (field_4 >= reinterpret_cast<int>(&PoseStack) && field_4 < reinterpret_cast<int>(&PoseStack) + 0x2800)
        PoseSP = *reinterpret_cast<int *>(reinterpret_cast<char *>(&PoseStack) + PoseSP - 4);
}

nalGenericPose &nalGenericPose::operator=(const nalGenericPose &source)
{
    void *out = reinterpret_cast<void *>(field_4);
    const void *in = reinterpret_cast<const void *>(source.field_4);
    for (int i = 0; i < field_0->field_88; ++i) {
        auto *info = &field_0->field_8C[i];
        component_call<void>(info->field_20, 13, info, &out, &in);
    }
    return *this;
}

void Blend(nalGenericPose *out, float weight, const nalGenericPose *a, const nalGenericPose *b)
{
    for (int i = 0; i < a->field_0->field_88; ++i) {
        const auto &info = a->field_0->field_8C[i];
        component_call<void>(info.field_20,
                             2,
                             info.field_28,
                             reinterpret_cast<void *>(out->field_4 + info.field_2C),
                             reinterpret_cast<const void *>(a->field_4 + info.field_2C),
                             reinterpret_cast<const void *>(b->field_4 + info.field_2C),
                             weight);
    }
}

void nalGenericSkeleton::_Process()
{
    field_68 = reinterpret_cast<int>(&field_E0);
    field_70 = field_68 + field_64;
    field_78 = aligned(field_70 + field_6C, 4);
    field_84 = aligned(field_78 + 48 * field_74, 4);
    field_8C = reinterpret_cast<nalComponentInfo *>(aligned(field_84 + 40 * field_7C, 4));
    field_98 = aligned(reinterpret_cast<int>(field_8C + field_88), field_94);
    field_A0 = aligned(field_98 + field_90, 4);
    field_A8 = reinterpret_cast<nalComponentInfo *>(aligned(field_A0 + field_9C, 4));
    field_B4 = aligned(reinterpret_cast<int>(field_A8 + field_A4), field_B0);
    field_BC = aligned(field_B4 + field_AC, 4);
    field_C8 = aligned(field_BC + field_B8, field_C4);
    field_CC.field_0 = this;
    field_CC.field_4 = field_98;
    for (int pass = 0; pass < 2; ++pass) {
        auto *infos = pass ? field_A8 : field_8C;
        const int count = pass ? field_A4 : field_88;
        for (int i = 0; i < count; ++i) {
            auto *instance = nalComponentInstanceBank.Search(infos[i].field_0);
            assert(instance && "missing generic component encoding");
            infos[i].field_20 = static_cast<nalComponentBase *>(instance->field_20);
        }
    }
    for (int pass = 0; pass < 2; ++pass) {
        auto *infos = pass ? field_A8 : field_8C;
        const int count = pass ? field_A4 : field_88;
        void *pose = reinterpret_cast<void *>(pass ? field_B4 : field_98);
        void *data = reinterpret_cast<void *>(pass ? field_BC : field_A0);
        for (int i = 0; i < count; ++i)
            infos[i].field_20->Process(&infos[i], pose, data);
    }
}

void nalGenericSkeleton::_Release() {}

void nalGenericSkeleton::GetTrajectoryData(const nalGenericPose *pose, nalPositionOrientation *out) const
{
    if (field_D8 < 0) {
        *out = {};
        out->field_0[3] = 1.0f;
    } else
        std::memcpy(out, reinterpret_cast<const void *>(pose->field_4 + field_D8), sizeof(*out));
}

nalGenericPose *nalGenericSkeleton::CreatePose() const
{
    auto *memory = tlMemAlloc(sizeof(nalGenericPose), 8, 0x2000000u);
    return memory ? new (memory) nalGenericPose(this) : nullptr;
}
void nalGenericSkeleton::DestroyPose(nalGenericPose *pose) const
{
    if (pose) {
        pose->~nalGenericPose();
        tlMemFree(pose);
    }
}
nalGenericPose *nalGenericSkeleton::GetDefaultPose()
{
    return &field_CC;
}
void nalGenericSkeleton::CopyPose(nalGenericPose &out, const nalGenericPose &source) const
{
    out = source;
}
void nalGenericSkeleton::BlendPose(nalGenericPose &out, Float weight, const nalGenericPose &a,
                                   const nalGenericPose &b) const
{
    Blend(&out, weight, &a, &b);
}

namespace {
void rotation(nalMatrix4x4 &out, const float *q)
{
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const float diagonal = 2.0f * w * w - 1.0f;
    out[0][0] = 2.0f * x * x + diagonal;
    out[0][1] = 2.0f * x * y - 2.0f * w * z;
    out[0][2] = 2.0f * x * z + 2.0f * w * y;
    out[1][0] = 2.0f * y * x + 2.0f * w * z;
    out[1][1] = 2.0f * y * y + diagonal;
    out[1][2] = 2.0f * y * z - 2.0f * w * x;
    out[2][0] = 2.0f * z * x - 2.0f * w * y;
    out[2][1] = 2.0f * z * y + 2.0f * w * x;
    out[2][2] = 2.0f * z * z + diagonal;
    out[0][3] = out[1][3] = out[2][3] = 0.0f;
}

vector4d rotation_quaternion(const nalMatrix4x4 &m)
{
    vector4d q;
    const float trace = m[0][0] + m[1][1] + m[2][2];
    float diagonal;
    if (trace >= -0.33333299f) {
        diagonal = trace + 1.0f;
        q = {m[2][1] - m[1][2], m[0][2] - m[2][0], m[1][0] - m[0][1], diagonal};
    } else if (m[0][0] >= m[1][1] && m[0][0] > m[2][2]) {
        diagonal = m[0][0] - m[1][1] - m[2][2] + 1.0f;
        q = {diagonal, m[0][1] + m[1][0], m[0][2] + m[2][0], m[2][1] - m[1][2]};
    } else if (m[1][1] > m[0][0] && m[1][1] > m[2][2]) {
        diagonal = m[1][1] - m[0][0] - m[2][2] + 1.0f;
        q = {m[0][1] + m[1][0], diagonal, m[1][2] + m[2][1], m[0][2] - m[2][0]};
    } else {
        diagonal = m[2][2] - m[0][0] - m[1][1] + 1.0f;
        q = {m[0][2] + m[2][0], m[1][2] + m[2][1], diagonal, m[1][0] - m[0][1]};
    }
    const float scale = 0.5f / std::sqrt(diagonal);
    for (int i = 0; i < 4; ++i)
        q[i] *= scale;
    return q;
}

void relative_matrix(nalMatrix4x4 &out, const nalMatrix4x4 &parent)
{
    nalMatrix4x4 inverse;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column)
            inverse[row][column] = parent[column][row];
        inverse[row][3] = 0.0f;
        inverse[3][row] =
            -(parent[3][2] * parent[row][2] + parent[3][1] * parent[row][1] + parent[3][0] * parent[row][0]);
    }
    inverse[3][3] = 1.0f;
    nalComposeMatrices(out, out, inverse);
}

struct IKPose {
    vector4d upper;
    vector4d end;
    vector4d target_rotation;
    vector3d target;
    float weight;
    int16_t bend;
    int16_t twist;
    char padding[12];
};
static_assert(sizeof(IKPose) == 80);

void flip_ik_basis(nalMatrix4x4 &m)
{
    for (int column = 0; column < 4; ++column) {
        m[0][column] = -m[0][column];
        const float old_y = m[1][column];
        m[1][column] = -m[2][column];
        m[2][column] = -old_y;
    }
}

void evaluate_ik(nalMatrix4x4 *matrices, const unsigned char *code, const IKPose &pose)
{
    const int parent_index = code[0], upper_index = code[1], lower_index = code[2], end_index = code[3];
    const int bend_mode = code[4];
    float coefficients[5];
    std::memcpy(coefficients, code + 5, sizeof(coefficients));
    const float upper_length = coefficients[4];
    const float lower_length = 0.5f / coefficients[1];
    constexpr float two_pi = 6.283185482025146484375f;
    constexpr float angle_units = 0.0000305180437862873077392578125f;
    const float bend_angle = (pose.bend - 0.5) * two_pi * angle_units;
    const vector4d bend_quat{0.0f, -std::sin(bend_angle * 0.5f), 0.0f, std::cos(bend_angle * 0.5f)};
    auto &upper = matrices[upper_index];
    auto &lower = matrices[lower_index];
    auto &end = matrices[end_index];
    auto &parent = matrices[parent_index];
    if (std::equal_to<float>{}(pose.weight, 0.0f)) {
        rotation(upper, &pose.upper.x);
        rotation(lower, &bend_quat.x);
        lower[3][0] = -upper_length;
        lower[3][1] = lower[3][2] = 0.0f;
        rotation(end, &pose.end.x);
        end[3][0] = -lower_length;
        end[3][1] = end[3][2] = 0.0f;
        nalComposeMatrices(upper, upper, parent);
        nalComposeMatrices(lower, lower, upper);
        nalComposeMatrices(end, end, lower);
        return;
    }

    nalMatrix4x4 solved_upper, solved_lower, solved_end;
    solved_upper[3][0] = upper[3][0];
    solved_upper[3][1] = upper[3][1];
    solved_upper[3][2] = upper[3][2];
    solved_upper[3][3] = upper[3][3];
    rotation(solved_end, &pose.target_rotation.x);
    solved_end[3][0] = pose.target.x;
    solved_end[3][1] = pose.target.y;
    solved_end[3][2] = pose.target.z;
    solved_end[3][3] = 1.0f;
    vector3d root{upper[3][0], upper[3][1], upper[3][2]};
    vector3d target = pose.target;
    vector3d base, direction;
    float sin_upper, cos_upper, sin_lower, cos_lower;
    inverse_kinematics::nalIKSolve2D(reinterpret_cast<matrix4x4 *>(&parent),
                                     &root,
                                     &target,
                                     coefficients[0],
                                     coefficients[1],
                                     coefficients[2],
                                     coefficients[3],
                                     &base,
                                     &direction,
                                     &sin_upper,
                                     &cos_upper,
                                     &sin_lower,
                                     &cos_lower);
    vector4d bend_direction;
    if (bend_mode == 0) {
        bend_direction.x = direction.y * solved_end[1][2] - solved_end[1][1] * direction.z;
        bend_direction.y = solved_end[1][0] * direction.z - direction.x * solved_end[1][2];
        bend_direction.z = direction.x * solved_end[1][1] - solved_end[1][0] * direction.y;
    } else {
        const float sign = bend_mode == 2 ? -1.0f : 1.0f;
        const float x = parent[1][0], y = parent[1][1], z = parent[1][2];
        const float dot = sign * (x * direction.x + y * direction.y + z * direction.z);
        const float cross[]{
            direction.y * z - y * direction.z, x * direction.z - direction.x * z, direction.x * y - x * direction.y};
        for (int axis = 0; axis < 3; ++axis) {
            if (dot < 0.0f)
                bend_direction[axis] = cross[axis] * (dot + 1.0f) + (-parent[0][axis] - parent[2][axis]) * -dot;
            else
                bend_direction[axis] = cross[axis] * (1.0f - dot) + (-parent[0][axis] + parent[2][axis]) * dot;
        }
    }
    bend_direction.w = 0.0f;
    const float twist = (pose.twist - 0.5) * two_pi * angle_units;
    inverse_kinematics::nalIKMap2DTo3D(upper_length,
                                       sin_upper,
                                       cos_upper,
                                       sin_lower,
                                       cos_lower,
                                       &base,
                                       &direction,
                                       &bend_direction,
                                       std::sin(twist),
                                       std::cos(twist),
                                       reinterpret_cast<matrix4x4 *>(&solved_upper),
                                       reinterpret_cast<matrix4x4 *>(&solved_lower));
    flip_ik_basis(solved_upper);
    flip_ik_basis(solved_lower);
    if (std::equal_to<float>{}(pose.weight, 1.0f)) {
        upper = solved_upper;
        lower = solved_lower;
        end = solved_end;
        return;
    }
    relative_matrix(solved_end, solved_lower);
    relative_matrix(solved_lower, solved_upper);
    relative_matrix(solved_upper, parent);
    const auto upper_q = math::Slerp(pose.weight, pose.upper, rotation_quaternion(solved_upper));
    const auto lower_q = math::Slerp(pose.weight, bend_quat, rotation_quaternion(solved_lower));
    const auto end_q = math::Slerp(pose.weight, pose.end, rotation_quaternion(solved_end));
    rotation(upper, &upper_q.x);
    rotation(lower, &lower_q.x);
    rotation(end, &end_q.x);
    lower[3][0] = -upper_length;
    lower[3][1] = lower[3][2] = 0.0f;
    end[3][0] = -lower_length;
    end[3][1] = end[3][2] = 0.0f;
    nalComposeMatrices(upper, upper, parent);
    nalComposeMatrices(lower, lower, upper);
    nalComposeMatrices(end, end, lower);
}
}  // namespace

nalMatrix4x4 *nalGenericSkeleton::GetBoneMatrices(const nalGenericPose *pose, nalMatrix4x4 *matrices) const
{
    for (int i = 0; i < field_60; ++i) {
        matrices[i][0][3] = matrices[i][1][3] = matrices[i][2][3] = 0.0f;
        matrices[i][3][3] = 1.0f;
    }
    const auto *code = reinterpret_cast<const unsigned char *>(field_68);
    const auto *end = code + field_64;
    const auto *data = reinterpret_cast<const char *>(pose->field_4);
    const auto *constant = reinterpret_cast<const char *>(field_B4);
    while (code < end) {
        const unsigned operation = *code++;
        auto *&cursor = operation & 0x80 ? constant : data;
        switch (operation & 0x7F) {
        case 0:
            std::memcpy(matrices[*code++][3], cursor, 12);
            cursor += 12;
            break;
        case 1:
            rotation(matrices[*code++], reinterpret_cast<const float *>(cursor));
            cursor += 16;
            break;
        case 2: {
            auto &matrix = matrices[*code++];
            rotation(matrix, reinterpret_cast<const float *>(cursor));
            std::memcpy(matrix[3], cursor + 16, 12);
            cursor += 28;
            break;
        }
        case 3: {
            auto &matrix = matrices[*code++];
            const auto *scale = reinterpret_cast<const float *>(cursor);
            for (int row = 0; row < 3; ++row)
                for (int column = 0; column < 4; ++column)
                    matrix[row][column] *= scale[row];
            cursor += 12;
            break;
        }
        case 5:
            evaluate_ik(matrices, code, *reinterpret_cast<const IKPose *>(data));
            code += 25;
            data += sizeof(IKPose);
            break;
        case 12: {
            const int parent = *code++, child = *code++;
            nalComposeMatrices(matrices[child], matrices[child], matrices[parent]);
            break;
        }
        case 14:
            cursor += *code++;
            break;
        }
    }
    return matrices;
}

void nalGenericSkeleton::GetPoseFromBoneMatrices(nalGenericPose &out, const nalMatrix4x4 *source, nalMatrix4x4 *scratch,
                                                 const nalGenericPose &reference) const
{
    std::copy_n(source, field_60, scratch);
    out = reference;
    const auto *code = reinterpret_cast<const unsigned char *>(field_70);
    const auto *end = code + field_6C;
    auto *data = reinterpret_cast<char *>(out.field_4);
    while (code < end) {
        switch (*code++) {
        case 6:
            std::memcpy(data, scratch[*code++][3], 12);
            data += 12;
            break;
        case 7: {
            const auto q = rotation_quaternion(scratch[*code++]);
            std::memcpy(data, &q, 16);
            data += 16;
            break;
        }
        case 8: {
            const auto &matrix = scratch[*code++];
            const auto q = rotation_quaternion(matrix);
            std::memcpy(data, &q, 16);
            std::memcpy(data + 16, matrix[3], 12);
            data += 28;
            break;
        }
        case 9:
            reinterpret_cast<float *>(data)[0] = reinterpret_cast<float *>(data)[1] =
                reinterpret_cast<float *>(data)[2] = 1.0f;
            data += 12;
            break;
        case 11: {
            const int parent = *code++, upper = *code++, lower = *code++, tip = *code++;
            float dot = 0.0f;
            for (int i = 0; i < 3; ++i)
                dot += source[upper][0][i] * source[lower][0][i];
            float angle = std::acos(std::max(-1.0f, std::min(1.0f, dot)));
            const float cross[]{source[upper][0][1] * source[lower][0][2] - source[upper][0][2] * source[lower][0][1],
                                source[upper][0][2] * source[lower][0][0] - source[upper][0][0] * source[lower][0][2],
                                source[upper][0][0] * source[lower][0][1] - source[upper][0][1] * source[lower][0][0]};
            if (cross[0] * source[lower][1][0] + cross[1] * source[lower][1][1] + cross[2] * source[lower][1][2] < 0.0f)
                angle = -angle;
            relative_matrix(scratch[tip], source[lower]);
            relative_matrix(scratch[lower], source[upper]);
            relative_matrix(scratch[upper], source[parent]);
            auto &ik = *reinterpret_cast<IKPose *>(data);
            ik.upper = rotation_quaternion(scratch[upper]);
            ik.end = rotation_quaternion(scratch[tip]);
            ik.weight = 0.0f;
            ik.bend = static_cast<int16_t>(angle * 32767.5f * 0.15915493667125702f + 0.5f);
            data += sizeof(IKPose);
            break;
        }
        case 13: {
            const int parent = *code++, child = *code++;
            relative_matrix(scratch[child], source[parent]);
            break;
        }
        case 14:
            data += *code++;
            break;
        }
    }
}

nalGenericSkeleton::nalGenericSkeleton()
{
    static void *table[]{func_address(&nalGenericSkeleton::_Release),
                         func_address(&nalGenericSkeleton::Finalize),
                         func_address(&nalGenericSkeleton::_Process),
                         func_address(&nalGenericSkeleton::_Release),
                         func_address(&nalGenericSkeleton::_CheckVersion),
                         func_address(&nalGenericSkeleton::GetBoneCount),
                         func_address(&nalGenericSkeleton::GetBoneMatrices),
                         func_address(&nalGenericSkeleton::GetTrajectoryData),
                         func_address(&nalGenericSkeleton::GetPoseFromBoneMatrices),
                         func_address(&nalGenericSkeleton::GetDefaultPose),
                         func_address(&nalGenericSkeleton::CreatePose),
                         func_address(&nalGenericSkeleton::DestroyPose),
                         func_address(&nalGenericSkeleton::CopyPose),
                         func_address(&nalGenericSkeleton::BlendPose)};
    m_vtbl = reinterpret_cast<int>(table);
}

void nalGenericSkeleton::Finalize(bool release)
{
    this->~nalGenericSkeleton();
    if (release)
        tlMemFree(this);
}

template <>
void nalGenericSkeleton::GetComponentHandle<nalPositionOrientation>(
    nalGenericComponentHandle<nalPositionOrientation> &out, tlFixedString &name, tlFixedString &channel)
{
    find_handle(this, &out, &name, channel, &nalComponentPOBase::TypeID, false, false);
}
template <>
void nalGenericSkeleton::GetComponentHandle<nalPositionOrientation>(
    nalGenericConstComponentHandle<nalPositionOrientation> &out, tlFixedString &name, tlFixedString &channel) const
{
    find_handle(this, &out, &name, channel, &nalComponentPOBase::TypeID, true, false);
}
template <>
void nalGenericSkeleton::GetComponentHandle<float>(nalGenericComponentHandle<float> &out, tlFixedString &name,
                                                   tlFixedString &channel)
{
    find_handle(this, &out, &name, channel, &nalComponentFloat1Base::TypeID, false, false);
}
template <>
void nalGenericSkeleton::GetComponentHandle<float>(nalGenericConstComponentHandle<float> &out, tlFixedString &name,
                                                   tlFixedString &channel) const
{
    find_handle(this, &out, &name, channel, &nalComponentFloat1Base::TypeID, true, false);
}
template <>
void nalGenericSkeleton::GetComponentHandle<unsigned char>(nalGenericComponentHandle<unsigned char> &out,
                                                           tlFixedString &name, tlFixedString &channel)
{
    find_handle(this, &out, &name, channel, &nalComponentU8Base::TypeID, false, false);
}
template <>
void nalGenericSkeleton::GetComponentHandle<nalVector3>(nalGenericConstComponentHandle<nalVector3> &out, uint32_t name,
                                                        tlFixedString &channel) const
{
    find_handle(this, &out, &name, channel, &nalComponentFloat3Base::TypeID, true, true);
}

template <>
void nalGenericSkeleton::GetComponentHandle<MorphSliderPoseTemplate<6>>(
    nalGenericComponentHandle<MorphSliderPoseTemplate<6>> &out, tlFixedString &name, tlFixedString &channel)
{
    find_handle(this, &out, &name, channel, &nalComponentMorphSliderBase::TypeID, false, false);
}

}  // namespace nalGeneric

void nalGeneric_patch() {}
