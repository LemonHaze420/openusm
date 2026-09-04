#include "resource_location.h"

#include "common.h"
#include "log.h"
#include "parse_generic_mash.h"

VALIDATE_SIZE(resource_location, 16u);

resource_location::resource_location() : field_0(), m_size(0)
{
    this->m_offset = 0u;
}

void resource_location::clear()
{
    field_0 = {};
    m_offset = 0u;
    m_size = 0;
}

void resource_location::operator=(const resource_location &loc)
{
    field_0 = loc.field_0;
    m_offset = loc.m_offset;
    m_size = loc.m_size;
}


void resource_location::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *)
{
    ;
}
