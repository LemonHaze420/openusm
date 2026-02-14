#include "ngl_dx_tex_create.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_dx_palette.h"
#include "osassert.h"
#include "trace.h"
#include "variable.h"
#include "variables.h"

#if !STANDALONE_SYSTEM
static auto & d3d_format_array = var<D3DFORMAT[50]>(0x0093C008);
#else
static auto & d3d_format_array = []() -> auto & {
    static int g_d3d_format_array[50] = {
        0, 21, 26, 25, 22, 23, 0, 41, 844715353, 50, 28, 51, 80, 75, 83, 71, 0, 0, 1065353216, 1065353216, 1065353216, 1065353216, 1078530011, 1078530011, 1078530011, 0, 1262485504, 0, 0, 0, 1262485504, 1262485504, 1262485504, 1262485504, 1065353216, 1065353216, 1065353216, 1065353216, 1078530011, 1078530011, 1078530011, 0, 1262485504, 0, 0, 0, 1262485504, 1262485504, 1262485504, 1262485504
    };
    return g_d3d_format_array;
}();
#endif

D3DFORMAT nglGetSwizzleTexFormat(unsigned int Format)
{
    if constexpr (0)
    {
        static D3DFORMAT Lookup[10];
        assert(NGLTEX_GET_FORMAT(Format) < sizeof(Lookup) && "nglGetSwizzleTexFormat() encountered invalid Format parameter.\n");
        assert(Lookup[NGLTEX_GET_FORMAT(Format)] != D3DFMT_UNKNOWN
        && "nglGetSwizzleTexFormat() encountered invalid Format parameter.\n");

        return Lookup[NGLTEX_GET_FORMAT(Format)];
    }
    else
    {
        return static_cast<D3DFORMAT>(d3d_format_array[NGLTEX_GET_FORMAT(Format)]);
    }
}

D3DFORMAT nglGetLinearTexFormat(uint32_t Format)
{
    if constexpr (0)
    {
        static D3DFORMAT Lookup[10];
        assert(NGLTEX_GET_FORMAT(Format) < sizeof(Lookup) &&
            "nglGetLinearTexFormat() encountered invalid Format parameter.\n");

        assert(Lookup[NGLTEX_GET_FORMAT(Format)] != D3DFMT_UNKNOWN &&
            "nglGetLinearTexFormat() encountered invalid Format parameter.\n");
        return Lookup[NGLTEX_GET_FORMAT(Format)];
    }
    else
    {
        return static_cast<D3DFORMAT>(d3d_format_array[NGLTEX_GET_FORMAT(Format)]);
    }
}

nglTexture *nglCreateTexture(uint32_t Format, int Width, int Height, int a4, bool a5)
{
    TRACE("nglCreateTexture");

    if constexpr (1)
    {
        assert(NGLTEX_GET_FORMAT(Format) != NGLTEX_ANIMATED &&
               "Cannot currently create animated textures using nglCreateTexture.");

        assert((Format & NGLTEX_SWIZZLED || Format & NGLTEX_LINEAR) &&
               "nglCreateTexture: you must specify if the texture to be created is swizzled or "
               "linear !");

        if ((Format & NGLTEX_SWIZZLED) != 0 && (!tlIsPow2(Width) || !tlIsPow2(Height))) {
            sp_log("Swizzled textures must have power of 2 dimensions !");
        }

        auto NMipmaps = a4 + 1;

        auto *tex = static_cast<nglTexture *>(tlMemAlloc(sizeof(nglTexture), 8, 0x1000000));
        if (tex != nullptr)
        {
            *tex = {};

#if !STANDALONE_SYSTEM
            static int & g_id_tex = var<int>(0x00975530);
#else
            static int & g_id_tex = []() -> auto & {
                static int g_id_tex1;
                return g_id_tex1;
            }();
#endif

            auto v11 = g_id_tex++;

            char Dest[128];
            sprintf(Dest, "created texture %d", v11);
            tex->field_4 = stru_975AC0.field_4;
            tex->field_0 = &stru_975AC0;
            stru_975AC0.field_4 = tex;
            tex->field_4->field_0 = tex;

            auto v7 = 2 * a5;
            int v8 = (Format & 0xFFFFFEFF) | NGLTEX_LINEAR;
            tex->field_34 = (v7 | tex->field_34) & 0xFFFFFFFD;
            tex->field_8 = 1;

            tex->FileName = tlFixedString {Dest};
            tex->m_numLevel = NMipmaps;
            tex->m_width = Width;
            tex->m_height = Height;
            tex->m_format = v8;

            if ( (Format & NGLTEX_SWIZZLED) != 0 )
            {
                tex->m_format |= NGLTEX_SWIZZLED;

                assert(tlIsPow2(Width) && tlIsPow2(Height) && "nglCreateTexture: trying to create a non-power of 2 SWIZZLED texture !");
                tex->m_d3d_format = nglGetSwizzleTexFormat(Format);
            }
            else if ( (Format & NGLTEX_LINEAR) != 0 )
            {
                assert(NMipmaps == 1 && "Linear textures cannot be mipmapped !");

                tex->m_format |= NGLTEX_LINEAR;
                tex->m_d3d_format = nglGetLinearTexFormat(Format);
            }

            if ( (Format & NGLTEX_CUBE) != 0 )
            {
                assert(Width == Height && "Cubemaps must have Width == Height !");

                assert(Format & NGLTEX_SWIZZLED && "Cubemaps cannot be linear !");
            }

            tex->CreateTextureOrSurface();
            tex->field_34 = (v7 | tex->field_34) & 0xFFFFFFFD;
            tex->SetupTextureLevels();

            if ((v8 & 0x1000u) != 0 && (v8 & 0x4000) != 0)
            {
                nglTexture *v9 {nullptr};
                if ((v8 & 0x400) != 0) {
                    v9 = nglCreateTexture(0x260Du, Width, Height, 0, 1);
                } else {
                    v9 = nglCreateTexture(0x220Du, Width, Height, 0, 1);
                }

                tex->field_44 = v9;
            }

            if (static_cast<uint8_t>(Format) == 7) {
                tex->field_48 = nglCreatePalette(0, 256u, nullptr);

                for (size_t i = 0; i < 256u; ++i) {
                    tex->field_48->sub_782A70(i, -1);
                }

                tex->field_48->sub_782A40();
            }

            tex->field_38 = -1;
        }

        return tex;
    } else {
        return bit_cast<nglTexture *>(CDECL_CALL(0x0077BC90, Format, Width, Height, a4, a5));
    }
}

