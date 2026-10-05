#include "generic_anim_controller.h"

#include "func_wrapper.h"

#include <nal_generic.h>
#include "trace.h"
#include "actor.h"
#include "anim_event.h"
#include "event_manager.h"
#include "nal_instance.h"
#include "oldmath_po.h"
#include "quaternion.h"
#include "utility/common.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <type_traits>

VALIDATE_SIZE(generic_anim_controller, 0x104);

#if STANDALONE_SYSTEM
namespace {
using player = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

template <typename Handle>
void find_component(nalGeneric::nalGenericSkeleton *skeleton, Handle &handle, uint32_t bone_hash,
                    const tlFixedString *bone_name, const tlFixedString &component_name,
                    const void *type, bool allow_constant)
{
    handle = {};
    int component_index = 0;
    for (int constant = 0; constant <= int(allow_constant); ++constant) {
        auto *infos = constant ? skeleton->field_A8 : skeleton->field_8C;
        const int count = constant ? skeleton->field_A4 : skeleton->field_88;
        for (int group = 0; group < count; ++group) {
            auto &info = infos[group];
            for (int i = 0; i < info.field_28; ++i, ++component_index) {
                const auto *name = reinterpret_cast<const char *>(skeleton->field_84) + 40 * component_index;
                if (std::memcmp(name, &component_name, sizeof(tlFixedString)) != 0)
                    continue;
                const int bone_index = *reinterpret_cast<const int *>(name + 32);
                const auto *bone = reinterpret_cast<const char *>(skeleton->field_78) + 48 * bone_index;
                if (bone_name ? std::memcmp(bone, bone_name, sizeof(tlFixedString)) != 0
                              : *reinterpret_cast<const uint32_t *>(bone) != bone_hash)
                    continue;
                auto **vtable = reinterpret_cast<void **>(info.field_20->m_vtbl);
                auto get_type = reinterpret_cast<const void *(__fastcall *)(nalComponentBase *, void *)>(vtable[0]);
                if (get_type(info.field_20, nullptr) != type)
                    continue;
                handle.Skeleton = skeleton;
                handle.field_4 = bit_cast<decltype(handle.field_4)>(&infos[group]);
                handle.field_8 = i;
                handle.field_C = constant;
                return;
            }
        }
    }
}

template <typename T>
void *component_type()
{
    if constexpr (std::is_same_v<T, float>)
        return &nalComponentFloat1Base::TypeID;
    else if constexpr (std::is_same_v<T, nalPositionOrientation>)
        return &nalComponentPOBase::TypeID;
    else if constexpr (std::is_same_v<T, nalVector3>)
        return &nalComponentFloat3Base::TypeID;
    else
        return &nalComponentU8Base::TypeID;
}

template <typename T>
void find_component(nalGeneric::nalGenericSkeleton *skeleton, nalGeneric::nalGenericComponentHandle<T> &handle,
                    tlFixedString &bone, tlFixedString &component)
{
    find_component(skeleton, handle, bone.m_hash, &bone, component, component_type<T>(), false);
}

template <typename T>
void find_component(nalGeneric::nalGenericSkeleton *skeleton, nalGeneric::nalGenericConstComponentHandle<T> &handle,
                    tlFixedString &bone, tlFixedString &component)
{
    find_component(skeleton, handle, bone.m_hash, &bone, component, component_type<T>(), true);
}

template <typename T, typename Handle>
T &pose_component(nalGeneric::nalGenericPose *pose, const Handle &handle, bool constant = false)
{
    auto *info = reinterpret_cast<const nalGeneric::nalComponentInfo *>(handle.field_4);
    const int data = constant ? pose->field_0->field_B4 : pose->field_4;
    return *reinterpret_cast<T *>(data + info->field_2C + sizeof(T) * handle.field_8);
}

template <typename T>
const T &pose_component(nalGeneric::nalGenericPose *pose, const nalGeneric::nalGenericConstComponentHandle<T> &handle)
{
    return pose_component<T, nalGeneric::nalGenericConstComponentHandle<T>>(pose, handle, (handle.field_C & 0xFF) != 0);
}


const char *event_data(nalGeneric::nalGenericAnim *anim,
                       const nalGeneric::nalGenericComponentHandle<unsigned char> &handle)
{
    struct cursor {
        const char *location;
        int field_4;
        nalGeneric::nalGenericSkeleton *skeleton;
    } stream{*reinterpret_cast<const char **>(reinterpret_cast<char *>(anim) + 0x5C), 0, anim->field_30};
    const auto *bits = *reinterpret_cast<const uint32_t **>(reinterpret_cast<char *>(anim) + 0x60);
    auto advance = [&](nalComponentBase *component, unsigned slot) {
        auto **table = reinterpret_cast<void **>(component->m_vtbl);
        reinterpret_cast<void(__fastcall *)(nalComponentBase *, void *, cursor *)>(table[slot])(
            component, nullptr, &stream);
    };
    auto *skeleton = anim->field_30;
    for (int group = 0; group < skeleton->field_88; ++group) {
        auto &info = skeleton->field_8C[group];
        advance(info.field_20, 19);
        advance(info.field_20, 20);
        for (int i = 0; i < info.field_28; ++i) {
            const unsigned bit = info.field_24 + i;
            if (!(bits[bit / 32] & (1u << (bit & 31))))
                continue;
            advance(info.field_20, 21);
            if (reinterpret_cast<const void *>(handle.field_4) == &info && handle.field_8 == i)
                return stream.location;
            advance(info.field_20, 22);
        }
    }
    return nullptr;
}

void fire_generic_signals(generic_anim_controller *controller, uint32_t &next_signal,
                          nalGeneric::nalGenericAnim *anim, nalGeneric::nalGenericPose *pose)
{
    if (!controller->field_A4.Skeleton)
        return;
    const unsigned count = pose_component<unsigned char>(pose, controller->field_A4);
    if (!count)
        return;
    nalGeneric::nalGenericComponentHandle<unsigned char> handle{};
    tlFixedString bone{"fakeroot"};
    tlFixedString component{"USMEvent"};
    find_component(anim->field_30, handle, bone, component);
    if (!handle.Skeleton)
        return;
    const char *data = event_data(anim, handle);
    if (!data)
        return;
    const unsigned total = *reinterpret_cast<const uint16_t *>(data + 2);
    const char *signal = total ? data + 4 : nullptr;
    auto advance = [&] {
        signal = static_cast<unsigned char>(signal[2]) == total - 1
            ? nullptr : signal + 12 + 4 * static_cast<unsigned char>(signal[3]);
    };
    for (unsigned i = 0; i < next_signal; ++i)
        advance();
    if (!signal)
        signal = total ? data + 4 : nullptr;
    for (unsigned i = 0; i < count; ++i) {
        const unsigned arguments = static_cast<unsigned char>(signal[3]);
        string_hash name{static_cast<int>(*reinterpret_cast<const uint32_t *>(signal + 4))};
        string_hash bone_name{static_cast<int>(*reinterpret_cast<const uint32_t *>(signal + 8))};
        anim_event event{name, bone_name, static_cast<int>(arguments)};
        for (unsigned argument = 0; argument < arguments; ++argument)
            event.field_10[argument] = string_hash{static_cast<int>(*reinterpret_cast<const uint32_t *>(signal + 12 + 4 * argument))};
        event_manager::raise_event(&event, controller->field_4->my_handle.field_0);
        ++next_signal;
        advance();
        if (!signal)
            signal = total ? data + 4 : nullptr;
    }
    next_signal %= total;
}

bool __fastcall generic_base_signals(generic_anim_controller::gen_base_play_method *self, void *,
                                     player::nalAnimState *state)
{
    return state->field_0->field_10 == self->field_4->get_base_layer_anim_ptr();
}

bool __fastcall generic_layer_signals(generic_anim_controller::gen_mod_play_method *, void *,
                                      player::nalAnimState *state)
{
    return !state->field_40 || !std::equal_to<float>{}(state->field_40->field_48, state->field_48);
}

void __fastcall reference_generic(void *, void *, player::nalAnimState *state)
{
    state->field_28 = 0;
}

void __fastcall compose_generic(generic_anim_controller::gen_base_play_method *self, void *,
                                player::nalAnimState *state, nalAnyPose &pose, nalAnyPose &temporary,
                                const nalAnyPose &default_pose)
{
    state->field_0->VirtualGetPose(state->field_18, state->field_1C, *temporary.field_0, *default_pose.field_0);
    sub_826190(*pose.field_0, state->field_20.field_0, *pose.field_0, *temporary.field_0);
    auto *destination = reinterpret_cast<nalGeneric::nalGenericPose *>(pose.field_0);
    auto *source = reinterpret_cast<nalGeneric::nalGenericPose *>(temporary.field_0);
    auto *controller = self->field_4;
    if (controller->field_94.Skeleton)
        pose_component<nalPositionOrientation>(destination, controller->field_94)
            = pose_component<nalPositionOrientation>(source, controller->field_94);
    if (controller->field_84.Skeleton)
        pose_component<float>(destination, controller->field_84) = pose_component<float>(source, controller->field_84);
    if (controller->field_A4.Skeleton) {
        auto **table = reinterpret_cast<void **>(self->m_vtbl);
        auto should_fire = reinterpret_cast<bool(__fastcall *)(void *, void *, player::nalAnimState *)>(table[5]);
        if (should_fire(self, nullptr, state))
            fire_generic_signals(controller, state->field_28,
                reinterpret_cast<nalGeneric::nalGenericAnim *>(state->field_0->field_10), destination);
    }
}

void *__fastcall destroy_generic(generic_anim_controller *self, void *, unsigned int flags)
{
    self->~generic_anim_controller();
    if (flags & 1)
        nal_anim_controller::operator delete(self, sizeof(generic_anim_controller));
    return self;
}

void __fastcall play_generic_layer(generic_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim,
                                   Float priority, Float blend, uint32_t domains, bool ordered, bool flag, void *parameter)
{
    self->my_player.PlayModifier(anim, static_cast<player::usm_anim_player_modifier_type>(1), blend, domains,
        ordered, priority, 0.0f, reinterpret_cast<player::nalPlayMethod *>(&self->field_5C),
        0.0f, nullptr, 1.0f, flag, parameter);
}

void __fastcall play_generic_base(generic_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim,
                                  void *token, Float blend, player::usm_anim_player_modifier_type type,
                                  bool flag, void *parameter)
{
    self->my_player.PlayModifier(anim, type, blend, reinterpret_cast<player::nalPlayMethod *>(&self->field_54),
                                0.0f, 0, 1.0f, token, flag, parameter);
}

double __fastcall generic_floor(generic_anim_controller *self, void *)
{
    return self->field_84.Skeleton ? pose_component<float>(self->GetPose(), self->field_84) : 1.0;
}

double __fastcall generic_fov(generic_anim_controller *self, void *)
{
    return self->field_B4.Skeleton ? pose_component(self->GetPose(), self->field_B4) : 0.0;
}

double __fastcall generic_far_clip(generic_anim_controller *self, void *)
{
    return self->field_C4.Skeleton ? pose_component(self->GetPose(), self->field_C4) * 0.01f : 10000.0;
}

double __fastcall generic_width(generic_anim_controller *self, void *, string_hash)
{
    return self->field_D4.Skeleton ? pose_component(self->GetPose(), self->field_D4) : 0.0;
}

double __fastcall generic_activity(generic_anim_controller *self, void *, string_hash)
{
    return self->field_E4.Skeleton ? pose_component(self->GetPose(), self->field_E4) : 0.0;
}

double __fastcall generic_pull(generic_anim_controller *self, void *, string_hash)
{
    return self->field_F4.Skeleton ? pose_component(self->GetPose(), self->field_F4) : 0.0;
}

po *generic_bone_po(generic_anim_controller *self, po *out,
                   const nalGeneric::nalGenericConstComponentHandle<nalPositionOrientation> &handle)
{
    if (handle.Skeleton) {
        const auto &value = pose_component(self->GetPose(), handle);
        quaternion rotation{};
        rotation[0] = value.field_0[3];
        rotation[1] = value.field_0[0];
        rotation[2] = value.field_0[1];
        rotation[3] = value.field_0[2];
        *out = po{vector3d{value.field_10[0], value.field_10[1], value.field_10[2]}, rotation, 1.0f};
    } else {
        *out = po_identity_matrix;
    }
    return out;
}

po *__fastcall generic_camera_po(generic_anim_controller *self, void *, po *out)
{
    return generic_bone_po(self, out, self->field_64);
}

po *__fastcall generic_shake_po(generic_anim_controller *self, void *, po *out)
{
    return generic_bone_po(self, out, self->field_74);
}

bool __fastcall generic_has_scale(generic_anim_controller *self, void *, string_hash bone)
{
    return self->will_have_hint_token_scale(bone);
}

vector3d *__fastcall generic_scale(generic_anim_controller *self, void *, vector3d *out, string_hash bone)
{
    *out = self->get_hint_token_scale(bone);
    return out;
}

void __fastcall generic_scene_pose(generic_anim_controller *self, void *, uint32_t &next,
                                   nalAnimClass<nalAnyPose> *anim, nalAnyPose &pose)
{
    fire_generic_signals(self, next, reinterpret_cast<nalGeneric::nalGenericAnim *>(anim),
                         reinterpret_cast<nalGeneric::nalGenericPose *>(pose.field_0));
}
}

