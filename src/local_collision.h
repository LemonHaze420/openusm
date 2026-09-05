#pragma once

#include "float.hpp"
#include "fixed_pool.h"
#include "variable.h"
#include "vector3d.h"

struct actor;
struct region;
struct entity;
struct subdivision_node_obb_base;
struct dynamic_conglomerate_clone;
struct line_info;
struct po;
struct line_segment_t;
struct intraframe_trajectory_t;

struct walkable_entfilter_t {};

struct walkable_obbfilter_t {};

struct crawlable_obbfilter_t {};

namespace local_collision {

struct closest_points_pair_t {
    static inline fixed_pool &pool = var<fixed_pool>(0x00922154);
};

struct intersection_list_t {
    int field_0;
    vector3d normal;
    vector3d point;
    float field_1C;
    int field_20;
    bool is_ent;
    void *intersection_node;
    void *field_2C;

    intersection_list_t() = default;
};

struct query_args_t {
    uint32_t initialized_flags;
    vector3d field_4;
    vector3d field_10;
    vector3d field_1C;
    float field_28;
    entity *field_2C;
    entity *field_30;

    query_args_t() = default;

    void set_entity(entity *a2);
};

struct entfilter_base {
    std::intptr_t m_vtbl;

    bool accept(actor *act, dynamic_conglomerate_clone *a2, const query_args_t &a3) const;
};

struct obbfilter_base {
    std::intptr_t m_vtbl;
};

struct obbfilter_OBB_LINE_SEGMENT_TEST {};

struct obbfilter_OBB_SPHERE_TEST {};

template <typename T>
struct obbfilter : obbfilter_base {
    obbfilter();

    //virtual
    bool accept(subdivision_node_obb_base *a1, const query_args_t &a2);
};

struct entfilter_BLOCKS_AI_LOS {};
struct entfilter_ENTITY {};
struct entfilter_COLLIDE_CAMERA {};
struct entfilter_LINESEG_TEST {};
struct entfilter_NO_CAPSULES {};
struct entfilter_BLOCKS_BEAMS {};
struct entfilter_EXCLUDE_ENTITY {};
struct entfilter_SPHERE_TEST {};
struct entfilter_VALID_COLLISION_PAIR {};

template <typename T0, typename T1>
struct obbfilter_AND {};

template <typename T0, typename T1>
struct entfilter_AND {};

template <typename T>
struct entfilter : entfilter_base {
    entfilter();

    //virtual
    bool accept(actor *a1, dynamic_conglomerate_clone *a2, const query_args_t &a3);
};


struct entfilter_reject_all_t : entfilter_base {};

struct entfilter_accept_all_t : entfilter_base {};

struct primitive_list_t {
    primitive_list_t *field_0;

    union {
        entity *ent;
        subdivision_node_obb_base *obb;
    } field_4;
    void *field_8;
    bool is_ent;
    intraframe_trajectory_t *field_10;

    primitive_list_t(void *a2, void *a3);

    bool is_entity()
    {
        return this->is_ent;
    }

    entity *get_entity();

    void *get_obb_node();

    static inline fixed_pool &pool = var<fixed_pool>(0x00922174);
};

bool test_line_intersection_ex(local_collision::primitive_list_t **a1, const line_info &a2,
                               local_collision::primitive_list_t ***occluder);

//0x00510720
extern void destroy_primitive_list(primitive_list_t **a1);

//0x0052F4E0
extern bool entity_line_segment_test(actor *a1, dynamic_conglomerate_clone *a2,
                                     const local_collision::query_args_t &a3);

//0x00533260
extern primitive_list_t *query_line_segment(const vector3d &a1, const vector3d &a2,
                                            const local_collision::entfilter_base &a3,
                                            const local_collision::obbfilter_base &a4,
                                            local_collision::query_args_t a5);

extern bool get_closest_line_intersection(local_collision::primitive_list_t *a1, line_segment_t *lif, bool a3,
                                          float *a4, const float *a5, local_collision::intersection_list_t *a6);

extern primitive_list_t *query_sphere(const vector3d &a1, Float a2, const entfilter_base &a3, const obbfilter_base &a4,
                                      query_args_t query_args);

extern bool get_closest_sphere_intersection(primitive_list_t *a1, const vector3d &a2, Float a3, vector3d *a4,
                                            vector3d *a5, intersection_list_t *best_intersection_record);

extern bool collision_pair_matches_query_constraints(actor *a1, dynamic_conglomerate_clone *a2,
                                                     local_collision::entfilter_base &a3,
                                                     local_collision::query_args_t &a4);

inline auto &entfilter_entity_no_capsules =
    var<entfilter<entfilter_AND<entfilter_ENTITY, entfilter_NO_CAPSULES>> *>(0x00960068);

inline auto &entfilter_entity_no_capsules_instance =
    var<entfilter<entfilter_AND<entfilter_ENTITY, entfilter_NO_CAPSULES>>>(0x00922474);

inline auto &entfilter_blocks_beams =
    var<entfilter<entfilter_AND<entfilter_BLOCKS_BEAMS, entfilter_LINESEG_TEST>> *>(0x00960060);

inline auto &entfilter_entity_collision = var<local_collision::entfilter_base *>(0x00960058);

inline entfilter_base *&entfilter_line_segment_camera_collision = var<entfilter_base *>(0x00960070);

inline entfilter_base *&entfilter_sphere_camera_collision = var<entfilter_base *>(0x00960074);

inline obbfilter_base *&obbfilter_lineseg_test = var<obbfilter_base *>(0x00960064);

inline obbfilter<obbfilter_OBB_SPHERE_TEST> *&obbfilter_sphere_test =
    var<obbfilter<obbfilter_OBB_SPHERE_TEST> *>(0x00960050);

inline obbfilter_base *&obbfilter_accept_all = var<obbfilter_base *>(0x00960048);

inline entfilter_reject_all_t *&entfilter_reject_all = var<entfilter_reject_all_t *>(0x00960054);

inline entfilter_accept_all_t *&entfilter_accept_all = var<entfilter_accept_all_t *>(0x0096004C);

}  // namespace local_collision

//0x005C4DD0
extern bool find_intersection(const vector3d &a1, const vector3d &a2, const local_collision::entfilter_base &a3,
                              const local_collision::obbfilter_base &a4, vector3d *a5, vector3d *a6, region **a7,
                              entity **a8, subdivision_node_obb_base **hit_obb, bool a10);

//0x005C31D0
extern void closest_point_line_segment_point(const vector3d &a1, const vector3d &a2, const vector3d &a3, float &a4);

extern bool sub_50D220(const vector3d &a1, const vector3d &a2, entity *a3);

extern void local_collision_patch();
