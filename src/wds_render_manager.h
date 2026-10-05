#pragma once

#include "float.hpp"

#include <vector.hpp>

#include "entity.h"
#include "entity_base_vhandle.h"
#include "ngl_vertexdef.h"

struct RenderOptimizations;
struct city_lod;
struct nglMesh;
struct camera;
struct fixed_pool;
struct light_source;
struct far_away_render_list_entry {
    far_away_render_list_entry *next;
    vhandle_type<entity> handle;
};

fixed_pool &far_away_render_list_pool();

struct render_data {
    struct region_info {
        region *field_0;
    };

    struct entity_info {
        vector3d center;
        float radius;
        float distance_squared;
        float fade;
        entity *ent;
    };

    _std::vector<render_data::region_info> field_0;
    _std::vector<render_data::entity_info> field_10;
    vector3d field_20;

    void sub_56FCB0();
};

struct wds_render_manager {
    RenderOptimizations *field_0;
    int empty0[3];
    float field_10[4];
    float field_20[4];
    render_data field_30;
    nglMesh *field_5C;
    color field_60;
    int empty2[5];
    float field_84;
    float field_88;
    float field_8C;
    float field_90;
    city_lod *field_94;
    far_away_render_list_entry *field_98;

    //0x00542270
    wds_render_manager();

    void debug_render();

    //0x0052A470
    int add_far_away_entity(vhandle_type<entity> a2);

    void update_occluders(camera &a2);

    //0x0054B250
    void render(camera &a2, int a3);

    //0x00550930
    void init_level(const char *a2);

    void create_colorvol_scene();

    //0x0053CED0
    void render_meshes(camera &a2);

    //0x0053D270
    void render_legos(camera &a2);

    void render_lowlods(camera &);

    void render_region_mesh(nglMesh *a2, Float fade);

    //0x0054ADE0
    void frame_advance(Float a2);

    void render_stencil_shadows(const camera &a2);

    //0x00547000
    void build_render_data_regions(render_data &a2, camera &a3);

    //0x00547250
    void build_render_data_ents(render_data &a2, camera &a3, int a4);


    _std::vector<light_source *> *find_lights(const vector3d &position);

    void clear_colorvol_scene();

    void sub_53D560(camera &a2);
};

extern nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator sub_CBFF50(int a3);

extern void wds_render_manager_patch();
