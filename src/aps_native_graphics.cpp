#include "aps_native_graphics.h"

#include "aeps.h"
#include "common.h"
#include "ngl.h"
#include "nglsortinfo.h"
#include "nglshader.h"
#include "variables.h"
#include <ngl_dx_scene.h>
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include <ngl_lighting.h>
#include <ngl_mesh.h>
#include <ngl_scene.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <new>
#include <cstring>
#include <stdexcept>
#include <functional>

extern nglDirLightInfo *nglGetLightAsDirLight(nglDirLightInfo *, nglLightNode *, math::VecClass<3, 1>);

namespace aps_native {
struct GraphicsVtable {
    void(__fastcall *post_load)(Graphics *, void *);
    void(__fastcall *unmash)(Graphics *, void *);
    void(__fastcall *serialize)(Graphics *, void *, void *, int);
    void *(__fastcall *destroy)(Graphics *, void *, unsigned);
    unsigned(__fastcall *type)(const Graphics *, void *);
    bool(__fastcall *base_type)(const Graphics *, void *, unsigned);
    bool(__fastcall *is_type)(const Graphics *, void *, unsigned);
    void(__fastcall *render)(Graphics *, void *, aeps::GroupRenderInfo *);
    unsigned(__fastcall *format)(const Graphics *, void *);
    unsigned(__fastcall *size)(const Graphics *, void *);
};
static_assert(offsetof(GraphicsVtable, render) == 0x1C);
}  // namespace aps_native

