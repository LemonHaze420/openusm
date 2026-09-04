#include "simple_region_visitor.h"

#include "common.h"

VALIDATE_SIZE(simple_region_visitor, 0x28);

simple_region_visitor::simple_region_visitor(const vector3d &a2, bool a3)
{
    this->m_vtbl = 0x00888BB4;
    this->field_4 = a2;
    this->region_count = 0;

    std::memset(this->field_14, 0, sizeof(this->field_14));
    this->field_24 = a3;
}

int simple_region_visitor::visit(subdivision_node *a2)
{
    for (auto i = 0; i < this->region_count; ++i) {
        if (this->field_14[i] == a2) {
            return 0;
        }
    }

    assert(region_count < MAX_REGIONS_TO_FIND_FROM_POINT && "why are there so many regions on this 1 point?");

    this->field_14[this->region_count++] = a2;
    if (this->field_24) {
        return 0;
    }

    return 3;
}
