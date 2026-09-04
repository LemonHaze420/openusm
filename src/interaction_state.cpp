#include "interaction_state.h"

#include "ai_interaction_data.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "func_wrapper.h"
#include "interaction_inode.h"
#include "oldmath_po.h"
#include "param_list.h"
#include "state_machine.h"
#include "vector3d.h"

namespace ai {

VALIDATE_SIZE(interaction_state, 0x58);

Var<string_hash> interaction_state::pre_interact_motion_hash = (0x00958714);
Var<string_hash> interaction_state::pre_interact_run_motion_hash = (0x00958844);

interaction_state::interaction_state(from_mash_in_place_constructor *a2) : enhanced_state(a2)
{
    this->field_30 = nullptr;
    this->field_50 = 0;
    this->field_54 = 0;
}

void interaction_state::move_actor_to_interact_point()
{
    vector3d a4;
    vector3d a2;
    vector3d a3;
    this->compute_interact_point(&a4, &a2, &a3);

    bool v12 = true;

    ai_core *v2 = this->get_core();
    als_inode *v3 = CAST(v3, v2->get_info_node(als_inode::default_id, v12));
    als::param_list v13 = als::param_list();

    v13.add_param(27u, a2);
    v13.add_param(33u, a4);
    v13.add_param(24u, a3);

    auto *v4 = v3->get_als_layer(static_cast<als::layer_types>(0));

    v4->set_desired_params(v13);
    this->field_30->set_curr_anim({0});
    auto *v6 = this->field_30->field_2C->does_anim_exist(enum_anim_key::key_enum{0}, true);

    string_hash v8 = (v6 != nullptr) ? pre_interact_motion_hash() : pre_interact_run_motion_hash();

    v3->request_category_transition(v8, static_cast<als::layer_types>(0), true, false, false);

    v13.clear();
}

int interaction_state::frame_advance(Float a2)
{
    return THISCALL(0x00462DF0, this, a2);
}

po *interaction_state::compute_interact_point(vector3d *a4, vector3d *a2, vector3d *a3)
{
#if 0
    LODWORD(v38[0]) = 1;
    v4 = this;
    LODWORD(v39[0]) = &v37;
    v37 = ai::als_inode::default_id;
    v46 = -1;
    v5 = ai::base_state::get_core(&this->base);
    ai::ai_core::get_info_node(v5, LODWORD(v37), SLOBYTE(v38[0]));
    v6 = (float *) &v4->field_30->field_2C->field_0;
    v7 = v6[5];
    v8 = v6[6];
    v9 = v6[4];
    v38[0] = 1.0;
    v42.arr[1] = v6[3];
    v42.arr[0] = v8;
    v42.arr[2] = v9;
    v42.arr[3] = v7;
    po::set_po_0(&v44, (const vector3d *) v6, &v42, 1.0); // po::po
    v10 = (vhandle_type__actor_vhandle_type__entity_vhandle_type__signaller_entity_base_vhandle
               *) &v4->field_30->field_1C;
    *(_QWORD *) &v41.field_0.arr[1][0] = 4575657221408423936i64;
    v41.field_0.arr[1][2] = 0.0;
    *(_QWORD *) &v41.field_0.arr[2][0] = 0i64;
    v41.field_0.arr[2][2] = 1.0;
    *(_QWORD *) &v41.field_0.arr[3][0] = 0i64;
    v41.field_0.arr[3][2] = 0.0;
    if (vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>::
            get_volatile_ptr(v10)) {
        v11 = vhandle_type<actor, vhandle_type<entity, vhandle_type<signaller, entity_base_vhandle>>>::
            get_volatile_ptr(v10);
    } else {
        v11 = 0;
    }

    if ((v11->field_8 >> 28) & 1) {
        entity_base::update_abs_po(v11, 1);
    }

    v12 = (float *) v11->field_14;
    LODWORD(v39[0]) = &v44;
    LODWORD(v39[1]) = v12;
    sub_415A30(&v41, v39);
    v13 = v41.field_0.arr[3][1];
    a4->arr[0] = v41.field_0.arr[3][0];
    v14 = v41.field_0.arr[3][2];
    a4->arr[1] = v13;
    a4->arr[2] = v14;
    v15 = v41.field_0.arr[2][1];
    a2->arr[0] = v41.field_0.arr[2][0];
    v16 = v41.field_0.arr[2][2];
    a2->arr[1] = v15;
    a2->arr[2] = v16;
    v17 = v41.field_0.arr[1][1];
    a3->arr[0] = v41.field_0.arr[1][0];
    v18 = v41.field_0.arr[1][2];
    a3->arr[1] = v17;
    a3->arr[2] = v18;
    if (interaction::get_approach(v4->field_30->field_30) == 1) {
        v19 = ai::base_state::get_actor(&v4->base);
        v20 = v19;
        if ((v19->base.field_8 >> 28) & 1)
            entity_base::update_abs_po(&v19->base, 1);
        v21 = (float *) v20->base.field_14;
        v22 = v21[14];
        v21 += 12;
        v23 = v22 - v12[14];
        v24 = v21[1] - v12[13];
        v25 = *v21;
        v26 = v4->field_30;
        v40.arr[0] = v25 - v12[12];
        v40.arr[1] = v24;
        v40.arr[2] = v23;
        v27 = (const vector3d *) v26->field_2C;
        v39[0] = v27->arr[2] - v23;
        v28 = v27->arr[1] - v24;
        if ((v27->arr[0] - v40.arr[0]) * (v27->arr[0] - v40.arr[0]) + v28 * v28 + v39[0] * v39[0] >
            EPSILON) {
            v39[0] = -calculate_xz_angle_relative_to_local_po_0(&po_identity_matrix_0, &v40, v27);
            *(_QWORD *) &v43.field_0.arr[0][0] = 0x3F800000i64;
            *(_QWORD *) &v43.field_0.arr[0][2] = 0i64;
            *(_QWORD *) &v43.field_0.arr[1][0] = 4575657221408423936i64;
            *(_QWORD *) &v43.field_0.arr[1][2] = 0i64;
            *(_QWORD *) &v43.field_0.arr[2][0] = 0i64;
            *(_QWORD *) &v43.field_0.arr[2][2] = 0x3F800000i64;
            *(_QWORD *) &v43.field_0.arr[3][0] = 0i64;
            *(_QWORD *) &v43.field_0.arr[3][2] = 4575657221408423936i64;
            po::set_rot(&v43, &UP(), (const vector3d *) LODWORD(v39[0]));
            v29 = po::non_affine_slow_xform(&v43, (vector3d *) v39, v27);
            a4->arr[0] = v29->arr[0];
            a4->arr[1] = v29->arr[1];
            a4->arr[2] = v29->arr[2];
            v30 = a4->arr[0] + v12[12];
            LODWORD(v39[1]) = &v43;
            a4->arr[0] = v30;
            LODWORD(v38[0]) = v39;
            v31 = v12[13] + a4->arr[1];
            LODWORD(v39[0]) = v12;
            a4->arr[1] = v31;
            a4->arr[2] = v12[14] + a4->arr[2];
            sub_415A30(&v41, (float *) LODWORD(v38[0]));
            if (v40.arr[2] * v41.field_0.arr[2][2] + v40.arr[1] * v41.field_0.arr[2][1] +
                    v41.field_0.arr[2][0] * v40.arr[0] <=
                0.0) {
                v32 = v41.field_0.arr[2][1];
                a2->arr[0] = v41.field_0.arr[2][0];
                v33 = v41.field_0.arr[2][2];
                a2->arr[1] = v32;
                a2->arr[2] = v33;
            } else {
                v40.arr[0] = -v41.field_0.arr[2][0];
                v40.arr[1] = -v41.field_0.arr[2][1];
                v40.arr[2] = -v41.field_0.arr[2][2];
                *a2 = v40;
            }
        }
    }
    if (v4->field_4C) {
        po::set_po(&v45, a2, a3, a4);
        qmemcpy(v4->field_4C, &v45, sizeof(po));
    } else {
        if (slab_allocator::get_max_object_size() < 0x40)
            v34 = (po *) operator new(0x40u);
        else
            v34 = (po *) slab_allocator::allocate(64, 0);
        v35 = v34;
        LODWORD(v39[0]) = v34;
        v46 = 1;
        if (v34) {
            po::set_po(v34, a2, a3, a4);
            v4->field_4C = v35;
        } else {
            v4->field_4C = 0;
        }
    }
    return v4->field_4C;
#else
    return bit_cast<po *>(THISCALL(0x0047E7F0, this, a4, a2, a3));
#endif
}

}  // namespace ai