namespace {
using namespace aps_native;
using V = Float4;
V add(V a, V b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
V sub(V a, V b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
V mul(V a, V b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
V scale(V a, float b)
{
    return {a.x * b, a.y * b, a.z * b, a.w * b};
}
V vec(const vector3d &a)
{
    return {a.x, a.y, a.z, 0};
}
V row(const matrix4x4 &m, unsigned i)
{
    return {m[i].x, m[i].y, m[i].z, m[i].w};
}
float dot(V a, V b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
V cross(V a, V b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0};
}
V normalized(V a)
{
    const float length = std::sqrt(dot(a, a));
    return length > 0.0f ? scale(a, 1.0f / length) : V{0, 0, 0, 0};
}
V point(const matrix4x4 &m, V a)
{
    return add(add(scale(row(m, 0), a.x), scale(row(m, 1), a.y)), add(scale(row(m, 2), a.z), row(m, 3)));
}
matrix4x4 identity()
{
    matrix4x4 m;
    for (unsigned i = 0; i < 4; ++i)
        m[i][i] = 1;
    return m;
}
matrix4x4 product(const matrix4x4 &a, const matrix4x4 &b)
{
    matrix4x4 out;
    for (unsigned i = 0; i < 4; ++i)
        for (unsigned j = 0; j < 4; ++j)
            for (unsigned k = 0; k < 4; ++k)
                out[i][j] += a[i][k] * b[k][j];
    return out;
}
matrix4x4 transpose(const matrix4x4 &a)
{
    matrix4x4 out;
    for (unsigned i = 0; i < 4; ++i)
        for (unsigned j = 0; j < 4; ++j)
            out[i][j] = a[j][i];
    return out;
}


constexpr DWORD sprite_vs[]{0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x1f,       0x8000000a, 0x900f0001,
                            0x1f,       0x80000005, 0x900f0002, 0x9,        0xc0010000, 0x90e40000, 0xa0e40000,
                            0x9,        0xc0020000, 0x90e40000, 0xa0e40001, 0x9,        0xc0040000, 0x90e40000,
                            0xa0e40002, 0x9,        0xc0080000, 0x90e40000, 0xa0e40003, 0x1,        0xd00f0000,
                            0x90e40001, 0x1,        0xe00f0000, 0x90e40002, 0xffff};
constexpr DWORD particle_ps[]{0xffff0101, 0x42, 0xb00f0000, 0x5, 0x800f0000, 0xb0e40000, 0x90e40000, 0xffff};

constexpr DWORD mesh_vs[]{
    0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x1f,       0x80000003, 0x900f0001, 0x1f,       0x80000005,
    0x900f0002, 0x9,        0xc0010000, 0x90e40000, 0xa0e40000, 0x9,        0xc0020000, 0x90e40000, 0xa0e40001,
    0x9,        0xc0040000, 0x90e40000, 0xa0e40002, 0x9,        0xc0080000, 0x90e40000, 0xa0e40003, 0x1,
    0xe0030000, 0x90e40002, 0x8,        0x80010000, 0x90e40001, 0xa0e40004, 0x8,        0x80020000, 0x90e40001,
    0xa0e40005, 0x8,        0x80040000, 0x90e40001, 0xa0e40006, 0x8,        0x80080000, 0x90e40001, 0xa0e40007,
    0xb,        0x800f0000, 0x80e40000, 0xa000000d, 0x9,        0x80010002, 0x80e40000, 0xa0e40008, 0x9,
    0x80020002, 0x80e40000, 0xa0e40009, 0x9,        0x80040002, 0x80e40000, 0xa0e4000a, 0x2,        0xd0070000,
    0x80a40002, 0xa0a4000b, 0x1,        0xd0080000, 0xa0ff000c, 0xffff};
struct ShaderResources {
    VShader sprite{}, mesh{};
    IDirect3DPixelShader9 *pixel{};
    IDirect3DVertexDeclaration9 *fixed_sprite{}, *fixed_mesh{};
    ~ShaderResources()
    {
        if (sprite.field_0)
            IDirect3DVertexShader9_Release(sprite.field_0);
        if (sprite.field_4)
            IDirect3DVertexDeclaration9_Release(sprite.field_4);
        if (mesh.field_0)
            IDirect3DVertexShader9_Release(mesh.field_0);
        if (mesh.field_4)
            IDirect3DVertexDeclaration9_Release(mesh.field_4);
        if (pixel)
            IDirect3DPixelShader9_Release(pixel);
        if (fixed_sprite)
            IDirect3DVertexDeclaration9_Release(fixed_sprite);
        if (fixed_mesh)
            IDirect3DVertexDeclaration9_Release(fixed_mesh);
    }
};
ShaderResources &shaders(bool mesh)
{
    static ShaderResources resources;
    static const D3DVERTEXELEMENT9 sprite_elements[]{
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        {0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        D3DDECL_END()};
    static const D3DVERTEXELEMENT9 mesh_elements[]{
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
        {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        D3DDECL_END()};
    auto &vs = mesh ? resources.mesh : resources.sprite;
    auto &fixed = mesh ? resources.fixed_mesh : resources.fixed_sprite;
    const auto *elements = mesh ? mesh_elements : sprite_elements;
    if (EnableShader) {
        if (!vs.field_0) {
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, elements, &vs.field_4);
            IDirect3DDevice9_CreateVertexShader(g_Direct3DDevice, mesh ? mesh_vs : sprite_vs, &vs.field_0);
        }
        if (!resources.pixel)
            IDirect3DDevice9_CreatePixelShader(g_Direct3DDevice, particle_ps, &resources.pixel);
        nglSetVertexDeclarationAndShader(&vs);
        SetPixelShader(&resources.pixel);
    } else {
        if (!fixed)
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, elements, &fixed);
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, fixed);
        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    return resources;
}
void matrix_constants(const matrix4x4 &local)
{
    if (EnableShader) {
        const auto projected = transpose(product(local, nglCurScene->WorldToScreen));
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &projected[0][0], 4);
    } else {
        IDirect3DDevice9_SetTransform(g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&local));
    }
}
void sprite_state(const Graphics &graphics, unsigned blend, bool wrap)
{
    nglDxSetTexture(0, graphics.texture, 2, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    auto &s = g_renderState();
    s.setAlphaBlending(true);
    s.setAlphaTesting(true);
    s.setAlphaFunction(D3DCMP_GREATER);
    s.setAlphaReferenceValue(0);
    s.setCullingMode(D3DCULL_NONE);
    s.setBlendOperation(blend == 2 ? D3DBLENDOP_REVSUBTRACT : D3DBLENDOP_ADD);
    s.setSrcBlend(D3DBLEND_SRCALPHA);
    s.setDestBlend(blend == 1 || blend == 2 ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
}
struct Vertex {
    float x, y, z;
    std::uint32_t colour;
    float u, v;
};
static_assert(sizeof(Vertex) == 24);
Vertex vertex(V p, std::uint32_t c, float u, float v)
{
    return {p.x, p.y, p.z, c, u, v};
}
std::uint32_t colour(V c, bool saturate)
{
    auto lane = [saturate](float f) {
        const auto n = static_cast<std::uint32_t>(static_cast<std::int64_t>(f * 255.0f));
        return saturate ? std::min(n, 255u) : (n & 255u);
    };
    return (lane(c.w) << 24) | (lane(c.x) << 16) | (lane(c.y) << 8) | lane(c.z);
}
struct Batch {
    Vertex vertices[128 * 4];
    unsigned count = 0;
    static constexpr auto indices = [] {
        std::array<std::uint16_t, 128 * 6> a{};
        for (unsigned i = 0; i < 128; ++i) {
            a[6 * i] = 4 * i;
            a[6 * i + 1] = 4 * i + 1;
            a[6 * i + 2] = 4 * i + 2;
            a[6 * i + 3] = 4 * i + 2;
            a[6 * i + 4] = 4 * i + 1;
            a[6 * i + 5] = 4 * i + 3;
        }
        return a;
    }();
    void flush()
    {
        if (!count)
            return;
        IDirect3DDevice9_DrawIndexedPrimitiveUP(g_Direct3DDevice,
                                                D3DPT_TRIANGLELIST,
                                                0,
                                                4 * count,
                                                2 * count,
                                                indices.data(),
                                                D3DFMT_INDEX16,
                                                vertices,
                                                sizeof(Vertex));
        count = 0;
    }
    void quad(V a, V b, V c, V d, std::uint32_t packed, float u0, float v0, float u1, float v1)
    {
        auto *v = vertices + 4 * count;
        v[0] = vertex(a, packed, u0, v0);
        v[1] = vertex(b, packed, u0, v1);
        v[2] = vertex(c, packed, u1, v0);
        v[3] = vertex(d, packed, u1, v1);
        if (++count == 128)
            flush();
    }
};
struct Node : nglRenderNode {
    Graphics *graphics;
    unsigned type;
    aeps::GroupRenderInfo info;
    matrix4x4 transform;
};

void render_sprites(const Node &node)
{
    const auto &g = *reinterpret_cast<const SpriteGraphics *>(node.graphics);
    const auto &info = node.info;
    sprite_state(g.base, g.blend, false);
    shaders(false);
    matrix_constants(identity());
    const V forward{nglCurScene->ViewDir.x, nglCurScene->ViewDir.y, nglCurScene->ViewDir.z, 0};
    const V right = normalized(cross(forward, {0, 1, 0, 0}));
    const V up = normalized(cross(right, forward));
    V tint{1, 1, 1, 1};
    if (g.blend == 3 && info.light_context)
        tint = scale({info.light_context->Ambient.x, info.light_context->Ambient.y, info.light_context->Ambient.z, 0},
                     1.5f);
    Batch batch;
    auto *bytes = static_cast<const std::uint8_t *>(info.particles);
    for (int i = 0; i < info.particle_count; ++i, bytes += info.format->stride) {
        const auto *p = reinterpret_cast<const float *>(bytes);
        const bool rgba = node.type >= 1 && node.type <= 3;
        V c = rgba ? V{tint.x * p[4], tint.y * p[5], tint.z * p[6], p[7]} : V{tint.x, tint.y, tint.z, p[4]};
        const float angle = p[rgba ? 8 : 5];
        const float sn = std::sin(angle), cs = std::cos(angle);
        const float height = node.type == 2 ? p[9] : (node.type == 4 ? p[6] : p[3]);
        const V vertical = scale(add(scale(up, cs), scale(right, sn)), height);
        const V horizontal = scale(sub(scale(right, cs), scale(up, sn)), p[3]);
        const V center = point(node.transform, {p[0], p[1], p[2], 1});
        float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
        if (node.type == 3 || node.type == 6) {
            const auto &atlas = *reinterpret_cast<const AtlasGraphics *>(&g);
            const float frame = std::clamp(p[node.type == 3 ? 9 : 6], 0.0f, atlas.last_frame);
            const float inverse_columns = 1.0f / atlas.columns;
            v0 = static_cast<float>(static_cast<int>(inverse_columns * frame));
            u0 = (static_cast<int>(frame) - static_cast<int>(v0 * atlas.columns)) * inverse_columns;
            u1 = u0 + inverse_columns;
            v1 = v0 + 1;
        }
        batch.quad(add(add(center, vertical), horizontal),
                   add(sub(center, vertical), horizontal),
                   sub(add(center, vertical), horizontal),
                   sub(sub(center, vertical), horizontal),
                   colour(c, true),
                   u0,
                   v0,
                   u1,
                   v1);
    }
    batch.flush();
}


Var<std::uint32_t> particle_seed{0x0093AD40};
std::uint32_t random_step()
{
    particle_seed() = 1103515245u * particle_seed() + 12345u;
    return particle_seed();
}
void seed(std::uint32_t a)
{
    const auto b = ((((a >> 11) ^ a) & 0xFF3A58ADu) << 7) ^ (a >> 11) ^ a;
    particle_seed() = b ^ ((b & 0xFFFFDF8Cu) << 15);
}
V random_vector(V extent)
{
    const float x = (random_step() >> 16) * (1.0f / 65536.0f) * 2 - 1;
    const float y = (random_step() >> 16) * (1.0f / 65536.0f) * 2 - 1;
    const float z = (random_step() >> 16) * (1.0f / 65536.0f) * 2 - 1;
    return {x * extent.x, y * extent.y, z * extent.z, 0};
}
V sample(const V *keys, float lifetime)
{
    const float position = std::clamp(lifetime, 0.0f, 0.99f) * 15.0f;
    const auto index = static_cast<unsigned>(position);
    return add(keys[index], scale(sub(keys[index + 1], keys[index]), position - index));
}
void render_programmable(const Node &node)
{
    const auto &g = *reinterpret_cast<const ProgrammableGraphics *>(node.graphics);
    const auto &info = node.info;
    matrix4x4 transform = node.transform;
    V center = info.transform ? V{0, 0, 0, 0} : vec(info.center);
    V x = g.x_axis, y = g.y_axis;
    if (g.face_camera) {
        const auto view = product(transform, nglCurScene->WorldToView);
        x = {view[0][0], view[1][0], view[2][0], 0};
        y = {view[0][1], view[1][1], view[2][1], 0};
        float offset;
        std::memcpy(&offset, &info.field_34, sizeof(offset));
        if (std::not_equal_to<float>{}(offset, 0.0f)) {
            const V world = point(transform, center);
            const V camera{nglCurScene->ViewPos.x, nglCurScene->ViewPos.y, nglCurScene->ViewPos.z, 0};
            const V direction = sub(world, camera);
            const V shift = scale(direction, offset / std::sqrt(dot(direction, direction)));
            for (unsigned k = 0; k < 3; ++k)
                transform[3][k] += (&shift.x)[k];
        }
    }
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned j = 0; j < 3; ++j)
            transform[i][j] *= g.position_precision;
    transform[3][0] += center.x;
    transform[3][1] += center.y;
    transform[3][2] += center.z;
    shaders(false);
    matrix_constants(transform);
    auto &state = g_renderState();
    state.setCullingMode(D3DCULL_NONE);
    state.setColourBufferWriteEnabled(15);
    state.setDepthBufferWriteEnabled(g.blend < 2);
    state.setBlending(static_cast<nglBlendModeType>(g.blend), 0, g.alpha_reference);
    nglDxSetTexture(0, g.base.texture, static_cast<std::uint8_t>(1u << g.filter), 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, g.wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, g.wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
    const V scale_value{info.scale.x / g.position_precision,
                        info.scale.y / g.position_precision,
                        info.scale.z / g.position_precision,
                        1};
    const auto packed = static_cast<std::uint32_t>(info.field_38);
    const V tint{(packed & 255) / 255.0f,
                 ((packed >> 8) & 255) / 255.0f,
                 ((packed >> 16) & 255) / 255.0f,
                 (packed >> 24) / 255.0f};
    Batch batch;
    auto *bytes = static_cast<const std::uint8_t *>(info.particles);
    for (int i = 0; i < info.particle_count; ++i, bytes += info.format->stride) {
        auto f = [&](unsigned slot) {
            return *reinterpret_cast<const float *>(bytes + info.format->offsets[slot]);
        };
        const auto *p = reinterpret_cast<const float *>(bytes + info.format->offsets[0]);
        const V position = sub({p[0], p[1], p[2], 0}, center);
        seed(*reinterpret_cast<const std::uint32_t *>(bytes + info.format->offsets[20]));
        const float age = f(9), time = age * f(10) * g.time_scale + g.time_offset;
        for (int copy = static_cast<int>(g.copies); copy > 0; --copy) {
            const auto colour_index = random_step() >> 29;
            const V c = mul(mul(tint, g.colour.random[colour_index]), sample(g.colour.lifetime, age));
            const auto size_index = random_step() >> 29;
            const V size = add(mul(g.size.random[size_index], sample(g.size.lifetime, age)), g.size.bias);
            V pos = position;
            if (g.move == 1) {
                pos = add(pos, random_vector(g.position_random));
                pos = add(pos, scale(add(g.velocity, random_vector(g.velocity_random)), time));
                pos = add(pos, scale(g.acceleration, time * time));
                pos.x = std::clamp(pos.x, g.position_min.x, g.position_max.x);
                pos.y = std::clamp(pos.y, g.position_min.y, g.position_max.y);
                pos.z = std::clamp(pos.z, g.position_min.z, g.position_max.z);
            }
            pos = mul(pos, scale_value);
            V dx = x, dy = y;
            if (g.rotate == 1) {
                random_step();
                const auto &r = g.rotation[random_step() >> 29];
                float angle = (time * r.y + r.x) * 0.017453292519943295f;
                if (info.format->flags & (1u << 6))
                    angle -= g.particle_rotation_scale * f(6);
                const float sn = std::sin(angle), cs = std::cos(angle);
                dx = sub(scale(x, cs), scale(y, sn));
                dy = add(scale(y, cs), scale(x, sn));
            }
            dx = scale(dx, size.x * scale_value.x);
            dy = scale(dy, size.y * scale_value.y);
            int frame = 0;
            if (g.animate == 1) {
                random_step();
                const auto &r = g.frame[random_step() >> 29];
                frame = static_cast<int>(time * r.y + r.x);
                frame = static_cast<int>((static_cast<std::uint32_t>(frame) + (g.frame_count << 16)) % g.frame_count);
            }
            const V origin = sub(add(pos, scale(dy, size.w)), scale(dx, size.z));
            const unsigned left = EnableShader ? (static_cast<unsigned>(frame) & 15u) : static_cast<unsigned>(frame);
            const unsigned right =
                EnableShader ? (static_cast<unsigned>(frame + 1) & 15u) : static_cast<unsigned>(frame + 1);
            const float u0 = static_cast<float>(left) / g.frame_count, u1 = static_cast<float>(right) / g.frame_count;
            auto output = [](V value) {
                if (EnableShader) {
                    value.x = static_cast<std::int16_t>(static_cast<std::int64_t>(value.x));
                    value.y = static_cast<std::int16_t>(static_cast<std::int64_t>(value.y));
                    value.z = static_cast<std::int16_t>(static_cast<std::int64_t>(value.z));
                }
                return value;
            };

            batch.quad(output(add(origin, dx)),
                       output(sub(add(origin, dx), dy)),
                       output(origin),
                       output(sub(origin, dy)),
                       colour(c, false),
                       u1,
                       0,
                       u0,
                       1);
        }
    }
    batch.flush();
}

matrix4x4 quaternion_matrix(const float *p)
{
    const float x = p[4] * 1.4142135623730951f, y = p[5] * 1.4142135623730951f;
    const float z = p[6] * 1.4142135623730951f, w = p[7] * 1.4142135623730951f;
    matrix4x4 out = identity();
    out[0][0] = 1 - y * y - z * z;
    out[0][1] = x * y - w * z;
    out[0][2] = x * z + w * y;
    out[1][0] = x * y + w * z;
    out[1][1] = 1 - x * x - z * z;
    out[1][2] = y * z - w * x;
    out[2][0] = x * z - w * y;
    out[2][1] = y * z + w * x;
    out[2][2] = 1 - x * x - y * y;
    out[3][0] = p[0];
    out[3][1] = p[1];
    out[3][2] = p[2];
    return out;
}

void mesh_lights(const matrix4x4 &world, nglLightContext &context, V position, float radius, float alpha)
{
    auto &head = context.Head;
    head.SelectedNext = &head;
    for (auto *light = head.Next[1]; light && light != &head; light = light->Next[1]) {
        bool selected = light->Type == NGL_LIGHT_DIRECTIONAL;
        if (light->Type == NGL_LIGHT_POINT) {
            const auto &point_light = *static_cast<const nglPointLightInfo *>(light->Data);
            const V delta = sub(position, {point_light.ViewPos.x, point_light.ViewPos.y, point_light.ViewPos.z, 0});
            const float extent = radius + point_light.Far;
            selected = dot(delta, delta) <= extent * extent;
        }
        if (selected) {
            light->SelectedNext = head.SelectedNext;
            head.SelectedNext = light;
        }
    }
    float constants[9][4]{};
    unsigned count = 0;
    for (auto *light = head.SelectedNext; light && light != &head && count < 4; light = light->SelectedNext, ++count) {
        nglDirLightInfo converted;
        const auto &dir = *nglGetLightAsDirLight(&converted, light, {position.x, position.y, position.z, 1});
        const V negative{-dir.Dir.x, -dir.Dir.y, -dir.Dir.z, 0};
        for (unsigned j = 0; j < 3; ++j)
            constants[count][j] = dot(negative, row(world, j));
        constants[4][count] = dir.Color.x;
        constants[5][count] = dir.Color.y;
        constants[6][count] = dir.Color.z;
        if (!EnableShader) {
            D3DLIGHT9 d{};
            d.Type = D3DLIGHT_DIRECTIONAL;
            d.Diffuse = {dir.Color.x, dir.Color.y, dir.Color.z, 1};
            d.Direction = {dir.Dir.x, dir.Dir.y, dir.Dir.z};
            IDirect3DDevice9_SetLight(g_Direct3DDevice, count, &d);
            IDirect3DDevice9_LightEnable(g_Direct3DDevice, count, TRUE);
        }
    }
    constants[7][0] = context.Ambient.x;
    constants[7][1] = context.Ambient.y;
    constants[7][2] = context.Ambient.z;
    constants[7][3] = context.Ambient.w;
    constants[8][3] = alpha;
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &constants[0][0], 9);
        const float zero[4]{};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 13, zero, 1);
    } else {
        for (unsigned i = count; i < 4; ++i)
            IDirect3DDevice9_LightEnable(g_Direct3DDevice, i, FALSE);
        g_renderState().setLighting(true);
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice,
                                        D3DRS_AMBIENT,
                                        colour({context.Ambient.x, context.Ambient.y, context.Ambient.z, 1}, true));
        D3DMATERIAL9 material{};
        material.Diffuse = {1, 1, 1, alpha};
        material.Ambient = {1, 1, 1, alpha};
        IDirect3DDevice9_SetMaterial(g_Direct3DDevice, &material);
    }
}

