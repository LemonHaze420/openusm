#include "interaction_inode.h"

#include "ai_interaction_data.h"
#include "ai_player_controller.h"
#include "ai_state_run.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "base_ai_state_machine.h"
#include "base_state.h"
#include "conglom.h"
#include "line_info.h"
#include "physical_interface.h"
#include "state_machine.h"
#include "base_ai_core.h"
#include "anim_record.h"
#include "attach_state.h"
#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "generic_interaction.h"
#include "interaction_state.h"
#include "pick_up_state.h"
#include "put_down_state.h"
#include "throw_state.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "oldmath_po.h"
#include "native_info_node_table.h"
#include <functional>
#include "controller_inode.h"
#include "core_ai_resource.h"
#include "hit_react_state.h"
#include "interactable_interface.h"
#include "layer_state_machine.h"
#include "resource_manager.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace ai {

VALIDATE_SIZE(interaction_inode, 0x4C);

namespace {
void __fastcall interaction_advance(interaction_inode *self, void *, Float elapsed)
{
    self->frame_advance(elapsed);
}
void __fastcall interaction_activate(interaction_inode *self, void *, ai_core *core)
{
    self->activate(core);
}
void __fastcall interaction_deactivate(interaction_inode *self, void *)
{
    self->deactivate();
}
}

void *interaction_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<interaction_inode, 148> callbacks;
        callbacks[6] = reinterpret_cast<void *>(&native_inode::always_advance);
        callbacks[7] = reinterpret_cast<void *>(&interaction_advance);
        callbacks[8] = reinterpret_cast<void *>(&interaction_activate);
        callbacks[9] = reinterpret_cast<void *>(&interaction_deactivate);
        return callbacks;
    }();
    return table.data();
}

interaction_inode::interaction_inode(from_mash_in_place_constructor *a2) : info_node(a2)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[148]);
    this->field_40 = 0;
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->field_28 = 2;
    this->target_handle.field_0 = 0;
    this->field_34 = nullptr;
    this->target_interaction_type = 4;
    this->field_49 = 0;
    this->curr_status = 0;
}

interaction_inode::interaction_inode() : info_node()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[148]);
    this->field_48 = 0;
    this->field_49 = 0;
    this->field_40 = 0;
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->field_28 = 2;
    this->target_handle = {0};
    this->field_34 = nullptr;
    this->target_interaction_type = 4;
    this->field_49 = 0;
    this->curr_status = 0;
}

void interaction_inode::unmash(mash_info_struct *info, void *data)
{
    info_node::_unmash(info, data);
}

void interaction_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    target_handle.field_0 = 0;
    target_als = nullptr;
    curr_status = 0;
    field_3C = -1;
    set_flag_from_param(string_hash{"interact_throwable_enabled"}, static_cast<interaction_type_enum>(3));
    set_flag_from_param(string_hash{"interact_rescuable_enabled"}, static_cast<interaction_type_enum>(1));
    set_flag_from_param(string_hash{"interact_generic_enabled"}, static_cast<interaction_type_enum>(0));
    set_flag_from_param(string_hash{"interact_attachable_enabled"}, static_cast<interaction_type_enum>(2));
}

void interaction_inode::frame_advance(Float)
{
    int carry_status = 0;
    bool has_target = false;
    if (is_in_master_mode()) {
        const int safe = field_8->field_50.get_optional_pb_int(
            string_hash{static_cast<int>(to_hash("safe_to_put_down"))}, 0, nullptr);
        field_49 = safe != 0;
        const auto &put_down_offset = field_2C->field_70;
        if ((std::not_equal_to<float>{}(put_down_offset.x, 0.0f) || std::not_equal_to<float>{}(put_down_offset.y, 0.0f) || std::not_equal_to<float>{}(put_down_offset.z, 0.0f)) && safe == 1) {
            const auto offset = field_C->get_abs_po().non_affine_slow_xform(field_2C->field_70);
            line_info line;
            line.field_0 = field_C->get_abs_position();
            line.field_C = line.field_0 + offset;
            line.check_collision(*local_collision::entfilter_entity_no_capsules,
                                 *local_collision::obbfilter_lineseg_test, nullptr);
            if (line.collision) {
                field_49 = false;
            }
        }
        if (field_C->m_player_controller != nullptr &&
            field_C->m_player_controller->m_spidey_loco_mode == CRAWLING) {
            field_49 = false;
        }
        if (field_C->has_physical_ifc() &&
            field_C->physical_ifc()->field_84.get_volatile_ptr() != nullptr) {
            field_49 = false;
        }
        auto *state = field_8->my_base_machine->my_curr_state;
        if (state != nullptr && state->get_name() != run_state::default_id) {
            field_49 = false;
        }
        if (auto *target = get_target()) {
            auto *carried = static_cast<conglomerate *>(target);
            if (field_49) {
                carried->field_110 |= 0x100;
                carry_status = 2;
            } else {
                carried->field_110 &= ~0x100u;
                carry_status = 1;
            }
            has_target = true;
        }
    }
    if (has_target || field_3C != 0) {
        auto *als_node = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
        const als::param param{20, static_cast<float>(carry_status)};
        als_node->get_als_layer(static_cast<als::layer_types>(0))->set_desired_param(param);
    }
    field_3C = carry_status;
}

