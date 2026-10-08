#include "ped_anim_controller.h"

#include "common.h"
#include "nal_skeleton.h"
#include "nal_instance.h"
#include "vtbl.h"

#include <array>
#include <algorithm>
#include <new>
#include <functional>

#if STANDALONE_SYSTEM
namespace {
using player = usm_anim_player<nalAnimClass<nalAnyPose>, 3>;

void __fastcall compose_ped(player::nalPlayMethod *, void *, player::nalAnimState *state, nalAnyPose &pose,
                            nalAnyPose &, const nalAnyPose &default_pose)
{
    state->field_0->VirtualGetPose(state->field_18, state->field_1C, *pose.field_0, *default_pose.field_0);
}

bool __fastcall ped_base_signals(void *method, void *, player::nalAnimState *state)
{
    auto *controller = *reinterpret_cast<ped_anim_controller **>(static_cast<char *>(method) + 4);
    return state->field_0->field_10 == controller->get_base_layer_anim_ptr();
}

bool __fastcall ped_layer_signals(void *, void *, player::nalAnimState *state)
{
    return !state->field_40 || !std::equal_to<float>{}(state->field_40->field_48, state->field_48);
}

void __fastcall reference_ped(player::nalPlayMethod *, void *, player::nalAnimState *state)
{
    state->field_28 = 0;
}

void *ped_play_method(bool layer)
{
    static const auto base = [] {
        std::array<void *, 6> table{};
        std::copy_n(static_cast<void **>(nal_anim_controller::std_play_method::native_vtable()), 5, table.data());
        table[1] = reinterpret_cast<void *>(&compose_ped);
        table[3] = reinterpret_cast<void *>(&reference_ped);
        table[5] = reinterpret_cast<void *>(&ped_base_signals);
        return table;
    }();
    static const auto modifier = [] {
        auto table = base;
        table[5] = reinterpret_cast<void *>(&ped_layer_signals);
        return table;
    }();
    return const_cast<void **>((layer ? modifier : base).data());
}

void *__fastcall destroy_ped(ped_anim_controller *self, void *, unsigned int flags)
{
    self->~ped_anim_controller();
    if (flags & 1)
        ::operator delete(self);
    return self;
}

void __fastcall play_ped_layer(ped_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim, Float priority,
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

void __fastcall play_ped_base(ped_anim_controller *self, void *, nalAnimClass<nalAnyPose> *anim, void *token,
                              Float blend, player::usm_anim_player_modifier_type type, bool flag, void *parameter)
{
    self->my_player.PlayModifier(anim,
                                 type,
                                 blend,
                                 reinterpret_cast<player::nalPlayMethod *>(&self->field_54),
                                 0.0f,
                                 0,
                                 1.0f,
                                 token,
                                 flag,
                                 parameter);
}

double __fastcall ped_floor(ped_anim_controller *self, void *)
{
    return *reinterpret_cast<float *>(reinterpret_cast<char *>(self->field_40.field_0) + 0xBC);
}


void __fastcall ped_scene_pose(ped_anim_controller *, void *, uint32_t &, nalAnimClass<nalAnyPose> *, nalAnyPose &) {}
}

void *ped_anim_controller::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 42> result{};
        std::copy_n(static_cast<void **>(nal_anim_controller::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&destroy_ped);
        result[1] = reinterpret_cast<void *>(&play_ped_layer);
        result[2] = reinterpret_cast<void *>(&play_ped_base);
        result[30] = reinterpret_cast<void *>(&ped_floor);
        result[40] = reinterpret_cast<void *>(&ped_scene_pose);
        return result;
    }();
    return const_cast<void **>(table.data());
}
#endif

VALIDATE_SIZE(ped_anim_controller, 0x64u);

static const char *PED_ANIMTYPE_NAME = "Ped";

ped_anim_controller::ped_anim_controller(actor *a2, nalBaseSkeleton *new_skel, uint32_t a4,
                                         als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, new_skel, a4, a5)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    this->field_54 = reinterpret_cast<int>(ped_play_method(false));
    this->field_5C.m_vtbl = reinterpret_cast<int>(ped_play_method(true));
#else
    this->m_vtbl = 0x00881038;
    this->field_54 = 0x00880BD0;
    this->field_5C.m_vtbl = 0x00880BE8;
#endif
    this->field_58 = this;
    this->field_60 = this;

    assert(new_skel->GetAnimTypeName() == tlFixedString(PED_ANIMTYPE_NAME));
}
