#include "ngl_dx_draw.h"

#include "func_wrapper.h"

#include <ngl.h>
#include <ngl_mesh.h>
#include "fixedstring.h"
#include <trace.h>
#include <variables.h>

#include <cstdint>

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>


static Var<int> g_MinVertexIndex{0x009729B0};

static Var<IDirect3DVertexBuffer9 *> dword_972964{0x00972964};

HRESULT nglDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, IDirect3DIndexBuffer9 *a2, UINT startIndex,
                                UINT NumIndices, UINT NumVertices)
{
    TRACE("nglDrawIndexedPrimitive");

    UINT primCount;

    static Var<IDirect3DIndexBuffer9 *> dword_972968{0x00972968};

    if (a2 != dword_972968()) {
        IDirect3DDevice9_SetIndices(g_Direct3DDevice, a2);
        dword_972968() = a2;
    }

    switch (PrimitiveType) {
    case D3DPT_POINTLIST:
        primCount = NumIndices;
        break;
    case D3DPT_LINELIST:
        primCount = NumIndices >> 1;
        break;
    case D3DPT_LINESTRIP:
        primCount = NumIndices - 1;
        break;
    case D3DPT_TRIANGLELIST:
        primCount = NumIndices / 3;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        primCount = NumIndices - 2;
        break;
    default:
        primCount = 0;
        break;
    }
    return IDirect3DDevice9_DrawIndexedPrimitive(
        g_Direct3DDevice, PrimitiveType, 0, g_MinVertexIndex(), NumVertices, startIndex, primCount);
}

HRESULT nglDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, int a2, UINT a3)
{
    UINT v3;

    switch (PrimitiveType) {
    case D3DPT_POINTLIST:
        v3 = a3;
        break;
    case D3DPT_LINELIST:
        v3 = a3 >> 1;
        break;
    case D3DPT_LINESTRIP:
        v3 = a3 - 1;
        break;
    case D3DPT_TRIANGLELIST:
        v3 = a3 / 3;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        v3 = a3 - 2;
        break;
    default:
        v3 = 0;
        break;
    }

    return IDirect3DDevice9_DrawPrimitive(g_Direct3DDevice, PrimitiveType, a2 + g_MinVertexIndex(), v3);
}

HRESULT nglSetStreamSourceAndDrawPrimitive(D3DPRIMITIVETYPE type, IDirect3DVertexBuffer9 *buffer, uint32_t numVertices,
                                           uint32_t baseVertexIndex, uint32_t stride,
                                           IDirect3DIndexBuffer9 *indexBuffer, uint32_t numIndices, uint32_t startIndex)
{
    IDirect3DDevice9_SetStreamSource(g_Direct3DDevice, 0, buffer, 0, stride);
    dword_972964() = buffer;
    if (numIndices != 0 && indexBuffer != nullptr) {
        static Var<IDirect3DIndexBuffer9 *> current_indices{0x00972968};
        if (current_indices() != indexBuffer) {
            IDirect3DDevice9_SetIndices(g_Direct3DDevice, indexBuffer);
            current_indices() = indexBuffer;
        }
        uint32_t count{};
        switch (type) {
        case D3DPT_POINTLIST:
            count = numIndices;
            break;
        case D3DPT_LINELIST:
            count = numIndices / 2;
            break;
        case D3DPT_LINESTRIP:
            count = numIndices - 1;
            break;
        case D3DPT_TRIANGLELIST:
            count = numIndices / 3;
            break;
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            count = numIndices - 2;
            break;
        default:
            break;
        }
        return IDirect3DDevice9_DrawIndexedPrimitive(
            g_Direct3DDevice, type, baseVertexIndex, startIndex, numVertices, startIndex, count);
    }
    uint32_t count{};
    switch (type) {
    case D3DPT_POINTLIST:
        count = numVertices;
        break;
    case D3DPT_LINELIST:
        count = numVertices / 2;
        break;
    case D3DPT_LINESTRIP:
        count = numVertices - 1;
        break;
    case D3DPT_TRIANGLELIST:
        count = numVertices / 3;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        count = numVertices - 2;
        break;
    default:
        break;
    }
    return IDirect3DDevice9_DrawPrimitive(g_Direct3DDevice, type, baseVertexIndex, count);
}


HRESULT nglSetStreamSourceAndDrawPrimitive(nglMeshSection *MeshSection)
{
    uint32_t stride = MeshSection->m_stride;
    g_MinVertexIndex() = MeshSection->field_4C / stride;
    auto *vertexBuffer = MeshSection->field_3C.getVertexBuffer();
    IDirect3DDevice9_SetStreamSource(g_Direct3DDevice, 0, vertexBuffer, 0, stride);
    dword_972964() = vertexBuffer;

    HRESULT result;

    auto numIndices = MeshSection->NIndices;
    if (numIndices != 0) {
        result = nglDrawIndexedPrimitive(MeshSection->m_primitiveType,
                                         MeshSection->m_indexBuffer,
                                         MeshSection->StartIndex,
                                         MeshSection->NIndices,
                                         MeshSection->NVertices);
    } else {
        result = nglDrawPrimitive(MeshSection->m_primitiveType, 0, MeshSection->NVertices);
    }

    return result;
}

void SetRenderTarget(nglTexture *texture, nglTexture *depth_texture, int level, int face)
{
#if STANDALONE_SYSTEM
    IDirect3DSurface9 *render_target{};
    if (face == 6) {
        render_target = texture->DXSurfaces[level];
    } else {
        auto **cube_surfaces = reinterpret_cast<IDirect3DSurface9 **>(texture->DXSurfaces[face]);
        render_target = cube_surfaces[level];
    }

    IDirect3DSurface9 *depth_surface{};
    if (depth_texture != nullptr) {
        depth_surface = reinterpret_cast<IDirect3DSurface9 *>(depth_texture->DXSurfaces);
    } else if (texture->field_44 != nullptr) {
        depth_surface = reinterpret_cast<IDirect3DSurface9 *>(texture->field_44->DXSurfaces);
    }

    IDirect3DDevice9_SetRenderTarget(g_Direct3DDevice, 0, render_target);
    IDirect3DDevice9_SetDepthStencilSurface(g_Direct3DDevice, depth_surface);
#else
    CDECL_CALL(0x00771970, texture, depth_texture, level, face);
#endif
}
