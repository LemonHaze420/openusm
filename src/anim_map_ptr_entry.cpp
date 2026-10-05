#include "anim_map_ptr_entry.h"

#include "common.h"
#include "anim_info.h"

VALIDATE_SIZE(anim_map_ptr_entry, 0x10);

anim_map_ptr_entry::~anim_map_ptr_entry()
{
    if (!field_8.from_mash())
        delete[] field_8.m_data;
    field_8.m_data = nullptr;
    field_8.m_size = 0;
}

void anim_map_ptr_entry::un_mash(generic_mash_header *a2, void *, generic_mash_data_ptrs *a4)
{
    this->field_8.custom_un_mash(a2, &this->field_8, a4, nullptr);
}
