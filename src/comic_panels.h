#pragma once

#include "aarect.h"
#include "entity_base_vhandle.h"
#include "fixed_vector.h"
#include "ngl_math.h"
#include "variable.h"
#include "vector2d.h"
#include "vector4d.h"
#include "camera.h"
#include "color.h"

struct cut_scene_player;
struct vector3d;
struct actor;
struct entity;
struct signaller;
struct nglTexture;

namespace nalPanel {
struct nalPanelAnim;
}

namespace comic_panels {

struct panel;

struct panel_params_t {
    uint32_t field_0;
    panel *field_4;
    struct {
        vector4d field_0;
        vector4d field_10;
        int field_20;
        int field_24;
    } field_8[5] {};
    uint8_t field_D0;
    uint8_t field_D1;
};

struct panel_component {

    struct render_info {

        math::MatClass<4, 3> field_0;
        panel_params_t field_40;
        aarect<float, vector2d> field_114;
        aarect<float, vector2d> field_124;
        float field_134;
        float field_138;
        panel *field_13C;
        bool field_140;
        bool field_141;
        bool field_142;
        bool field_143;
        bool field_144;

        //0x00745F50
        render_info(panel &a2, int a3);
    };

    int m_vtbl;
    panel_component *field_4;

    //virtual
    void setup_geomgr(render_info &a3);
};

void set_default_bgcolor(const color &a1);

//0x0x00736A60
void init();

//0x0073EA70
void render();

void frame_advance(Float a1);

bool render_panels();

struct panel_component_base;

struct panel {
    std::intptr_t m_vtbl;
    matrix4x4 field_4;
    vector2d m_size;
    cut_scene_player *field_4C;
    float field_50;
    int field_54;
    float field_58;
    bool field_5C;
    panel_component_base *field_60;
    char field_64;
    char field_65;
    char field_66;
    bool field_67;
    bool field_68;

    void init_anim(nalPanel::nalPanelAnim *a2);

    void capture();

    matrix4x4 get_transform() const;

    //0x007334E0
    bool is_square() const;

    //0x007360B0
    void add_camera_component(const char *a2, bool a3, int a4);

    bool render();

    aarect<float, vector2d> get_rect() const;

    vector3d get_loc() const;

    //0x007319C0
    void set_size(const vector2d &a2);

    //0x007319A0
    void set_loc(const vector3d &a2);
};

struct panel_component_camera : panel_component {
    aarect<float, vector2d> field_8;
    int field_18;
    int field_1C;
    bool field_20;
    float field_24;
    int field_28;
    int field_2C;
    int field_30;
    vhandle_type<camera> field_34[4];
    nglTexture *field_44;
    char field_48;

    camera * get_default_camera() const;

    //0x00733420
    void register_camera(uint32_t a2, const char *a3);

    //0x007388A0
    void set_scene_params(render_info &a2);

    //virtual
    void _render(panel_component::render_info &a2);

    //virtual
    void _capture(panel_component::render_info &a2);
};

struct panel_component_base {
    int m_vtbl;
    panel_component_base *field_4;
    float field_8;
    bool field_C;

    //virtual
    void _render(panel_component::render_info &a2);

    //virtual
    void capture(panel_component::render_info &a2);
};

//0x00738CB0
extern panel_params_t *get_panel_params();

//0x00731150
extern camera *get_current_view_camera(int);

//0x00733AD0
panel *acquire_panel(const char *a1);

extern Var<camera *> current_view_camera;

extern Var<panel *> game_play_panel;

extern Var<fixed_vector<panel *, 48>> panels;

inline Var<color> default_bgcol {0x009392EC};

extern bool & world_has_been_rendered;

} // namespace comic_panels

extern void comic_panels_patch();
