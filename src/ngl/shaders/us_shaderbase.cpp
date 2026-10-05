#include "us_shaderbase.h"

#include "trace.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "ngl_mesh.h"
#include "ngl_dx_state.h"
#include "color.h"
#include "fixedstring.h"
#include "variables.h"
#include <algorithm>
#include <cstring>

void nglInitShaderLighting()
{
    static Var<uint32_t[4][4]> colors{0x0091E028};
    static Var<uint32_t[4][4]> alternate_colors{0x0091E068};
#if STANDALONE_SYSTEM
    static const bool initialized = [&] {
        const uint32_t primary[4][4]{
            {0x80AEAEAE, 0x80B9B9B9, 0x80D3D3D3, 0x80EDEDED},
            {0x808ACBD2, 0x806B9298, 0x80322C2E, 0x80190000},
            {0x80808080, 0x80959595, 0x80B5B5B5, 0x80C8C8C8},
            {0x80838383, 0x809D9D9D, 0x80CDCDCD, 0x80F0F0F0}};
        const uint32_t alternate[4][4]{
            {0x80737373, 0x80949494, 0x80D4D4D4, 0x80FCFCFC},
            {0x8072ADB1, 0x80577C7F, 0x80494943, 0x80918A81},
            {0x80B999B5, 0x80BB9CB7, 0x80CAB1C7, 0x80E7DCE5},
            {0x803C6292, 0x806188AE, 0x80BFCBDE, 0x80DDEAF3}};
        std::memcpy(colors(), primary, sizeof(primary));
        std::memcpy(alternate_colors(), alternate, sizeof(alternate));
        auto &height = nglHeightLightingConstants();
        height[0] = 0.02f;
        height[1] = 0.0f;
        height[2] = 4.0f;
        height[3] = 3.0f;
        return true;
    }();
    (void)initialized;
#endif
    for (uint32_t tod = 0; tod < 4; ++tod) {
        for (uint32_t i = 0; i < 4; ++i) {
            for (uint32_t channel = 0; channel < 4; ++channel) {
                const uint32_t shift = channel == 0 ? 16 : channel == 2 ? 0 : 8 * channel;
                const float primary = static_cast<float>((colors()[tod][i] >> shift) & 255);
                const float alternate = static_cast<float>((alternate_colors()[tod][i] >> shift) & 255);
                nglLightColors()[tod][4 * i + channel] = primary * 0.0039215689f;
                nglAltLightColors()[tod][4 * i + channel] = alternate * 0.0039215689f;
            }
        }
    }
    static Var<uint8_t> colors_initialized{0x00956309};
    colors_initialized() = 1;
}

void nglSetupMaterialLighting(nglMaterialBase *material, nglMeshNode *mesh, uint32_t matrix_register,
                              uint32_t color_register, uint32_t height_register,
                              uint32_t palette_register, uint32_t palette_count)
{
    static Var<nglTexture *[4]> light_textures{0x0095632C};
    static constexpr const char *names[]{"us_day_light", "us_night_light", "us_rainy_light", "us_sunset_light"};
    if (light_textures()[g_TOD] == nullptr) {
        light_textures()[g_TOD] = nglLoadTexture(tlFixedString{names[g_TOD]});
    }
    if (EnableShader) {
        float transform[3][4];
        for (uint32_t row = 0; row < 3; ++row) {
            for (uint32_t column = 0; column < 4; ++column) {
                transform[row][column] = mesh->LocalToWorld[column][row];
            }
        }
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, matrix_register, &transform[0][0], 3);
    }
    color out;
    sub_413F80(&out, material, &mesh->field_8C, color_register);
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, height_register, nglHeightLightingConstants(), 1);
        IDirect3DDevice9_SetVertexShaderConstantF(
            g_Direct3DDevice, palette_register, nglLightColors()[g_TOD], palette_count);
    }
}

void nglUpdateLODVertexColors(nglMeshNode *mesh, nglMeshSection *section)
{
    const auto *source = reinterpret_cast<const float *>(section->field_3C.getVertexData());
    const uint32_t count = section->field_3C.getSize() / 12;
    uint32_t *output{};
    IDirect3DVertexBuffer9_Lock(section->field_3C.getVertexBuffer(), 0, 0,
                               reinterpret_cast<void **>(&output), D3DLOCK_DISCARD);
    const auto &height = nglHeightLightingConstants();
    for (uint32_t i = 0; i < count; ++i, source += 3, output += 4) {
        std::memcpy(output, source, 12);
        const double world_y = static_cast<double>(source[2]) * mesh->LocalToWorld[1][2] +
                               static_cast<double>(source[1]) * mesh->LocalToWorld[1][1] +
                               static_cast<double>(source[0]) * mesh->LocalToWorld[1][0] +
                               static_cast<double>(source[3]) * mesh->LocalToWorld[1][3];
        double index = (world_y + height[1]) * height[0] * height[2];
        if (index >= height[3]) {
            index = height[3];
        }
        if (index < 0.0) {
            index = 0.0;
        }
        const auto *rgba = nglLightColors()[g_TOD] + 4 * static_cast<uint32_t>(index);
        output[3] = ((static_cast<uint32_t>(rgba[3] * 255.0f) & 255) << 24) |
                    ((static_cast<uint32_t>(rgba[0] * 255.0f) & 255) << 16) |
                    ((static_cast<uint32_t>(rgba[1] * 255.0f) & 255) << 8) |
                    (static_cast<uint32_t>(rgba[2] * 255.0f) & 255);
    }
    IDirect3DVertexBuffer9_Unlock(section->field_3C.getVertexBuffer());
}

bool USShaderBase::_IsSwitchable() const
{
    TRACE("USShaderBase::IsSwitchable");

    return false;
}
