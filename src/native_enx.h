#pragma once

struct generic_mash_data_ptrs;
struct pfx_interface;
struct entity_base;

namespace native_enx {
void *load(generic_mash_data_ptrs *data, pfx_interface *ifc, entity_base *owner);
void *vtable(unsigned type);
unsigned size(unsigned type);
void *construct(unsigned type, void *storage, unsigned *size);
void release(void *root, pfx_interface *ifc);
}
