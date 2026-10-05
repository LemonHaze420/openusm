#include "colgeom_alter_sys.h"

#include "actor.h"
#include "collision_capsule.h"
#include "common.h"
#include "conglom.h"
#include "oldmath_po.h"
#include "physical_interface.h"

#include <cmath>

VALIDATE_SIZE(capsule_alter_sys, 0xA4u);
VALIDATE_OFFSET(capsule_alter_sys, field_58, 0x58);
VALIDATE_OFFSET(capsule_alter_sys, field_6C, 0x6C);
VALIDATE_OFFSET(capsule_alter_sys, field_80, 0x80);
VALIDATE_OFFSET(capsule_alter_sys, field_98, 0x98);

capsule_alter_sys::capsule_alter_sys(actor *owner)
{
    field_A0 = field_A1 = field_A2 = dynamic = false;
    field_0 = owner;
    auto *geometry = owner->get_colgeom();
    assert(geometry != nullptr && geometry->get_type() == collision_geometry::CAPSULE);
    field_8 = static_cast<collision_capsule *>(geometry);
    base_rel_cap = field_8->rel_cap;
    field_28 = base_rel_cap;
    for (int i = 0; i != 5; ++i) {
        field_44[i] = field_58[i] = nullptr;
        field_6C[i] = field_80[i] = 1.0f;
    }
    field_98 = field_9C = 0.0f;
    field_94 = 0.25f;
    field_A1 = true;
    compute_avg_values();
    dynamic = true;
    field_4 = static_cast<eAlterMode>(0);
}

void capsule_alter_sys::compute_avg_values()
{
    for (int end = 0; end != 2; ++end) {
        auto **nodes = end == 0 ? field_44 : field_58;
        auto *weights = end == 0 ? field_6C : field_80;
        double total = 0.0;
        int count = 0;
        while (count != 5 && nodes[count] != nullptr)
            total += weights[count++];
        if (total > 0.0) {
            const float inverse = static_cast<float>(1.0 / total);
            for (int i = 0; i != count; ++i)
                weights[i] *= inverse;
        }
    }
    field_A1 = false;
}

void capsule_alter_sys::set_avoid_floor(bool a2)
{
    this->field_A0 = a2;
}

void capsule_alter_sys::set_avg_radius(Float a2)
{
    this->field_94 = a2;
}

static constexpr auto _CAPSULE_MAX_DYN_AVG_NODES = 5;

void capsule_alter_sys::set_base_avg_node(int index, entity_base *a3, Float a4)
{
    assert(index >= 0 && index < _CAPSULE_MAX_DYN_AVG_NODES);

    this->field_6C[index] = a4;
    this->field_44[index] = a3;
    this->field_A1 = true;
}

