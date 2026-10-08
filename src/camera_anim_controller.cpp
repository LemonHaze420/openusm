#include "camera_anim_controller.h"

#include "common.h"

#include "nal_skeleton.h"
#include "nal_instance.h"

#include <array>
#include <algorithm>
#include <new>

#if STANDALONE_SYSTEM
namespace {
using player = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

void __fastcall compose_camera(player::nalPlayMethod *, void *, player::nalAnimState *state, nalAnyPose &pose,
                               nalAnyPose &, const nalAnyPose &default_pose)
{
    state->field_0->VirtualGetPose(state->field_18, state->field_1C, *pose.field_0, *default_pose.field_0);
}

void __fastcall reference_camera(player::nalPlayMethod *, void *, player::nalAnimState *state)
{
    state->field_28 = 0;
}

void *camera_play_method()
{
    static const auto table = [] {
        std::array<void *, 5> result{};
        std::copy_n(
            static_cast<void **>(nal_anim_controller::std_play_method::native_vtable()), result.size(), result.data());
        result[1] = reinterpret_cast<void *>(&compose_camera);
        result[3] = reinterpret_cast<void *>(&reference_camera);
        return result;
    }();
    return const_cast<void **>(table.data());
}

void *__fastcall destroy_camera(camera_anim_controller *self, void *, unsigned int flags)
{
    self->~camera_anim_controller();
    if (flags & 1)
        ::operator delete(self);
    return self;
}

void __fastcall play_camera_layer(camera_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim, Float priority,
                                  Float blend, uint32_t domains, bool ordered, bool flag, void *parameter)
{
    self->my_player.PlayModifier(anim,
                                 static_cast<player::usm_anim_player_modifier_type>(1),
                                 blend,
                                 domains,
                                 ordered,
                                 priority,
                                 0.0f,
                                 &self->field_5C,
                                 0.0f,
                                 nullptr,
                                 1.0f,
                                 flag,
                                 parameter);
}

void __fastcall play_camera_base(camera_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim, void *token,
                                 Float blend, player::usm_anim_player_modifier_type type, bool flag, void *parameter)
{
    self->my_player.PlayModifier(anim, type, blend, &self->field_54, 0.0f, 0, 1.0f, token, flag, parameter);
}

double __fastcall camera_fov(camera_anim_controller *self, void *)
{
    return *reinterpret_cast<float *>(reinterpret_cast<char *>(self->field_40.field_0) + 4);
}

double __fastcall camera_far_clip(camera_anim_controller *self, void *)
{
    return *reinterpret_cast<float *>(reinterpret_cast<char *>(self->field_40.field_0) + 8);
}


void __fastcall camera_scene_pose(camera_anim_controller *, void *, uint32_t &, nalAnimClass<nalAnyPose> *,
                                  nalAnyPose &)
{}
}

void *camera_anim_controller::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 42> result{};
        std::copy_n(static_cast<void **>(nal_anim_controller::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&destroy_camera);
        result[1] = reinterpret_cast<void *>(&play_camera_layer);
        result[2] = reinterpret_cast<void *>(&play_camera_base);
        result[31] = reinterpret_cast<void *>(&camera_fov);
        result[32] = reinterpret_cast<void *>(&camera_far_clip);
        result[40] = reinterpret_cast<void *>(&camera_scene_pose);
        return result;
    }();
    return const_cast<void **>(table.data());
}
#endif

VALIDATE_SIZE(camera_anim_controller, 0x64u);

static const char *CAMERA_ANIMTYPE_NAME = "Camera";

camera_anim_controller::camera_anim_controller(actor *a2, nalBaseSkeleton *new_skel, unsigned int a4,
                                               als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, new_skel, a4, a5)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    this->field_54.m_vtbl = reinterpret_cast<int>(camera_play_method());
    this->field_5C.m_vtbl = reinterpret_cast<int>(camera_play_method());
#else
    this->m_vtbl = 0x008810E0;
    this->field_54.m_vtbl = 0x00880C14;
    this->field_5C.m_vtbl = 0x00880C28;
#endif
    this->field_58 = this;
    this->field_60 = this;

    assert(new_skel->GetAnimTypeName() == tlFixedString(CAMERA_ANIMTYPE_NAME));
}
