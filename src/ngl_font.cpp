#include "ngl_font.h"

#include "log.h"
#include "ngl.h"
#include "osassert.h"
#include "trace.h"
#include "utility.h"

Var<nglFont *> nglSysFont = {0x00975208};

//0x00778ED0
int nglGetTokenU32(char *&a1, const char *Token, uint32_t Base)
{
    auto TokenLength = strlen(Token);
    while (*a1 != '\0') {
        uint32_t i;
        for (i = 0; i < TokenLength && (a1)[i] && (a1)[i] == Token[i]; ++i) {
            ;
        }

        if (i == TokenLength) {
            a1 += TokenLength;
            return strtol(a1, &a1, Base);
        }

        ++a1;
    }

    return 0;
}

void nglParseFDF(char *a3, nglFont *font)
{
    TRACE("nglParseFDF");
    font->Header.Version = nglGetTokenU32(a3, "version", 16);

    static constexpr auto NGLFONT_VERSION = 258;

    if (font->Header.Version != NGLFONT_VERSION) {
        error("Unsupported font version %d.\n", font->Header.Version);
    }

    font->Header.CellHeight = nglGetTokenU32(a3, "cellheight", 10);

    assert(font->Header.CellHeight != 0 && "Height must not be zero.\n");

    font->Header.Ascent = nglGetTokenU32(a3, "ascent", 10);

    assert(font->Header.Ascent != 0 && "Ascent must not be zero.\n");

    font->Header.FirstGlyph = nglGetTokenU32(a3, "firstglyph", 10);

    assert(font->Header.FirstGlyph != 0 && "FirstGlyph must not be zero.\n");

    font->Header.NumGlyphs = nglGetTokenU32(a3, "numglyphs", 10);

    assert(font->Header.NumGlyphs > 0 && "NumGlyphs must not be zero.\n");

    font->GlyphInfo = static_cast<decltype(font->GlyphInfo)>(
        tlMemAlloc(sizeof(nglGlyphInfo) * font->Header.NumGlyphs, 8u, 0x1000000u));
    font->field_4C =
        static_cast<decltype(font->field_4C)>(tlMemAlloc(sizeof(nglGlyphSize) * font->Header.NumGlyphs, 4u, 0));
    auto *tex = font->field_24;
    float width = 1.0f / tex->m_width;

    float height = 1.0f / tex->m_height;

    for (int i = 0; i < font->Header.NumGlyphs; ++i) {
        auto *v6 = a3;
        for (auto ch = *a3; *v6; ch = *v6) {
            if (ch >= '0' && ch <= '9') {
                break;
            }

            a3 = ++v6;
        }

        auto n = strtol(v6, &a3, 10);

        assert(n == i + font->Header.FirstGlyph && "Character out of sequence in FDF file.\n");

        auto *v8 = &font->GlyphInfo[(n - font->Header.FirstGlyph)];
        v8->TexOfs[0] = nglGetTokenU32(a3, "x", 10);
        v8->TexOfs[1] = nglGetTokenU32(a3, "y", 10);
        v8->CellWidth = nglGetTokenU32(a3, "w", 10);
        v8->GlyphOrigin[0] = nglGetTokenU32(a3, "gx", 10);
        v8->GlyphOrigin[1] = nglGetTokenU32(a3, "gy", 10);
        v8->GlyphSize[0] = nglGetTokenU32(a3, "gw", 10);
        v8->GlyphSize[1] = nglGetTokenU32(a3, "gh", 10);

        auto &v11 = font->field_4C[i];
        v11.field_0 = (v8->TexOfs[0] - 1) * width;
        v11.field_4 = (v8->TexOfs[1] - 1) * height;
        v11.field_8 = v8->GlyphSize[0] * width + v11.field_0;
        v11.field_C = v8->GlyphSize[1] * height + v11.field_4;
    }
}

nglGlyphInfo *nglFont::GetGlyphInfo(unsigned char Character)
{
    if (Character < this->Header.FirstGlyph ||
        Character >= this->Header.NumGlyphs + this->Header.FirstGlyph) {
        return &this->GlyphInfo[32 - this->Header.FirstGlyph];
    }

    assert(Character >= Header.FirstGlyph && Character < Header.FirstGlyph + Header.NumGlyphs &&
           "Out of range character in string.");

    auto *glyph = &this->GlyphInfo[Character - this->Header.FirstGlyph];
    if (Character != ' ' && glyph->GlyphSize[0] == 0) {
        return &this->GlyphInfo['?' - this->Header.FirstGlyph];
    }

    return glyph;
}

void nglFont::sub_77E2F0(uint8_t a2, float *a3, float *a4, float *a5, float *a6, Float a7, Float a8)
{
    auto FirstGlyph = this->Header.FirstGlyph;
    auto v10 = a2;
    if (a2 < FirstGlyph || a2 >= FirstGlyph + this->Header.NumGlyphs) {
        v10 = 32;
    }

    nglGlyphInfo v12 = this->GlyphInfo[v10 - FirstGlyph];
    a3[0] = (double)v12.GlyphOrigin[0] * a7;
    a3[1] = (double)v12.GlyphOrigin[1] * a8;

    a4[0] = (double)v12.GlyphSize[0] * a7;
    a4[1] = (double)v12.GlyphSize[1] * a8;

    auto v11 = (uint8_t)(a2 - static_cast<uint8_t>(this->Header.FirstGlyph));
    a5[0] = this->field_4C[v11].field_0;
    a5[1] = this->field_4C[v11].field_4;

    a6[0] = this->field_4C[v11].field_8 - this->field_4C[v11].field_0;
    a6[1] = this->field_4C[v11].field_C - this->field_4C[v11].field_4;
}