void capsule_alter_sys::adjust_colgeom(bool force)
{
    switch (field_4) {
    case 0:
        if (force)
            field_8->rel_cap = base_rel_cap;
        return;
    case 1:
        if (force)
            field_8->rel_cap = field_28;
        return;
    case 2: {
        auto *pelvis = static_cast<conglomerate *>(field_0)->get_member(bip01_pelvis, true);
        if (pelvis != nullptr) {
            const auto &pelvis_po = pelvis->get_abs_po();
            const auto *inverse = field_0->get_abs_po().inverse();
            const ptr_to_po composition{&pelvis_po.m, &inverse->m};
            po relative;
            relative.set_from_ptr_to_po_world(composition);
            field_8->rel_cap.base = relative.slow_xform(field_28.base);
            field_8->rel_cap.end = relative.slow_xform(field_28.end);
            field_8->rel_cap.radius = field_28.radius;
        } else {
            field_8->rel_cap = field_28;
        }
        return;
    }
    case 3:
        break;
    default:
        return;
    }

    if (field_A1)
        compute_avg_values();
    const auto &actor_po = field_0->get_abs_po();
    po basis;
    basis.set_po(actor_po.get_z_facing(), actor_po.get_y_facing(), actor_po.get_position());
    const vector3d up = basis.get_y_facing();
    const bool avoid_floor = field_A0 && field_0->has_physical_ifc();
    vector3d floor;
    if (avoid_floor)
        floor = basis.get_position() - up * field_0->physical_ifc()->get_floor_offset();

    bool scaled = field_A2;
    float radius = field_94;
    vector3d scale;
    if (scaled) {
        scale = field_0->get_render_scale();
        const double average = (static_cast<double>(scale.z) + scale.y + scale.x) * 0.3333333432674408f;
        if (std::fabs(average - 1.0) >= 0.10000000149011612f)
            radius = static_cast<float>(average * radius);
        else
            scaled = false;
    }

    capsule result;
    for (int end = 0; end != 2; ++end) {
        auto **nodes = end == 0 ? field_44 : field_58;
        const auto *weights = end == 0 ? field_6C : field_80;
        vector3d point = ZEROVEC;
        if (nodes[0] != nullptr) {
            for (int i = 0; i != 5 && nodes[i] != nullptr; ++i) {
                vector3d position = nodes[i]->get_abs_po().get_position();
                if (scaled) {
                    position = basis.inverse_xform(position);
                    position.x *= scale.x;
                    position.y *= scale.y;
                    position.z *= scale.z;
                    position = basis.slow_xform(position);
                }
                point += position * weights[i];
            }
            point += up * (end == 0 ? field_98 : field_9C);
            if (avoid_floor) {
                const auto delta = point - up * (radius + static_cast<float>(LARGE_EPSILON)) - floor;
                if (delta.length2() > 0.001f) {
                    const float penetration = dot(delta, up);
                    if (penetration < 0.0f)
                        point -= up * penetration;
                }
            }
            point = basis.inverse_xform(point);
        }
        (end == 0 ? result.base : result.end) = point;
    }
    result.radius = std::fabs(radius);
    field_8->rel_cap = result;
}

void capsule_alter_sys::restore_colgeom()
{
    field_8->rel_cap = base_rel_cap;
}

void capsule_alter_sys::set_mode(eAlterMode a2)
{
    auto v2 = this->field_4;
    this->field_4 = a2;
    if (a2 != v2) {
        this->adjust_colgeom(true);
    }
}

void capsule_alter_sys::set_static_capsule(const vector3d &a2, const vector3d &a3, Float a4)
{
    this->field_28.base = a2;
    this->field_28.end = a3;
    this->field_28.radius = a4;
    auto v4 = (this->field_4 == 1);
    this->adjust_colgeom(v4);
}

void capsule_alter_sys::set_end_avg_node(int index, entity_base *a3, Float a4)
{
    assert(index >= 0 && index < _CAPSULE_MAX_DYN_AVG_NODES);

    this->field_80[index] = a4;
    this->field_58[index] = a3;
    this->field_A1 = 1;
}

void set_to_default_capsule_alter(capsule_alter_sys *alter, conglomerate *owner)
{
    alter->set_avoid_floor(true);
    auto *geometry = owner->get_colgeom();
    alter->set_avg_radius(geometry != nullptr && geometry->get_type() == collision_geometry::CAPSULE
                              ? geometry->get_core_radius() : 0.5f);
    alter->set_mode(static_cast<capsule_alter_sys::eAlterMode>(3));
    alter->set_base_avg_node(0, owner->get_bone(bip01_l_calf, true), 0.5f);
    alter->set_base_avg_node(1, owner->get_bone(bip01_r_calf, true), 0.5f);
    alter->set_base_avg_node(2, owner->get_bone(bip01_pelvis, true), 2.5f);
    alter->set_base_avg_node(3, nullptr, 0.0f);
    alter->set_end_avg_node(0, owner->get_bone(string_hash{"BIP01 SPINE2"}, true), 1.0f);
    alter->set_end_avg_node(1, nullptr, 0.0f);
}
