#include "als_animation_logic_system.h"

#include "actor.h"
#include "als_animation_logic_system_shared.h"
#include "als_state.h"
#include "als_use_anim_only.h"
#include "biped_physics.h"
#include "common.h"
#include "func_wrapper.h"
#include "layer_state_machine.h"
#include "layer_state_machine_shared.h"
#include "nal_anim_controller.h"
#include "memory.h"
#include "physical_interface.h"
#include "resource_manager.h"
#include "time_interface.h"
#include "trace.h"
#include "traffic.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"

namespace als {

VALIDATE_SIZE(animation_logic_system, 0x80u);

animation_logic_system::animation_logic_system(actor *a1)
{
    TRACE("animation_logic_system::animation_logic_system");

    if constexpr (STANDALONE_SYSTEM) {
        static void *g_vtbl[] = {func_address(&animation_logic_system::_get_als_layer),
                                 func_address(&animation_logic_system::_kill_all_domains),
                                 func_address(&animation_logic_system::_suspend_logic_system),
                                 func_address(&animation_logic_system::_create_instance_data),
                                 func_address(&animation_logic_system::_delete_instance_data),
                                 func_address(&animation_logic_system::_reset_animation_player),
                                 func_address(&animation_logic_system::_frame_advance_should_do_frame_advance),
                                 func_address(&animation_logic_system::_frame_advance_main_als_advance),
                                 func_address(&animation_logic_system::_frame_advance_post_request_processing),
                                 func_address(&animation_logic_system::_frame_advance_on_layer_trans),
                                 func_address(&animation_logic_system::_frame_advance_post_logic_processing),
                                 func_address(&animation_logic_system::_frame_advance_play_new_animations),
                                 func_address(&animation_logic_system::_frame_advance_update_pending_params),
                                 func_address(&animation_logic_system::_frame_advance_change_mocomp),
                                 func_address(&animation_logic_system::_frame_advance_run_mocomp_pre_anim),
                                 func_address(&animation_logic_system::_frame_advance_controller),
                                 func_address(&animation_logic_system::_frame_advance_post_controller),
                                 func_address(&animation_logic_system::_sub_4933E0),
                                 func_address(&animation_logic_system::_change_mocomp)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
        this->field_7C = false;
        this->field_7D = false;
        this->field_74 = nullptr;
        this->field_6C = a1;
        this->the_controller = nullptr;
        this->als_shared = nullptr;

        auto allocate = [](uint32_t sz) -> void * {
            if (slab_allocator::get_max_object_size() < sz) {
                return ::operator new(sz);
            }
            return slab_allocator::allocate(sz, nullptr);
        };

        this->field_78 = allocate(0x80u);

        value_t entry{};
        entry.field_0 = this;
        the_als_list.push_back(entry);
    } else {
        THISCALL(0x004ABB80, this, a1);
    }
}

animation_logic_system::~animation_logic_system()
{
    if constexpr (!STANDALONE_SYSTEM) {
        this->m_vtbl = 0x00881460;
    }
    auto *v2 = this->field_74;
    if (v2 != nullptr) {
        v2->deactivate();
        v2->finalize(false);
        this->field_74 = nullptr;
    }

    auto *v3 = this->field_78;
    auto func = [](void *a1) -> void {
        auto *slab_for_object = slab_allocator::find_slab_for_object(a1);
        if (slab_for_object != nullptr) {
            slab_allocator::deallocate(a1, slab_for_object);
        } else {
            ::operator delete(a1);
        }
    };

    func(v3);
    this->field_78 = nullptr;

    remove_from_als_list(this);
}

void *animation_logic_system::operator new(std::size_t sz)
{
    return mem_alloc(sz);
}

void animation_logic_system::operator delete(void *ptr, std::size_t sz)
{
    mem_dealloc(ptr, sz);
}

als_meta_anim_table_shared *animation_logic_system::get_meta_anim_table()
{
    return this->als_shared->field_18;
}

bool animation_logic_system::sub_49F2A0()
{
    auto func = [](auto *v4) -> bool {
        return (v4->is_active() && v4->get_curr_state()->is_flag_set(static_cast<state_flags>(2u)));
    };

    if (func(&this->field_18)) {
        return false;
    }

    for (auto &the_machine : this->field_8) {
        if (func(the_machine)) {
            return false;
        }
    }

    return true;
}

bool animation_logic_system::_sub_4933E0()
{
    if (this->the_controller != nullptr) {
        return this->the_controller->sub_49C180();
    }

    return false;
}

#if STANDALONE_SYSTEM
static matrix4x4 *dword_959568 = nullptr;
static matrix4x4 *dword_95956C = nullptr;
static int s_bone_count = 0;
#else
static auto &dword_959568 = var<matrix4x4 *>(0x00959568);
static auto &dword_95956C = var<matrix4x4 *>(0x0095956C);
static int &s_bone_count = var<int>(0x00959564);
#endif

static constexpr int MAX_PHYS_ANIM_BONES = 90;

void sub_493210(entity_base *a1)
{
    dword_959568[s_bone_count] = a1->get_abs_po().m;

    assert(s_bone_count > 0 && s_bone_count <= MAX_PHYS_ANIM_BONES);

    dword_95956C[s_bone_count] = a1->get_rel_po().m;

    for (auto *the_child = a1->get_first_child(); the_child != nullptr; the_child = the_child->field_28) {
        if (the_child->get_bone_idx() != -1) {
            sub_493210(the_child);
        }
    }
}

void sub_4932B0(entity_base *a1)
{
    a1->my_abs_po->m = dword_959568[s_bone_count];

    a1->get_rel_po().m = dword_95956C[s_bone_count];

    a1->set_ext_flag_recursive_internal(static_cast<entity_ext_flag_t>(0x10000000u), false);

    assert(s_bone_count > 0 && s_bone_count <= MAX_PHYS_ANIM_BONES);

    for (auto *the_child = a1->get_first_child(); the_child != nullptr; the_child = the_child->field_28) {
        if (the_child->get_bone_idx() == -1) {
            the_child->dirty_family(true);
        } else {
            sub_4932B0(the_child);
        }
    }
}

void animation_logic_system::enter_biped_physics()
{
    TRACE("animation_logic_system::enter_biped_physics");

    if constexpr (1) {
        auto *the_actor = this->field_6C;

        assert(the_actor->is_a_conglomerate());
        assert(the_actor->has_physical_ifc() && "Cannot run biped physics on something that has no physical interface");

        if ((this->field_18.get_curr_state()->field_C & 0x2000) != 0) {
            auto *v4 = the_actor->physical_ifc();
            v4->set_velocity(ZEROVEC, false);
        } else {
            matrix4x4 v18[90]{};
            matrix4x4 v19[90]{};
            dword_959568 = v18;
            dword_95956C = v19;

            s_bone_count = 0;
            sub_493210(the_actor);

            biped_physics::capture_frame(bit_cast<conglomerate *>(this->field_6C), 0);
            this->field_74->pre_anim_action(0.033333335);
            this->the_controller->frame_advance(0.033333335, false, false);
            this->field_74->post_anim_action(0.033333335f);
            biped_physics::capture_frame(bit_cast<conglomerate *>(this->field_6C), 1);
            biped_physics::set_capture_frame_delta(0.033333335f);
            auto v13 = this->field_6C;

            s_bone_count = 0;
            sub_4932B0(v13);
        }

        physical_interface::biped_physics_body_types a2 = static_cast<physical_interface::biped_physics_body_types>(0);

        static string_hash phys_character_type_hash{int(to_hash("phys_character_type"))};

        static string_hash spiderman_hash{to_hash("spiderman")};

        static string_hash venom_hash{to_hash("venom")};

        auto *v6 = &this->field_18;
        if (v6->does_parameter_exist(phys_character_type_hash)) {
            if (v6->get_parameter_data_type(phys_character_type_hash) == 2) {
                auto v17 = v6->get_pb_hash(phys_character_type_hash);
                if (v17 == spiderman_hash) {
                    a2 = static_cast<physical_interface::biped_physics_body_types>(1);
                } else if (v17 == venom_hash) {
                    a2 = static_cast<physical_interface::biped_physics_body_types>(2);
                }
            }
        }

        the_actor->physical_ifc()->field_9C = 0.1f;
        auto *v10 = the_actor->physical_ifc();
        v10->start_biped_physics(a2);
        auto *v12 = the_actor->physical_ifc();
        v12->set_allow_manage_standing(0);
    } else {
        THISCALL(0x00498F70, this);
    }
}

void animation_logic_system::exit_biped_physics()
{
    auto *v2 = this->field_6C->physical_ifc();
    v2->stop_biped_physics(false);

    v2->set_allow_manage_standing(true);
}

void animation_logic_system::sub_4A6630(layer_types a2)
{
    TRACE("als::animation_logic_system::sub_4A6630");

    THISCALL(0x004A6630, this, a2);
}

void animation_logic_system::suspend_logic_system(bool a2)
{
    TRACE("animation_logic_system::suspend_logic_system");

    if (this->field_7C && !a2) {
        this->field_7D = true;
    }

    this->field_7C = a2;
}

void animation_logic_system::_create_instance_data(animation_logic_system_shared *system_shared)
{
    TRACE("als::animation_logic_system::create_instance_data");

    if constexpr (1) {
        assert(als_shared == nullptr);

        this->als_shared = system_shared;
        this->field_18.init(this->als_shared->field_14);

        auto &list = this->als_shared->field_0;
        std::transform(list.begin(), list.end(), std::back_inserter(this->field_8), [](auto &machine_shared) {
            auto *mem = mem_alloc(sizeof(layer_state_machine));
            auto *v11 = new (mem) layer_state_machine{};
            v11->init(machine_shared);
            return v11;
        });

        this->change_mocomp();
    } else {
        THISCALL(0x004ABC60, this, system_shared);
    }
}

void animation_logic_system::_delete_instance_data()
{
    TRACE("animation_logic_system::delete_instance_data");

    for (auto &the_state_machine : this->field_8) {
        if (the_state_machine != nullptr) {
            void(__fastcall * finalize)(void *, void *, bool) =
                CAST(finalize, get_vfunc(the_state_machine->m_vtbl, 0x60u));
            finalize(the_state_machine, nullptr, true);
        }
    }

    this->field_8.clear();
}

base_state_machine *animation_logic_system::get_als_layer_internal(layer_types a2)
{
    TRACE("animation_logic_system::get_als_layer_internal");

    if constexpr (1) {
        auto *v3 = &this->field_18;
        if (a2 == v3->get_layer_id()) {
            return v3;
        }

        for (uint32_t i = 0; i < this->field_8.size(); ++i) {
            if (a2 == this->field_8[i]->get_layer_id()) {
                return (base_state_machine *)this->field_8[i];
            }
        }

        return nullptr;
    } else {
        return (base_state_machine *)THISCALL(0x0049F300, this, a2);
    }
}

void animation_logic_system::transition_layer(layer_types a2, string_hash a3)
{
    auto *the_layer = this->get_als_layer_internal(a2);
    if (the_layer != nullptr) {
        the_layer->set_active(this, a3);
    }
}

state_machine *animation_logic_system::_get_als_layer(layer_types a2)
{
    return this->get_als_layer_internal(a2);
}

void animation_logic_system::_kill_all_domains(uint32_t a2)
{
    TRACE("animation_logic_system::kill_all_domains");

    for (uint32_t i = 0; i < this->field_8.size(); ++i) {
        if ((a2 & this->field_8[i]->shared_portion->field_40) != 0) {
            this->field_8[i]->kill_layer();
        }
    }
}

void animation_logic_system::_suspend_logic_system(bool a2)
{
    if (this->field_7C && !a2) {
        this->field_7D = true;
    }

    this->field_7C = a2;
}

void animation_logic_system::_reset_animation_player()
{
    auto *the_controller = this->the_controller;
    if (the_controller != nullptr) {
        the_controller->reset();
    }
}

bool animation_logic_system::_frame_advance_should_do_frame_advance([[maybe_unused]] Float a2)
{
    TRACE("animation_logic_system::frame_advance_should_do_frame_advance");

    if (this->field_6C->has_time_ifc()) {
        this->field_6C->time_ifc();
    }

    if (this->the_controller == nullptr) {
        auto *v3 = this->field_6C;
        if (v3->anim_ctrl == nullptr) {
            v3->allocate_anim_controller(0, nullptr);
        }

        this->the_controller = this->field_6C->anim_ctrl;
        assert(this->the_controller != nullptr);
    }

    auto *v4 = this->field_6C;
    if (v4->is_suspended() || v4->is_in_limbo()) {
        return false;
    }

    if (this->field_7C) {
        return true;
    }

    return !traffic::is_unanimated_car(this->field_6C);
}

void animation_logic_system::_frame_advance_post_logic_processing([[maybe_unused]] Float a2)
{
    TRACE("animation_logic_system::frame_advance_post_logic_processing");

    if constexpr (STANDALONE_SYSTEM) {
        if (!this->field_7C) {
            [[maybe_unused]] time_interface *time_ifc =
                (this->field_6C->has_time_ifc() ? this->field_6C->time_ifc() : nullptr);

            if (this->field_6C->has_physical_ifc() && this->field_18.did_do_transition()) {
                if (!this->field_6C->physical_ifc()->is_biped_physics_running() ||
                    this->field_18.is_curr_state_biped_physics()) {
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

float animation_logic_system::convert_layer_id_to_priority(layer_types a2)
{
    float(__fastcall * func)(void *, void *, als::layer_types) = CAST(func, 0x0049F360);
    return func(this, nullptr, a2);
}

void animation_logic_system::_frame_advance_play_new_animations(Float a2)
{
    TRACE("animation_logic_system::frame_advance_play_new_animations");

    if constexpr (1) {
        if (!this->field_7C) {
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            auto func = [this](state_machine *the_state_machine, int i) {
                if (!the_state_machine->is_active())
                    return;

                bool v19 = [](state_machine *the_state_machine, int i) -> bool {
                    if (!the_state_machine->did_do_transition()) {
                        if (i != -1 || the_state_machine->get_anim_handle().is_anim_active()) {
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
                    if (i == -1) {
                        float a4 = this->field_18.get_optional_pb_int(anim_start_frame_hash, 0, nullptr) / 30.0;
                        auto v18 = curr_state->field_C;
                        v9 = this->the_controller->play_base_layer_anim(anim_name, a4, v18, true);
                    } else {
                        auto v10 = bit_cast<layer_state_machine *>(this->field_8[i])->get_domain_bitmask();
                        auto v11 = curr_state->field_C;
                        auto v19 = static_cast<als::layer_types>(the_state_machine->get_layer_id());
                        auto v12 = static_cast<als::layer_types>(the_state_machine->get_layer_id());
                        auto a5 = this->convert_layer_id_to_priority(v12);
                        v9 = this->the_controller->play_layer_anim(anim_name, v11, a5, v10, true, v19);
                    }

                    the_state_machine->set_anim_handle(v9);
                    this->field_7D = false;
                }
            };

            auto *old_context = resource_manager::push_resource_context(this->field_6C->get_resource_context());

            func(&this->field_18, -1);
            for (auto i = 0u; i < this->field_8.size(); ++i) {
                state_machine *the_state_machine = this->field_8[i];
                func(the_state_machine, i);
            }

            resource_manager::pop_resource_context();
            assert(resource_manager::get_resource_context() == old_context);
        }
    } else {
        THISCALL(0x004A6400, this, a2);
    }
}

void animation_logic_system::_frame_advance_update_pending_params(Float a2)
{
    TRACE("animation_logic_system::frame_advance_update_pending_params");

    if constexpr (STANDALONE_SYSTEM) {
        if (!this->field_7C) {
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            this->field_18.update_pending_params(this);

            for (auto &the_machine : this->field_8) {
                the_machine->update_pending_params(this);
            }
        }
    } else {
        THISCALL(0x004A65C0, this, a2);
    }
}

void animation_logic_system::_change_mocomp()
{
    TRACE("animation_logic_system::change_mocomp");

    if constexpr (1) {
        auto *v2 = this->field_74;
        if (v2 != nullptr) {
            v2->deactivate();
            v2->finalize(false);
        }

        auto *v5 = static_cast<mash_virtual_base *>(this->field_78);
        auto v3 = static_cast<mash::virtual_types_enum>(this->field_18.m_curr_state->get_mocomp_type());
        this->field_74 = (motion_compensator *)mash_virtual_base::create_subclass_by_enum_in_place(
            v3, v5, motion_compensator::get_size_of_memory_block());
        this->field_74->activate(this);
    } else {
        THISCALL(0x00498F30, this);
    }
}

void animation_logic_system::_frame_advance_change_mocomp(Float a2)
{
    TRACE("animation_logic_system::frame_advance_change_mocomp");

    if constexpr (1) {
        if (!this->field_7C) {
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            if (this->field_18.field_14.m_trans_succeed) {
                auto *v3 = this->field_18.m_curr_state;
                int v4 = this->field_74->get_virtual_type_enum();
                if (v4 != v3->get_mocomp_type() || (this->field_18.m_curr_state->field_C & 0x200) != 0) {
                    this->change_mocomp();
                }
            }
        }
    } else {
        THISCALL(0x00498DB0, this, a2);
    }
}

void animation_logic_system::_frame_advance_run_mocomp_pre_anim(Float a2)
{
    if (!this->field_7C) {
        float v4;
        if (this->field_6C->has_time_ifc()) {
            auto *v3 = this->field_6C->time_ifc();
            v4 = v3->sub_4ADE50() * a2;
        } else {
            v4 = g_world_ptr->field_158.field_0 * a2;
        }

        this->field_74->pre_anim_action(v4);
    }
}

void animation_logic_system::_frame_advance_controller(Float a2)
{
    TRACE("animation_logic_system::frame_advance_controller");

    if constexpr (1) {
        double v4;
        if (this->field_6C->has_time_ifc()) {
            auto *v3 = this->field_6C->time_ifc();
            v4 = v3->sub_4ADE50();
        } else {
            v4 = g_world_ptr->field_158.field_0;
        }

        auto a2a = v4 * a2;
        resource_manager::push_resource_context(this->field_6C->get_resource_context());
        this->the_controller->frame_advance(a2a, false, false);
        resource_manager::pop_resource_context();
    } else {
        THISCALL(0x00498E80, this, a2);
    }
}

void animation_logic_system::_frame_advance_post_controller(Float a1)
{
    TRACE("animation_logic_system::frame_advance_post_controller");

    if constexpr (STANDALONE_SYSTEM) {
        double v4;
        if (this->field_6C->has_time_ifc()) {
            auto *v3 = this->field_6C->time_ifc();
            v4 = v3->sub_4ADE50();
        } else {
            v4 = g_world_ptr->field_158.field_0;
        }

        auto v9 = v4 * a1;
        resource_manager::push_resource_context(this->field_6C->get_resource_context());
        if (this->field_7C) {
            use_anim_only v6{};
            v6.activate(this);
            v6.post_anim_action(v9);
        } else {
            auto *v1 = this->field_74;
            v1->post_anim_action(v9);
        }

        resource_manager::pop_resource_context();
    } else {
        THISCALL(0x004AB700, this, a1);
    }
}

void animation_logic_system::_frame_advance_post_request_processing(Float a2)
{
    TRACE("animation_logic_system::frame_advance_post_request_processing");

    if constexpr (1) {
        if (!this->field_7C) {
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            for (int i = this->field_8.size() - 1; i >= -1; --i) {
                auto *v7 = (i == -1 ? &this->field_18 : this->field_8[i]);
                if (v7->is_active() && v7->curr_req_data.field_C.field_0 != nullptr) {
                    v7->process_post_requests(this);
                    this->field_7E = true;
                }
            }
        }
    } else {
        THISCALL(0x0049F1A0, this, a2);
    }
}

void animation_logic_system::_frame_advance_main_als_advance(Float a2)
{
    TRACE("animation_logic_system::frame_advance_main_als_advance");

    if constexpr (STANDALONE_SYSTEM) {
        if (!this->field_7C) {
            this->field_7E = false;
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            for (int i = -1; i < static_cast<int>(this->field_8.size()); ++i) {
                state_machine &the_machine = (i == -1 ? this->field_18 : *this->field_8[i]);

                the_machine.process_requests(this);

                if (the_machine.did_do_transition()) {
                    this->field_7E = true;
                }
            }
        }
    } else {
        THISCALL(0x004A90B0, this, a2);
    }
}

void animation_logic_system::_frame_advance_on_layer_trans(Float a2)
{
    TRACE("animation_logic_system::frame_advance_on_layer_trans");

    if constexpr (1) {
        if (!this->field_7C && this->field_7E) {
            if (this->field_6C->has_time_ifc()) {
                this->field_6C->time_ifc();
            }

            for (int i = this->field_8.size() - 1; i >= -1; --i) {
                auto *v7 = (i == -1 ? &this->field_18 : this->field_8[i]);
                if (v7->is_active()) {
                    v7->process_layer_response_rules(this);
                }
            }
        }
    } else {
        THISCALL(0x0049F220, this, a2);
    }
}

}  // namespace als

void animation_logic_system_patch()
{
    {
        auto constexpr address_vtbl = 0x00881460;

        set_vfunc(address_vtbl + 0x0, func_address(&als::animation_logic_system::_get_als_layer));
        set_vfunc(address_vtbl + 0x4, func_address(&als::animation_logic_system::_kill_all_domains));
        set_vfunc(address_vtbl + 0x8, func_address(&als::animation_logic_system::_suspend_logic_system));
        set_vfunc(address_vtbl + 0xC, func_address(&als::animation_logic_system::_create_instance_data));
        set_vfunc(address_vtbl + 0x10, func_address(&als::animation_logic_system::_delete_instance_data));
        set_vfunc(address_vtbl + 0x14, func_address(&als::animation_logic_system::_reset_animation_player));
        set_vfunc(address_vtbl + 0x18,
                  func_address(&als::animation_logic_system::_frame_advance_should_do_frame_advance));
        set_vfunc(address_vtbl + 0x1C, func_address(&als::animation_logic_system::_frame_advance_main_als_advance));
        set_vfunc(address_vtbl + 0x20,
                  func_address(&als::animation_logic_system::_frame_advance_post_request_processing));
        set_vfunc(address_vtbl + 0x24, func_address(&als::animation_logic_system::_frame_advance_on_layer_trans));
        set_vfunc(address_vtbl + 0x28,
                  func_address(&als::animation_logic_system::_frame_advance_post_logic_processing));
        set_vfunc(address_vtbl + 0x2C, func_address(&als::animation_logic_system::_frame_advance_play_new_animations));
        set_vfunc(address_vtbl + 0x30,
                  func_address(&als::animation_logic_system::_frame_advance_update_pending_params));
        set_vfunc(address_vtbl + 0x34, func_address(&als::animation_logic_system::_frame_advance_change_mocomp));
        set_vfunc(address_vtbl + 0x38, func_address(&als::animation_logic_system::_frame_advance_run_mocomp_pre_anim));
        set_vfunc(address_vtbl + 0x3C, func_address(&als::animation_logic_system::_frame_advance_controller));
        set_vfunc(address_vtbl + 0x40, func_address(&als::animation_logic_system::_frame_advance_post_controller));
        set_vfunc(address_vtbl + 0x44, func_address(&als::animation_logic_system::_sub_4933E0));
        set_vfunc(address_vtbl + 0x48, func_address(&als::animation_logic_system::_change_mocomp));
    }

    {
        FUNC_ADDRESS(address, &als::animation_logic_system::enter_biped_physics);
        REDIRECT(0x0049CD21, address);
    }
}
