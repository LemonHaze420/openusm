#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "local_collision.h"
#include "simple_classes.h"
#include "variable.h"
#include "vector3d.h"

struct line_info_local_query {};

struct line_info {
    vector3d field_0;
    vector3d field_C;
    vector3d hit_pos;
    vector3d hit_norm;
    vector3d field_30;
    vector3d field_3C;
    vhandle_type<entity> hit_entity;
    subdivision_node_obb_base *m_obb;
    const local_collision::entfilter_base *ent_filter;
    const local_collision::obbfilter_base *obb_filter;
    bool collision;
    bool field_59;
    bool queued_for_collision_check;
    bool field_5C;

    //0x0052EDD0
    line_info();

    line_info(const vector3d &a2, const vector3d &a3);

    ~line_info();

    void render(int num, bool a3);

    //0x0048C9D0
    void clear();

    //0x0052EE20
    bool check_collision(const local_collision::entfilter_base &a2, const local_collision::obbfilter_base &a3,
                         line_info_local_query *a4);

    //0x006B6E00
    void copy(const line_info &a2);

    //0x0052EE00
    bool remove_to_collision_check_queue();

    bool release_mem();

    void sub_48B410(Float a2);

    //0x0052F120
    static void frame_advance(int a1);

    static simple_queue<line_info *, 16> &queued_collision_checks;
};

inline constexpr auto MAX_RENDERABLE_LINE_INFOS = 64;

#ifndef TEST_CASE

extern int num_debug_line_info[2];

extern std::array<line_info[MAX_RENDERABLE_LINE_INFOS], 2> debug_line_info;

#endif

extern void line_info_patch();
