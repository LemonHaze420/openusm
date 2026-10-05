#include "ngl_lighting.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "utility.h"

#include <ngl_dx_scene.h>
#include "ngl_mesh.h"
#include "ngl_scene.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

VALIDATE_SIZE(nglDirProjectorLightInfo, 0x13C);
static_assert(offsetof(nglDirProjectorLightInfo, BlendMode) == 0x80);
static_assert(offsetof(nglDirProjectorLightInfo, Texture) == 0x88);
static_assert(offsetof(nglDirProjectorLightInfo, Planes) == 0xDC);

VALIDATE_SIZE(nglDirLightInfo, 0x20);

VALIDATE_SIZE(nglPointLightInfo, 0x28);

VALIDATE_SIZE(nglLightNode, 0x30);

VALIDATE_SIZE(nglLightContext, 0x70);

Var<nglLightContext *> nglDefaultLightContext{0x00973B70};

Var<nglLightContext *> nglCurLightContext{0x00973B74};


bool nglGetFakePointLight(nglDirLightInfo *a1, nglPointLightInfo *a2,
                          math::VecClass<3, 1, void, void, math::Rep_Std<false>> a3)
{
    const vector4d delta{a3.x - a2->ViewPos.x, a3.y - a2->ViewPos.y,
                         a3.z - a2->ViewPos.z, a3.w - a2->ViewPos.w};
    const float lengthSquared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    if (lengthSquared < 0.000001f) {
        a1->Dir = {0.0f, 1.0f, 0.0f, 0.0f};
        a1->Color = a2->Pos;
        return false;
    }
    const float inverseLength = 1.0f / std::sqrt(lengthSquared);
    const float attenuation = 1.0f - (inverseLength * lengthSquared - a2->Near) / (a2->Far - a2->Near);
    const float clamped = std::clamp(attenuation, 0.0f, 1.0f);
    a1->Color = {a2->Pos.x * clamped, a2->Pos.y * clamped,
                 a2->Pos.z * clamped, a2->Pos.w * clamped};
    a1->Dir = {delta.x * inverseLength, delta.y * inverseLength,
               delta.z * inverseLength, delta.w * inverseLength};
    return attenuation >= 0.0f;
}

nglDirLightInfo *nglGetLightAsDirLight(nglDirLightInfo *a1, nglLightNode *a2,
                                       math::VecClass<3, 1, void, void, math::Rep_Std<false>> a3)
{
    auto Type = a2->Type;
    if (Type) {
        if (Type == 1) {
            return static_cast<nglDirLightInfo *>(a2->Data);
        } else {
            return nullptr;
        }
    } else {
        nglGetFakePointLight(a1, static_cast<nglPointLightInfo *>(a2->Data), a3);
        return a1;
    }
}


void nglListAddLight(nglLightType Type, void *Data, uint32_t LightCat)
{
    auto *node = static_cast<nglLightNode *>(nglListAlloc(sizeof(nglLightNode), 16));
    if (node != nullptr) {
        node->Data = Data;
        node->Type = Type;
        node->LightCat = LightCat;
        for (int i = 0; i < NGL_MAX_LIGHTS; ++i) {
            if (LightCat & (1u << (i + NGL_LIGHTCAT_SHIFT))) {
                node->Next[i] = nglCurLightContext()->Head.Next[i];
                nglCurLightContext()->Head.Next[i] = node;
            }
        }
    }
}


void nglListAddPointLight(uint32_t LightCat, math::VecClass<3, 1> a2, Float a6, Float radius, math::VecClass<4, -1> a8)
{
    if (nglIsSphereVisible(a2, radius)) {
        auto *Light = static_cast<nglPointLightInfo *>(nglListAlloc(sizeof(nglPointLightInfo), 16));
        if (Light != nullptr) {
            Light->ViewPos = a2;
            Light->Pos[0] = a8[0];
            Light->Pos[1] = a8[1];
            Light->Pos[2] = a8[2];
            Light->Pos.w = 1.0f;
            Light->Near = a6;
            Light->Far = radius;
            nglListAddLight(NGL_LIGHT_POINT, Light, LightCat);
        }
    }
}

void nglListAddDirLight(unsigned int a2, math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> a3,
                        math::VecClass<4, -1, void, void, math::Rep_Std<false>> a4)
{
    auto *Light = static_cast<nglDirLightInfo *>(nglListAlloc(sizeof(nglDirLightInfo), 16));
    if (Light != nullptr) {
        Light->Dir = a3;
        Light->Color[0] = a4[0];
        Light->Color[1] = a4[1];
        Light->Color[2] = a4[2];
        Light->Color.w = 1.0f;
        nglListAddLight(NGL_LIGHT_DIRECTIONAL, Light, a2);
    }

}