void render_meshes(const Node &node)
{
    const auto &g = *reinterpret_cast<const MeshGraphics *>(node.graphics);
    auto *mesh = *reinterpret_cast<nglMesh *const *>(static_cast<const std::uint8_t *>(g.mesh_instance) + 0x138);
    const auto &info = node.info;
    auto *context = info.light_context ? info.light_context : nglDefaultLightContext();
    sprite_state(g.base, 0, true);
    shaders(true);
    const bool write = g_renderState().field_74;
    const bool lighting = g_renderState().field_A0;
    auto *bytes = static_cast<const std::uint8_t *>(info.particles);
    for (int i = 0; i < info.particle_count; ++i, bytes += info.format->stride) {
        const auto *p = reinterpret_cast<const float *>(bytes);
        const auto world = product(quaternion_matrix(p), node.transform);
        matrix_constants(world);
        g_renderState().setDepthBufferWriteEnabled(p[3] >= 0.4f);
        mesh_lights(world, *context, {p[0], p[1], p[2], 1}, mesh->SphereRadius, p[3]);
        nglSetStreamSourceAndDrawPrimitive(mesh->Sections[0].Section);
    }
    g_renderState().setDepthBufferWriteEnabled(write);
    if (!EnableShader)
        g_renderState().setLighting(lighting);
}

