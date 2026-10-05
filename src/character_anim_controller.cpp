#include "character_anim_controller.h"

#include "actor.h"
#include "anim_event.h"
#include "arbitrarypocharcomp.h"
#include "charcomponentbase.h"
#include "character_pose_skel.h"
#include "character_anim_inst.h"
#include "common.h"
#include "event_manager.h"
#include "fakerootposedesc.h"
#include "func_wrapper.h"
#include "nal_instance.h"
#include "nal_skeleton.h"
#include "oldmath_po.h"
#include "osassert.h"
#include "quaternion.h"
#include "tentaclesposedesc.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cassert>
#include <array>
#include <cstddef>
#include <cstring>

VALIDATE_SIZE(character_anim_controller, 0x70);

void fire_the_signal(const FakerootPoseDesc::PerAnimData::EventIterator &a1, vhandle_type<actor> a2)
{
    int NumArguments = a1.GetNumArguments();
    auto NameOfBone = a1.GetNameOfBone();
    string_hash v10{NameOfBone};

    auto NameOfSignal = a1.GetNameOfSignal();
    string_hash v9{NameOfSignal};
    anim_event event_to_raise{v9, v10, NumArguments};

    for (uint32_t i = 0; i < a1.GetNumArguments(); ++i) {
        auto Argument = a1.GetArgument(i);
        string_hash v12{Argument};
        event_to_raise.field_10[i] = v12;
    }

    event_manager::raise_event(&event_to_raise, a2.field_0);
}

void fire_signals(const FakerootPoseDesc::PerAnimData *a1, const FakerootPoseDesc::StdPoseData &a2,
                  vhandle_type<actor> a3)
{
    if (a2.field_24) {
        auto it = a1->GetStartIterator();
        it.PositionToSignalIx(a2.field_20);
        for (int i = 0; i < a2.field_24; ++i) {
            fire_the_signal(it, a3);
            ++it;
        }
    }
}

character_anim_controller::character_anim_controller(actor *a2, nalBaseSkeleton *new_skel, unsigned int a4,
                                                     const als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, new_skel, a4, a5), field_54(this), field_5C(this), field_64(nullptr), field_68(nullptr),
      field_6C(nullptr)
{
    TRACE("character_anim_controller::character_anim_controller");

#if defined(STANDALONE_SYSTEM)
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
    this->m_vtbl = 0x00880F90;
#endif

    assert(new_skel->GetAnimTypeName() == tlFixedString(CHARACTER_ANIMTYPE_NAME));

    ArbitraryPOCharComp::PerSkelData *NamedPerSkelData = CAST(
        NamedPerSkelData,
        bit_cast<nalChar::nalCharSkeleton *>(new_skel)->GetNamedPerSkelData(CharComponentBase::Names::ArbitraryPO));

    if (NamedPerSkelData != nullptr) {
        this->field_6C = new_skel;
        tlFixedString v14{"shake_root"};
        tlFixedString v13{"camera_root"};

        for (uint32_t i = 0; i < NamedPerSkelData->field_0; ++i) {
            auto *v9 = &NamedPerSkelData->field_18[i];
            if (v9->field_0 == v13) {
                this->field_64 = v9;
            }

            if (v9->field_0 == v14) {
                this->field_68 = v9;
            }
        }
    }
}

void *character_anim_controller::operator new(size_t sz)
{
    return mem_alloc(sz);
}

void *character_anim_controller::operator new(size_t, void *ptr)
{
    return ptr;
}

void character_anim_controller::_play_layer_anim(nalAnimClass<nalAnyPose> *anim, Float blend_time, Float priority,
                                                uint32_t domains, bool force_restart, bool completion_flag,
                                                void *parameter)
{
#if defined(STANDALONE_SYSTEM)
    my_player.PlayModifier(anim, static_cast<decltype(my_player)::usm_anim_player_modifier_type>(1),
                           priority, domains, force_restart, blend_time, 0.0f, &field_5C, 0.0f, nullptr,
                           1.0f, completion_flag, parameter);
#else
    THISCALL(0x0049EBA0, this, anim, blend_time, priority, domains, force_restart, completion_flag, parameter);
#endif
}

float character_anim_controller::get_floor_offset()
{
    auto *base_pose = field_40.field_0;
    auto *pose = base_pose == nullptr ? nullptr : reinterpret_cast<nalChar::nalCharPose *>(
        reinterpret_cast<char *>(base_pose) - offsetof(nalComp::nalCompPose, field_4));
    auto *data = static_cast<FakerootPoseDesc::StdPoseData *>(
        pose->GetNamedPoseData(CharComponentBase::Names::FakerootEntropyCompressed));
    return data == nullptr ? 1.0f : data->field_1C;
}

