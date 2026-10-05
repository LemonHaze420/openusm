#include "marker.h"

#include "common.h"
#include "entity_mash.h"

VALIDATE_SIZE(marker, 0x68);

marker::marker(const string_hash &a2, uint32_t a3) : entity(a2, a3)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[16];
#endif
}
