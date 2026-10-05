#include "polytubecustommaterial.h"

#include "common.h"
#include "us_tentacle.h"
#include "vtbl.h"
#include <algorithm>

VALIDATE_SIZE(PolytubeCustomMaterial, 0x30);

PolytubeCustomMaterial::PolytubeCustomMaterial(nglTexture *a2, nglBlendModeType a3, int a5)
    : PCUV_ShaderMaterial(a2, a3, 0, a5)
{
    this->field_1C = &a2->FileName;
    static void *table[]{func_address(&PolytubeCustomMaterial::destroy)};
    this->m_vtbl = reinterpret_cast<std::intptr_t>(table);
}

PolytubeCustomMaterial::PolytubeCustomMaterial(const PolytubeCustomMaterial &other)
    : PCUV_ShaderMaterial(other.m_texture, other.m_blend_mode, other.field_2C, other.field_28)
{
    static void *table[]{func_address(&PolytubeCustomMaterial::destroy)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
    if (m_texture)
        nglAddTextureRef(m_texture);
}

PolytubeCustomMaterial &PolytubeCustomMaterial::operator=(const PolytubeCustomMaterial &other)
{
    m_texture = other.m_texture;
    m_blend_mode = other.m_blend_mode;
    field_2C = other.field_2C;
    field_28 = other.field_28;
    if (m_texture)
        nglAddTextureRef(m_texture);
    return *this;
}

PolytubeCustomMaterial::~PolytubeCustomMaterial()
{
    if (m_texture)
        nglReleaseTexture(m_texture);
}

void *PolytubeCustomMaterial::destroy(unsigned char flags)
{
    this->~PolytubeCustomMaterial();
    if (flags & 1)
        operator delete(this);
    return this;
}

VALIDATE_SIZE(Tentacle_ShaderMaterial, 0x30);

Tentacle_ShaderMaterial::Tentacle_ShaderMaterial(nglTexture *texture, nglTexture *sphere_map, bool enabled)
    : shader(&getTentacle_Shader()), texture(texture), sphere_map(sphere_map), enabled(enabled)
{
    static void *table[]{func_address(&Tentacle_ShaderMaterial::destroy)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
}

Tentacle_ShaderMaterial::Tentacle_ShaderMaterial(const Tentacle_ShaderMaterial &other)
    : Tentacle_ShaderMaterial(other.texture, other.sphere_map, other.enabled)
{
    if (texture)
        nglAddTextureRef(texture);
    if (sphere_map)
        nglAddTextureRef(sphere_map);
}

Tentacle_ShaderMaterial &Tentacle_ShaderMaterial::operator=(const Tentacle_ShaderMaterial &other)
{
    texture = other.texture;
    sphere_map = other.sphere_map;
    enabled = other.enabled;
    if (texture)
        nglAddTextureRef(texture);
    if (sphere_map)
        nglAddTextureRef(sphere_map);
    return *this;
}

Tentacle_ShaderMaterial::~Tentacle_ShaderMaterial()
{
    if (texture)
        nglReleaseTexture(texture);
    if (sphere_map)
        nglReleaseTexture(sphere_map);
}

void *Tentacle_ShaderMaterial::destroy(unsigned char flags)
{
    this->~Tentacle_ShaderMaterial();
    if (flags & 1)
        operator delete(this);
    return this;
}

bool Tentacle_ShaderMaterial::SampleTentacle(int id, float percent, float &radius, float &angle)
{
    struct Sample { float distance, radius, angle; };
    static const Sample shape[7][8] = {
        {{0, .234f, 0}, {4.523f, .045f, 0}, {4.85f, .028f, 0}, {5, 0, 0}},
        {{0, .228f, 45}, {2.478f, .083f, 320}, {2.826f, 0, 345}},
        {{0, .113f, -160}, {1.672f, .041f, 115}, {1.874f, 0, 135}},
        {{0, .088f, -120}, {1.672f, .037f, -180}, {1.874f, 0, -190}},
        {{0, .081f, -5}, {.9f, .031f, 45}, {1.029f, 0, 50}},
        {{0, .2f, 0}, {.6f, 0, 0}},
        {{0, .21f, 0}, {2.2f, .2f, 0}, {2.4f, .11f, 0}, {2.6f, .19f, 0},
         {3.8f, .18f, 0}, {4, .09f, 0}, {4.25f, .16f, 0}, {5, 0, 0}}
    };
    static const int counts[]{4, 3, 3, 3, 3, 2, 8};
    const float distance = std::max(.0001f, std::min(1.0f, percent)) * shape[id][counts[id] - 1].distance;
    int index = 0;
    while (distance > shape[id][index].distance)
        ++index;
    const auto &previous = shape[id][index - 1];
    const auto &next = shape[id][index];
    const float interval = next.distance - previous.distance;
    radius = (next.radius - previous.radius) / interval * (distance - previous.distance) + previous.radius;
    angle = (next.angle - previous.angle) / interval * (distance - previous.distance) + previous.angle;
    return true;
}
