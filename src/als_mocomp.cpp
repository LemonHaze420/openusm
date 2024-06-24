#include "als_mocomp.h"

#include "actor.h"
#include "biped_system.h"
#include "common.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "phys_vector3d.h"
#include "rigid_body.h"
#include "state_machine.h"
#include "trace.h"
#include "utility.h"

namespace als {

VALIDATE_SIZE(begin_biped_physics, 0x20);

float begin_biped_physics::activate(animation_logic_system *a1)
{
    TRACE("als::begin_biped_physics::activate");

    motion_compensator::activate(a1);
    this->field_14 = true;
    this->field_18 = 0;
    assert(the_actor->has_physical_ifc() && the_actor->physical_ifc()->is_biped_physics_running() 
            && "The user of the begin_biped_physics mocomp must be running biped physics.");

    auto *v4 = the_actor->physical_ifc();
    auto *biped_system = v4->get_biped_system();

    auto *v8 = &biped_system->field_0;
    float v44 = 0.0;
    float v45 = 10.0;
    float a2 = 0.0;
    auto *v9 = the_actor->physical_ifc();
    auto v43 = v9->get_velocity();

    vector3d v50;
    if ( this->field_8->does_parameter_exist(velocity_left_c_param_hash) )
    {
        auto *v12 = this->field_8;
        auto v14 = v12->get_pb_float(velocity_left_c_param_hash);
        v50 *= v14;
    }

    if ( this->field_8->does_parameter_exist(velocity_y_left_c_param_hash) )
    {
        auto *v17 = this->field_8;
        auto v19 = v17->get_pb_float(velocity_y_left_c_param_hash);
        v50[1] *= v19;
    }

    auto *v22 = the_actor->physical_ifc();
    v22->set_velocity(v43, false);
    auto *v23 = this->field_8;
    this->field_1C = v23->get_optional_pb_float(velocity_left_deactivate_c_param_hash, 1.0, nullptr);
    auto *v25 = this->field_8;
    float v29;
    if ( v25->does_parameter_exist(force_scale_param_hash) )
    {
        v29 = this->field_8->get_pb_float(force_scale_param_hash);
    }
    else if ( not_equal(0.0f, this->field_8->get_param(this->field_4, 86u)) )
    {
        v29 = this->field_8->get_param(this->field_4, 86u);
    }

    v44 = v29;
    if ( this->field_8->does_parameter_exist(force_y_param_hash) )
    {
        auto *v32 = this->field_8;
        v45 = v32->get_pb_float(force_y_param_hash);
    }

    if ( this->field_8->does_parameter_exist(rotate_xz_ang_param_hash) )
    {
        a2 = this->field_8->get_pb_float(rotate_xz_ang_param_hash) * (3.1415927 / 180.0);
    }

    if ( not_equal(v44, 0.0f) || not_equal(v45, 0.0f) )
    {
        auto *v38 = this->field_4;
        auto *v39 = this->field_8;
        auto a3 = v39->get_vector_param(v38, 83u) * v44;
        if ( not_equal(v45, 0.0f) ) {
            a3.y = v45;
        }

        if ( not_equal(a2, 0.0f) )
        {
            static Var<po> stru_91F8D8 {0x0091F8D8};
            po v54 = stru_91F8D8();
            v54.set_rotate_y(a2);
            a3 = v54.non_affine_slow_xform(a3);
        }

        auto v41 = 1.0f / (*v8->m_list_rigid_body.m_data)->field_130;
        a3 *= v41;
        v8->apply_pulse(0, a3);
    }

    return this->field_8->get_param(this->field_4, 90u);
}

}

void als_mocomp_patch()
{
    {
        FUNC_ADDRESS(address, &als::begin_biped_physics::activate);
        //SET_JUMP(0x004A48C0, address);
    }

}
