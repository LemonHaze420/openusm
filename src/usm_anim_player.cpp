#include "usm_anim_player.h"

#include "common.h"
#include "func_wrapper.h"
#include "nal_anim.h"
#include <nal_anim_comp.h>
#include <nal_instance.h>
#include "nal_system.h"
#include "ngl.h"
#include "utility.h"
#include "vtbl.h"

#include <functional>

template <>
bool usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback::Invoke(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3> *player);
template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback::Release();

using type = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

VALIDATE_SIZE(type, 0x2C);
VALIDATE_OFFSET(type::nalAnimState, field_28, 0x28);

void linear_interpolator::setup(Float a2, Float a3)
{
    this->field_0 = a2;
    this->field_4 = a3;
}

bool linear_interpolator::update(Float a2)
{
    this->field_0 = (this->field_4 * a2) + this->field_0;
    if (this->field_0 < 1.0) {
        if (this->field_0 > 0.0) {
            return equal(this->field_4, 0.0f);
        } else {
            this->field_0 = 0.0;
            return this->field_4 <= 0.0;
        }
    } else {
        this->field_0 = 1.0;
        return this->field_4 >= 0.0;
    }
}


template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_853C80(nalAnyPose &a2, nalAnyPose &a3,
                                                                            const nalAnyPose &a4)
{
    this->field_0->VirtualGetPose(this->field_18, this->field_1C, *a3.field_0, *a4.field_0);

    sub_826140(a2, this->field_20.field_0, a2, a3);
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod::Advance(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1, Float a2)
{
    a1->field_18 = a1->field_0->field_8 * a1->field_4 * a2 + a1->field_18;
}


template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod::Compose(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2, nalAnyPose &a3, nalAnyPose &a4,
    const nalAnyPose &a5)
{
    if constexpr (0) {
        a2->sub_853C80(a3, a4, a5);
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *,
                                nalAnyPose *,
                                nalAnyPose *,
                                const nalAnyPose *) = CAST(func, get_vfunc(m_vtbl, 0x4));
        func(this, nullptr, a2, &a3, &a4, &a5);
    }
}

template <>
nalComp::nalCompInstance *
usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod::CreateInstance(nalAnimClass<nalAnyPose> *a1,
                                                                            nalBaseSkeleton *a2, void *a3)
{
    nalComp::nalCompInstance *(__fastcall *func)(
        void *, void *, nalAnimClass<nalAnyPose> *, nalBaseSkeleton *, void *) =
        CAST(func, get_vfunc(m_vtbl, 0x8));
    return func(this, nullptr, a1, a2, a3);
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod::Reference(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2)
{
    void(__fastcall * func)(void *, void *edx, usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *) =
        CAST(func, get_vfunc(m_vtbl, 0xC));
    func(this, nullptr, a2);
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod::Release()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x10));
    func(this);
}


template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_853CF0(nalAnyPose &a2, nalAnyPose &a3)
{
    TRACE("sub_853CF0");

    auto &v3 = this->field_14->field_C;
    if (this->field_10 != nullptr) {
        this->field_10->Compose(this, a2, a3, v3);
    } else {
        this->sub_853C80(a2, a3, v3);
    }

    this->field_1C = this->field_18;
}


template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_854140(nalAnyPose &a2, nalAnyPose &a3)
{
    TRACE("sub_854140");

    nalAnyPose *v3 = &a2;
    if (this->field_50 == 2) {
        v3 = &this->field_14->field_C;
    }

    if (this->field_10 != nullptr) {
        this->field_10->Compose(this, a2, a3, *v3);
    } else {
        this->sub_853C80(a2, a3, *v3);
    }

    this->field_1C = this->field_18;
}

