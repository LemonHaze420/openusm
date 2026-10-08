#pragma once

#include "float.hpp"
#include "matrix4x4.h"
#include "nal_anim.h"
#include "nal_generic.h"
#include "nal_system.h"
#include "variable.h"
#include "vector2d.h"
#include "quaternion.h"

namespace comic_panels {

struct page_camera {
    struct callbacks {
        void *(__fastcall *CreateInstance)(page_camera *, void *, nalAnimClass<nalAnyPose> *);
        void(__fastcall *Advance)(page_camera *, void *, nalGeneric::nalGenericInstance *, Float, Float, int, int);
        void(__fastcall *Render)(page_camera *, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float);
        void(__fastcall *Release)(page_camera *, void *);
        void(__fastcall *destroy)(page_camera *, void *, bool);
    } *m_vtbl;
    struct pose_layers {
        nalGeneric::nalGenericSkeleton *skeleton;
        nalGeneric::nalGenericPose field_4;
        nalGeneric::nalGenericPose field_10;
        int count;
        void *layers[3];
        struct page_camera_layer *active;
        struct page_camera_layer *free;
        int generation;
        explicit pose_layers(nalGeneric::nalGenericSkeleton *skeleton);
    } field_4;
    vector4d field_3C;
    matrix4x4 field_4C;
    matrix4x4 field_8C;
    float field_CC;
    float field_D0;
    float field_D4;
    float field_D8;
    float field_DC;
    float field_E0;
    float field_E4;
    float field_E8;
    bool field_EC;
    bool field_ED;
    bool field_EE;
    char field_EF;

    page_camera();
    ~page_camera();

    auto get_transform() const
    {
        return field_4C;
    }
    vector2d ortho_size() const;
    void interpret_pose(nalGeneric::nalGenericPose &pose);
    void Advance(nalGeneric::nalGenericInstance *instance, Float time, Float previous, int, int);
    void advance(Float dt);
    void finalize(bool release);

    //virtual
    void *CreateInstance(nalAnimClass<nalAnyPose> *a2);

    //virtual
    void Render(nalAnimClass<nalAnyPose>::nalInstanceClass *a1, Float a2);
};

extern Var<page_camera *> cur_page_camera;
page_camera *create_page_camera();

}  // namespace comic_panels
