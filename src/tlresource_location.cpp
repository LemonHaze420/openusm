#include "tlresource_location.h"

#include "common.h"

VALIDATE_SIZE(tlresource_location, 12u);

void tlresource_location::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *)
{
    ;
}

tlresource_type tlresource_location::get_type() const
{
    return static_cast<tlresource_type>(this->m_type & 0xFF);
}

uint32_t tlresource_location::get_size()
{
    return this->m_type >> 8;
}

char *tlresource_location::get_data()
{
    return this->field_8;
}

void tlresource_location::set_data(char *a2)
{
    this->field_8 = a2;
}

string_hash tlresource_location::get_name() const
{
    return this->name;
}

void tlresource_location::sub_672BDD(uint32_t a2)
{
    this->m_type = a2 + (this->m_type & 0xFFFFFF00);
}

void tlresource_location::add_base(uint32_t base)
{
    this->field_8 += base;
}