void __fastcall render_node(Node *node, void *)
{
    if (node->type == 7)
        render_programmable(*node);
    else if (node->type == 5)
        render_meshes(*node);
    else
        render_sprites(*node);
}
void __fastcall sort_node(Node *node, void *, nglSortInfo *sort)
{
    const bool programmable = node->type == 7;
    sort->Type =
        programmable
            ? static_cast<nglSortType>(reinterpret_cast<const ProgrammableGraphics *>(node->graphics)->list_type)
            : NGLSORT_TRANSLUCENT;
    const V center = programmable && node->info.transform ? row(node->transform, 3) : vec(node->info.center);
    sort->Dist = point(nglCurScene->WorldToView, center).z;
    if (!node->graphics->sort_by_distance) {
        const float bias = static_cast<float>(node->info.field_4) * 0.125f;
        sort->Dist += programmable ? -bias : bias;
    }
}
void enqueue(Graphics *graphics, unsigned type, aeps::GroupRenderInfo *info)
{
    if (type == 5 && !reinterpret_cast<const MeshGraphics *>(graphics)->mesh_instance)
        return;
    if (type != 7 && !nglIsSphereVisible({info->center.x, info->center.y, info->center.z, 1}, info->radius))
        return;
    static const std::intptr_t table[]{reinterpret_cast<std::intptr_t>(&render_node),
                                       reinterpret_cast<std::intptr_t>(&sort_node)};
    auto *node = new (nglListAlloc(sizeof(Node), 16)) Node;
    node->m_vtbl = reinterpret_cast<std::intptr_t>(table);
    node->graphics = graphics;
    node->type = type;
    node->info = *info;
    node->transform = info->transform ? *info->transform : identity();
    if (info->transform)
        node->info.transform = &node->transform;
    nglListAddNode(node);
}

