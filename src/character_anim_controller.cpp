#include "character_anim_controller.h"

#include "actor.h"
#include "anim_event.h"
#include "charcomponentbase.h"
#include "character_pose_skel.h"
#include "character_anim_inst.h"
#include "common.h"
#include "event_manager.h"
#include "fakerootposedesc.h"
#include "func_wrapper.h"
#include "nal_instance.h"
#include "nal_skeleton.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(character_anim_controller, 0x70);

static constexpr auto CHARACTER_ANIMTYPE_NAME = "Character";

void fire_the_signal(const FakerootPoseDesc::PerAnimData::EventIterator &a1, vhandle_type<actor> a2)
{
    int NumArguments = a1.GetNumArguments();
    auto NameOfBone = a1.GetNameOfBone();
    string_hash v10 {NameOfBone};

    auto NameOfSignal = a1.GetNameOfSignal();
    string_hash v9 {NameOfSignal};
    anim_event event_to_raise {v9, v10, NumArguments};

    for ( uint32_t i = 0; i < a1.GetNumArguments(); ++i )
    {
        auto Argument = a1.GetArgument(i);
        string_hash v12 {Argument};
        event_to_raise.field_10[i] = v12;
    }

    event_manager::raise_event(&event_to_raise, a2.field_0);
}

void fire_signals(
        const FakerootPoseDesc::PerAnimData *a1,
        const FakerootPoseDesc::StdPoseData &a2,
        vhandle_type<actor> a3)
{
    if ( a2.field_24 )
    {
        auto it = a1->GetStartIterator();
        it.PositionToSignalIx(a2.field_20);
        for ( int i = 0; i < a2.field_24; ++i )
        {
            fire_the_signal(it, a3);
            ++it;
        }
    }
}

character_anim_controller::character_anim_controller(
        actor *a2,
        nalBaseSkeleton *new_skel,
        unsigned int a4,
        const als::als_meta_anim_table_shared *a5)
        : nal_anim_controller(a2, new_skel, a4, a5),
            field_54(this),
            field_5C(this),
            field_64(nullptr),
            field_68(nullptr),
            field_6C(nullptr)
{
    this->m_vtbl = 0x00880F90;

    assert(new_skel->GetAnimTypeName() == tlFixedString(CHARACTER_ANIMTYPE_NAME));

    struct {
        uint32_t field_0;
        int field_4;
        int field_8;
        int field_C;
        int field_10;
        int field_14;
        char *field_18;
    } *NamedPerSkelData = CAST(NamedPerSkelData, bit_cast<nalChar::nalCharSkeleton *>(new_skel)->GetNamedPerSkelData(static_cast<CharComponentBase::Names>(1)));

    if ( NamedPerSkelData != nullptr )
    {
        this->field_6C = new_skel;
        tlFixedString v14 {"shake_root"};
        tlFixedString v13 {"camera_root"};
        
        for ( uint32_t a5a = 0; a5a < NamedPerSkelData->field_0; ++a5a )
        {
            auto *v9 = bit_cast<tlFixedString *>(&NamedPerSkelData->field_18[0x30 * a5a]);
            if ( *v9 == v13 ) {
                this->field_64 = v9;
            }

            if ( *v9 == v14 ) {
                this->field_68 = v9;
            }
        }
    }
}

void character_anim_controller::play_base_layer_anim(
        nalAnimClass<nalAnyPose> *anim_ptr,
        Float a3,
        Float a4,
        bool a5,
        bool a6,
        void *a7)
{
    TRACE("character_anim_controller::play_base_layer_anim");

    assert(anim_ptr->GetSkeleton()->GetAnimTypeName() == tlFixedString(CHARACTER_ANIMTYPE_NAME));

    this->my_player.PlayModifier(
        anim_ptr,
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

void character_anim_controller::gen_std_play_method::Compose(
        usm_anim_player<nalAnimClass<nalAnyPose>,3>::nalAnimState *a2,
        nalAnyPose &a3,
        nalAnyPose &a4,
        const nalAnyPose &a5)
{
    TRACE("character_anim_controller::gen_std_play_method::Compose");

    if constexpr (0) {
        a2->field_0->VirtualGetPose(a2->field_18, a2->field_1C, a4.field_0, a5.field_0);
        a3.field_0->field_0->VirtualBlend(
                a3.field_0,
                a2->field_20.field_0,
                a3.field_0,
                a4.field_0);

        nalChar::nalCharPose *v6 = nullptr;
        if ( a3.field_0 != nullptr ) {
            v6 = (nalChar::nalCharPose *)&a3.field_0[-1];
        }

        nalChar::nalCharPose *v7 = nullptr;
        if ( a4.field_0 != nullptr ) {
            v7 = (nalChar::nalCharPose *)&a4.field_0[-1];
        }

        auto *NamedPoseData = (FakerootPoseDesc::StdPoseData *) v6->GetNamedPoseData(
                static_cast<CharComponentBase::Names>(6));
        auto *v9 = (FakerootPoseDesc::StdPoseData *) v7->GetNamedPoseData(static_cast<CharComponentBase::Names>(6));
        NamedPoseData->field_0[0] = v9->field_0[0];
        NamedPoseData->field_0[1] = v9->field_0[1];
        NamedPoseData->field_0[2] = v9->field_0[2];
        NamedPoseData->field_0[3] = v9->field_0[3];
        NamedPoseData->field_10[0] = v9->field_10[0];
        NamedPoseData->field_10[1] = v9->field_10[1];
        NamedPoseData->field_10[2] = v9->field_10[2];
        NamedPoseData->field_1C = v9->field_1C;

        if ( this->ShouldFireSignals(a2) )
        {
            auto *PerAnimDataByName = (const FakerootPoseDesc::PerAnimData *) bit_cast<nalChar::nalCharAnim *>(a2->field_0->field_10)->GetPerAnimDataByName(
                    static_cast<CharComponentBase::Names>(6));
            fire_signals(PerAnimDataByName, *NamedPoseData, {this->field_4->field_4->my_handle.field_0});
        }

    } else {
        THISCALL(0x0049EC30, this, a2, &a3, &a4, &a5);
    }
}

void character_anim_controller::gen_std_play_method::Reference(
        usm_anim_player<nalAnimClass<nalAnyPose>,3>::nalAnimState *a1)
{
    a1->field_28 = 0;
}

void character_anim_controller_patch()
{
    FUNC_ADDRESS(address, &character_anim_controller::play_base_layer_anim);
    SET_JUMP(0x004A6220, address);
}
