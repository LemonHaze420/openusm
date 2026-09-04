#include "resource_pack_location.h"

#include "common.h"
#include "log.h"

VALIDATE_SIZE(resource_pack_location, 0x50u);
VALIDATE_OFFSET(resource_pack_location, m_name, 0x30u);

resource_pack_location::resource_pack_location() : loc()
{
    loc.clear();
    this->clear();
}

void resource_pack_location::clear()
{
    field_10 = 0;
    field_14 = 0;
    field_18 = 0;
    field_1C = 0;
    prerequisite_offset = 0;
    prerequisite_count = 0;
    field_28 = 0;
    m_name[0] = '\0';
    field_2C = 0;
}

resource_pack_location &resource_pack_location::operator=(const resource_pack_location &a2)
{
    if (&a2 != this) {
        this->loc.field_0.set(a2.loc.field_0);
    }

    this->loc.m_offset = a2.loc.m_offset;
    this->loc.m_size = a2.loc.m_size;
    this->field_10 = a2.field_10;
    this->field_14 = a2.field_14;
    this->field_18 = a2.field_18;
    this->field_1C = a2.field_1C;
    this->prerequisite_offset = a2.prerequisite_offset;
    this->prerequisite_count = a2.prerequisite_count;
    this->field_28 = a2.field_28;
    this->field_2C = a2.field_2C;

    for (int i = 0; i < 32; ++i) {
        this->m_name[i] = a2.m_name[i];
    }

    return (*this);
}
