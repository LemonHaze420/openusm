#include "lego_map.h"

#include "func_wrapper.h"
#include "trace.h"

lego_map_root_node::lego_map_root_node() {}

void lego_map_root_node::un_mash(char *image, int *a3, region *reg)
{
    TRACE("lego_map_root_node::un_mash");

    THISCALL(0x0054E5A0, this, image, a3, reg);

#ifdef OPENUSM_XBPACK_V10
    auto *legos = reinterpret_cast<uint8_t *>(field_8);
    for (uint16_t i = 0; i < static_cast<uint16_t>(field_14); ++i) {
        legos[i * 0x20 + 0x11] = 0;
    }
#endif
}
