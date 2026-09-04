#pragma once

#include "nal_anim_controller.h"

#include "arbitrarypocharcomp.h"
#include "vector3d.h"

struct character_anim_controller : nal_anim_controller {
    struct gen_std_play_method : std_play_method {
        nal_anim_controller *field_4;

        gen_std_play_method(character_anim_controller *a2) : field_4(a2)
        {
            this->m_vtbl = 0x00880B70;
        }

        //virtual
        //0x0049EC30
        void Compose(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a2, nalAnyPose &a3, nalAnyPose &a4,
                     const nalAnyPose &a5);

        //0x00492D10
        void Reference(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1);

        //virtual
        bool ShouldFireSignals(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1);  // = 0;
    };

    struct gen_base_play_method : gen_std_play_method {
        gen_base_play_method(character_anim_controller *a2) : gen_std_play_method(a2)
        {
            this->m_vtbl = 0x00880B88;
        }

        //virtual
        bool ShouldFireSignals(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1);
    };

    struct gen_mod_play_method : gen_std_play_method {
        gen_mod_play_method(character_anim_controller *a2) : gen_std_play_method(a2)
        {
            this->m_vtbl = 0x00880BA0;
        }

        //virtual
        bool ShouldFireSignals(usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalAnimState *a1);
    };

    character_anim_controller::gen_base_play_method field_54;
    character_anim_controller::gen_mod_play_method field_5C;
    ArbitraryPOCharComp::BoneData *field_64;
    ArbitraryPOCharComp::BoneData *field_68;
    nalBaseSkeleton *field_6C;

    //0x0049CA30
    character_anim_controller(actor *a2, nalBaseSkeleton *new_skel, unsigned int a4,
                              const als::als_meta_anim_table_shared *a5);


    void *operator new(size_t sz);

    void *operator new(size_t size, void *);

    //virtual
    //0x004A6220
    void _play_base_layer_anim(nalAnimClass<nalAnyPose> *a2, Float a3, Float a4, bool a5, bool a6, void *a7);

    //virtual
    //0x00492C20
    float get_tentacle_width(string_hash a2);


    //virtual
    //0x00492CA0
    float get_tentacle_pull_factor(string_hash a2);


    //virtual
    //0x004982D0
    void get_camera_root_rel_po(po &a2);

    //virtual
    //0x00498310
    void get_shake_root_rel_po(po &a2);

    //virtual
    //0x00492CE0
    bool will_have_hint_token_scale(string_hash);

    //virtual
    //0x00492CF0
    vector3d get_hint_token_scale(string_hash a2);


    //virtual
    //0x0049EBE0
    void post_get_pose_in_scene_anims(uint32_t &a2, nalAnimClass<nalAnyPose> *a3, nalAnyPose &a4);
};

inline constexpr auto CHARACTER_ANIMTYPE_NAME = "Character";

extern void character_anim_controller_patch();
