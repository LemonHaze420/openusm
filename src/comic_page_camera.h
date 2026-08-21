#pragma once

#include "float.hpp"
#include "matrix4x4.h"
#include "nal_anim.h"
#include "nal_generic.h"
#include "nal_system.h"
#include "variable.h"

namespace comic_panels {

struct page_camera {
    struct {
        void *(__fastcall *CreateInstance)(void *, void *, nalAnimClass<nalAnyPose> *);
        int field_4;
        void(__fastcall *Render)(void *, void *, nalAnimClass<nalAnyPose>::nalInstanceClass *, Float);
    } *m_vtbl;
    struct {
        int field_0;
        nalGeneric::nalGenericPose field_4;
        nalGeneric::nalGenericPose field_10;
        int field_1C;
        int field_20;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        int field_34;
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

    auto get_transform() const
    {
        return field_4C;
    }

    //virtual
    void *CreateInstance(nalAnimClass<nalAnyPose> *a2);

    //virtual
    void Render(nalAnimClass<nalAnyPose>::nalInstanceClass *a1, Float a2);
};

extern Var<page_camera *> cur_page_camera;

}  // namespace comic_panels