constexpr unsigned formats[]{83, 91, 221, 347, 213, 49, 339, 0x100601};
constexpr unsigned sizes[]{16, 16, 16, 24, 16, 16, 24, 1344};


void __fastcall post_load(Graphics *, void *) {}
void __fastcall unmash(Graphics *, void *) {}


void __fastcall serialize(Graphics *, void *, void *, int) {}
void *__fastcall destroy(Graphics *object, void *, unsigned flags)
{
    if (flags & 1u)
        ::operator delete(object);
    return object;
}
bool __fastcall base_type(const Graphics *, void *, unsigned type)
{
    return type == 58 || type == 60;
}
template <unsigned Type>
unsigned __fastcall type_id(const Graphics *, void *)
{
    return Type;
}
template <unsigned Type>
unsigned __fastcall format(const Graphics *, void *)
{
    return formats[Type];
}
template <unsigned Type>
unsigned __fastcall object_size(const Graphics *, void *)
{
    return sizes[Type];
}
template <unsigned Type>
bool __fastcall is_type(const Graphics *, void *, unsigned type)
{
    return type == Type || type == 58 || type == 60;
}
template <unsigned Type>
void __fastcall render(Graphics *object, void *, aeps::GroupRenderInfo *info)
{
    enqueue(object, Type, info);
}
template <unsigned Type>
constexpr GraphicsVtable make_table()
{
    return {post_load,
            unmash,
            serialize,
            destroy,
            type_id<Type>,
            base_type,
            is_type<Type>,
            render<Type>,
            format<Type>,
            object_size<Type>};
}
const GraphicsVtable tables[]{make_table<0>(),
                              make_table<1>(),
                              make_table<2>(),
                              make_table<3>(),
                              make_table<4>(),
                              make_table<5>(),
                              make_table<6>(),
                              make_table<7>()};