float character_anim_controller::get_camera_fov()
{
    return 0.0f;
}

float character_anim_controller::get_camera_far_clip()
{

    return 10000.0f;
}

float character_anim_controller::get_tentacle_activity(string_hash bone)
{
    auto *base_pose = field_40.field_0;
    auto *pose = base_pose == nullptr ? nullptr : reinterpret_cast<nalChar::nalCharPose *>(
        reinterpret_cast<char *>(base_pose) - offsetof(nalComp::nalCompPose, field_4));
    auto *data = static_cast<TentaclesPoseDesc::StdPoseData *>(
        pose->GetNamedPoseData(CharComponentBase::Names::TentaclesCompressed));
    return data == nullptr ? 0.0f : data->GetActivityFromBone(bone.source_hash_code);
}

void character_anim_controller::_play_base_layer_anim(nalAnimClass<nalAnyPose> *anim_ptr, Float a3, Float a4, bool a5,
                                                      bool a6, void *a7)
{
    TRACE("character_anim_controller::play_base_layer_anim");

    assert(anim_ptr->GetSkeleton()->GetAnimTypeName() == tlFixedString(CHARACTER_ANIMTYPE_NAME));

    this->my_player.PlayModifier(anim_ptr,
                                 static_cast<decltype(this->my_player)::usm_anim_player_modifier_type>(a5),
                                 a4,
                                 bit_cast<decltype(this->my_player)::nalPlayMethod *>(&this->field_54),
                                 0.0,
                                 0,
                                 1.0,
                                 bit_cast<void *>(a3),
                                 a6,
                                 a7);
}

float character_anim_controller::get_tentacle_width(string_hash a2)
{
    auto *base_pose = this->field_40.field_0;
    auto *v3 = base_pose == nullptr ? nullptr : reinterpret_cast<nalChar::nalCharPose *>(
        reinterpret_cast<char *>(base_pose) - offsetof(nalComp::nalCompPose, field_4));

    auto *NamedPoseData =
        bit_cast<TentaclesPoseDesc::StdPoseData *>(v3->GetNamedPoseData(CharComponentBase::Names::TentaclesCompressed));
    if (NamedPoseData != nullptr) {
        return NamedPoseData->GetDiameterFromBone(a2.source_hash_code);
    } else {
        return 0.0f;
    }
}

float character_anim_controller::get_tentacle_pull_factor(string_hash a2)
{
    auto *base_pose = this->field_40.field_0;
    auto *v3 = base_pose == nullptr ? nullptr : reinterpret_cast<nalChar::nalCharPose *>(
        reinterpret_cast<char *>(base_pose) - offsetof(nalComp::nalCompPose, field_4));

    if (auto *NamedPoseData = static_cast<TentaclesPoseDesc::StdPoseData *>(
            v3->GetNamedPoseData(CharComponentBase::Names::TentaclesCompressed));
        NamedPoseData != nullptr) {
        return NamedPoseData->GetPullFromBone(a2.source_hash_code);
    } else {
        return 0.0f;
    }
}

void get_po_from_bone_data(po &a1, const ArbitraryPOCharComp::BoneData *a2, const ArbitraryPOCharComp::PerSkelData *a3,
                           const ArbitraryPOCharComp::StdPoseData *a4)
{
    auto *v4 = bit_cast<const nalVector3 *>(&a4->field_10[a4->field_0]);
    float v12[4]{};
    if (a2->field_28) {
        const auto &v5 = a4->field_10[a2->field_20];
        v12[0] = v5[0];
        v12[1] = v5[1];
        v12[2] = v5[2];
        v12[3] = v5[3];
    } else {
        const auto &v7 = a3->field_10[a2->field_20];
        v12[0] = v7[0];
        v12[1] = v7[1];
        v12[2] = v7[2];
        v12[3] = v7[3];
    }

    auto v9 = a2->field_2A == 0;
    auto *v10 = bit_cast<const vector3d *>(v9 ? &a3->field_14[a2->field_22] : &v4[a2->field_22]);

    vector3d a2a = *v10;

    quaternion a3a{};
    a3a[0] = v12[3];
    a3a[1] = v12[0];
    a3a[2] = v12[1];
    a3a[3] = v12[2];

    a1 = po{a2a, a3a, 1.0f};
}

