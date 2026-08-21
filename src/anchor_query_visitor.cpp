#include "anchor_query_visitor.h"

#include "ai_find_best_swing_anchor.h"
#include "common.h"
#include "conglom.h"
#include "conglomerate_clone.h"
#include "func_wrapper.h"
#include "line_marker_base.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include "utility.h"

VALIDATE_SIZE(anchor_query_visitor, 0x28);

anchor_query_visitor::anchor_query_visitor(quick_anchor_container_t *a2, const vector3d &a1, const vector3d &a4,
                                           bool a5, occupancy_voxels_t *a6)
{
    this->m_vtbl = 0x0087E9A4;
    this->field_4 = a2;
    this->field_8 = a1;
    this->field_20 = a5;
    this->field_24 = a6;
    this->field_14 = a4;
}

int anchor_query_visitor::visit(subdivision_node *a2)
{
    auto *v2 = (conglomerate *)a2;
    if (v2 == nullptr) {
        return 0;
    }

    if (v2->field_64 == entity::visit_key3) {
        return 0;
    }

    v2->field_64 = entity::visit_key3;
    if (v2->is_flagged(0x800)) {
        return 0;
    }

    v2->get_abs_po();

    if (this->field_20 && v2->is_a_conglomerate_clone()) {
        return 0;
    }

    conglomerate *v4 = nullptr;
    if (v2->is_a_conglomerate_clone()) {
        vhandle_type<conglomerate> tmp;
        tmp.field_0 = CAST(tmp.field_0, v2->members.m_data);

        v4 = tmp.get_volatile_ptr();
    } else if (v2->is_a_conglomerate()) {
        v4 = v2;
    }

    if (v4 != nullptr) {
        auto func = [this](conglomerate *v2, entity_base *anchor) -> void {
            auto v9 = v2->is_a_conglomerate_clone();
            this->add_quick_anchor(anchor, v9 ? v2 : nullptr);
        };

        auto it = v2->members.begin();
        auto end = v2->members.end();

        if (this->field_20) {
            for (; it != end; ++it) {
                if ((*it)->get_flavor() == ANCHOR_MARKER ||
                    ((*it)->get_flavor() == LINE_ANCHOR && !(*it)->is_walkable())) {
                    func(v2, *it);
                }
            }
        } else {
            for (; it != end; ++it) {
                if ((*it)->get_flavor() == LINE_ANCHOR && (*it)->is_walkable()) {
                    func(v2, *it);
                }
            }
        }
    } else if (this->field_20) {
        if ((v2->is_a_line_marker_base() && !v2->is_walkable()) || !v2->is_an_anchor_marker()) {
            this->add_quick_anchor(v2, nullptr);
        }
    } else if (v2->is_a_line_marker_base() && v2->is_walkable()) {
        this->add_quick_anchor(v2, nullptr);
    }

    return 0;
}

void anchor_query_visitor::add_quick_anchor(entity_base *anchor, entity *clone_ptr)
{
    assert(clone_ptr == nullptr || clone_ptr->is_a_conglomerate_clone());

    if constexpr (1) {
        vector3d target;

        vector3d origin;

        auto *v3 = bit_cast<line_marker_base *>(anchor);

        sp_log("%u", anchor->get_flavor());

        if (anchor->get_flavor() == LINE_ANCHOR) {
            target = v3->get_target();

            origin = v3->get_origin();

            if (clone_ptr != nullptr) {
                vector3d v7 = clone_ptr->get_abs_po().slow_xform(target);

                target = v7;

                origin = clone_ptr->get_abs_po().slow_xform(origin);
            }
        } else if (v3->get_flavor() == ANCHOR_MARKER) {
            target = v3->get_abs_position();

            if (clone_ptr != nullptr) {
                target = clone_ptr->get_abs_po().slow_xform(target);
            }

            origin = target;
        } else {
            assert(0 && "Entity in anchor_query_visitor::add_quick_anchor must be either an anchor "
                        "marker or a line anchor");
        }

        double v16;
        if (!this->field_20 || (v16 = this->field_14[1] + 5.0f, target[1] >= v16) || origin[1] >= v16) {
            float tmp;
            closest_point_line_segment_point(target, origin, this->field_8, tmp);

            vector3d v20 = (origin - target) * tmp + target;

            vector3d a5 = v20;
            auto v17 = v20 - this->field_8;
            auto a6 = v17.length2() * 0.5f * 0.5f;

            this->field_4->add_anchor(
                this->field_24, a5, YVEC, a5, a6, bit_cast<entity *>(v3), bit_cast<conglomerate_clone *>(clone_ptr));
        }
    } else {
        THISCALL(0x004901E0, this, anchor, clone_ptr);
    }
}

void anchor_query_visitor_patch()
{
    {
        FUNC_ADDRESS(address, &anchor_query_visitor::visit);
        set_vfunc(0x0087E9A4, address);
    }

    {
        auto addr = &quick_anchor_container_t::add_anchor;
        FUNC_ADDRESS(address, addr);
        REDIRECT(0x00486A2A, address);
        //REDIRECT(0x00490448, address);
    }
}