namespace {
float projectorDot(const vector4d &a, const vector4d &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vector4d projectorNormal(const vector4d &a, const vector4d &b)
{
    vector4d n{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
               a.x * b.y - a.y * b.x, 0.0f};
    const float squared = projectorDot(n, n);
    const float scale = (squared <= 0.0f && squared >= 0.0f)
        ? 0.0f : 1.0f / std::sqrt(std::fabs(squared));
    return {n.x * scale, n.y * scale, n.z * scale, 0.0f};
}

void projectorPlanePair(vector4d *planes, const vector4d &normal,
                        const vector4d &position, const vector4d &axis)
{
    const float distance = projectorDot(position, normal);
    planes[0] = {normal.x, normal.y, normal.z, -distance};
    planes[1] = {-normal.x, -normal.y, -normal.z, distance + projectorDot(axis, normal)};
}
}

void nglListAddDirProjectorLight(uint32_t lightCat, const matrix4x4 &localToWorld,
                               float width, float height, float depth, float unusedW,
                               int blendMode, uint32_t color, nglTexture *texture)
{
    auto *light = static_cast<nglDirProjectorLightInfo *>(
        nglListAlloc(sizeof(nglDirProjectorLightInfo), 16));
    if (!light)
        return;

    light->BlendMode = blendMode;
    light->Color = color;

    if (texture)
        light->Texture = texture;
    light->Extent = {width, height, depth, unusedW};
    light->Position = localToWorld.w;
    light->XAxis = localToWorld.arr[0];
    light->YAxis = localToWorld.arr[1];
    light->ZAxis = localToWorld.arr[2];




    const vector4d inverse{1.0f / width, 1.0f / height, 1.0f / depth, 0.0f};
    for (int i = 0; i < 3; ++i) {
        light->WorldToUV.arr[i] = {localToWorld.arr[0][i] * inverse.x,
                                  localToWorld.arr[1][i] * inverse.y,
                                  localToWorld.arr[2][i] * inverse.z, 0.0f};
        const float extent = light->Extent[i];
        light->UVToWorld.arr[i] = {localToWorld.arr[i].x * extent,
                                  localToWorld.arr[i].y * extent,
                                  localToWorld.arr[i].z * extent, 0.0f};
    }
    const auto &position = localToWorld.w;
    light->WorldToUV.w = {
        -projectorDot(position, localToWorld.arr[0]) * inverse.x + 0.5f,
        -projectorDot(position, localToWorld.arr[1]) * inverse.y + 0.5f,
        -projectorDot(position, localToWorld.arr[2]) * inverse.z, 1.0f};
    light->UVToWorld.w = {
        position.x - 0.5f * light->UVToWorld.arr[0].x - 0.5f * light->UVToWorld.arr[1].x,
        position.y - 0.5f * light->UVToWorld.arr[0].y - 0.5f * light->UVToWorld.arr[1].y,
        position.z - 0.5f * light->UVToWorld.arr[0].z - 0.5f * light->UVToWorld.arr[1].z, 1.0f};
    const auto &basis = light->UVToWorld;
    projectorPlanePair(&light->Planes[4], projectorNormal(basis.arr[0], basis.arr[1]),
                       basis.w, basis.arr[2]);
    projectorPlanePair(&light->Planes[2], projectorNormal(basis.arr[1], basis.arr[2]),
                       basis.w, basis.arr[0]);
    vector4d yPlanes[2];
    projectorPlanePair(yPlanes, projectorNormal(basis.arr[2], basis.arr[0]),
                       basis.w, basis.arr[1]);
    light->Planes[1] = yPlanes[0];
    light->Planes[0] = yPlanes[1];

    auto *node = static_cast<nglLightNode *>(nglListAlloc(sizeof(nglLightNode), 16));
    if (!node)
        return;
    node->Data = light;
    node->Type = NGL_LIGHT_DIR_PROJECTOR;
    node->LightCat = lightCat;
    for (int i = 0; i < NGL_MAX_LIGHTS; ++i) {
        if (lightCat & (1u << (i + NGL_LIGHTCAT_SHIFT))) {
            node->Next[i] = nglCurLightContext()->ProjectorHead.Next[i];
            nglCurLightContext()->ProjectorHead.Next[i] = node;
        }
    }
}

bool nglProjectorSphereVisible(const nglDirProjectorLightInfo &light,
                               const vector4d &center, float radius)
{

    for (const int index : {2, 3, 4, 0, 1, 5}) {
        const auto &plane = light.Planes[index];
        if (projectorDot(plane, center) + plane.w < -radius)
            return false;
    }
    return true;
}

void nglDetermineProjLights(nglMeshNode *node)
{
    auto *context = node->field_8C.IsSetParam<nglLightContextParam>()
        ? node->field_8C.Get<nglLightContextParam>()->field_0
        : nglCurScene->field_350;
    nglCurLightContext() = context;
    auto *head = &context->ProjectorHead;
    head->SelectedNext = head;
    const uint32_t category = node->Mesh->Flags >> NGL_LIGHTCAT_SHIFT;
    if (!category)
        return;
    const auto &matrix = node->LocalToWorld;
    const auto &center = node->Mesh->SphereCenter;
    const vector4d worldCenter{
        center.x * matrix.arr[0].x + center.y * matrix.arr[1].x + center.z * matrix.arr[2].x + matrix.w.x,
        center.x * matrix.arr[0].y + center.y * matrix.arr[1].y + center.z * matrix.arr[2].y + matrix.w.y,
        center.x * matrix.arr[0].z + center.y * matrix.arr[1].z + center.z * matrix.arr[2].z + matrix.w.z, 1.0f};
    for (auto *light = head->Next[category - 1]; light != head; light = light->Next[category - 1]) {
        if (light->Type == NGL_LIGHT_DIR_PROJECTOR &&
            nglProjectorSphereVisible(*static_cast<nglDirProjectorLightInfo *>(light->Data),
                                      worldCenter, node->Mesh->SphereRadius)) {
            light->SelectedNext = head->SelectedNext;
            head->SelectedNext = light;
        }
    }
}

void ngl_lighting_patch()
{
    SET_JUMP(0x00776140, nglListAddPointLight);

    SET_JUMP(0x007760C0, nglListAddDirLight);
}