void *generic_anim_controller::gen_base_play_method::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 6> result{};
        std::copy_n(static_cast<void **>(nal_anim_controller::std_play_method::native_vtable()), 5, result.data());
        result[1] = reinterpret_cast<void *>(&compose_generic);
        result[3] = reinterpret_cast<void *>(&reference_generic);
        result[5] = reinterpret_cast<void *>(&generic_base_signals);
        return result;
    }();
    return const_cast<void **>(table.data());
}

void *generic_anim_controller::gen_mod_play_method::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 6> result{};
        std::copy_n(static_cast<void **>(gen_base_play_method::native_vtable()), result.size(), result.data());
        result[5] = reinterpret_cast<void *>(&generic_layer_signals);
        return result;
    }();
    return const_cast<void **>(table.data());
}

void *generic_anim_controller::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 42> result{};
        std::copy_n(static_cast<void **>(nal_anim_controller::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&destroy_generic);
        result[1] = reinterpret_cast<void *>(&play_generic_layer);
        result[2] = reinterpret_cast<void *>(&play_generic_base);
        result[30] = reinterpret_cast<void *>(&generic_floor);
        result[31] = reinterpret_cast<void *>(&generic_fov);
        result[32] = reinterpret_cast<void *>(&generic_far_clip);
        result[33] = reinterpret_cast<void *>(&generic_width);
        result[34] = reinterpret_cast<void *>(&generic_activity);
        result[35] = reinterpret_cast<void *>(&generic_pull);
        result[36] = reinterpret_cast<void *>(&generic_camera_po);
        result[37] = reinterpret_cast<void *>(&generic_shake_po);
        result[38] = reinterpret_cast<void *>(&generic_has_scale);
        result[39] = reinterpret_cast<void *>(&generic_scale);
        result[40] = reinterpret_cast<void *>(&generic_scene_pose);
        return result;
    }();
    return const_cast<void **>(table.data());
}
#endif


