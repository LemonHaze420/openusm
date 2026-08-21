#include "potential_shadow.h"

#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"

Var<potential_shadow[1]> shadow_candidates{0x00965F78};

VALIDATE_SIZE(potential_shadow, 0x2C);

potential_shadow::potential_shadow() {}

void potential_shadow::sub_5932C0()
{
    auto *v2 = this->field_18;
    if (v2 != nullptr) {
        v2->render_simple_shadow(this->field_1C, 1.0);
    }

    this->field_18 = nullptr;
    this->field_28 = 0.0;
}

void potential_shadow::sub_593280()
{
    auto v2 = this->m_radius;
    auto v3 = this->field_18;
    this->field_28 = v2;

    if (v3->is_hero()) {
        this->field_28 = this->field_28 + 10.0f;
    }

    this->field_28 = this->m_fade / (this->field_1C + 1.0f) * this->field_28;
}
