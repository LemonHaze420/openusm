#include "als_animation_logic_system.h"

#include "actor.h"
#include "als_animation_logic_system_shared.h"
#include "als_state.h"
#include "als_use_anim_only.h"
#include "common.h"
#include "func_wrapper.h"
#include "layer_state_machine_shared.h"
#include "physical_interface.h"
#include "resource_manager.h"
#include "time_interface.h"
#include "trace.h"
#include "traffic.h"
#include "utility.h"
#include "wds.h"

namespace als {

VALIDATE_SIZE(animation_logic_system, 0x80u);

animation_logic_system::animation_logic_system(actor *a1) {
    THISCALL(0x004ABB80, this, a1);
}

void animation_logic_system::change_mocomp() {
    THISCALL(0x00498F30, this);
}

void animation_logic_system::enter_biped_physics() {
    if constexpr (0) {
    } else {
        THISCALL(0x00498F70, this);
    }
}

void animation_logic_system::exit_biped_physics() {
    auto *v2 = this->field_6C->physical_ifc();
    v2->stop_biped_physics(false);

    v2->set_allow_manage_standing(true);
}

void animation_logic_system::sub_4A6630(layer_types a2)
{
    TRACE("als::animation_logic_system::sub_4A6630");

    THISCALL(0x004A6630, this, a2);
}

void animation_logic_system::create_instance_data(animation_logic_system_shared *a2)
{
    TRACE("als::animation_logic_system::create_instance_data");

    THISCALL(0x004ABC60, this, a2);
}

base_state_machine *animation_logic_system::get_als_layer_internal(layer_types a2)
{
    return (base_state_machine *) THISCALL(0x0049F300, this, a2);
}

void animation_logic_system::transition_layer(
        layer_types a2,
        string_hash a3)
{
    auto *the_layer = this->get_als_layer_internal(a2);
    if ( the_layer != nullptr ) {
        the_layer->set_active(this, a3);
    }
}

state_machine *animation_logic_system::get_als_layer(layer_types a2) {
    return this->get_als_layer_internal(a2);
}

void animation_logic_system::reset_animation_player()
{
    auto *the_controller = this->the_controller;
    if ( the_controller != nullptr ) {
        the_controller->reset();
    }
}

bool animation_logic_system::frame_advance_should_do_frame_advance([[maybe_unused]] Float a2) {
    TRACE("animation_logic_system::frame_advance_should_do_frame_advance");

    if (this->field_6C->has_time_ifc()) {
        this->field_6C->time_ifc();
    }

    if (this->the_controller == nullptr) {
        auto *v3 = this->field_6C;
        if (v3->anim_ctrl == nullptr) {
            v3->allocate_anim_controller(0, nullptr);
        }

        this->the_controller = CAST(the_controller, this->field_6C->anim_ctrl);
        assert(this->the_controller != nullptr);
    }

    auto *v4 = this->field_6C;
    if ((v4->field_4 & 0x40000000) != 0 ||
        ((v4->field_8 & 0x2000) != 0 || this->field_6C->get_primary_region() == nullptr) &&
            (v4->field_4 & 8) == 0) {
        return false;
    }

    if (this->field_7C) {
        return true;
    }

    return !traffic::is_unanimated_car(this->field_6C);
}

void animation_logic_system::frame_advance_post_logic_processing([[maybe_unused]] Float a2)
{
    TRACE("als::animation_logic_system::frame_advance_post_logic_processing");

    if constexpr (0) {
        if (!this->field_7C) {
            time_interface *time_ifc = nullptr;
            if (this->field_6C->has_time_ifc()) {
                time_ifc = this->field_6C->time_ifc();
            }

            if (this->field_6C->has_physical_ifc() && this->field_18.did_do_transition()) {
                if (!this->field_6C->physical_ifc()->is_biped_physics_running()
                        || this->field_18.is_curr_state_biped_physics()) {

                    if (this->field_18.is_curr_state_biped_physics()) {
                        if (!this->field_6C->physical_ifc()->is_biped_physics_running()) {
                            this->enter_biped_physics();
                        }
                    }
                } else {
                    this->exit_biped_physics();
                }
            }
        }

    } else {
        THISCALL(0x0049CC90, this, a2);
    }
}

float animation_logic_system::convert_layer_id_to_priority(layer_types a2) {
    float (__fastcall *func)(void *, void *, als::layer_types) = CAST(func, 0x0049F360);
    return func(this, nullptr, a2);
}

void animation_logic_system::frame_advance_play_new_animations(Float a2)
{
    TRACE("animation_logic_system::frame_advance_play_new_animations");

    {
        auto v5 = this->field_8.size();
        if (v5 == 1) {
            state_machine *the_state_machine = &this->field_18;
            auto &anim_handle = the_state_machine->get_anim_handle();
            if (auto *ctrl = anim_handle.field_8; ctrl != nullptr) {
                sp_log("0x%08X", ctrl->m_vtbl);
            }
        }
    }

    if constexpr (0) {
        if ( !this->field_7C ) {
            if ( this->field_6C->has_time_ifc() ) {
                this->field_6C->time_ifc();
            }

            auto func = [this](state_machine *the_state_machine, int i) {
                if ( the_state_machine->is_active() ) {
                    bool v19 = [](state_machine *the_state_machine, int i) -> bool { 
                        if (!the_state_machine->did_do_transition()) {
                            if (i != -1
                                || the_state_machine->get_anim_handle().is_anim_active()) {
                                return false;
                            }
                        }

                        return true;
                    }(the_state_machine, i);

                    if (v19 || this->field_7D) {
                        auto *curr_state = the_state_machine->get_curr_state();
                        assert(curr_state != nullptr);

                        auto anim_name = curr_state->get_nal_anim_name();

                        animation_controller::anim_ctrl_handle v9{};
                        if ( i == -1 )
                        {
                            float a4 = this->field_18.get_optional_pb_int(
                                                anim_start_frame_hash,
                                                0,
                                                nullptr) / 30.0;
                            auto v18 = curr_state->field_C;
                            v9 = this->the_controller->play_base_layer_anim(
                                                                        anim_name,
                                                                        a4,
                                                                        v18,
                                                                        true);
                        }
                        else
                        {
                            auto v10 = this->field_8[i]->shared_portion->field_40;
                            auto v11 = curr_state->field_C;
                            auto v19 = static_cast<als::layer_types>(the_state_machine->get_layer_id());
                            auto v12 = static_cast<als::layer_types>(the_state_machine->get_layer_id());
                            auto a5 = this->convert_layer_id_to_priority(v12);
                            v9 = this->the_controller->play_layer_anim(anim_name, v11, a5, v10, true, v19);
                        }

                        the_state_machine->field_48 = v9;
                        this->field_7D = false;
                    }

                }
            };


            auto *old_context = resource_manager::push_resource_context(this->field_6C->field_BC);
            for ( auto i = -1; i < this->field_8.size(); ++i ) {
                state_machine *the_state_machine = (i == -1 ? &this->field_18 : this->field_8[i]);
                func(the_state_machine, i);
            }

            resource_manager::pop_resource_context();
            assert(resource_manager::get_resource_context() == old_context);
        }
    } else {
        THISCALL(0x004A6400, this, a2);
    }
}

void animation_logic_system::frame_advance_change_mocomp(Float a2)
{
    TRACE("animation_logic_system::frame_advance_change_mocomp");

    THISCALL(0x00498DB0, this, a2);
}

void animation_logic_system::frame_advance_controller(Float a2)
{
    TRACE("animation_logic_system::frame_advance_controller");

    if constexpr (1) {
        double v4;
        if ( this->field_6C->has_time_ifc() ) {
            auto *v3 = this->field_6C->time_ifc();
            v4 = v3->sub_4ADE50();
        } else {
            v4 = g_world_ptr()->field_158.field_0;
        }

        auto a2a = v4 * a2;
        resource_manager::push_resource_context(this->field_6C->field_BC);
        this->the_controller->frame_advance(a2a, false, false);
        resource_manager::pop_resource_context();
    } else {
        THISCALL(0x00498E80, this, a2);
    }
}

void animation_logic_system::frame_advance_post_controller(Float arg0)
{
    TRACE("animation_logic_system::frame_advance_post_controller");

    if constexpr (0) {
        double v4;
        if ( this->field_6C->has_time_ifc() )
        {
            auto *v3 = this->field_6C->time_ifc();
            v4 = v3->sub_4ADE50();
        }
        else
        {
            v4 = g_world_ptr()->field_158.field_0;
        }

        auto arg0a = v4 * arg0;
        resource_manager::push_resource_context(this->field_6C->field_BC);
        if ( this->field_7C )
        {
            use_anim_only v6 {};
            v6.activate(this);
            v6.post_anim_action(arg0a);
        }
        else
        {
            auto *v1 = this->field_74;
            v1->post_anim_action(arg0a);
        }

        resource_manager::pop_resource_context();
    } else {
        THISCALL(0x004AB700, this, arg0);
    }
}

void animation_logic_system::frame_advance_post_request_processing(Float a2)
{
    TRACE("animation_logic_system::frame_advance_post_request_processing");

    THISCALL(0x0049F1A0, this, a2);
}

void animation_logic_system::frame_advance_main_als_advance(Float a2)
{
    TRACE("animation_logic_system::frame_advance_main_als_advance");

    if constexpr (1) {
        if ( !this->field_7C ) {
            this->field_7E = false;
            if ( this->field_6C->has_time_ifc() ) {
                this->field_6C->time_ifc();
            }

            static auto func = [this](auto &the_state_machine) {
                the_state_machine->process_requests(this);

                if ( the_state_machine->field_14 ) {
                    this->field_7E = true;
                }
            };

            sp_log("%d", this->field_8.size());

            auto *v6 = &this->field_18;
            func(v6);

            std::for_each(this->field_8.begin(), this->field_8.end(), [](auto *v6)
            {
                func(v6);
            });
        }
    } else {
        THISCALL(0x004A90B0, this, a2);
    }
}

void animation_logic_system::frame_advance_on_layer_trans(Float a2)
{
    TRACE("animation_logic_system::frame_advance_on_layer_trans");

    THISCALL(0x0049F220, this, a2);
}


} // namespace als

void animation_logic_system_patch()
{
    {
        FUNC_ADDRESS(address, &als::animation_logic_system::create_instance_data);
        set_vfunc(0x0088146C, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_should_do_frame_advance);
        set_vfunc(0x00881478, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_change_mocomp);
        set_vfunc(0x00881494, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_controller);
        set_vfunc(0x0088149C, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_post_controller);
        set_vfunc(0x008814A0, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_on_layer_trans);
        set_vfunc(0x00881484, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_post_request_processing);
        set_vfunc(0x00881480, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_main_als_advance);
        set_vfunc(0x0088147C, address);
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::frame_advance_play_new_animations);
        set_vfunc(0x0088148C, address);
    }
    return;


    {
        FUNC_ADDRESS(address, &als::animation_logic_system::enter_biped_physics);
        REDIRECT(0x0049CD21, address);
    }
}
