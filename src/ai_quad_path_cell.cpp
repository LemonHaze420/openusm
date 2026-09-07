#include "ai_quad_path_cell.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(ai_quad_path_cell, 0x50);

ai_quad_path_cell::ai_quad_path_cell() {}

vector3d ai_quad_path_cell::get_edge_midpoint(ai_quad_path_cell *a3)
{
    vector3d result;
    THISCALL(0x004648C0, this, &result, a3);

    return result;
}

void ai_quad_path_cell::un_mash(void *a2, int a3, int a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *counts = reinterpret_cast<std::uint8_t *>(this) + 0x40;
        auto *cell_lists = &field_0[12];
        const auto buffer = reinterpret_cast<std::uintptr_t>(a2);
        for (int list_index = 0; list_index < 4; ++list_index) {
            if (counts[list_index] == 0) {
                cell_lists[list_index] = 0;
                continue;
            }

            cell_lists[list_index] += static_cast<int>(buffer) + a4;
            auto *cells = reinterpret_cast<std::uint32_t *>(cell_lists[list_index]);
            for (std::uint8_t cell_index = 0; cell_index < counts[list_index]; ++cell_index) {
                cells[cell_index] =
                    static_cast<std::uint32_t>(buffer + a3 + 0x50u * cells[cell_index]);
            }
        }
        field_0[17] = 0;
    } else {
        THISCALL(0x00452C60, this, a2, a3, a4);
    }
}
