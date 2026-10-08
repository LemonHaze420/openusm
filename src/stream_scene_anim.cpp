#include "stream_scene_anim.h"

#include "common.h"
#include "nal_skeleton.h"
#include "nal_list.h"
#include "nal_system.h"
#include "tl_instance_bank.h"
#include "tl_system.h"
#include "utility.h"

#include <algorithm>
#include <cstring>
#include <new>

VALIDATE_SIZE(nalStreamInstance, 0x7Cu);
VALIDATE_OFFSET(nalStreamInstance, m_callback, 0x20);

namespace {
void *__fastcall stream_destroy(nalSceneAnimInstance *instance, void *, unsigned flags)
{
    auto *stream = static_cast<nalStreamInstance *>(instance);
    stream->~nalStreamInstance();
    if ((flags & 1) != 0)
        tlMemFree(stream);
    return instance;
}

bool __fastcall stream_ready(const nalSceneAnimInstance *instance, void *)
{
    return static_cast<const nalStreamInstance *>(instance)->IsReady();
}

bool __fastcall stream_advance(nalSceneAnimInstance *instance, void *, Float dt)
{
    return static_cast<nalStreamInstance *>(instance)->Advance(dt);
}

nalSceneAnimInstance::vtable stream_vtable{stream_destroy, stream_ready, stream_advance};

void destroy_stream_anim(nalSceneAnimInstance::client_anim *entry)
{
    if (entry->field_8 != nullptr) {
        auto *anim = entry->field_8->field_10;
        using destroy_fn = void *(__fastcall *)(void *, void *, unsigned);
        reinterpret_cast<destroy_fn>(get_vfunc(entry->field_8->m_vtbl, 0))(entry->field_8, nullptr, 1);
        anim->Release();
    }
}

int segment_size(const nalSceneAnimSegment *segment)
{
    return static_cast<int>(reinterpret_cast<std::intptr_t>(segment->next));
}

int header_size(const nalSceneAnim *header)
{
    return static_cast<int>(reinterpret_cast<std::intptr_t>(header->field_34));
}

void process_segment(nalStreamInstance *stream)
{
    auto *segment = stream->field_18;
    if (segment->anim != nullptr) {
        segment->anim = reinterpret_cast<nalAnimClass<nalAnyPose> *>(reinterpret_cast<char *>(segment) +
                                                                     reinterpret_cast<std::intptr_t>(segment->anim));
        for (auto *anim = segment->anim; anim != nullptr; anim = anim->field_4) {
            if (anim->field_4 != nullptr)
                anim->field_4 = reinterpret_cast<nalAnimClass<nalAnyPose> *>(
                    reinterpret_cast<char *>(anim) + reinterpret_cast<std::intptr_t>(anim->field_4));
            auto *skeleton = stream->field_38[anim->field_28];
            anim->Skeleton = skeleton;
            auto *type = nalTypeInstanceBank.Search(skeleton->GetAnimTypeName());
            anim->m_vtbl = static_cast<nalInitListAnimType *>(type->field_20)->anim_vtbl_ptr;
            anim->CheckVersion();
            anim->Process();
        }
    }
}
}

void *nalStreamInstance::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

void nalStreamInstance::operator delete(void *ptr, size_t)
{
    tlMemFree(ptr);
}

bool nalStreamInstance::IsReady() const
{
    return field_1C == 5;
}

nalStreamInstance::~nalStreamInstance()
{
    while (field_8 != nullptr) {
        auto *entry = field_8;
        field_8 = entry->field_C;
        destroy_stream_anim(entry);
        if (entry->field_0 != nullptr)
            entry->field_0->m_vtbl->Release(entry->field_0, nullptr);
        tlMemFree(entry);
    }
    tlMemFree(field_38);
    tlMemFree(field_48[0]);
    tlMemFree(field_48[1]);
    tlMemFree(field_4);
}

void nalStreamInstance::AdvanceStream()
{
    if (m_requestID.field_0 == -1) {
        if (field_79 == field_78)
            return;
    } else {
        if (nflGetRequestState(m_requestID) == 4)
            return;
        field_58 += field_68;
        m_requestID.field_0 = -1;
        field_79 ^= 1;
        if (field_79 == field_78)
            return;
    }
    if (field_58 >= field_70) {
        if ((field_4->field_4 & 2) != 0) {
            field_58 = 0;
            field_50[field_79] = 0;
            field_68 = BufferSize;
            m_requestID = nflReadFileAsync(field_28, field_5C, field_48[field_79], BufferSize);
        }
    } else {
        field_50[field_79] = field_58 - field_3C;
        field_68 = BufferSize - field_3C;
        m_requestID = nflReadFileAsync(field_28, field_58 + field_5C, field_48[field_79] + field_3C, field_68);
    }
}

