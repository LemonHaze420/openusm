#include "scene_anim.h"

#include "common.h"
#include "func_wrapper.h"
#include "tl_system.h"
#include "utility.h"

#include <algorithm>
#include <cstring>
#include <new>

VALIDATE_SIZE(nalSceneAnimInstance, 0x1C);
VALIDATE_SIZE(nalStaticInstance, 0x20);
VALIDATE_SIZE(nalSceneAnimSegment, 0xC);

namespace {
void *__fastcall static_destroy(nalSceneAnimInstance *instance, void *, unsigned flags)
{
    instance->~nalSceneAnimInstance();
    if ((flags & 1) != 0)
        tlMemFree(instance);
    return instance;
}

bool __fastcall static_ready(const nalSceneAnimInstance *, void *)
{
    return true;
}

bool __fastcall static_advance(nalSceneAnimInstance *instance, void *, Float dt)
{
    return static_cast<nalStaticInstance *>(instance)->Advance(dt);
}

nalSceneAnimInstance::vtable static_vtable{static_destroy, static_ready, static_advance};

void destroy_anim_instance(nalAnimClass<nalAnyPose>::nalInstanceClass *instance)
{
    using destroy_fn = void *(__fastcall *)(void *, void *, unsigned);
    reinterpret_cast<destroy_fn>(get_vfunc(instance->m_vtbl, 0))(instance, nullptr, 1);
}
}

nalSceneAnimInstance *nalSceneAnim::CreateInstance(nalSceneAnimCallback callback, void *parameter)
{
    auto *memory = tlMemAlloc(sizeof(nalStaticInstance), 8, 0x2000000u);
    return memory != nullptr ? ::new (memory) nalStaticInstance(this, callback, parameter) : nullptr;
}

nalStaticInstance::nalStaticInstance(nalSceneAnim *scene, nalSceneAnimCallback callback, void *parameter)
{
    m_vtbl = &static_vtable;
    field_4 = scene;
    field_8 = nullptr;
    field_C = nullptr;
    field_10 = 0.0f;
    field_14 = 0.0f;
    field_1C = scene->field_34;
    field_18 = field_1C;
    for (auto *anim = field_18->anim; anim != nullptr; anim = anim->field_4) {
        auto *client = callback(anim->field_8, parameter);
        if (client != nullptr)
            AddClientAnim(client, anim);
    }
}

nalSceneAnimInstance::~nalSceneAnimInstance()
{
    while (field_8 != nullptr) {
        auto *entry = field_8;
        field_8 = entry->field_C;
        entry->field_0->m_vtbl->Release(entry->field_0, nullptr);
        if (entry->field_8 != nullptr)
            destroy_anim_instance(entry->field_8);
        tlMemFree(entry);
    }
}

void nalSceneAnimInstance::Destroy()
{
    m_vtbl->Destroy(this, nullptr, 1);
}

bool nalSceneAnimInstance::IsReady() const
{
    return m_vtbl->IsReady(this, nullptr);
}

bool nalSceneAnimInstance::Advance(Float dt)
{
    return m_vtbl->Advance(this, nullptr, dt);
}

bool nalSceneAnimInstance::IsFinished() const
{
    return (field_4->field_4 & 2) == 0 && field_18->next == nullptr && field_14 >= field_18->anim->field_38;
}

void nalSceneAnimInstance::AddClientAnim(nalClientSceneAnim *client, nalAnimClass<nalAnyPose> *anim)
{
    auto *entry = static_cast<client_anim *>(tlMemAlloc(sizeof(client_anim), 8, 0x2000000u));
    entry->field_0 = client;
    entry->field_4 = anim;
    entry->field_8 = nullptr;
    entry->field_C = nullptr;
    if (field_C != nullptr)
        field_C->field_C = entry;
    else
        field_8 = entry;
    field_C = entry;
}

bool nalStaticInstance::Advance(Float dt)
{
    if ((field_4->field_4 & 2) != 0 || field_10 < bit_cast<float>(field_4->field_3C) + bit_cast<float>(0x3D088889u)) {
        float previous = field_14;
        float previous_total = field_10;
        field_14 += dt;
        float duration = field_18->anim->field_38;
        while (field_14 >= duration) {
            const auto step = duration - previous;
            dt = Float{dt.value - step};
            field_10 += step;
            const double inverse = 1.0 / duration;
            const float time = static_cast<float>(std::min(inverse * field_14, 1.0));
            const float last = static_cast<float>(inverse * previous);
            for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
                auto *client = entry->field_0;
                if (entry->field_8 == nullptr)
                    entry->field_8 = client->m_vtbl->CreateInstance(client, nullptr, entry->field_4);
                client->m_vtbl->Advance(client, nullptr, entry->field_8, time, last, field_10, previous_total);
            }
            if (field_18->next != nullptr)
                field_18 = field_18->next;
            else {
                if ((field_4->field_4 & 2) == 0)
                    break;
                field_18 = field_1C;
            }
            field_14 -= duration;
            auto *anim = field_18->anim;
            for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
                while (std::memcmp(&anim->field_8, &entry->field_4->field_8, sizeof(tlFixedString)) != 0)
                    anim = anim->field_4;
                if (entry->field_8 != nullptr)
                    destroy_anim_instance(entry->field_8);
                entry->field_4 = anim;
                entry->field_8 = nullptr;
                anim = anim->field_4;
            }
            previous = 0.0f;
            duration = field_18->anim->field_38;
            previous_total = field_10;
        }
        field_10 += dt;
        const double inverse = 1.0 / duration;
        for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
            auto *client = entry->field_0;
            if (entry->field_8 == nullptr)
                entry->field_8 = client->m_vtbl->CreateInstance(client, nullptr, entry->field_4);
            const float rounded_inverse = static_cast<float>(inverse);
            client->m_vtbl->Advance(client,
                                    nullptr,
                                    entry->field_8,
                                    rounded_inverse * field_14,
                                    static_cast<float>(inverse * previous),
                                    field_10,
                                    previous_total);
        }
    }
    return true;
}

void nalSceneAnimInstance::Render() const
{
    const float time = field_14 / field_18->anim->field_38;
    for (auto *entry = field_8; entry != nullptr; entry = entry->field_C) {
        auto *client = entry->field_0;
        if (entry->field_8 == nullptr)
            entry->field_8 = client->m_vtbl->CreateInstance(client, nullptr, entry->field_4);
        client->m_vtbl->Render(client, nullptr, entry->field_8, time);
    }
}

void scene_anim_patch()
{
    FUNC_ADDRESS(address, &nalSceneAnimInstance::Render);
    REDIRECT(0x00741734, address);
    REDIRECT(0x007417F5, address);
    REDIRECT(0x00742086, address);
}
