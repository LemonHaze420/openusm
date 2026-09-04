#include "collision_capsule.h"

#include "actor.h"
#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "trace.h"
#include "variable.h"

#include <cassert>
#include <cmath>

VALIDATE_SIZE(collision_capsule, 0x2C);

std::intptr_t &collision_capsule_v_table = var<std::intptr_t>(0x0095A664);

collision_capsule::collision_capsule() : collision_geometry() {}

collision_capsule::collision_capsule(actor *a2)
{
    this->field_8 = 0;
    this->field_9 = 0;
    this->field_8 = 0;
    this->field_9 = 1;
    this->field_C = 2;
    this->owner = a2;
    this->compute_dimensions();
    this->field_C |= 0x80000u;
}

capsule collision_capsule::get_abs_capsule(const po &a3)
{
    capsule v12;
    v12.base = a3.m * this->rel_cap.base;
    v12.end = a3.m * this->rel_cap.end;
    v12.radius = this->rel_cap.radius;

    return v12;
}

void collision_capsule::compute_dimensions()
{
    if constexpr (1) {
        vector3d v9;
        double v4;

        auto *v2 = this->owner;
        if (v2->colgeom != nullptr) {
            v4 = std::abs(v2->get_colgeom_radius());
            v9 = this->owner->get_colgeom_center();

        } else {
            v9 = v2->get_visual_center();
            v4 = std::abs(this->owner->get_visual_radius());
        }

        auto v5 = 0.125f * v4;

        vector3d v14;

        this->rel_cap.base = v9 + YVEC * v5;

        const auto v6 = 0.5f;
        auto v7 = v6 * v4;

        this->rel_cap.end = YVEC * v7 + this->rel_cap.base;
        this->rel_cap.radius = std::abs(v4 * 0.25f);

    } else {
        THISCALL(0x00515F20, this);
    }
}

void collision_capsule::validate()
{
    assert(rel_cap.base.length() < 10.0f && rel_cap.end.length() < 10.0f);
}

collision_geometry *collision_capsule::make_instance(actor *a2)
{
    auto *result = new collision_capsule{a2};

    auto v5 = result->field_C;
    if ((this->field_C & 0x80000) != 0) {
        result->field_C = v5 | 0x80000;
    } else {
        result->field_C = v5 & 0xFFF7FFFF;
    }

    return result;
}

vector3d collision_capsule::get_local_space_bounding_sphere_center()
{
    auto tmp = this->rel_cap.base + this->rel_cap.end;

    return tmp * 0.5f;
}

float collision_capsule::get_bounding_sphere_radius()
{
    assert(rel_cap.radius >= 0.0f);

    auto tmp = this->rel_cap.base - this->rel_cap.end;
    auto length = tmp.length();
    return length * 0.5f + this->rel_cap.radius;
}

float collision_capsule::get_core_radius()
{
    return this->rel_cap.radius;
}

int collision_capsule::get_type()
{
    return collision_geometry::CAPSULE;
}

void collision_capsule::un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3)
{
    collision_geometry::un_mash(a1, a2, a3);

    this->field_9 = false;
}

void collision_capsule::render(const po &a1)
{
    TRACE("collision_capsule::render");

    auto a4 = this->get_abs_capsule(a1);
    auto v2 = static_cast<uint8_t>(255.0 * 0.5f);
    color32 v3{255, 255, 255, v2};
    render_debug_capsule(a4.base, a4.end, a4.radius, v3);
}

void collision_capsule_patch()
{
    {
        FUNC_ADDRESS(address, &collision_capsule::render);
        set_vfunc(0x00882D1C, address);
    }
}
