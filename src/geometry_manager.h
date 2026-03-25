#pragma once

#include "float.hpp"
#include "hull.h"
#include "variable.h"

struct po;

struct vector2d;
struct vector3d;
struct matrix4x4;

template<typename, typename>
struct aarect;

namespace geometry_manager {

    enum xform_t 
    {
        XFORM_WORLD_TO_VIEW = 0,
        XFORM_VIEW_TO_WORLD = 1,
        XFORM_VIEW_TO_PROJECTION = 2,
        XFORM_PROJECTION_TO_SCREEN = 3,
        XFORM_EFFECTIVE_WORLD_TO_VIEW = 4,
        XFORM_VIEW_TO_SCREEN = 5,
        XFORM_WORLD_TO_PROJECTION = 6,
        XFORM_WORLD_TO_SCREEN = 7,
        NUM_XFORMS = 9
    };

    //0x00515740
    extern matrix4x4 &get_xform(geometry_manager::xform_t a1);

    extern void set_far_plane(Float a1);

    //0x00515290
    extern float get_aspect_ratio();

    //0x00515270
    extern float get_field_of_view();

    //0x0053FFF0
    extern void set_field_of_view(Float fov);

    //0x00540130
    extern void set_viewport(const aarect<float, vector2d> &a1);

    //0x
    extern bool get_auto_rebuild_view_frame();

    //0x
    extern void set_auto_rebuild_view_frame(bool a1);

    //0x005153E0
    extern void set_look_at(matrix4x4 *a1,
                            const vector3d &eye,
                            const vector3d &a3,
                            const vector3d &a4);

    //0x00515750
    extern void set_xform(xform_t xformtype, const matrix4x4 &a2);

    //0x00524930
    extern void set_view(const vector3d &a1, const vector3d &a2, const vector3d &a3);

    //0x00540170
    extern void set_scissor(const aarect<float, vector2d> &a1);

    extern void set_zoom(Float zoom);

    extern float get_zoom();

    //0x00524650
    extern void compute_view_frustum_in_world_space();

    //0x00524760
    extern void compute_view_frustum_verts_in_world_space();

    //0x0053A930
    extern void rebuild_view_frame();

    extern hull & view_frustum;

    extern hull & world_space_frustum;

    extern fixed_vector<vector3d, 5> & frustum_verts;

    extern float & PROJ_ASPECT;

    extern float & PROJ_FIELD_OF_VIEW;

    extern float & PROJ_ZOOM;
    extern float & PROJ_FAR_PLANE_D;

    extern bool & scene_analyzer_enabled;
    extern bool & auto_rebuild_view_frame;
    extern bool & view_frame_dirty;

    extern matrix4x4 (& xforms)[NUM_XFORMS];

    extern aarect<float, vector2d> & viewport_rect;

    extern aarect<float, vector2d> & scissor_rect;

    //0x00544E70
    extern void create_inst();

    //0x00544DE0
    extern void reset();

    //0x00540070
    extern void set_aspect_ratio(Float a1);

    //0x00515720
    extern bool is_scene_analyzer_enabled();

    extern void enable_scene_analyzer(bool a1);
} // namespace geometry_manager

extern void geometry_manager_patch();