#if STANDALONE_SYSTEM
struct AepsShader : nglShader {
    static void __fastcall Name(AepsShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{"AEPS"};
    }
    static bool __fastcall Switchable(AepsShader *, void *)
    {
        return false;
    }
    void Add(nglMeshNode *, nglMeshSection *, nglMaterialBase *) {}
    void Material(nglMaterialBase *) {}
    void Rebase(nglMaterialBase *, unsigned) {}
    AepsShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    AepsShader()
    {
        static void *table[]{func_address(&nglShader::_Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&AepsShader::Add),
                             func_address(&AepsShader::Material),
                             func_address(&AepsShader::Material),
                             func_address(&AepsShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&AepsShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
};
#endif
}  // namespace

#if STANDALONE_SYSTEM
void initialize_aeps_material_shader()
{
    static AepsShader shader;
}
#endif

void aps_native_graphics_fixup(void *object, unsigned type)
{
    if (type >= 8)
        throw std::invalid_argument("invalid APS graphics type");
    static_cast<aps_native::Graphics *>(object)->vtable = &tables[type];
}
unsigned aps_native_graphics_type(const void *object)
{
    const auto *g = static_cast<const aps_native::Graphics *>(object);
    return g->vtable->type(g, nullptr);
}
unsigned aps_native_graphics_format(const void *object)
{
    const auto *g = static_cast<const aps_native::Graphics *>(object);
    return g->vtable->format(g, nullptr);
}
void aps_native_graphics_render(void *object, aeps::GroupRenderInfo *info)
{
    auto *g = static_cast<aps_native::Graphics *>(object);
    g->vtable->render(g, nullptr, info);
}
