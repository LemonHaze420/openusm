#include "colgeom_alter_sys.h"

#include "actor.h"
#include "collision_capsule.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"

VALIDATE_SIZE(capsule_alter_sys, 0xA4u);

capsule_alter_sys::capsule_alter_sys(actor *a2)
{
    if constexpr (1) {
        this->field_A0 = false;
        this->field_A1 = false;
        this->field_A2 = false;
        this->dynamic = false;
        this->field_0 = a2;
        auto v3 = a2->get_colgeom();
        if (v3 == nullptr || (v3->get_type() != collision_geometry::CAPSULE)) {
            assert(0 && "Bad colgeom for capsule_alter_sys");
        }

        this->field_8 = CAST(field_8, v3);

        this->base_rel_cap = this->field_8->rel_cap;
        assert(base_rel_cap.base.is_valid());
        assert(base_rel_cap.end.is_valid());

        this->field_28 = this->base_rel_cap;

        auto *v5 = this->field_6C;
        auto *v6 = &this->field_98;
        int v7 = 2;
        do {
            *v6 = 0;
            int v8 = 5;
            do {
                *(v5 - 10) = 0.0;
                *v5++ = 1.0;
                --v8;
            } while (v8);
            ++v6;
            --v7;
        } while (v7);

        this->field_94 = 0.25;
        this->field_A0 = 0;
        this->field_A1 = 1;
        this->compute_avg_values();
        this->field_A2 = 0;
        this->dynamic = true;
        this->field_4 = (eAlterMode)0;

    } else {
        THISCALL(0x005C51D0, this, a2);
    }
}

void capsule_alter_sys::compute_avg_values()
{
    THISCALL(0x005BA2F0, this);
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

void capsule_alter_sys::adjust_colgeom(bool a2)
{
    THISCALL(0x005D7170, this, a2);
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

void set_to_default_capsule_alter(capsule_alter_sys *a1, conglomerate *a2)
{
    CDECL_CALL(0x00687B70, a1, a2);
}
