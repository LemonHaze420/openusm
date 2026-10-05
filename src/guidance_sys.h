#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "vector3d.h"

struct physical_interface;
struct thrown_item;

struct guidance_system {
    int m_vtbl;
    physical_interface *owner;
    float field_8;
    vector3d field_C;
    int field_18;
    bool field_1C;
};

struct rocket_guidance_sys : guidance_system {
    vhandle_type<entity> field_20;
    vector3d field_24;
    vector3d field_30;
    float field_3C;
    float field_40;
    float field_44;
    float field_48;
    float field_4C;
    float field_50;
    float field_54;
    float field_58;
    float field_5C;
    float field_60;
    float field_64;
    float field_68;
    float field_6C;
    float field_70;
    float field_74;
    float field_78;
    float field_7C;
    float field_80;
    float field_84;
    float field_88;
    vector3d field_8C;
    vector3d field_98;
    thrown_item *field_A4;

    explicit rocket_guidance_sys(physical_interface *);
    static void *native_vtable();
    void set_target(vhandle_type<entity>);
    void launch(const vector3d &, float);
    void frame_advance(Float);
    void wobble();
    void reacquire_target(float radius, float cosine, float delay, entity_base *exclude);
};
