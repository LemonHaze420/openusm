#include "occlusion.h"

#include "app.h"
#include "beam.h"
#include "camera.h"
#include "color32.h"
#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "game.h"
#include "oldmath_po.h"
#include "trace.h"
#include "vector3d.h"

VALIDATE_SIZE(occlusion::quad, 0x30);
VALIDATE_SIZE(occlusion::quad_shadow_volume, 0x90);

Var<occlusion::quad *> occlusion::quad_database = (0x0095C880);

Var<int> occlusion::quad_database_count = (0x0095C884);

Var<bool> occlusion::initialized = (0x0095C87C);
Var<int> occlusion::num_active_shadow_volumes = (0x0095C88C);

void occlusion::term()
{
    TRACE("occlusion::term");

    operator delete[](quad_database());
    quad_database() = nullptr;
    quad_database_count() = 0;
}

void occlusion::init()
{
    quad_database() = new quad[400u];
    quad_database_count() = 0;
    initialized() = true;
    num_active_shadow_volumes() = 0;
}

void occlusion::add_quad_to_database(const occlusion::quad &a1)
{
    if (quad_database_count() < 399) {
        std::memcpy(&quad_database()[quad_database_count()++], &a1, sizeof(occlusion::quad));
    }
}

void occlusion::reset_active_occluders()
{
    num_active_shadow_volumes() = 0;
}

void occlusion::empty_quad_database()
{
    assert(initialized());

    occlusion::quad_database_count() = 0;
}

void occlusion::debug_render_occluders()
{
    const color32 blue{255, 0, 0, 255};
    const color32 green{0, 255, 0, 128};
    for (auto i = 0; i < num_active_shadow_volumes(); ++i) {
        auto *v12 = &active_shadow_volumes()[i];
        auto *v1 = app::instance;
        auto *v2 = v1->m_game;
        auto *v4 = v2->get_current_view_camera(0);
        auto &v5 = v4->get_abs_po();
        [[maybe_unused]] auto v11 = v5;

        vector3d v10;
        vector3d v9;
        vector3d v8;
        vector3d v7;
        v10 = v12->field_0[0];
        v9 = v12->field_0[1];
        v8 = v12->field_0[2];
        v7 = v12->field_0[3];

        render_beam(v10, v9, blue, 1.0, false);
        render_beam(v9, v8, blue, 1.0, false);
        render_beam(v8, v7, blue, 1.0, false);
        render_beam(v7, v10, blue, 1.0, false);
        render_beam(v10, v8, blue, 1.0, false);
        render_beam(v9, v7, blue, 1.0, false);

        render_quad(v12->field_0[0], v12->field_0[1], v12->field_0[2], v12->field_0[3], green, true);
    }
}

void occlusion::init_frame(const vector3d &a1)
{
    ++init_frame_count;
    update_based_on_scores(a1);
    active_shadow_volumes_scratchpad_mirror() = active_shadow_volumes();
}

void occlusion::term_frame()
{
    //assert(init_frame_count == 0);
}

void occlusion::update_based_on_scores(const vector3d &a1)
{
    CDECL_CALL(0x00521850, &a1);
}
