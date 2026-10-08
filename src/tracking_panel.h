#pragma once

#include "mstring.h"
#include "resource_key.h"
#include "string_hash.h"
#include "aarect.h"
#include "float.hpp"
#include "vector2d.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct tracking_panel {
    mString field_0;
    resource_key field_10;
    string_hash field_18;
    int camera_index;
    float enter_duration;
    float hold_duration;
    float exit_duration;
    float transition_bias;
    float gutter_width;
    vector3d start_location;
    vector3d end_location;
    vector2d panel_size;
    vector2d final_size;
    float camera_elevation;
    float camera_azimuth;
    float camera_distance;
    float camera_fov;
    string_hash camera_bone;
    string_hash camera_target_bone;
    char field_74[8];

    tracking_panel(from_mash_in_place_constructor *a2);
    void unmash(mash_info_struct *a1, void *a3);
};

struct tracking_panel_anim {
    struct callbacks {
        int(__fastcall *flags)(tracking_panel_anim *, void *);
        vector3d *(__fastcall *location)(tracking_panel_anim *, void *, vector3d *);
        vector2d *(__fastcall *size)(tracking_panel_anim *, void *, vector2d *);
        aarect<float, vector2d> *(__fastcall *gutter)(tracking_panel_anim *, void *, aarect<float, vector2d> *);
        void(__fastcall *advance)(tracking_panel_anim *, void *, Float);
        bool(__fastcall *playing)(tracking_panel_anim *, void *);
    } *m_vtbl;

    vector3d get_loc();
    vector2d get_size();
    aarect<float, vector2d> get_gutter_rect();
    void advance(Float dt);
    bool is_playing();
    void destroy();
};

struct exiting_tracking_panel_anim : tracking_panel_anim {
    float enter_remaining;
    float hold_remaining;
    float exit_remaining;
    vector3d location;
    vector2d size;
    vector3d velocity;
    vector2d size_velocity;
    tracking_panel *definition;

    explicit exiting_tracking_panel_anim(tracking_panel *definition);
    void advance_exit(Float dt);
};

struct continuous_tracking_panel_anim : tracking_panel_anim {
    float enter_remaining;
    float hold_remaining;
    float exit_remaining;
    tracking_panel *definition;
    vector3d initial_location;
    vector2d initial_size;
    vector2d final_size;
    aarect<float, vector2d> gutter_rect;

    explicit continuous_tracking_panel_anim(tracking_panel *definition);
    vector3d tracked_location() const;
    vector3d location() const;
    float blend_weight() const;
    void advance_continuous(Float dt);
};