generic_anim_controller::generic_anim_controller(actor *a2, nalBaseSkeleton *a3, unsigned int a4,
                                                 als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, a3, a4, a5), field_54(this), field_5C(this)
{
    TRACE("generic_anim_controller::generic_anim_controller");

#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x00880EE8;
#endif
    {

        this->field_64 = {};
        this->field_74 = {};
        this->field_84 = {};
        this->field_94 = {};
        this->field_A4 = {};
        this->field_B4 = {};
        this->field_C4 = {};
        this->field_D4 = {};
        this->field_E4 = {};
        this->field_F4 = {};

        tlFixedString v7{"AE_Base_Bone"};
        tlFixedString v8{"camera_root"};

        auto *Skel = bit_cast<nalGeneric::nalGenericSkeleton *>(this->field_40.field_0->field_0);
#if STANDALONE_SYSTEM
        auto lookup = [Skel](auto &handle, tlFixedString &bone, tlFixedString &component) {
            find_component(Skel, handle, bone, component);
        };
#else
        auto lookup = [Skel](auto &handle, tlFixedString &bone, tlFixedString &component) {
            Skel->GetComponentHandle(handle, bone, component);
        };
#endif
        lookup(this->field_64, v8, v7);

        v8 = {"AE_Base_Bone"};
        v7 = {"shake_root"};
        lookup(this->field_74, v7, v8);

        v8 = {"AE_Floor_Offset"};
        v7 = {"fakeroot"};
        lookup(this->field_84, v7, v8);

        v8 = {"NAL_TRAJECTORY"};
        v7 = {"fakeroot"};
        lookup(this->field_94, v7, v8);

        v8 = {"USMEvent"};
        v7 = {"fakeroot"};
        lookup(this->field_A4, v7, v8);

        v8 = {"MaxParamFloat.FOV"};
        v7 = {"camera"};
        lookup(this->field_B4, v7, v8);

        v8 = {"MaxParamFloat.Far Env Range"};
        v7 = {"camera"};
        lookup(this->field_C4, v7, v8);

        v8 = {"MaxParamFloat.Tentacle Base Diameter"};
        v7 = {"fakeroot"};
        lookup(this->field_D4, v7, v8);

        v8 = {"MaxParamFloat.Subtentacle Activity"};
        v7 = {"fakeroot"};
        lookup(this->field_E4, v7, v8);

        v8 = {"MaxParamFloat.Pull Factor"};
        v7 = {"fakeroot"};
        lookup(this->field_F4, v7, v8);
    }
}

