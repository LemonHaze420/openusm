#include "lego_map.h"

#include "func_wrapper.h"
#include "trace.h"

lego_map_root_node::lego_map_root_node() {}

void lego_map_root_node::un_mash(char *image, int *a3, region *reg)
{
    TRACE("lego_map_root_node::un_mash");

    if constexpr (STANDALONE_SYSTEM) {
        const auto mesh_count = static_cast<std::uint16_t>(field_10);
        const auto material_count = static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(field_10) >> 16u);
        const auto lego_count = static_cast<std::uint16_t>(field_14);

        auto cursor = (reinterpret_cast<std::uintptr_t>(this) + 0x1Fu) & ~std::uintptr_t {7u};
        field_0 = reinterpret_cast<nglMesh **>(cursor);
        field_4 = reinterpret_cast<nglMaterialBase **>(field_0 + mesh_count);
        field_8 = reinterpret_cast<scene_entity *>(
            (reinterpret_cast<std::uintptr_t>(field_4 + material_count) + 3u) & ~std::uintptr_t {3u});
        field_C = mesh_count != 0
                      ? reinterpret_cast<proximity_map *>(
                            reinterpret_cast<std::uintptr_t>(field_8) +
                            reinterpret_cast<std::uintptr_t>(field_C))
                      : nullptr;
        *a3 = static_cast<std::uint16_t>(static_cast<std::uint32_t>(field_14) >> 16u);

        for (std::uint16_t i = 0; i < material_count; ++i) {
            field_4[i] = nullptr;
        }
        for (std::uint16_t i = 0; i < mesh_count; ++i) {
            field_0[i] = nullptr;
        }
        for (std::uint16_t i = 0; i < lego_count; ++i) {
            field_8[i].mesh = nullptr;
        }
    } else {
        THISCALL(0x0054E5A0, this, image, a3, reg);
    }

#ifdef OPENUSM_XBPACK_V10
    auto *legos = reinterpret_cast<uint8_t *>(field_8);
    for (uint16_t i = 0; i < static_cast<uint16_t>(field_14); ++i) {
        legos[i * 0x20 + 0x11] = 0;
    }
#endif
}
