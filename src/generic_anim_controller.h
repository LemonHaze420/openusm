#pragma once

#include "nal_anim_controller.h"

#include <nal_generic.h>
#include <nal_system.h>

#include "vector3d.h"

struct actor;
struct nalBaseSkeleton;

namespace als {
struct als_meta_anim_table_shared;
}

struct generic_anim_controller : nal_anim_controller {
    struct gen_base_play_method {
        std::intptr_t m_vtbl;
        generic_anim_controller *field_4;
        static void *native_vtable();

        gen_base_play_method(generic_anim_controller *a2)
        {
#if STANDALONE_SYSTEM
            this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
            this->m_vtbl = 0x00880B40;
#endif
            this->field_4 = a2;
        }
    };

    struct gen_mod_play_method {
        std::intptr_t m_vtbl;
        generic_anim_controller *field_4;
        static void *native_vtable();

        gen_mod_play_method(generic_anim_controller *a2)
        {
#if STANDALONE_SYSTEM
            this->m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
#else
            this->m_vtbl = 0x00880B58;
#endif
            this->field_4 = a2;
        }
    };

    gen_base_play_method field_54;
    gen_mod_play_method field_5C;
    nalGeneric::nalGenericConstComponentHandle<nalPositionOrientation> field_64;
    nalGeneric::nalGenericConstComponentHandle<nalPositionOrientation> field_74;
    nalGeneric::nalGenericComponentHandle<float> field_84;
    nalGeneric::nalGenericComponentHandle<nalPositionOrientation> field_94;
    nalGeneric::nalGenericComponentHandle<unsigned char> field_A4;
    nalGeneric::nalGenericConstComponentHandle<float> field_B4;
    nalGeneric::nalGenericConstComponentHandle<float> field_C4;
    nalGeneric::nalGenericConstComponentHandle<float> field_D4;
    nalGeneric::nalGenericConstComponentHandle<float> field_E4;
    nalGeneric::nalGenericConstComponentHandle<float> field_F4;

    generic_anim_controller(actor *, nalBaseSkeleton *, unsigned int, als::als_meta_anim_table_shared *);
    static void *native_vtable();

    nalGeneric::nalGenericPose *GetPose();


    //virtual
    //0x0049C7B0
    bool will_have_hint_token_scale(string_hash a2);

    //virtual
    //0x0049C7F0
    vector3d get_hint_token_scale(string_hash a2);
};

extern void generic_anim_controller_patch();
