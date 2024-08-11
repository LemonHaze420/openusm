#include "ngl_dx_palette.h"

#include "ngl.h"
#include "variables.h"

nglPalette *nglCreatePalette([[maybe_unused]] int a1, uint32_t entries, const void *a3)
{
    auto *v3 = static_cast<nglPalette *>(tlMemAlloc(sizeof(nglPalette), 8, 0x1000000));
    v3->m_palette_idx = 0;
    v3->m_palette_entries = nullptr;
    v3->m_size = 0;
    v3->m_size = entries;

    v3->m_palette_entries = static_cast<PALETTEENTRY *>(
        tlMemAlloc(sizeof(PALETTEENTRY) * entries, 8, 0x1000000));
    v3->m_palette_idx = dword_975BE8()[dword_975BE0()--];

    if ( a3 != nullptr ) {
        std::memcpy(v3->m_palette_entries, a3, 4 * ((4 * entries) >> 2));
    }

    if ( !g_valid_texture_format ) {
        IDirect3DDevice9_SetPaletteEntries(g_Direct3DDevice,
                                                      v3->m_palette_idx,
                                                      v3->m_palette_entries);
    }

    return v3;
}

