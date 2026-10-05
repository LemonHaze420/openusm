#include "rb_capsule_pair.h"

#include "collide.h"
#include "func_wrapper.h"
#include "rb_sphere_list.h"
#include "physics_system.h"
#include "rbc_def_contact.h"

void rb_capsule_pair::do_test()
{
    if constexpr (1) {
        vector3d cp1{};
        vector3d cp2{};
        vector3d normal{};
        if (collide_capsule_capsule(this->field_0->capsule.field_1C,
                                    this->field_0->capsule.field_28,
                                    this->field_0->capsule.m_radius,
                                    this->field_4->capsule.field_1C,
                                    this->field_4->capsule.field_28,
                                    this->field_4->capsule.m_radius,
                                    cp1,
                                    cp2,
                                    normal)) {
            auto *v2 = phys_sys::create_no_error_rbc_contact(this->field_0->body, this->field_4->body);
            if (v2 != nullptr) {
                auto v3 = sub_5B1370(this->field_4->body->field_40, cp2);
                auto v4 = sub_5B1370(this->field_0->body->field_40, cp1);
                v2->add_point(this->field_0->body,
                              this->field_4->body,
                              *bit_cast<phys_vector3d *>(&v4),
                              *bit_cast<phys_vector3d *>(&v3),
                              *bit_cast<phys_vector3d *>(&normal),
                              0.0,
                              0.0,
                              100.0,
                              1);
            }
        }
    } else {
        THISCALL(0x00592B80, this);
    }
}
