#include "entity_base_vtable.h"

#include "common.h"
#include "entity_mash.h"

VALIDATE_SIZE(entity_base_vtable, 0x4);

entity_base_vtable::entity_base_vtable()
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    m_vtbl = ent_v_table_lookup[0];
#endif
}
