#pragma once

struct generic_mash_data_ptrs;
struct generic_mash_header;
struct entity_base;

bool actor_xbpack_unmash_entity_prefix(entity_base *self, generic_mash_header *header,
                                       generic_mash_data_ptrs *data);

bool actor_xbpack_prepare_mash(generic_mash_header *header, generic_mash_data_ptrs *data);

void actor_xbpack_finish(generic_mash_data_ptrs *data);
void actor_xbpack_patch();
