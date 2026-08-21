#include "anim_map_ptr_entry.h"

#include "common.h"

VALIDATE_SIZE(anim_map_ptr_entry, 0x10);

void anim_map_ptr_entry::un_mash(generic_mash_header *a2, void *, generic_mash_data_ptrs *a4)
{
    this->field_8.custom_un_mash(a2, &this->field_8, a4, nullptr);
}