void interaction_inode::deactivate()
{
    if (is_in_master_mode()) {
        carry_shutdown_part1();
        carry_shutdown_part2(false);
    }
    clear_interaction(static_cast<interaction_result_enum>(2));
}

void interaction_inode::carry_shutdown_part1()
{
    auto *als_node = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    const bool was_suspended = (field_C->field_4 & 0x40000000) != 0;
    if (was_suspended) {
        field_C->unsuspend(true);
    }
    auto *owner = static_cast<conglomerate *>(field_C);
    als_node->kill_layer(static_cast<als::layer_types>(2));
    owner->get_my_als()->force_update();
    if (was_suspended) {
        field_C->suspend(true);
    }
}

void interaction_inode::carry_shutdown_part2(bool raise_put_down)
{
    const auto carried_handle = target_handle;
    field_C->remove_collision_ignorance(carried_handle.field_0);
    if (auto *target = carried_handle.get_volatile_ptr()) {
        if (target->get_ai_core() != nullptr) {
            target->remove_collision_ignorance(field_C->get_my_vhandle());
        }
    }
    if (auto *target = get_target()) {
        entity_set_abs_parent(target, nullptr);
        if (target->has_physical_ifc()) {
            target->physical_ifc()->enable(true);
        }
    }
    if (field_2C != nullptr) {
        field_2C->unregister_interactor(vhandle_type<actor>{field_C->get_my_vhandle().field_0});
    }
    if (raise_put_down) {
        event_manager::raise_event(event::PUT_DOWN, target_handle.field_0);
    }
    deslave_target(true);
}

void interaction_inode::clear_slave()
{
    if (!is_in_master_mode()) {
        return;
    }
    curr_status = 0;
    carry_shutdown_part1();
    field_C->remove_collision_ignorance(target_handle.field_0);
    if (field_C->has_physical_ifc()) {
        field_C->physical_ifc()->field_C &= ~0x100u;
    }
    if (auto *target = get_target()) {
        static_cast<conglomerate *>(target)->field_110 &= ~0x80u;
        target_als->get_system()->suspend_logic_system(false);
        if (target->has_physical_ifc()) {
            target->physical_ifc()->field_C &= ~0x100u;
        }
        target->get_ai_core()->remove_slave(vhandle_type<actor>{field_C->get_my_vhandle().field_0});
    }
    target_als = nullptr;
    target_handle.field_0 = 0;
}

void interaction_inode::deslave_target(bool request_exit_category)
{
    auto *target = get_target();
    clear_slave();
    if (target == nullptr) {
        return;
    }
    target->kill_interact_anim();
    target->set_collisions_active(true, true);
    target->set_allow_tunnelling_into_next_frame(true);
    if (auto *core = target->get_ai_core()) {
        auto *als_node = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
        auto *system = static_cast<conglomerate *>(target)->get_my_als();
        if (request_exit_category && field_2C->field_6C != string_hash{0}) {
            als_node->request_category_transition(field_2C->field_6C, static_cast<als::layer_types>(0),
                                                  true, false, false);
            const auto facing = -target->get_abs_po().get_z_facing();
            als::param_list params;
            params.add_param(0x37, facing);
            params.add_param(0x53, facing);
            als_node->get_als_layer(static_cast<als::layer_types>(0))->set_desired_params(params);
            params.clear();
        } else {
            system->get_als_layer(static_cast<als::layer_types>(0))
                ->force_als_state(string_hash{"Idle_No_Blend"}, static_cast<int>(0xDEADBEEF));
        }
        system->force_update();
    }
}

bool interaction_inode::is_eligible(string_hash, bool unrestricted)
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    auto *als = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    if (!als->get_als_layer(static_cast<als::layer_types>(0))->is_interruptable() ||
        (curr_status == 2 && !field_49))
        return false;
    if (field_8->my_base_machine->my_curr_state->get_name() == hit_react_state::default_id)
        return false;
    if (field_48)
        return true;
    const auto triggered = [controller](int button) {
        return controller->get_button(static_cast<controller_inode::eControllerButton>(button)).is_triggered();
    };
    const auto held = [controller](int button) {
        return controller->get_button(static_cast<controller_inode::eControllerButton>(button)).is_pressed();
    };
    if (triggered(1) &&
        attempt_interaction(0, unrestricted))
        return true;
    if (held(1) &&
        attempt_interaction(6, unrestricted))
        return true;
    if (triggered(3) &&
        attempt_interaction(2, unrestricted))
        return true;
    if (triggered(2) &&
        attempt_interaction(1, unrestricted))
        return true;
    if (triggered(4) &&
        attempt_interaction(3, unrestricted))
        return true;
    return attempt_interaction(5, unrestricted);
}