nalGeneric::nalGenericPose *generic_anim_controller::GetPose()
{
    return bit_cast<nalGeneric::nalGenericPose *>(this->field_40.field_0);
}

tlFixedString tlfs_AE_SCALE{"AE_SCALE"};

bool generic_anim_controller::will_have_hint_token_scale(string_hash a2)
{
    auto *pose = this->GetPose();
    auto *v2 = pose->GetSkeleton();
    nalGeneric::nalGenericConstComponentHandle<nalVector3> v4{};
#if STANDALONE_SYSTEM
    find_component(v2, v4, a2.source_hash_code, nullptr, tlfs_AE_SCALE, component_type<nalVector3>(), true);
#else
    v2->GetComponentHandle(v4, a2.source_hash_code, tlfs_AE_SCALE);
#endif
    return v4.Skeleton != nullptr;
}

tlFixedString tlfs_NAL_SCALE{"NAL_SCALE"};

vector3d generic_anim_controller::get_hint_token_scale(string_hash a2)
{
    TRACE("generic_anim_controller::get_hint_token_scale");

#if STANDALONE_SYSTEM
    {
        auto *v3 = this->GetPose();
        nalGeneric::nalGenericSkeleton *v4 = v3->GetSkeleton();

        nalGeneric::nalGenericConstComponentHandle<nalVector3> v10{};
        find_component(v4, v10, a2.source_hash_code, nullptr, tlfs_NAL_SCALE, component_type<nalVector3>(), true);
        if (v10.Skeleton != nullptr) {
            const auto &v5 = pose_component(v3, v10);
            return vector3d{v5[0], v5[1], v5[2]};
        }

        return vector3d{1.f, 1.f, 1.f};
    }
#else
    {
        vector3d result;
        THISCALL(0x0049C7F0, this, &result, a2);
        return result;
    }
#endif
}

void generic_anim_controller_patch()
{
    FUNC_ADDRESS(address, &generic_anim_controller::get_hint_token_scale);
    set_vfunc(0x00880F84, address);
}