void character_anim_controller::get_camera_root_rel_po(po &a2)
{
    auto *v2 = this->field_64;
    if (v2 != nullptr) {
        auto *v3 = bit_cast<ArbitraryPOCharComp::PerSkelData *>(this->field_6C);
        auto *v4 = this->field_40.field_0;
        nalChar::nalCharPose *v5 = (v4 != nullptr ? bit_cast<nalChar::nalCharPose *>(v4 - 1) : nullptr);

        auto *NamedPoseData = static_cast<ArbitraryPOCharComp::StdPoseData *>(
            v5->GetNamedPoseData(CharComponentBase::Names::ArbitraryPO));
        ::get_po_from_bone_data(a2, v2, v3, NamedPoseData);

    } else {
        a2 = po{};
    }
}

void character_anim_controller::get_shake_root_rel_po(po &a2)
{
    auto *v2 = this->field_68;
    if (v2 != nullptr) {
        auto *v3 = bit_cast<const ArbitraryPOCharComp::PerSkelData *>(this->field_6C);
        auto *v4 = this->field_40.field_0;
        nalChar::nalCharPose *v5 = (v4 != nullptr ? bit_cast<nalChar::nalCharPose *>(&v4[-1]) : nullptr);

        auto *NamedPoseData = static_cast<ArbitraryPOCharComp::StdPoseData *>(
            v5->GetNamedPoseData(CharComponentBase::Names::ArbitraryPO));
        get_po_from_bone_data(a2, v2, v3, NamedPoseData);
    } else {
        a2 = po{};
    }
}

bool character_anim_controller::will_have_hint_token_scale(string_hash)
{
    return false;
}

vector3d character_anim_controller::get_hint_token_scale(string_hash)
{
    return vector3d{1.f, 1.f, 1.f};
}

void character_anim_controller::post_get_pose_in_scene_anims(uint32_t &, nalAnimClass<nalAnyPose> *a3, nalAnyPose &a4)
{
    auto *PerAnimDataByName =
        bit_cast<const FakerootPoseDesc::PerAnimData *>(bit_cast<nalChar::nalCharAnim *>(a3)->GetPerAnimDataByName(
            CharComponentBase::Names::FakerootEntropyCompressed));
    nalChar::nalCharPose *v6 = (a4.field_0 != nullptr ? bit_cast<nalChar::nalCharPose *>(a4.field_0 - 1) : nullptr);

    auto *NamedPoseData = bit_cast<const FakerootPoseDesc::StdPoseData *>(
        v6->GetNamedPoseData(CharComponentBase::Names::FakerootEntropyCompressed));
    if (NamedPoseData != nullptr)
        fire_signals(PerAnimDataByName, *NamedPoseData, (vhandle_type<actor>)this->field_4->my_handle.field_0);
}

void character_anim_controller::gen_std_play_method::_Compose(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2, nalAnyPose &a3, nalAnyPose &a4,
    const nalAnyPose &a5)
{
    TRACE("character_anim_controller::gen_std_play_method::Compose");

    {
        a2->field_0->VirtualGetPose(a2->field_18, a2->field_1C, *a4.field_0, *a5.field_0);
        sub_826190(*a3.field_0, a2->field_20.field_0, *a3.field_0, *a4.field_0);

        nalChar::nalCharPose *v6 = nullptr;
        if (a3.field_0 != nullptr) {
            v6 = (nalChar::nalCharPose *)&a3.field_0[-1];
        }

        nalChar::nalCharPose *v7 = nullptr;
        if (a4.field_0 != nullptr) {
            v7 = (nalChar::nalCharPose *)&a4.field_0[-1];
        }

        auto *NamedPoseData =
            (FakerootPoseDesc::StdPoseData *)v6->GetNamedPoseData(CharComponentBase::Names::FakerootEntropyCompressed);
        auto *v9 =
            (FakerootPoseDesc::StdPoseData *)v7->GetNamedPoseData(CharComponentBase::Names::FakerootEntropyCompressed);
        NamedPoseData->field_0[0] = v9->field_0[0];
        NamedPoseData->field_0[1] = v9->field_0[1];
        NamedPoseData->field_0[2] = v9->field_0[2];
        NamedPoseData->field_0[3] = v9->field_0[3];
        NamedPoseData->field_10[0] = v9->field_10[0];
        NamedPoseData->field_10[1] = v9->field_10[1];
        NamedPoseData->field_10[2] = v9->field_10[2];
        NamedPoseData->field_1C = v9->field_1C;

        if (this->ShouldFireSignals(a2)) {
            auto *PerAnimDataByName =
                (const FakerootPoseDesc::PerAnimData *)bit_cast<nalChar::nalCharAnim *>(a2->field_0->field_10)
                    ->GetPerAnimDataByName(CharComponentBase::Names::FakerootEntropyCompressed);
            fire_signals(PerAnimDataByName, *NamedPoseData, {this->field_4->field_4->my_handle.field_0});
        }
    }
}