bool interaction_inode::attempt_interaction(int button, bool unrestricted)
{
    const bool scripted = button == 4;
    if ((!field_40 && !scripted) || curr_status == 1)
        return false;
    field_8->get_info_node(als_inode::default_id, true);
    bool chosen = false;
    if (curr_status == 2) {
        if (!field_49 || button == 5)
            return false;
        if ((field_40 & 2) != 0 && button == field_30->field_3C) {
            if (auto *target = target_handle.get_volatile_ptr()) {
                set_interaction(field_2C, target, {1}, field_30);
                chosen = true;
            }
        }
    } else {

        for (int type : {0, 1, 3, 2}) {
            if (scripted || (field_40 & (1 << type)) != 0) {
                chosen = attempt_interaction_type({type}, button, unrestricted);
                if (chosen)
                    break;
            }
        }
    }
    return target_handle.get_volatile_ptr() != nullptr && chosen;
}

bool interaction_inode::attempt_interaction_type(interaction_type_enum type, int button, bool unrestricted)
{

    auto &list = type.value == 3 ? interactable_interface::throw_list() : interactable_interface::generic_list();
    const auto &pose = field_C->get_abs_po();
    const vector3d position = pose.get_position();
    const vector3d facing = pose.get_z_facing();
    const float threshold = unrestricted ? std::numeric_limits<float>::max()
        : static_cast<float>(std::cos(1.5707963705062866));
    float nearest = std::numeric_limits<float>::max();
    interaction *chosen = nullptr;
    vhandle_type<actor> chosen_actor{0};
    for (int index = 0; index < list.m_size;) {
        auto *target = list.m_data[index].get_volatile_ptr();
        if (target == nullptr) {
            if (index + 1 < list.m_size)
                std::memmove(list.m_data + index, list.m_data + index + 1,
                             sizeof(list.m_data[0]) * (list.m_size - index - 1));
            --list.m_size;
            continue;
        }
        ++index;
        interaction *triggered = nullptr;
        for (auto *entry : target->m_interactable_ifc->field_4) {
            if (entry->field_28.value != type.value || entry->field_3C != button ||
                !entry->field_44 || entry->field_38 > 0.0f)
                continue;
            using contains_fn = bool (__fastcall *)(interaction *, void *, const vector3d *, actor *);
            if (reinterpret_cast<contains_fn>(get_vfunc(entry->m_vtbl, 0x18))(
                    entry, nullptr, &position, target->m_interactable_ifc->field_0)) {
                triggered = entry;
                break;
            }
        }
        if (triggered == nullptr)
            continue;
        bool permitted = triggered->field_18.m_size == 0;
        for (int allowed = 0; !permitted && allowed < triggered->field_18.m_size; ++allowed)
            permitted = triggered->field_18.m_data[allowed].field_0 == field_C->get_my_vhandle().field_0;
        if (!permitted)
            continue;
        vector3d direction = target->get_abs_position() - position;
        const float squared = direction.length2();
        if (squared >= nearest)
            continue;
        if (!(threshold <= std::numeric_limits<float>::max() &&
              threshold >= std::numeric_limits<float>::max()) && !triggered->field_46) {
            if (squared > 0.0f)
                direction *= 1.0f / std::sqrt(squared);
            if (dot(direction, facing) < threshold)
                continue;
        }
        nearest = squared;
        chosen = triggered;
        chosen_actor = {target->get_my_vhandle().field_0};
    }
    auto *target = chosen_actor.get_volatile_ptr();
    if (target == nullptr)
        return false;
    resource_manager::push_resource_context(field_8->field_6C->field_3C);
    ai_interaction_data *data = nullptr;
    if (chosen->field_28.value >= 0 && chosen->field_28.value <= 2) {
        using resource_fn = ai_interaction_data *(__fastcall *)(interaction *, void *);
        data = reinterpret_cast<resource_fn>(get_vfunc(chosen->m_vtbl, 0x20))(chosen, nullptr);
    }
    set_interaction(data, target, type, chosen);
    resource_manager::pop_resource_context();
    return true;
}

void interaction_inode::set_curr_anim(anim_key *the_anim_key)
{
    assert(the_anim_key != nullptr);
    this->field_34 = the_anim_key;
}