template <>
usm_anim_player<nalAnimClass<nalAnyPose>, 3>::usm_anim_player(nalBaseSkeleton *a2) : field_0(a2)
{
    auto *v3 = a2->VirtualGetDefaultPose();
    auto *v4 = v3->field_0->VirtualCreatePose();

    this->field_4.field_0 = v4;
    v4->field_0->VirtualCopyPose(*v4, *v3);
    this->field_8.field_0 = a2->VirtualCreatePose();
    auto *v5 = a2->VirtualGetDefaultPose();
    auto *v6 = v5->field_0->VirtualCreatePose();
    this->field_C.field_0 = v6;
    v6->field_0->VirtualCopyPose(*v6, *v5);

    this->field_10 = 0;
    this->field_20 = nullptr;
    this->field_24 = nullptr;
    this->field_28 = 0;

    for (int i = 0; i < 3; ++i) {
        this->field_14[i] = new nalAnimState{};
    }
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4AE5F0(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2)
{
    auto v3 = a2->field_44;
    auto *v4 = this->field_20;
    for (auto i = a2->field_48; v4 != nullptr; v4 = v4->field_40) {
        if ((v3 & v4->field_44) == v4->field_44 && v4 != a2 && v4->field_48 <= i) {
            auto *v5 = v4->field_C;
            v4->field_38 = 2;
            if (v5 != nullptr) {
                v5->Release();
            }
            v4->field_C = nullptr;
            v4->field_38 = 3;
            v4->field_20.setup(0.0, 0.0);
        }
    }
}

template <>
bool usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_4B0020(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3> *a2, Float a3)
{
    {
        auto *v4 = this->field_10;
        if (v4 != nullptr) {
            v4->Advance(this, a3);
        } else {
            this->field_18 = this->field_0->field_8 * this->field_4 * a3 + this->field_18;
        }

        auto *callback = this->field_C;
        if (callback != nullptr && this->field_18 >= this->field_8) {
            const int previous_state = this->field_38;
            this->field_C = nullptr;
            this->field_38 = 2;
            if (!callback->Invoke(a2) && this->field_38 == 2) {
                this->field_38 = previous_state;
            }
            callback->Release();
        }

        auto v7 = this->field_38;
        if (v7) {
            auto v8 = v7 - 2;
            if (v8) {
                if (v8 != 1) {
                    return false;
                }
            } else {
                this->field_20.setup(this->field_20.field_0, -this->field_4C);
                if (equal(this->field_4C, 0.0f)) {
                    return true;
                }

                this->field_38 = 3;
            }

            if (this->field_20.update(a3)) {
                return true;
            }

        } else if (this->field_20.update(a3)) {
            auto v11 = this->field_50;
            if (v11 == 1 || v11 == 2) {
                a2->sub_4AE5F0(this);
            }

            this->field_38 = 1;
        }

        return false;
    }
}

template <>
bool usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback::Invoke(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3> *a1)
{
    bool(__fastcall * func)(void *, void *edx, void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    return func(this, nullptr, a1);
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback::Reference()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this);
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback::Release()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this);
}


template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState::sub_4AD850(
    nalAnimClass<nalAnyPose> *a2, nalBaseSkeleton *a3, Float a4,
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod *a5, Float a6,
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimCallback *a7, Float a8,
    usm_anim_player<nalAnimClass<nalAnyPose>, 3> *a9, void *a10, bool a11, Float a12)
{
    nalComp::nalCompInstance *v13 = nullptr;
    this->field_14 = a9;
    if (a5 != nullptr) {
        v13 = a5->CreateInstance(a2, a3, a10);
    } else {
        v13 = static_cast<nalComp::nalCompInstance *>(a2->CreateInstance(a3));
    }

    float v14 = 0.0f;
    this->field_0 = v13;
    this->field_4 = a8;

    float v15;
    if (equal<float>(a2->field_38, v14)) {
        v15 = 0.0f;
    } else {
        v15 = 1.0f / a2->field_38;
    }

    this->field_C = a7;
    this->field_8 = v15 * a6 + 1.0f;
    if (a7 != nullptr) {
        a7->Reference();
    }

    auto v16 = equal<float>(a4, 0.0f);
    this->field_18 = a12;
    this->field_10 = a5;
    this->field_1C = a12;
    this->field_38 = 1;
    this->field_34 = -a11;
    if (v16) {
        this->field_20.setup(1.0, a4);
    } else {
        this->field_20.setup(0.0, a4);
    }

    auto *v17 = this->field_10;
    if (v17 != nullptr) {
        v17->Reference(this);
    }
}

