#pragma once

#include "fixedstring.h"
#include "nal_anim.h"
#include "nal_system.h"

namespace comic_panels {
struct page_camera;
}

struct nalSceneAnim {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    tlFixedString field_10;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    tlFileBuf field_40;
    int field_4C;
    tlFixedString field_50;

    static tlFixedString * get_string(nalSceneAnim *a1)
    {
        return &a1->field_10;
    }
};

template<typename>
struct nalAnimClass;

struct nalSceneAnimInstance {
    int m_vtbl;
    nalSceneAnim *field_4;
    struct {
        comic_panels::page_camera *field_0;
        nalAnimClass<nalAnyPose> *field_4;
        nalAnimClass<nalAnyPose>::nalInstanceClass *field_8;
        void *field_C;
    } *field_8;
    int field_C;
    float field_10;
    float field_14;
    struct {
        char field_0[0x8];
        struct {
            char field_0[0x38];
            float field_38;
        } *field_8;
    } *field_18;

    void Render() const;
};

extern void scene_anim_patch();