void interaction_inode::set_curr_anim(enum_anim_key::key_enum a2)
{
    TRACE("interaction_inode::set_curr_anim:");

    if constexpr (0) {
        auto *the_record = this->field_2C->does_anim_exist(a2, false);
        if (the_record != nullptr) {
            assert(the_record->my_key != nullptr);
            this->set_curr_anim(the_record->my_key);
        } else {
            this->field_34 = nullptr;
        }
    } else {
        THISCALL(0x00451340, this, a2);
    }
}

bool interaction_inode::is_in_master_mode()
{
    return this->curr_status == 2;
}

void interaction_inode::init_interaction()
{
    this->field_2C = nullptr;
    this->field_30 = nullptr;
    this->target_handle.field_0 = 0;
    this->target_interaction_type = 4;
    this->field_34 = nullptr;
    this->field_49 = false;
    this->curr_status = 0;
}

void interaction_inode::clear_interaction(interaction_result_enum result)
{
    field_28 = result;
    if (result == 0) {
        event_manager::raise_event(event::INTERACTION_FAILURE, target_handle.field_0);
    } else if (result == 1) {
        if (field_30 != nullptr && field_30->field_45) {
            field_30->set_enabled(false);
        }
        event_manager::raise_event(event::INTERACTION_SUCCESS, target_handle.field_0);
    } else if (result == 2 && !is_interacting()) {
        return;
    }
    if (auto *target = get_target()) {
        target->kill_interact_anim();
    }
    if (!is_in_master_mode()) {
        if (field_2C != nullptr) {
            field_2C->unregister_interactor(vhandle_type<actor>{field_C->get_my_vhandle().field_0});
        }
        init_interaction();
    }
}


string_hash interaction_inode::get_chosen_interact_state_id()
{
    static constexpr auto NUM_INTERACT_TYPES = 4;

    assert(target_interaction_type != NUM_INTERACT_TYPES);

    string_hash result;

    switch (this->target_interaction_type) {
    case 0:
        result = interaction_state::default_id;
        break;
    case 1:

        if (this->is_in_master_mode()) {
            result = put_down_state::default_id;
        } else {
            result = pick_up_state::default_id;
        }

        break;
    case 2:
        result = ai::attach_state::default_id;
        break;
    case 3:
        result = throw_state::default_id;
        break;
    default:
        assert(0 && "Unhandled interaction type.");

        result = string_hash{};

        break;
    }

    return result;
}

actor *interaction_inode::get_target()
{
    actor *target = nullptr;

    if (this->target_handle.get_volatile_ptr()) {
        target = this->target_handle.get_volatile_ptr();
    }

    return target;
}

void interaction_inode::set_flag_from_param(const string_hash &a2, interaction_type_enum a3)
{
    auto &params = field_8->field_50;
    if (params.does_parameter_exist(a2) && params.get_pb_int(a2)) {
        field_40 |= 1 << static_cast<int>(a3);
    }
}

void interaction_inode::set_interaction(const ai_interaction_data *a2, actor *a3, interaction_type_enum a4,
                                        interaction *a5)
{
    assert(is_in_master_mode() || !is_interacting());

    if (!this->is_in_master_mode()) {
        this->curr_status = 1;
    }

    this->target_handle = {a3->my_handle};
    this->field_2C = CAST(this->field_2C, a2);
    if (this->field_2C != nullptr) {
        this->field_2C->register_interactor(vhandle_type<actor>{field_C->get_my_vhandle().field_0});
    }

    this->field_30 = a5;
    this->target_interaction_type = a4;
    this->field_28 = 2;
}

void interaction_inode::set_scripted_start(actor *a2, generic_interaction *a3)
{
    if constexpr (1) {
        this->field_48 = true;
        interaction_type_enum v4 = a3->field_28;

        ai_interaction_data *(__fastcall * get_interaction_data)(void *) =
            CAST(get_interaction_data, get_vfunc(a3->m_vtbl, 0x20));

        auto *v5 = get_interaction_data(a3);
        this->set_interaction(v5, a2, v4, a3);
    } else {
        THISCALL(0x0046E380, this, a2, a3);
    }
}

}  // namespace ai

void interaction_inode_patch()
{
    void (ai::interaction_inode::*func)(enum_anim_key::key_enum) = &ai::interaction_inode::set_curr_anim;
    FUNC_ADDRESS(address, func);
    REDIRECT(0x00463C59, address);

    {
        FUNC_ADDRESS(address, &ai::interaction_inode::is_eligible);
        REDIRECT(0x00488711, address);
        REDIRECT(0x00488B15, address);
        REDIRECT(0x00488DCD, address);
        REDIRECT(0x00488FEE, address);
    }
}
