#include "us_colorvol.h"

#include "common.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "nglsortinfo.h"
#include "utility.h"
#include "variables.h"
#include "ngl_dx_shader.h"
#include "ngl_dx_state.h"
#include "ngl_dx_texture.h"

#include <new>

namespace USColorVolShaderSpace {
namespace {
Var<VShader> vertex_shader{0x00970570};
Var<IDirect3DPixelShader9 *> pixel_shader{0x00970560};
Var<int> suppress_render{0x00956974};



constexpr DWORD vertex_program[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000,
    0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000,
    0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002,
    0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003,
    0x0000FFFF,
};
constexpr DWORD pixel_program[] = {
    0xFFFF0101, 0x00000001, 0x800F0000, 0x90E40000, 0x0000FFFF,
};
const D3DVERTEXELEMENT9 vertex_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    D3DDECL_END(),
};

void __fastcall shader_get_name(USColorVolShader *shader, void *, tlFixedString *out)
{
    *out = shader->_GetName();
}



void __fastcall material_callback(USColorVolShader *, void *, nglMaterialBase *) {}
void __fastcall rebase_callback(USColorVolShader *, void *, nglMaterialBase *, unsigned int) {}
void __fastcall section_callback(USColorVolShader *, void *, nglMeshSection *) {}

bool __fastcall material_version(USColorVolShader *, void *, nglMaterialBase *) { return true; }
bool __fastcall vertex_version(USColorVolShader *, void *, nglMeshSection *) { return true; }
bool __fastcall switchable(USColorVolShader *, void *) { return true; }

vector3d transform_point(const vector3d &point, const matrix4x4 &matrix)
{
    return vector3d{
        matrix[0].x * point.x + matrix[1].x * point.y + matrix[2].x * point.z + matrix[3].x,
        matrix[0].y * point.x + matrix[1].y * point.y + matrix[2].y * point.z + matrix[3].y,
        matrix[0].z * point.x + matrix[1].z * point.y + matrix[2].z * point.z + matrix[3].z};
}
}

VALIDATE_SIZE(USColorVolShader, 0xC);
VALIDATE_SIZE(USColorVolNode, 0x18);

bool color_volumes_enabled()
{
    return g_enable_colorvols() && volume_time_of_day().enabled[g_TOD];
}


void USColorVolPreSceneCallback(unsigned int *&, void *) {}

void USColorVolPostSceneCallback(unsigned int *&, void *) {}

USColorVolShader::USColorVolShader()
{
    static void *table[] = {
        func_address(&USColorVolShader::Register), reinterpret_cast<void *>(shader_get_name),
        func_address(&USColorVolShader::_AddNode), reinterpret_cast<void *>(material_callback),
        reinterpret_cast<void *>(material_callback), reinterpret_cast<void *>(rebase_callback),
        reinterpret_cast<void *>(material_version), reinterpret_cast<void *>(vertex_version),
        reinterpret_cast<void *>(section_callback), reinterpret_cast<void *>(switchable),
    };
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

USColorVolShader &getUSColorVolShader()
{
    static Var<USColorVolShader> shader{0x0091E454};
    return shader();
}

tlFixedString USColorVolShader::_GetName() const
{
    return tlFixedString{"USColorVol"};
}

void USColorVolShader::Register()
{
    nglShader::_Register();
    if (EnableShader) {
        nglCreateVertexDeclarationAndShader(&vertex_shader(), vertex_elements, vertex_program);
        CreatePixelShader(&pixel_shader(), pixel_program);
    } else if (dword_9738E0[18] == nullptr) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, vertex_elements, &dword_9738E0[18]);
    }
}

USColorVolNode::USColorVolNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
    : nglShaderNode(mesh, section), material(mat)
{
    static void *table[] = {func_address(&USColorVolNode::Render),
        func_address(&USColorVolNode::GetSortInfo), func_address(&USColorVolNode::Delete)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

void USColorVolNode::Delete(unsigned char flags)
{
    if ((flags & 1) != 0)
        ::operator delete(this);
}

void USColorVolShader::_AddNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
{
    if (!color_volumes_enabled())
        return;
    auto *node = new (nglListAlloc(sizeof(USColorVolNode), 16)) USColorVolNode{mesh, section, mat};
    auto *scene = nglListSelectScene(gUSColorVolScene());
    nglListAddNode(node);
    nglListSelectScene(scene);
}

float USColorVolNode::GetNearestDist() const
{
    const auto &center = m_meshSection->SphereCenter;
    const auto world = transform_point(
        vector3d{center.x, center.y, center.z}, m_meshNode->LocalToWorld);
    const auto view = transform_point(world, nglCurScene->WorldToView);
    return view.z - m_meshSection->SphereRadius;
}

void USColorVolNode::GetSortInfo(nglSortInfo &info)
{
    info.Type = NGLSORT_TRANSLUCENT;
    info.Dist = GetNearestDist();
    if ((reinterpret_cast<uintptr_t>(material->field_18) & 0x40u) != 0) {

        info.Dist -= 10000.0f;
    }
}

void USColorVolNode::Render()
{
    if (suppress_render())
        return;
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
        nglSetVertexDeclarationAndShader(&vertex_shader());
        SetPixelShader(&pixel_shader());
    } else {
        IDirect3DDevice9_SetTransform(g_Direct3DDevice, D3DTS_WORLD,
            reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[18]);
        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    auto &state = g_renderState();
    const auto write_mask = state.field_A8;
    const auto depth_enabled = state.field_78 != D3DZB_FALSE;
    const auto depth_function = state.field_7C;
    state.setBlending(NGLBM_OPAQUE, 0, 0);
    state.setDepthBuffer(D3DZB_TRUE);
    state.setDepthBufferFunction(D3DCMP_LESSEQUAL);
    state.setColourBufferWriteEnabled(0);
    state.setAlphaTesting(false);
    state.setAlphaBlending(false);
    state.setStencilCheckEnabled(true);
    state.setStencilPassOperation(D3DSTENCILOP_KEEP);
    state.setStencilFailOperation(D3DSTENCILOP_KEEP);

    state.setStencilRefValue(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(material->field_18)) | 0x80u);
    state.setCullingMode(D3DCULL_CCW);
    state.setStencilBufferTestFunction(D3DCMP_GREATEREQUAL);
    state.setStencilBufferCompareMask(0xFFFFFF80u);
    state.setStencilBufferWriteMask(0xFFFFFFFFu);
    state.setStencilDepthFailOperation(D3DSTENCILOP_REPLACE);
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    state.setCullingMode(D3DCULL_CW);
    state.setStencilBufferTestFunction(D3DCMP_EQUAL);
    state.setStencilBufferCompareMask(0xFFFFFFFFu);
    state.setStencilBufferWriteMask(0xFFFFFFFFu);
    state.setStencilDepthFailOperation(D3DSTENCILOP_ZERO);
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    state.setDepthBuffer(depth_enabled ? D3DZB_TRUE : D3DZB_FALSE);
    state.setDepthBufferFunction(depth_function);
    state.setColourBufferWriteEnabled(write_mask);
}

}
