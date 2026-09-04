#include "region_intersect_visitor.h"

#include "region.h"

#include "common.h"

VALIDATE_SIZE(region_intersect_visitor, 0x44);

region_intersect_visitor::region_intersect_visitor(region *r)
{
    m_vtbl = 0x0088897C;

    if (r != nullptr) {
        this->field_4 = 1;
        this->field_8[0] = r;
    } else {
        this->field_4 = 0;
    }
}

int region_intersect_visitor::visit(subdivision_node *a1)
{
    auto *r = bit_cast<region *>(a1);

    assert(r != nullptr);

    for (auto i = 0; i < this->field_4; ++i) {
        if (r == this->field_8[i]) {
            return 0;
        }
    }

    if (this->field_4 < 15) {
        this->field_8[this->field_4++] = r;
    }

    return 0;
}
