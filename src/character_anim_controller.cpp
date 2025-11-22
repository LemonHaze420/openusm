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
    TRACE("character_anim_controller::character_anim_controller");

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

void * character_anim_controller::operator new(size_t sz)
{
    return mem_alloc(sz);
}

void * character_anim_controller::operator new(size_t , void *ptr)
{
    return ptr;
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

float character_anim_controller::get_tentacle_width(string_hash a2)
{
    auto *v2 = this->field_40.field_0;
    auto *v3 = ( v2 != nullptr ? ((nalChar::nalCharPose *)&v2[-1]) : nullptr );

    auto *NamedPoseData = bit_cast<TentaclesPoseDesc::StdPoseData *>(v3->GetNamedPoseData(static_cast<CharComponentBase::Names>(7)));
    if ( NamedPoseData != nullptr ) {
        return NamedPoseData->GetDiameterFromBone(
                a2.source_hash_code);
    } else {
        return 0.0f;
    }
}

float character_anim_controller::get_tentacle_pull_factor(string_hash a2)
{
    auto v2 = this->field_40.field_0;
    nalChar::nalCharPose *v3 = (v2 != nullptr
            ? bit_cast<nalChar::nalCharPose *>(v2 - 1)
            : nullptr);

    if (auto *NamedPoseData = static_cast<TentaclesPoseDesc::StdPoseData *>(v3->GetNamedPoseData(static_cast<CharComponentBase::Names>(7))); NamedPoseData != nullptr ) {
        return NamedPoseData->GetPullFromBone(a2.source_hash_code);
    } else {
        return 0.0f;
    }
}

void get_po_from_bone_data(
        po &a1,
        const ArbitraryPOCharComp::BoneData *a2,
        const ArbitraryPOCharComp::PerSkelData *a3,
        const ArbitraryPOCharComp::StdPoseData *a4)
{
    vector3d *v4 = bit_cast<vector3d *>(a4->field_10[4 * a4->field_0]);
    float v12[4] {};
    if ( a2->field_28 )
    {
        const float *v5 = a4->field_10[4 * a2->field_20];
        v12[0] = v5[0];
        v12[1] = v5[1];
        v12[2] = v5[2];
        v12[3] = v5[3];
    }
    else
    {
        const float *v7 = &a3->field_10[4 * a2->field_20];
        v12[0] = v7[0];
        v12[1] = v7[1];
        v12[2] = v7[2];
        v12[3] = v7[3];
    }

    auto v9 = a2->field_2A == 0;
    vector3d *v10 = (v9
            ? &a3->field_14[a2->field_22]
            : &v4[a2->field_22]);

    vector3d a2a = *v10;

    quaternion a3a {};
    a3a[0] = v12[3];
    a3a[1] = v12[0];
    a3a[2] = v12[1];
    a3a[3] = v12[2];

    a1 = po {a2a, a3a, 1.0f};
}

void character_anim_controller::get_camera_root_rel_po(po &a2)
{
    auto v2 = bit_cast<ArbitraryPOCharComp::BoneData *>(this->field_64);
    if ( v2 != nullptr )
    {
        auto *v3 = bit_cast<ArbitraryPOCharComp::PerSkelData *>(this->field_6C);
        auto *v4 = this->field_40.field_0;
        nalChar::nalCharPose *v5 = (v4 != nullptr ? bit_cast<nalChar::nalCharPose *>(v4 - 1) : nullptr);

        auto *NamedPoseData = static_cast<ArbitraryPOCharComp::StdPoseData *>(v5->GetNamedPoseData(static_cast<CharComponentBase::Names>(1)));
        ::get_po_from_bone_data(a2, v2, v3, NamedPoseData);

        auto position = a2.get_position();
        if ( position.length() > 50000.0 )
        {
            auto v3 = a2.get_position();
            auto v8 = v3[2];
            auto v4 = a2.get_position();
            auto y = v4[1];
            auto v5 = a2.get_position();
            auto v6 = v5[0];
            error("Bad PO: pos = (%f, %f, %f)", v6, y, v8);
        }
    }
    else
    {
        this->get_shake_root_rel_po(a2);
    }
}

void character_anim_controller::get_shake_root_rel_po(po &a2)
{
    auto *v2 = bit_cast<const ArbitraryPOCharComp::BoneData *>(this->field_68);
    if ( v2 != nullptr )
    {
        auto *v3 = bit_cast<const ArbitraryPOCharComp::PerSkelData *>(this->field_6C);
        auto *v4 = this->field_40.field_0;
        nalChar::nalCharPose *v5 = (v4 != nullptr ? bit_cast<nalChar::nalCharPose *>(&v4[-1]) : nullptr);

        auto *NamedPoseData = static_cast<ArbitraryPOCharComp::StdPoseData *>(v5->GetNamedPoseData(static_cast<CharComponentBase::Names>(1)));
        get_po_from_bone_data(a2, v2, v3, NamedPoseData);
    }
    else
    {
        this->get_shake_root_rel_po(a2);
    }
}

bool character_anim_controller::will_have_hint_token_scale(string_hash )
{
    return false;
}

vector3d character_anim_controller::get_hint_token_scale(
        string_hash )
{
    return vector3d {1.f, 1.f, 1.f};
}

void character_anim_controller::post_get_pose_in_scene_anims(
        uint32_t &,
        nalAnimClass<nalAnyPose> *a3,
        nalAnyPose &a4)
{
    auto *PerAnimDataByName = bit_cast<const FakerootPoseDesc::PerAnimData *>(bit_cast<nalChar::nalCharAnim *>(a3)->GetPerAnimDataByName(
            static_cast<CharComponentBase::Names>(6)));
    nalChar::nalCharPose *v6 = (a4.field_0 != nullptr
            ? bit_cast<nalChar::nalCharPose *>(a4.field_0 - 1)
            : nullptr);

    auto *NamedPoseData = bit_cast<const FakerootPoseDesc::StdPoseData *>(v6->GetNamedPoseData(
            static_cast<CharComponentBase::Names>(6)));
    if ( NamedPoseData != nullptr )
        fire_signals(
                PerAnimDataByName,
                *NamedPoseData,
                (vhandle_type<actor>)this->field_4->my_handle.field_0);
}

void character_anim_controller::gen_std_play_method::Compose(
        usm_anim_player<nalAnimClass<nalAnyPose>,3>::nalAnimState *a2,
        nalAnyPose &a3,
        nalAnyPose &a4,
        const nalAnyPose &a5)
{
    TRACE("character_anim_controller::gen_std_play_method::Compose");

    if constexpr (0)
    {
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
    }
    else
    {
        THISCALL(0x0049EC30, this, a2, &a3, &a4, &a5);
    }
}

void character_anim_controller::gen_std_play_method::Reference(
        usm_anim_player<nalAnimClass<nalAnyPose>,3>::nalAnimState *a1)
{
    a1->field_28 = 0;
}

bool character_anim_controller::gen_std_play_method::ShouldFireSignals(
        usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1)
{
    bool (__fastcall *func)(void *, void *edx, usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *) = CAST(func, get_vfunc(m_vtbl, 0x14));
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


void * __fastcall character_anim_controller__ctor(
        void *self,
        void *,
        actor *a2,
        nalBaseSkeleton *new_skel,
        unsigned int a4,
        const als::als_meta_anim_table_shared *a5)
{
    auto *result = new (self) character_anim_controller {a2, new_skel, a4, a5};
    return result;
}

void character_anim_controller_patch()
{
    REDIRECT(0x004CC50D, &character_anim_controller__ctor);

    {
        FUNC_ADDRESS(address, &character_anim_controller::play_base_layer_anim);
        SET_JUMP(0x004A6220, address);
    }

    {
        FUNC_ADDRESS(address, &character_anim_controller::gen_std_play_method::Compose);
        set_vfunc(0x00880B74, address);
        set_vfunc(0x00880B8C, address);
        set_vfunc(0x00880BA4, address);
    }

    {
        FUNC_ADDRESS(address, &character_anim_controller::gen_base_play_method::ShouldFireSignals);
        set_vfunc(0x00880B9C, address);
    }
}