namespace {
void release_anim_state(type::nalAnimState *state)
{
    if (state->field_C != nullptr)
        state->field_C->Release();
    if (state->field_10 != nullptr)
        state->field_10->Release();
    if (state->field_0 != nullptr)
        state->field_0->finalize(true);
}

bool __fastcall nonlooping_invoke(type::nalAnimCallback *, void *, type *) { return true; }
void __fastcall nonlooping_reference(type::nalAnimCallback *, void *) {}
void __fastcall nonlooping_release(type::nalAnimCallback *, void *) {}

type::nalAnimCallback *nonlooping_callback()
{
    static void *table[] = {
        reinterpret_cast<void *>(&nonlooping_invoke),
        reinterpret_cast<void *>(&nonlooping_reference),
        reinterpret_cast<void *>(&nonlooping_release),
    };
    static type::nalAnimCallback callback{static_cast<int>(reinterpret_cast<std::intptr_t>(table))};
    return &callback;
}
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::PlayModifier(nalAnimClass<nalAnyPose> *a2,
                                                                usm_anim_player_modifier_type a3, Float a4,
                                                                nalPlayMethod *a5, Float a6, int a7, Float a8, void *a9,
                                                                bool a10, void *a11)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *previous = field_10 > 0 ? field_14[0]->field_0->field_10 : nullptr;
        if (field_10 <= 0)
            a4 = 0.0f;
        auto *callback = reinterpret_cast<nalAnimCallback *>(a7);
        if (static_cast<unsigned>(a3) == 0 && a2 == previous &&
            !(callback != nullptr && field_14[0]->field_C == nullptr))
            return;
        if (field_10 == 3) {
            field_14[2]->sub_853CF0(field_4, field_8);
            release_anim_state(field_14[2]);
        } else {
            ++field_10;
        }
        auto *state = field_14[field_10 - 1];
        for (int index = field_10 - 1; index > 0; --index)
            field_14[index] = field_14[index - 1];
        field_14[0] = state;
        const float rate = equal<float>(a4, 0.0f) ? 0.0f : 1.0f / a4;
        const float start = bit_cast<float>(static_cast<uint32_t>(reinterpret_cast<std::uintptr_t>(a9)));
        state->sub_4AD850(a2, field_0, rate, a5, a6, callback, a8, this, a11, a10, start);
        if ((a2->field_34 & 1) != 0 && previous != nullptr && (previous->field_34 & 1) != 0 &&
            static_cast<unsigned>(a3) == 0) {
            state->field_8 += field_14[1]->field_18;
            state->field_18 = field_14[1]->field_18;
            state->field_1C = a2 == previous ? field_14[1]->field_1C : state->field_18;
        }
    } else {
        THISCALL(0x004B0530, this, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
}

template <>
void type::PlayModifier(nalAnimClass<nalAnyPose> *anim, usm_anim_player_modifier_type modifier,
                        Float priority, uint32_t domains, bool force_restart, Float blend_time,
                        Float fade_out_time, nalPlayMethod *method, Float extra_time,
                        nalAnimCallback *callback, Float speed, bool completion_flag, void *parameter)
{
    const bool looping = (anim->field_34 & 1) != 0;
    nalAnimState *previous = nullptr;
    if (looping || !force_restart) {
        for (auto *state = field_20; state != nullptr; state = state->field_40) {
            if (state->field_44 == static_cast<int>(domains) && equal<float>(state->field_48, priority))
                previous = state;
        }
        if (previous != nullptr && previous->field_0->field_10 == anim && !force_restart &&
            (callback == nullptr || previous->field_C != nullptr))
            return;
    }
    if (callback == nullptr && !looping)
        callback = nonlooping_callback();
    const auto mode = static_cast<unsigned>(modifier);
    for (auto *state = field_20; state != nullptr; state = state->field_40) {
        if ((domains & static_cast<uint32_t>(state->field_44)) == static_cast<uint32_t>(state->field_44)) {
            state->field_38 = 2;
            if ((mode == 1 || mode == 2) && state->field_48 <= priority)
                state->field_38 = 1;
            if (state->field_C != nullptr)
                state->field_C->Release();
            state->field_C = nullptr;
        }
    }
    auto **position = &field_20;
    while (*position != nullptr && (*position)->field_48 <= priority)
        position = &(*position)->field_40;
    auto *state = field_24;
    if (state != nullptr) {
        field_24 = state->field_40;
    } else {
        state = new nalAnimState;
        state->field_0 = nullptr;
        state->field_C = nullptr;
        state->field_10 = nullptr;
    }
    state->field_40 = *position;
    *position = state;
    const float blend_rate = equal<float>(blend_time, 0.0f) ? 0.0f : 1.0f / blend_time;
    state->sub_4AD850(anim, field_0, blend_rate, method, extra_time, callback,
                     speed, this, parameter, completion_flag, 0.0f);
    state->field_50 = static_cast<int>(modifier);
    state->field_44 = static_cast<int>(domains);
    state->field_48 = priority;
    state->field_38 = 0;
    state->field_4C = equal<float>(fade_out_time, 0.0f) ? 0.0f : 1.0f / fade_out_time;
    state->field_3C = field_28;
    if (previous != nullptr && looping && (previous->field_0->field_10->field_34 & 1) != 0 &&
        !force_restart) {
        state->field_18 = previous->field_18;
        state->field_1C = previous->field_0->field_10 == anim ? previous->field_1C : state->field_18;
    }
}

template <>
void type::Reset()
{
    auto *source = field_0->VirtualGetDefaultPose();
    auto *pose = source->field_0->VirtualCreatePose();
    pose->field_0->VirtualCopyPose(*pose, *source);
    field_4.field_0->field_0->VirtualCopyPose(*field_4.field_0, *pose);
    pose->field_0->VirtualDestroyPose(pose);
    for (int index = 0; index < field_10; ++index)
        release_anim_state(field_14[index]);
    field_10 = 0;
    while (field_20 != nullptr) {
        auto *state = field_20;
        field_20 = state->field_40;
        release_anim_state(state);
        state->field_40 = field_24;
        field_24 = state;
    }
}

template <>
void type::KillDomain(uint32_t domains)
{
    auto **position = &field_20;
    while (*position != nullptr) {
        auto *state = *position;
        if ((domains & static_cast<uint32_t>(state->field_44)) != 0) {
            *position = state->field_40;
            release_anim_state(state);
            state->field_40 = field_24;
            field_24 = state;
        } else {
            position = &state->field_40;
        }
    }
}

template <>
void type::KillPriority(Float priority)
{
    auto **position = &field_20;
    while (*position != nullptr) {
        auto *state = *position;
        if (equal<float>(state->field_48, priority)) {
            *position = state->field_40;
            release_anim_state(state);
            state->field_40 = field_24;
            field_24 = state;
        } else {
            position = &state->field_40;
        }
    }
}




template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4B06A0(Float a2)
{
    TRACE("sub_4B06A0");

    {
        auto v4 = &this->field_20;
        ++this->field_28;
        auto v5 = this->field_20;
        while (v5 != nullptr) {
            if (v5->field_3C != this->field_28) {
                if (v5->sub_4B0020(this, a2)) {
                    if (*v4 != v5) {
                        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *v7 = nullptr;
                        do {
                            auto v6 = *v4;
                            v7 = v6->field_40;
                            v4 = &v6->field_40;
                        } while (v7 != v5);
                    }
                    *v4 = v5->field_40;
                    auto *v8 = v5->field_C;
                    if (v8 != nullptr) {
                        v8->Release();
                    }

                    auto v9 = v5->field_10;
                    if (v9 != nullptr) {
                        v9->Release();
                    }

                    if (v5->field_0 != nullptr) {
                        v5->field_0->finalize(true);
                    }

                    v5->field_40 = this->field_24;
                    this->field_24 = v5;
                } else {
                    v4 = &v5->field_40;
                }
                v5 = *v4;
            }
        }

        int v10 = 0;
        if (this->field_10 > 0) {
            while (1) {
                auto *v11 = this->field_14[v10];
                auto *v12 = v11->field_10;
                if (v12 != nullptr) {
                    v12->Advance(v11, a2);
                } else {
                    v11->field_18 = v11->field_0->field_8 * v11->field_4 * a2 + v11->field_18;
                }

                auto *v13 = v11->field_C;
                if (v13 != nullptr && v11->field_18 >= v11->field_8) {
                    v11->field_C = nullptr;
                    v13->Invoke(this);
                    v13->Release();
                }

                auto v14 = v11->field_20.update(a2);
                auto v15 = this->field_10;
                auto v16 = v14;
                if (v10 < v15) {
                    auto v17 = &this->field_14[v10];
                    do {
                        if (*v17 == v11) {
                            break;
                        }

                        ++v10;
                        ++v17;
                    } while (v10 < this->field_10);
                }

                if (v16 && v10 < v15) {
                    break;
                }

                if (++v10 >= this->field_10) {
                    goto LABEL_32;
                }
            }
            ++v10;
        }
    LABEL_32:
        auto a2a = v10;
        if (v10 < this->field_10) {
            auto v18 = &this->field_14[v10];
            do {
                auto v19 = *v18;
                auto *v20 = (*v18)->field_C;
                if (v20 != nullptr) {
                    v20->Release();
                }

                auto *v21 = v19->field_10;
                if (v21 != nullptr) {
                    v21->Release();
                }

                if (v19->field_0 != nullptr) {
                    v19->field_0->finalize(true);
                }

                ++v10;
                ++v18;
            } while (v10 < this->field_10);
        }

        this->field_10 = a2a;
    }
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4B0860(nalAnyPose &pose)
{
    TRACE("sub_4B0860");

    {
        pose = this->field_4;

        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *v2 = nullptr;
        for (auto *i = this->field_20; i != nullptr; i = i->field_40) {
            if (i->field_50 == 2 && i->field_20.field_0 >= 1.0) {
                v2 = i;
            }
        }

        if (v2 == nullptr) {
            for (auto j = this->field_10 - 1; j >= 0; --j) {
                this->field_14[j]->sub_853CF0(pose, this->field_8);
            }

            v2 = this->field_20;
        }

        for (auto *k = v2; k != nullptr; k = k->field_40) {
            k->sub_854140(pose, this->field_8);
        }

    }
}

template <>
void usm_anim_player<nalAnimClass<nalAnyPose>, 3>::sub_4AE210()
{
    if constexpr (STANDALONE_SYSTEM) {
        Reset();
        for (auto *state : field_14)
            delete state;
        while (field_24 != nullptr) {
            auto *state = field_24;
            field_24 = state->field_40;
            delete state;
        }
        field_C.field_0->field_0->VirtualDestroyPose(field_C.field_0);
        field_8.field_0->field_0->VirtualDestroyPose(field_8.field_0);
        field_4.field_0->field_0->VirtualDestroyPose(field_4.field_0);
    } else {
        THISCALL(0x004AE210, this);
    }
}

template <>
usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *
usm_anim_player<nalAnimClass<nalAnyPose>, 3>::Advance(Float priority)
{

    auto *state = this->field_20;
    return state != nullptr && std::equal_to<float>{}(state->field_48, priority.value) ? state : nullptr;
}

bool __fastcall sub_4B0020(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *self, void *, int a2, Float a3)
{
    TRACE("sub_4B0020");

    sp_log("0x%08X", self->field_C->m_vtbl);
    sp_log("0x%08X", self->field_10->m_vtbl);

    return THISCALL(0x004B0020, self, a2, a3);
}

void __fastcall sub_4AD850(void *self, void *, nalAnimClass<nalAnyPose> *a2, int a3, Float a4,
                           usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod *a5, Float a6, void *a7, int a8,
                           int a9, void *a10, bool a11, int a12)
{
    sp_log("0x%08X", a5->m_vtbl);
    sp_log("0x%08X", a2->m_vtbl);

    THISCALL(0x004AD850, self, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

void usm_anim_player_patch()
{
    REDIRECT(0x004B06D7, sub_4B0020);

    if constexpr (0) {
        REDIRECT(0x004AFF2E, sub_4AD850);
        REDIRECT(0x004B063A, sub_4AD850);
    }

    {
        using type = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;
        FUNC_ADDRESS(address, &type::sub_4B0860);
        REDIRECT(0x004A6172, address);
    }
}
