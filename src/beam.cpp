#include "beam.h"

#include "app.h"
#include "camera.h"
#include "comic_panels.h"
#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "game.h"
#include "geometry_manager.h"
#include "local_collision.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_dx_vertexdef.h"
#include "oldmath_po.h"
#include "us_pcuv_shader.h"
#include "trace.h"

#include <cmath>

VALIDATE_SIZE(beam, 0xE8u);

beam::beam() {}

void beam::_render(Float a1)
{
    TRACE("beam::render");

    if constexpr (1) {
        if (this->field_A8 > 0.0f && this->field_78 < this->field_A8) {
            auto *the_game = app::instance->m_game;
            auto *cam = the_game->get_current_view_camera(0);

            vector3d abs_position = this->get_abs_position();
            po &abs_po = this->get_abs_po();

            vector3d cam_abs_pos = cam->get_abs_position();

            auto minus_result = cam_abs_pos - abs_position;
            auto y = dot(minus_result, abs_po.get_y_facing());
            auto x = dot(minus_result, abs_po.get_x_facing());
            minus_result = vector3d{x, y, 0.0};

            auto len = minus_result.length2();
            if (len > 0.001f) {
                auto v16 = 1.0f / std::sqrt(len);
                minus_result *= v16;

                auto v52 = this->field_70 * 0.5f;

                vector3d v45[3]{
                    vector3d{0.0, 0.0, this->field_A8},
                    vector3d{0.0, 0.0, this->field_78},
                    vector3d{static_cast<float>((0.0 - minus_result[1]) * v52), minus_result[0] * v52, 0.0}};

                vector3d v44[4]{};
                v44[3] = v45[1] - v45[2];
                v44[2] = v45[1] + v45[2];
                v44[1] = v45[0] + v45[2];
                v44[0] = v45[0] - v45[2];

                vector2d v43[4]{};

                if (this->field_DC <= 0.0f) {
                    v43[3] = this->field_CC[0];
                    v43[2][0] = this->field_CC[1][0];
                    v43[2][1] = this->field_CC[0][1];
                    v43[1] = this->field_CC[1];
                    v43[0][0] = this->field_CC[0][0];
                    v43[0][1] = this->field_CC[1][1];
                } else {
                    float v42 = (this->field_A8 * this->field_DC) - 1.0;
                    v43[3] = this->field_CC[0];
                    v43[2][0] = this->field_CC[1][0];
                    v43[2][1] = this->field_CC[0][1];
                    v43[1][0] = this->field_CC[1][0];
                    v43[1][1] = this->field_CC[1][1] + v42;
                    v43[0][0] = this->field_CC[0][0];
                    v43[0][1] = this->field_CC[1][1] + v42;
                }

                nglCreateMesh(0x40000u, 2u, 0, nullptr);

                assert(this->my_material != nullptr);

                auto v24 = (this->field_E2 != 0);
                this->my_material->m_blend_mode = static_cast<nglBlendModeType>(v24 + 2);

                uint32_t color0 = color32::to_int(this->field_7C);
                uint32_t color1 = color32::to_int(this->field_80);

                nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator Iter{};
                nglMaterialBase *v29 =
                    (this->my_material != nullptr ? bit_cast<nglMaterialBase *>(&this->my_material->field_4) : nullptr);

                auto *v30 = sub_507920(v29, 3, 1, 0, nullptr, D3DPT_TRIANGLESTRIP, true);
                Iter = v30->CreateIterator();
                Iter.BeginStrip(3u);

                Iter.Write(v44[3], color0, v43[3]);
                ++Iter;

                Iter.Write(v44[2], color0, v43[2]);
                ++Iter;

                Iter.Write(v44[1], color1, v43[1]);
                ++Iter;

                auto *v33 = Iter.field_4->field_4;
                if ((v33->Flags & 0x40000) == 0) {
                    IDirect3DVertexBuffer9_Unlock(v33->field_3C.getVertexBuffer());
                }

                nglMaterialBase *v36 =
                    (this->my_material != nullptr ? bit_cast<nglMaterialBase *>(&this->my_material->field_4) : nullptr);

                auto *v37 = sub_507920(v36, 3, 1, 0, nullptr, D3DPT_TRIANGLESTRIP, true);
                Iter = v37->CreateIterator();
                Iter.BeginStrip(3u);

                Iter.Write(v44[3], color0, v43[3]);
                ++Iter;

                Iter.Write(v44[0], color1, v43[0]);
                ++Iter;

                Iter.Write(v44[1], color1, v43[1]);
                ++Iter;

                auto *v40 = Iter.field_4->field_4;
                if ((v40->Flags & 0x40000) == 0) {
                    IDirect3DVertexBuffer9_Unlock(v40->field_3C.getVertexBuffer());
                }


                auto v49 = *bit_cast<math::MatClass<4, 3> *>(&this->get_abs_po().get_matrix());
                auto v42 = nglCloseMesh();
                nglListAddMesh(v42, v49, nullptr, nullptr);
            }
        }
    } else {
        THISCALL(0x005406D0, this, a1);
    }
}

