#include "simple_region_visitor.h"

#include "common.h"
#include "config.h"
#include "subdivision_node.h"

namespace {
int visit_region(subdivision_visitor &visitor, const subdivision_node &node)
{
    return static_cast<simple_region_visitor &>(visitor).visit(const_cast<subdivision_node *>(&node));
}
const subdivision_visitor::native_vtable simple_region_table{visit_region, nullptr};
}  // namespace

VALIDATE_SIZE(simple_region_visitor, 0x28);

simple_region_visitor::simple_region_visitor(const vector3d &a2, bool a3)
{
#if STANDALONE_SYSTEM
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&simple_region_table);
#else
    this->m_vtbl = 0x00888BB4;
#endif
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
