#include "swing_anchor_obbfilter.h"

#include "subdivision_obb.h"

#include "utility.h"

swing_anchor_obbfilter_t::swing_anchor_obbfilter_t(const vector3d &a1, Float a3) {
    this->m_vtbl = 0x0087EDF8;
    this->field_4 = a1;
    this->field_10 = a3;
}

bool swing_anchor_obbfilter_t::accept(subdivision_node_obb_base *node,
                                      [[maybe_unused]] const local_collision::query_args_t &a3) {
    return node->sphere_intersection(this->field_4, this->field_10);
}

void swing_anchor_obbfilter_patch() {
    FUNC_ADDRESS(address, &swing_anchor_obbfilter_t::accept);
    set_vfunc(0x0087EDF8, address);
}
