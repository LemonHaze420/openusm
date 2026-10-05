#include "potential_shadow.h"

#include "common.h"
#include "conglom.h"
#include "geometry_manager.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "ngl_texture.h"
#include "oldmath_po.h"
#include "shadow.h"
#include "variables.h"

Var<potential_shadow[2]> shadow_candidates{0x00965F78};

VALIDATE_SIZE(potential_shadow, 0x2C);

potential_shadow::potential_shadow() {}

void potential_shadow::sub_5932C0()
{
    auto *v2 = this->field_18;
    if (v2 != nullptr) {
        v2->render_simple_shadow(this->field_1C, 1.0);
    }

    this->field_18 = nullptr;
    this->field_28 = 0.0;
}

void potential_shadow::sub_593280()
{
    auto v2 = this->m_radius;
    auto v3 = this->field_18;
    this->field_28 = v2;

    if (v3->is_hero()) {
        this->field_28 = this->field_28 + 10.0f;
    }

    this->field_28 = this->m_fade / (this->field_1C + 1.0f) * this->field_28;
}

namespace {

void shadow_scene_callback(unsigned int *&, void *) {}

void copy_shadow_target(nglTexture *destination, nglTexture *source)
{
    nglListBeginScene(static_cast<nglSceneParamType>(0));
    nglSetRenderTarget(destination);
    nglSetViewport(0.0f, 0.0f, 649.0f, 649.0f);
    nglSetClearFlags(0);
    nglSetZWriteEnable(false);
    nglSetZTestEnable(false);
    nglQuad quad;
    nglInitQuad(&quad);
    const float width = static_cast<float>(destination->m_width);
    nglSetQuadRect(&quad, 0.0f, 0.0f, width, width);
    nglSetQuadTex(&quad, source);
    nglSetQuadUV(&quad, 0.0f, 0, 1.0f, 1.0f);
    nglSetQuadMapFlags(&quad, 194);
    nglSetQuadBlend(&quad, static_cast<nglBlendModeType>(0), 0);
    nglListAddQuad(&quad);
    nglListEndScene();
}

void clip_shadow_edges(nglTexture *texture)
{
    const float width = static_cast<float>(texture->m_width);
    const float last = width - 1.0f;
    nglListBeginScene(static_cast<nglSceneParamType>(0));
    nglSetRenderTarget(texture);
    nglSetViewport(0.0f, 0.0f, last, last);
    nglSetClearFlags(0);
    nglSetZWriteEnable(false);
    nglSetZTestEnable(false);
    nglQuad quad;
    nglInitQuad(&quad);
    nglSetQuadColor(&quad, 0);
    nglSetQuadBlend(&quad, static_cast<nglBlendModeType>(0), 0);
    nglSetQuadRect(&quad, 0.0f, 0.0f, width, 1.0f);
    nglListAddQuad(&quad);
    nglSetQuadRect(&quad, 0.0f, 1.0f, 1.0f, width);
    nglListAddQuad(&quad);
    nglSetQuadRect(&quad, 1.0f, last, width, width);
    nglListAddQuad(&quad);
    nglSetQuadRect(&quad, last, 0.0f, width, last);
    nglListAddQuad(&quad);
    nglListEndScene();
}
}

void potential_shadow::commit()
{
    if (field_18 == nullptr || g_cur_shadow_target >= 2)
        return;

    auto &destination = g_shadow()[g_cur_shadow_target++];
    auto *previous_scene = nglListSelectScene(nglRootScene);

    nglListSelectScene(g_shadow_scene);
    auto *unblurred = var<nglTexture *>(0x00965F48);
    const float last = static_cast<float>(unblurred->m_width - 1);
    nglListBeginScene(static_cast<nglSceneParamType>(0));
    nglSetRenderTarget(unblurred);
    nglSetViewport(0.0f, 0.0f, last, last);
    nglSetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    nglSetClearFlags(1);
    nglSetZTestEnable(false);
    nglListEndScene();

    nglListBeginScene(static_cast<nglSceneParamType>(0));
    nglSetRenderTarget(unblurred);
    nglSetViewport(2.0f, 2.0f, last - 2.0f, last - 2.0f);
    nglSetClearFlags(0);
    nglSetSceneCallBack(static_cast<nglSceneCallbackType>(0), shadow_scene_callback, nullptr);
    nglSetZWriteEnable(false);
    nglSetZTestEnable(false);
    nglSetFBWriteMask(8);
    nglSetAspectRatio(1.0f);
    nglSetOrthoMatrix(0.1f, 1000.0f);
    matrix4x4 view;
    geometry_manager::set_look_at(&view, field_C + field_0 * (m_radius * 100.0f),
                                  field_C, vector3d{0.0f, 0.0f, 1.0f});
    po camera_transform{view};
    auto projector = camera_transform.inverse()->m;
    projector[3] = field_C;
    nglSetWorldToViewMatrix({view});
    const float inverse_radius = 1.0f / m_radius;
    nglSetView(-inverse_radius, -inverse_radius, inverse_radius, inverse_radius);
    field_18->draw_projected_shadow(m_fade);
    destination.field_0 = projector;
    destination.field_40 = m_radius;
    destination.field_48 = true;
    nglListEndScene();
    copy_shadow_target(destination.field_44, unblurred);
    nglListSelectScene(previous_scene);
    clip_shadow_edges(destination.field_44);
    field_18 = nullptr;
    field_28 = 8999999500.0f;
}