void beam::frame_advance_all_beams(Float a3)
{
    CDECL_CALL(0x0051CED0, a3);
}

bool sub_CB3D60(unsigned int a5)
{
    nglCreateMesh(0x40000, a5, 0, nullptr);
    assert(nglScratch() != nullptr);
    return (nglScratch() != nullptr);
}

void sub_78EA60(vector3d &a1, float a2)
{
    auto v4 = a1.length2();
    if (v4 > (0.0000099999997 * 0.0000099999997)) {
        auto v3 = [](float a1) {
            return 1.0 / std::sqrt(a1);
        }(v4)*a2;

        a1 *= v3;
    }
}

void sub_CB4800(const vector3d &a1, const vector3d &arg4, int a3, float a4, void *a6)
{
    auto v44 = arg4 - a1;
    auto a3a = v44.length2();
    if (a3a >= 9.9999997e-10) {
        a3a = std::sqrt(a3a);
        auto v42 = v44 / a3a;

        auto camera_pos = g_game_ptr->get_current_view_camera(0)->get_abs_position();

        auto v40 = camera_pos - a1;
        auto v39 = camera_pos - arg4;
        auto v38 = v40.length2();
        auto v37 = v39.length2();

        auto sub_6BAED0 = [](float a1) {
            return 1.0 / std::sqrt(a1);
        };

        if (not_equal<float>(v38, 0.0f)) {
            auto v17 = 0.5 * a4;

            v40 *= sub_6BAED0(v38) * v17;
        }

        if (not_equal<float>(v37, 0.0f)) {
            auto v18 = 0.5 * a4;
            v39 *= sub_6BAED0(v37) * v18;
        }

        v40 += a1;
        v39 += arg4;

        auto sub_CB4C90 = [](float a1) {
            return 1.0 - 0.75 / ((0.25 * 0.25) * a1 + 1.0);
        };

        auto v36 = sub_CB4C90(v38);
        auto v35 = sub_CB4C90(v37);
        auto v5 = a1 + arg4;
        auto v6 = v5 * 0.5;
        auto v34 = v6 - camera_pos;
        auto v33 = vector3d::cross(v34, v42);
        if (v33.length2() >= 9.9999997e-10 || geometry_manager::is_scene_analyzer_enabled()) {
            sub_78EA60(v33, a4 * 0.5);
            vector3d v32{};
            vector3d v31{};
            vector3d v30{};
            vector3d a4a{};
            auto v7 = v33 * v36;
            v32 = v40 - v7;

            auto v9 = v33 * v36;
            v31 = v40 + v9;

            auto v11 = v33 * v35;
            v30 = v39 + v11;

            auto v13 = v33 * v35;
            a4a = v39 - v13;
            sub_CB3F80(v32, v31, v30, a4a, a3, a6);
        }
    }
}

void beam_patch()
{
    {
        FUNC_ADDRESS(address, &beam::_render);
        set_vfunc(0x00889B9C, address);
    }
}