bool nalStreamInstance::Advance(Float dt)
{
    nflRequestInfo info;
    switch (field_1C) {
    case 0: {
        int alignment[3];
        sub_79E020(sub_79DA70(field_28), alignment);
        field_34 = std::max(std::max(alignment[0], alignment[1]), alignment[2]);
        Size = (field_34 + 79) & ~(field_34 - 1);
        field_2C = static_cast<nalSceneAnim *>(tlMemAlloc(Size, field_34, 0));
        m_requestID = nflReadFileAsync(field_28, field_5C, field_2C, Size);
        field_1C = 1;
        return false;
    }
    case 1:
        nflGetRequestInfo(m_requestID, &info);
        if (nflGetRequestState(m_requestID) == 4)
            return false;
        if (header_size(field_2C) > Size) {
            const int size = (field_34 + header_size(field_2C) + 3) & ~(field_34 - 1);
            auto *memory = static_cast<char *>(tlMemAlloc(size, field_34, 0));
            std::memcpy(memory, field_2C, Size);
            tlMemFree(field_2C);
            field_2C = reinterpret_cast<nalSceneAnim *>(memory);
            m_requestID = nflReadFileAsync(field_28, Size + field_5C, memory + Size, size - Size);
            Size = size;
            field_1C = 2;
        } else
            field_1C = 3;
        return false;
    case 2:
        nflGetRequestInfo(m_requestID, &info);
        if (nflGetRequestState(m_requestID) != 4)
            field_1C = 3;
        return false;
    case 3: {
        field_4 = static_cast<nalSceneAnim *>(tlMemAlloc(header_size(field_2C), 8, 0));
        std::memcpy(static_cast<void *>(field_4), field_2C, header_size(field_2C));
        field_3C = (field_4->field_38 + field_34 - 1) & ~(field_34 - 1);
        field_40 = (BufferSize / field_3C) & ~1;
        if (field_40 < 4)
            field_40 = 4;
        BufferSize = field_3C * (field_40 / 2);
        field_48[0] = static_cast<char *>(tlMemAlloc(BufferSize, field_34, 0));
        field_48[1] = static_cast<char *>(tlMemAlloc(BufferSize, field_34, 0));
        field_50[0] = 0;
        std::memcpy(field_48[field_78], field_2C, Size);
        tlMemFree(field_2C);
        field_68 = BufferSize - Size;
        field_58 = Size;
        m_requestID = nflReadFileAsync(field_28, Size + field_5C, field_48[field_79] + Size, field_68);
        CurrentOffset = header_size(field_4);
        field_18 = reinterpret_cast<nalSceneAnimSegment *>(field_48[field_78] + CurrentOffset);
        field_1C = 4;
        return false;
    }
    case 4:
        nflGetRequestInfo(m_requestID, &info);
        if ((info.field_0 < field_50[field_78] + CurrentOffset + 12 ||
             info.field_0 < CurrentOffset + field_50[field_78] + segment_size(field_18)) &&
            nflGetRequestState(m_requestID) == 4)
            return false;
        field_1C = 5;
        return false;
    case 5:
        field_38 = static_cast<nalBaseSkeleton **>(tlMemAlloc(4 * field_4->field_C, 8, 0));
        for (int i = 0; i < field_4->field_C; ++i)
            field_38[i] = nalGetSkeletonDirectory()->Find((&field_4->field_50)[i]);
        field_74 = segment_size(field_18);
        process_segment(this);
        for (auto *anim = field_18->anim; anim != nullptr; anim = anim->field_4) {
            auto *client = m_callback(anim->field_8, field_24);
            if (client != nullptr)
                AddClientAnim(client, anim);
        }
        field_1C = 6;
        break;
    case 6:
        break;
    default:
        return false;
    }
    if ((field_4->field_4 & 2) == 0 && field_10 >= bit_cast<float>(field_4->field_3C))
        return true;
    for (;;) {
        AdvanceStream();
        const float previous_total = field_10;
        const float previous = field_14;
        const float duration = field_18->anim->field_38;
        if (dt + previous < duration || segment_size(field_18) == 0) {
            field_10 += dt;
            field_14 += dt;
            const double inverse = 1.0 / duration;
            for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
                auto *client = entry->field_0;
                if (client != nullptr) {
                    if (entry->field_8 == nullptr)
                        entry->field_8 = client->m_vtbl->CreateInstance(client, nullptr, entry->field_4);
                    client->m_vtbl->Advance(client,
                                            nullptr,
                                            entry->field_8,
                                            static_cast<float>(inverse * field_14),
                                            static_cast<float>(inverse * previous),
                                            field_10,
                                            previous_total);
                }
            }
            return true;
        }
        while (m_requestID.field_0 != -1) {
            nflGetRequestInfo(m_requestID, &info);
            const int end =
                segment_size(field_18) != 0 ? segment_size(field_18) + CurrentOffset + field_50[field_78] : field_70;
            if (info.field_0 + field_58 >= end && nflGetRequestState(m_requestID) == 0)
                break;
            AdvanceStream();
            nflUpdate();
        }
        const double step = static_cast<double>(duration) - field_14;
        field_14 = duration;
        field_10 = static_cast<float>(step + field_10);
        dt = static_cast<float>(dt - step);
        for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
            auto *client = entry->field_0;
            if (client != nullptr) {
                if (entry->field_8 == nullptr)
                    entry->field_8 = client->m_vtbl->CreateInstance(client, nullptr, entry->field_4);
                client->m_vtbl->Advance(
                    client, nullptr, entry->field_8, 1.0f, previous / duration, field_10, previous_total);
            }
        }
        field_14 = 0.0f;
        const int size = segment_size(field_18);
        if (size != 0) {
            CurrentOffset += size;
            if (CurrentOffset < BufferSize) {
                field_18 = reinterpret_cast<nalSceneAnimSegment *>(field_48[field_78] + CurrentOffset);
                const int end = segment_size(field_18) != 0 ? segment_size(field_18) + CurrentOffset
                                                            : field_70 - field_50[field_78];
                if (end > BufferSize) {
                    const int offset = CurrentOffset % field_3C;
                    std::memcpy(field_48[field_78 ^ 1] + offset, field_48[field_78] + CurrentOffset, field_3C - offset);
                    field_78 ^= 1;
                    CurrentOffset = offset;
                    field_18 = reinterpret_cast<nalSceneAnimSegment *>(field_48[field_78] + offset);
                }
            } else {
                field_78 ^= 1;
                CurrentOffset = field_3C;
                field_18 = reinterpret_cast<nalSceneAnimSegment *>(field_48[field_78] + CurrentOffset);
            }
        } else {
            if ((field_4->field_4 & 2) == 0) {
                field_10 += dt;
                return true;
            }
            if (field_58 != 0) {
                nflCancelRequest(m_requestID);
                while (nflGetRequestState(m_requestID) == 4)
                    nflUpdate();
                field_79 = field_78 ^ 1;
                field_58 = 0;
                field_50[field_79] = 0;
                field_68 = BufferSize;
                m_requestID = nflReadFileAsync(field_28, field_5C, field_48[field_79], BufferSize);
            }
            field_78 ^= 1;
            CurrentOffset = header_size(field_4);
            field_18 = reinterpret_cast<nalSceneAnimSegment *>(field_48[field_78] + CurrentOffset);
        }
        process_segment(this);
        auto *anim = field_18->anim;
        for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
            destroy_stream_anim(entry);
            entry->field_4 = anim;
            entry->field_8 = nullptr;
            anim = anim != nullptr ? anim->field_4 : nullptr;
        }
        if (dt <= 0.0f)
            return true;
    }
}

nalStreamInstance *create_stream_instance(uint32_t file, uint32_t offset, uint32_t size, int,
                                          nalSceneAnimCallback callback, void *parameter)
{
    auto *memory = tlMemAlloc(sizeof(nalStreamInstance), 8, 0);
    if (memory == nullptr)
        return nullptr;
    auto *instance = ::new (memory) nalStreamInstance;
    instance->m_callback = callback;
    instance->field_24 = parameter;
    instance->field_28 = nflFileID(static_cast<int>(file));
    instance->field_4 = nullptr;
    instance->field_8 = nullptr;
    instance->field_C = nullptr;
    instance->field_10 = 0.0f;
    instance->field_14 = 0.0f;
    instance->field_18 = nullptr;
    instance->field_1C = 0;
    instance->BufferSize = 0;
    instance->field_78 = 0;
    instance->field_79 = 0;
    instance->m_vtbl = &stream_vtable;
    instance->field_5C = offset;
    instance->field_70 = size;
    return instance;
}