void character_anim_controller::gen_std_play_method::Reference(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1)
{
    a1->field_28 = 0;
}

bool character_anim_controller::gen_std_play_method::ShouldFireSignals(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1)
{
    bool(__fastcall * func)(void *, void *edx, usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *) =
        CAST(func, get_vfunc(m_vtbl, 0x14));
    return func(this, nullptr, a1);
}

bool character_anim_controller::gen_mod_play_method::ShouldFireSignals(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1)
{
    TRACE("gen_mod_play_method::ShouldFireSignals");

    auto *v1 = a1->field_40;
    return (v1 == nullptr || not_equal(v1->field_48, a1->field_48));
}

bool character_anim_controller::gen_base_play_method::ShouldFireSignals(
    usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2)
{
    TRACE("gen_base_play_method::ShouldFireSignals");

    auto *v2 = a2->field_0->field_10;
    return v2 == this->field_4->get_base_layer_anim_ptr();
}


#if defined(STANDALONE_SYSTEM)
extern "C" int __cdecl _purecall();

namespace {
using character_player = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

void *__fastcall character_destroy(character_anim_controller *self, void *, unsigned int flags)
{
    self->~character_anim_controller();
    if ((flags & 1u) != 0)
        nal_anim_controller::operator delete(self, sizeof(character_anim_controller));
    return self;
}

void __fastcall character_play_layer(character_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim,
                                      Float blend_time, Float priority, uint32_t domains, bool force_restart,
                                      bool completion_flag, void *parameter)
{
    self->_play_layer_anim(anim, blend_time, priority, domains, force_restart, completion_flag, parameter);
}

void __fastcall character_play_base(character_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim,
                                     Float parameter, Float blend_time, bool type, bool flag, void *context)
{
    self->_play_base_layer_anim(anim, parameter, blend_time, type, flag, context);
}

float __fastcall character_floor(character_anim_controller *self, void *)
{
    return self->get_floor_offset();
}

float __fastcall character_fov(character_anim_controller *self, void *)
{
    return self->get_camera_fov();
}

float __fastcall character_far_clip(character_anim_controller *self, void *)
{
    return self->get_camera_far_clip();
}

float __fastcall character_width(character_anim_controller *self, void *, string_hash bone)
{
    return self->get_tentacle_width(bone);
}

float __fastcall character_activity(character_anim_controller *self, void *, string_hash bone)
{
    return self->get_tentacle_activity(bone);
}

float __fastcall character_pull(character_anim_controller *self, void *, string_hash bone)
{
    return self->get_tentacle_pull_factor(bone);
}

void __fastcall character_camera_root(character_anim_controller *self, void *, po &result)
{
    self->get_camera_root_rel_po(result);
}

void __fastcall character_shake_root(character_anim_controller *self, void *, po &result)
{
    self->get_shake_root_rel_po(result);
}

bool __fastcall character_has_scale(character_anim_controller *self, void *, string_hash bone)
{
    return self->will_have_hint_token_scale(bone);
}

vector3d *__fastcall character_scale(character_anim_controller *self, void *, vector3d *result, string_hash bone)
{
    *result = self->get_hint_token_scale(bone);
    return result;
}

void __fastcall character_scene_pose(character_anim_controller *self, void *, uint32_t &state,
                                     nalAnimClass<nalAnyPose> *anim, nalAnyPose &pose)
{
    self->post_get_pose_in_scene_anims(state, anim, pose);
}

void __fastcall character_compose(character_anim_controller::gen_std_play_method *self, void *,
                                  character_player::nalAnimState *state, nalAnyPose &dst,
                                  nalAnyPose &scratch, const nalAnyPose &reference)
{
    self->_Compose(state, dst, scratch, reference);
}

void __fastcall character_reference(character_anim_controller::gen_std_play_method *self, void *,
                                    character_player::nalAnimState *state)
{
    self->Reference(state);
}

bool __fastcall character_abstract_signals(character_anim_controller::gen_std_play_method *, void *,
                                          character_player::nalAnimState *)
{
    return _purecall() != 0;
}

bool __fastcall character_base_signals(character_anim_controller::gen_base_play_method *self, void *,
                                      character_player::nalAnimState *state)
{
    return self->ShouldFireSignals(state);
}

bool __fastcall character_mod_signals(character_anim_controller::gen_mod_play_method *self, void *,
                                     character_player::nalAnimState *state)
{
    return self->ShouldFireSignals(state);
}
}
#endif

