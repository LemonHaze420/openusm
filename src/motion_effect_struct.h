#pragma once

#include "color32.h"
#include "entity_base_vhandle.h"
#include "quaternion.h"
#include "vector2d.h"
#include "vector3d.h"
#include <list.hpp>

struct camera;
struct entity_base;
struct hull;
struct mString;
struct vector4d;
struct po;

struct motion_pose_sample {
    quaternion rotation;
    vector3d position;
};

struct motion_pose_history {
    int field_0;
    int next_sample;
    int sample_count;
    int field_C[2];
    motion_pose_sample *samples;
    int field_18[8];
    int capacity;
    float interval;
    float remaining;
};

struct motion_trail_sample { vector3d first, second; };
struct motion_trail_info {
    int field_0;
    int sample_count;
    int next_sample;
    motion_trail_sample *samples;
    entity_base *first;
    entity_base *second;
    bool single_entity;
    bool additive;
    uint16_t field_1A;
    color32 first_color;
    color32 second_color;
    uint8_t alpha;
    uint8_t field_25[3];
    int capacity;
    int axis;
    float width;
    float remaining;
    float interval;
};

struct motion_distorted_sample { vector3d first, second, third; };
struct motion_distorted_trail_info {
    entity_base *owner;
    entity_base *first;
    entity_base *second;
    int field_C[3];
    int sample_count;
    int next_sample;
    motion_distorted_sample *samples;
    bool single_entity;
    bool additive;
    uint16_t field_26;
    color32 first_color;
    color32 second_color;
    uint8_t alpha;
    uint8_t field_31[3];
    int capacity;
    int axis;
    float width;
    float remaining;
    float interval;
};

struct motion_afterimage_info {
    float field_0, field_4;
    int field_8, field_C, field_10;
    bool active;
    uint8_t field_15[3];
    int bone_count;
    int field_1C;
    po *bones;
};

struct motion_effect_struct {
    motion_effect_struct *next;
    motion_effect_struct *previous;
    _std::list<motion_pose_sample> history;
    entity_base_vhandle owner;
    motion_pose_history *pose_history;
    motion_trail_info *trail;
    motion_distorted_trail_info *distorted_trail;
    motion_afterimage_info *afterimage;
    bool draining_trail;
    bool trail_active;
    bool field_2A;
    bool pose_recording;
    bool draining_distorted_trail;
    bool distorted_trail_active;
    bool field_2E;
    bool field_2F;

    //0x004DC560
    motion_effect_struct(entity_base_vhandle handle, const mString &texture);
    ~motion_effect_struct();
    void remove_from_list();
    void activate_trail(entity_base *, int axis, float width, color32 color,
                        int alpha, float interval, int samples, bool additive);
    void record(Float elapsed);
    void render_trail();
    void render_distorted_trail();
    static void render_all_motion_fx(camera &, hull &);
    //0x004DC820
    void render_trail(vector3d, vector3d, vector3d, vector2d, vector2d, vector2d,
                      color32, color32, color32, bool, vector3d, vector3d);
    //0x004DCA10
    void render_distorted_trail(const vector3d &, const vector3d &, const vector3d &,
                                const vector4d &, const vector4d &, const vector4d &,
                                color32, color32, color32, bool, vector3d &, vector3d &);
    //0x004EFA50
    static void record_all_motion_fx(Float elapsed);
};

extern void motion_effect_struct_patch();
