#pragma once

#include "entity.h"

#include "ai_tentacle_info.h"
#include "ngl_vertexdef.h"
#include "spline.h"

#include <cstdint>

struct string_hash;
struct PCUV_ShaderMaterial;
struct PolytubeCustomMaterial;
struct Tentacle_ShaderMaterial;
struct polytube_misc_render_object {
    simple_list<polytube_misc_render_object *>::vars_t simple_list_vars;
    entity_base_vhandle object;
    float percent;
    color32 color;
    bool enabled;

    void render(polytube *tube, Float dt);
};

struct polytube_pt_anim {
    uint32_t field_0;
    vector3d field_4;
    vector3d field_10;
    float field_1C;
    float field_20;
    float field_24;
    float field_28;

    polytube_pt_anim();
    void frame_advance(Float dt, vector3d &point);
    void set_anim(const vector3d &start, const vector3d &direction, float duration, unsigned flags);
};

struct PolytubeCustomOffset {
    struct Iterator {
        vector2d *field_0;
        int field_4;
        uint32_t field_8;
        explicit Iterator(unsigned count);
        ~Iterator();
    };
};

struct PolytubeCustomVertex {
    struct Iterator {
        int field_0;
        uint32_t field_4;
        PolytubeCustomOffset::Iterator *field_8;
        nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator field_C;
        vector3d field_18;
        vector3d field_24;
        vector3d field_30;
        float field_3C;
        uint32_t field_40;
        float field_44;
        float field_48;

        void Write(const vector3d &a2, const vector3d &a3);
        Iterator() = default;
        Iterator(unsigned count, PCUV_ShaderMaterial *material, PolytubeCustomOffset::Iterator *offsets, uint32_t color,
                 float tiles, float phase);
    };
};

struct polytube : entity {
    polytube *field_68;
    polytube *field_6C;
    PolytubeCustomOffset::Iterator *field_70;
    PolytubeCustomOffset::Iterator *field_74;

    char field_78;
    char field_79;
    char field_7A;
    char field_7B;
    po *field_7C;
    spline the_spline;
    PolytubeCustomMaterial *field_D0;
    PolytubeCustomMaterial *field_D4;
    PolytubeCustomMaterial *field_D8;
    int field_DC;
    int field_E0;
    Tentacle_ShaderMaterial *field_E4;
    int field_E8;
    int field_EC;
    float tube_radius;
    int num_sides;
    float tiles_per_meter;
    float max_length;
    float field_100;
    float field_104;
    int field_108;
    _std::vector<polytube_pt_anim> pt_anims;
    entity_base_vhandle field_11C;
    entity_base_vhandle field_120;
    float field_124;
    color32 field_128;
    float field_12C;
    ai_tentacle_info *field_130;
    float tentacle_width;
    float tentacle_activity;
    float tentacle_pull_factor;
    int16_t field_140;
    char field_142;
    char field_143;
    simple_list<polytube_misc_render_object *> misc_render_objects;
    nglMesh *field_150;
    nglMesh *field_154;
    nglMesh *field_158;
    int field_15C[7];

    //0x005A57A0
    polytube(const string_hash &a2, uint32_t a3);
    ~polytube();
    void *destroy(unsigned char flags);
    void frame_advance(Float dt);
    void remove_from_list();
    void update_active_list();
    void kill_anim(int index, bool restore_position);
    void clear_simulations();
    void set_anim(int index, const vector3d &start, const vector3d &direction, float duration, unsigned flags);
    void set_random_pt_anim(int index, float radius, float duration, unsigned flags);
    void ifl_lock(int frame);
    void ifl_play();

    void rebuild_helper();

    //0x0048F090
    void set_abs_control_pt(int index, const vector3d &a3);

    void set_max_length(Float a2);

    void set_control_pt(int index, const vector3d &a2);

    void set_tiles_per_meter(Float a2);

    vector3d get_control_pt(int a3);

    int get_num_control_pts();

    void build(int a1, spline::eSplineType a2);

    //0x005A3960
    void init();


    void init_offsets();

    //0x005A2390
    void set_material(string_hash a2);

    //0x005A2460
    void set_material(PolytubeCustomMaterial *a2);
    void set_material(Tentacle_ShaderMaterial *material);
    void set_tentacle_width(Float value)
    {
        tentacle_width = value;
    }
    void set_tentacle_activity(Float value)
    {
        tentacle_activity = value;
    }
    void set_tentacle_pull_factor(Float value)
    {
        tentacle_pull_factor = value;
    }
    float get_tentacle_width() const
    {
        return tentacle_width;
    }
    float get_tentacle_activity() const
    {
        return tentacle_activity;
    }
    float get_tentacle_pull_factor() const
    {
        return tentacle_pull_factor;
    }
    void set_render_color(color32 value)
    {
        field_128 = value;
    }
    color32 get_render_color() const
    {
        return field_128;
    }
    void set_visible(bool visible, bool include_children);
    vector3d get_visual_center();
    float get_visual_radius();
    void destroy_offsets();
    void simulate_slack(const vector3d &start, const vector3d &end, float length);
    void add_misc_render_object(polytube_misc_render_object *object);
    bool remove_misc_render_object(polytube_misc_render_object *object);

    void add_control_pt(const vector3d &a2);

    void check_anims(bool a2);

    void destroy_tentacle_info();

    void create_tentacle_info();

    void reserve_control_pts(int num);

    void set_force_start(bool a1);

    //0x005A5B10
    //virtual
    void _render(Float a2);

    //0x0059B490
    static void frame_advance_all_polytubes(Float a1);
};

extern void polytube_patch();
