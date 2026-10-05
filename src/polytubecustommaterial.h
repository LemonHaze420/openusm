#pragma once

#include "us_pcuv_shader.h"

struct nglTexture;

struct PolytubeCustomMaterial : PCUV_ShaderMaterial {
    PolytubeCustomMaterial(nglTexture *a2, nglBlendModeType a3, int a5);
    PolytubeCustomMaterial(const PolytubeCustomMaterial &other);
    PolytubeCustomMaterial &operator=(const PolytubeCustomMaterial &other);
    ~PolytubeCustomMaterial();
    void *destroy(unsigned char flags);
};

struct Tentacle_ShaderMaterial {
    std::intptr_t m_vtbl;
    tlFixedString *name;
    nglShader *shader;
    nglMeshFile *file;
    nglMaterialBase *next;
    uint32_t version;
    int field_18;
    tlFixedString *texture_name;
    nglTexture *texture;
    tlFixedString *sphere_map_name;
    nglTexture *sphere_map;
    bool enabled;
    uint8_t field_2D;
    uint8_t field_2E;
    uint8_t field_2F;

    Tentacle_ShaderMaterial(nglTexture *texture, nglTexture *sphere_map, bool enabled);
    Tentacle_ShaderMaterial(const Tentacle_ShaderMaterial &other);
    Tentacle_ShaderMaterial &operator=(const Tentacle_ShaderMaterial &other);
    ~Tentacle_ShaderMaterial();
    void *destroy(unsigned char flags);
    nglMaterialBase *material() { return reinterpret_cast<nglMaterialBase *>(&name); }
    static bool SampleTentacle(int id, float percent, float &radius, float &angle);
};
