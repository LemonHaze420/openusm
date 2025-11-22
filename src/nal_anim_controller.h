#pragma once

#include "animation_controller.h"

#include "als_meta_anim_table_shared.h"
#include "nal_anim.h"
#include "usm_anim_player.h"

struct actor;
struct nalBaseSkeleton;
struct po;

namespace als {
struct als_meta_anim_table_shared;
}

struct nal_anim_controller : animation_controller {

    struct scene_anim_client {
        int m_vtbl;
        nal_anim_controller *field_4;
        int field_8;

        scene_anim_client(nal_anim_controller *a2);

        //0x00492890
        //virtual
        nalAnimClass<nalAnyPose>::nalInstanceClass *CreateInstance(nalAnimClass<nalAnyPose> *a2);

        //0x0049C090
        //virtual
        int Advance(
            nalAnimClass<nalAnyPose>::nalInstanceClass *a2, Float a3, Float a4, Float a5, Float a6);

        //virtual
        void Render(
            nalAnimClass<nalAnyPose>::nalInstanceClass *a2,
            Float a3);
    };

    struct std_play_method : usm_anim_player<nalAnimClass<nalAnyPose>, 3>::nalPlayMethod {
        void *CreateInstance(
            nalAnimClass<nalAnyPose> *a1,
            nalBaseSkeleton *a2,
            void *pParameter);
    };

    usm_anim_player<nalAnimClass<nalAnyPose>, 3> my_player;
    nalAnyPose field_40;
    scene_anim_client field_44;
    bool field_50;

    //0x0049BCF0
    nal_anim_controller(actor *a2,
                        nalBaseSkeleton *a3,
                        unsigned int a4,
                        const als::als_meta_anim_table_shared *a5);

    ~nal_anim_controller();

    void * operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    //virtual
    void * get_base_layer_anim_ptr();

    //virtual
    void * get_anim_ptr(Float);

    void get_curr_po_offset(po &a2);

    //virtual
    bool scene_animation_playing() const;

    //virtual
    bool is_anim_active(Float a1);

    double _get_base_anim_time_in_sec() const;

    //virtual
    //0x0049BE60
    double get_anim_time_in_sec(Float a2);

    //virtual
    double get_total_base_anim_time_in_sec() const;

    //virtual
    double _get_base_anim_speed();

    //virtual
    void _set_base_anim_time_in_sec(Float a2);

    //virtual
    double _get_anim_speed(Float );

    //virtual
    void _set_base_anim_speed(Float speed);

    //virtual
    void _frame_advance(Float a2, bool a3, bool a4);

    void get_matrix_data_from_pose(nalAnyPose &arg0);
};

extern void nal_anim_controller_patch();
