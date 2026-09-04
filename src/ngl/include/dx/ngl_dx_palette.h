#pragma once

#include <cstdint>

struct nglPalette;

//0x00782950
extern nglPalette *nglCreatePalette([[maybe_unused]] int a1, uint32_t entries, const void *a3);