void *character_anim_controller::native_vtable()
{
#if defined(STANDALONE_SYSTEM)
    static auto table = [] {
        std::array<std::intptr_t, 42> result{};
        std::memcpy(result.data(), nal_anim_controller::native_vtable(), sizeof(result));
        result[0] = reinterpret_cast<std::intptr_t>(&character_destroy);
        result[1] = reinterpret_cast<std::intptr_t>(&character_play_layer);
        result[2] = reinterpret_cast<std::intptr_t>(&character_play_base);
        result[30] = reinterpret_cast<std::intptr_t>(&character_floor);
        result[31] = reinterpret_cast<std::intptr_t>(&character_fov);
        result[32] = reinterpret_cast<std::intptr_t>(&character_far_clip);
        result[33] = reinterpret_cast<std::intptr_t>(&character_width);
        result[34] = reinterpret_cast<std::intptr_t>(&character_activity);
        result[35] = reinterpret_cast<std::intptr_t>(&character_pull);
        result[36] = reinterpret_cast<std::intptr_t>(&character_camera_root);
        result[37] = reinterpret_cast<std::intptr_t>(&character_shake_root);
        result[38] = reinterpret_cast<std::intptr_t>(&character_has_scale);
        result[39] = reinterpret_cast<std::intptr_t>(&character_scale);
        result[40] = reinterpret_cast<std::intptr_t>(&character_scene_pose);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x00880F90);
#endif
}

void *character_anim_controller::gen_std_play_method::native_vtable()
{
#if defined(STANDALONE_SYSTEM)
    static auto table = [] {
        std::array<std::intptr_t, 6> result{};
        std::memcpy(result.data(), std_play_method::native_vtable(), 5 * sizeof(std::intptr_t));
        result[1] = reinterpret_cast<std::intptr_t>(&character_compose);
        result[3] = reinterpret_cast<std::intptr_t>(&character_reference);
        result[5] = reinterpret_cast<std::intptr_t>(&character_abstract_signals);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x00880B70);
#endif
}

void *character_anim_controller::gen_base_play_method::native_vtable()
{
#if defined(STANDALONE_SYSTEM)
    static auto table = [] {
        std::array<std::intptr_t, 6> result{};
        std::memcpy(result.data(), gen_std_play_method::native_vtable(), sizeof(result));
        result[5] = reinterpret_cast<std::intptr_t>(&character_base_signals);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x00880B88);
#endif
}

void *character_anim_controller::gen_mod_play_method::native_vtable()
{
#if defined(STANDALONE_SYSTEM)
    static auto table = [] {
        std::array<std::intptr_t, 6> result{};
        std::memcpy(result.data(), gen_std_play_method::native_vtable(), sizeof(result));
        result[5] = reinterpret_cast<std::intptr_t>(&character_mod_signals);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x00880BA0);
#endif
}

void *__fastcall character_anim_controller__ctor(void *self, void *, actor *a2, nalBaseSkeleton *new_skel,
                                                 unsigned int a4, const als::als_meta_anim_table_shared *a5)
{
    auto *result = new (self) character_anim_controller{a2, new_skel, a4, a5};
    return result;
}

void character_anim_controller_patch()
{
    REDIRECT(0x004CC50D, &character_anim_controller__ctor);

    {
        FUNC_ADDRESS(address, &character_anim_controller::_play_base_layer_anim);
        set_vfunc(0x00880F98, address);
    }

    {
        FUNC_ADDRESS(address, &character_anim_controller::gen_std_play_method::_Compose);
        set_vfunc(0x00880B74, address);
        set_vfunc(0x00880B8C, address);
        set_vfunc(0x00880BA4, address);
    }

    {
        FUNC_ADDRESS(address, &character_anim_controller::gen_base_play_method::ShouldFireSignals);
        set_vfunc(0x00880B9C, address);
    }
}
