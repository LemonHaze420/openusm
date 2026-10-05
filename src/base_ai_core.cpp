#include <algorithm>
#include <cmath>

#include "base_ai_core.h"
#include "core_ai_resource.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "collision_geometry.h"
#include "ai_pedestrian.h"
#include "ai_std_combat_target.h"
#include "base_ai_graph_manager.h"
#include "base_ai_state_machine.h"
#include "colgeom_alter_sys.h"
#include "combat_inode.h"
#include "common.h"
#include "core_ai_resource.h"
#include "debugutil.h"
#include "func_wrapper.h"
#include "ghetto_mash_file_header.h"
#include "info_node.h"
#include "loco_inode.h"
#include "slave_inode.h"
#include "memory.h"
#include "mstring.h"
#include "resource_manager.h"
#include "trace.h"
#include "traffic_inode.h"
#include "utility.h"
#include "wds.h"
#include "vtbl.h"
#include "als_inode.h"
#include "als_animation_logic_system.h"
#include "param_list.h"
#include "camera.h"
#include "game.h"
#include "physical_interface.h"
#include "renderoptimizations.h"
#include <limits>
#include "conglom.h"

namespace ai {

VALIDATE_SIZE(ai_core, 0x74u);

_std::list<ai_core *> *&ai_core::the_ai_core_list_high = var<_std::list<ai_core *> *>(0x0096BE24);

_std::list<ai_core *> *&ai_core::the_ai_core_list_low = var<_std::list<ai_core *> *>(0x0096BE28);

void *&ai_core::next_ai_core_list_low_iter = var<void *>(0x0096C110);

ai_core::~ai_core()
{
    while (!my_machine_list.empty()) {
        for (auto it = my_machine_list.begin(); it != my_machine_list.end();) {
            auto *machine = it->machine;
            if (machine->field_1C.empty()) {
                delete machine;
                it = my_machine_list.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (int index = 0; index < my_info_node_list->m_size; ++index)
        my_info_node_list->m_data[index]->deactivate();
    if (!pedestrian_inode::is_a_pedestrian(this) && get_info_node(traffic_inode::default_id, false) == nullptr)
        pedestrian_inode::unregister_non_ped(vhandle_type<actor>{field_64->my_handle});
    if (field_5C != 0) {
        if (my_info_node_list->field_10) {
            for (int index = 0; index < my_info_node_list->m_size; ++index) {
                auto *node = my_info_node_list->m_data[index];
                if (my_info_node_list->is_pointer_in_mash_image(node)) {
                    using destroy_fn = void(__fastcall *)(info_node *, void *);
                    reinterpret_cast<destroy_fn>(get_vfunc(node->m_vtbl, 0))(node, nullptr);
                } else if (node != nullptr) {
                    using delete_fn = void(__fastcall *)(info_node *, void *, bool);
                    reinterpret_cast<delete_fn>(get_vfunc(node->m_vtbl, 0x8))(node, nullptr, true);
                }
                my_info_node_list->m_data[index] = nullptr;
            }
        }
        if (!my_info_node_list->is_pointer_in_mash_image(my_info_node_list->m_data))
            mem_dealloc(my_info_node_list->m_data, 4 * my_info_node_list->m_max_size);
        my_info_node_list->m_data = nullptr;
        my_info_node_list->m_max_size = 0;
        my_info_node_list->mContainer_base::clear();
        mem_freealign(my_info_node_list);
        my_info_node_list = nullptr;
    }
    delete field_70;
    field_70 = nullptr;
    auto *&registry = !field_6C->field_44 || (field_4C & 1) != 0 ? the_ai_core_list_high : the_ai_core_list_low;
    auto it = std::find(registry->begin(), registry->end(), this);
    if (it != registry->end()) {
        auto next = it;
        ++next;
        if (&registry == &the_ai_core_list_low && next_ai_core_list_low_iter == it._Mynode()) {
            next_ai_core_list_low_iter = next._Mynode();
            if (next == registry->end())
                next_ai_core_list_low_iter = registry->begin()._Mynode();
        }
        registry->erase(it);
    }
    if (registry->empty()) {
        delete registry;
        registry = nullptr;
    }
}

ai_core::ai_core(core_ai_resource *a2, const param_block *a3, actor *a4)
{
    if constexpr (1) {
        this->my_base_machine = nullptr;
        this->my_locomotion_machine = nullptr;
        this->my_mode = static_cast<mode_e>(0);
        this->field_30 = {};
        this->my_locomotion_mode = static_cast<mode_e>(0);
        this->field_40 = nullptr;
        this->field_44 = 1;
        this->field_48 = {};
        this->field_4C = 0;
        auto *v5 = a2;
        this->field_64 = a4;

        static constexpr int dword_937CF0 = 8;
        auto v6 = dword_937CF0;
        this->field_6C = v5;
        this->field_70 = nullptr;
        auto v7 = g_world_ptr->time_manager.field_C;

        this->field_38 = v7 + static_cast<int>(static_cast<double>(rand()) * v6 / 32768.0);
        this->field_48 = {0};
        this->field_50.copy_from_pb_override(v5->field_0);
        this->field_50.copy_from_pb_override(*a3);
        this->field_5C = true;
        auto v8 = v5->field_40;
        if (v8 != 0) {
            this->my_info_node_list = static_cast<mVector<info_node> *>(arch_memalign(16u, v8));
            assert(this->my_info_node_list != nullptr);
            std::memcpy(this->my_info_node_list, v5->field_C, v8);

            mash_info_struct v33{(uint8_t *)this->my_info_node_list, v8};
            v33.unmash_class(this->my_info_node_list, this);
            mash_info_struct::construct_class(this->my_info_node_list);
        } else {
            this->my_info_node_list = nullptr;
        }

        assert(this->my_base_machine == nullptr);

        auto v14 = this->field_48;
        auto *v28 = &this->my_base_machine;
        auto v27 = v5->sub_6B6D50();
        this->spawn_state_machine_internal(nullptr, v27, v28, v14);

        assert(this->my_base_machine != nullptr);

        this->field_48.source_hash_code = 0;
        if (v5->field_44) {
            if (the_ai_core_list_low == nullptr) {
                the_ai_core_list_low = new _std::list<ai_core *>{};
            }

            the_ai_core_list_low->push_back(this);
            if (the_ai_core_list_low->size() == 1) {
                next_ai_core_list_low_iter = the_ai_core_list_low->m_head->_Next;
            }

        } else {
            if (the_ai_core_list_high == nullptr) {
                the_ai_core_list_high = new _std::list<ai_core *>{};
            }

            the_ai_core_list_high->push_back(this);
        }

        if (!pedestrian_inode::is_a_pedestrian(this)) {
            if (this->get_info_node(traffic_inode::default_id, false) == nullptr) {
                ai::pedestrian_inode::register_non_ped(vhandle_type<actor>{this->field_64->my_handle.field_0});
            }
        }
    } else {
        THISCALL(0x006AEA90, this, a2, a3, a4);
    }
}

void sub_86AD60()
{
    CDECL_CALL(0x0086AD60);
}

template <typename T>
bool binary_search_array_deref(T *a1, T **a2, int a3, int *index)
{
    int v4 = a3;
    int v5 = 0;
    int v6 = a3;
    if (a3 <= 0) {
    LABEL_9:
        if (index) {
            if (v6 == v4 - 1 && a1->field_4.source_hash_code > a2[v6]->field_4.source_hash_code) {
                ++v6;
            }

            *index = v6;
        }

        return false;
    }

    auto v7 = a1->field_4.source_hash_code;
    int v8;
    uint32_t v9;
    while (1) {
        v8 = (v6 + v5) / 2;
        v9 = a2[v8]->field_4.source_hash_code;
        if (v7 >= v9) {
            break;
        }

        v6 = (v6 + v5) / 2;
    LABEL_7:
        if (v5 >= v6) {
            v4 = a3;
            goto LABEL_9;
        }
    }

    if (v7 > v9) {
        v5 = v8 + 1;
        goto LABEL_7;
    }

    if (index != nullptr) {
        *index = v8;
    }

    return true;
}

template <typename T>
bool binary_search_array_deref1(T *a1, T **a2, int a3, int *index)
{
    bool result = false;
    int v7 = 0;
    int v6 = a3;
    while (v7 < v6) {
        auto v5 = (v6 + v7) / 2;
        if (a1->field_4.source_hash_code < a2[v5]->field_4.source_hash_code) {
            v6 = (v6 + v7) / 2;
        } else {
            if (!a1->field_4.source_hash_code < a2[v5]->field_4.source_hash_code) {
                result = true;
                if (index != nullptr) {
                    *index = v5;
                }

                break;
            }

            v7 = v5 + 1;
        }
    }

    if (!result && index != nullptr) {
        if (v6 == a3 - 1 && a1->field_4.source_hash_code < a2[v6]->field_4.source_hash_code) {
            ++v6;
        }

        *index = v6;
    }

    return result;
}

bool ai_core::push_base_machine(resource_key a2, int)
{
    TRACE("ai::ai_core::push_base_machine");

    assert(my_base_machine != nullptr);
    assert(my_mode != AI_KILLING_MACHINES && "trying to push a new machine while a change is in progress");

    resource_key name = (this->my_mode == 1 ? this->field_30 : this->my_base_machine->get_name());

    bool result = false;
    if (this->change_base_machine(a2, 1, string_hash{0})) {
        if constexpr (0) {
            this->field_0.push_back(name);
        } else {
            auto *m_head = this->field_0.m_head;
            auto *Prev = m_head->_Prev;
            decltype(Prev)(__fastcall * sub_6B7660)(
                _std::list<resource_key> *, void *, decltype(Prev), decltype(Prev), resource_key *a2) =
                CAST(sub_6B7660, 0x006B7660);
            auto *v8 = sub_6B7660(&this->field_0, nullptr, Prev, Prev->_Next, &name);

            void(__fastcall * sub_6B76F0)(_std::list<resource_key> *, void *, uint32_t) = CAST(sub_6B76F0, 0x006B76F0);
            sub_6B76F0(&this->field_0, nullptr, 1u);

            Prev->_Next = v8;
            v8->_Next->_Prev = v8;
        }

        result = true;
    }

    if (result) {
        auto *the_actor = this->get_actor(0);
        auto id = the_actor->get_id();
        auto *v17 = id.to_string();
        auto *v7 = a2.m_hash.to_string();
        debug_print_va("\n--- successful AI machine push to %s  (ent %s)", v7, v17);
    } else {
        auto *v8 = this->get_actor(0);
        auto v10 = v8->get_id();
        auto *v17 = v10.to_string();
        auto *v11 = a2.m_hash.to_string();
        debug_print_va("\n--- failed AI machine push to %s  (ent %s)", v11, v17);
    }

    {
        int v37 = 0;
        for (auto name : this->field_0) {
            auto *v13 = name.m_hash.to_string();
            debug_print_va("    [%d] %s", v37, v13);
            ++v37;
        }
    }

    return result;
}

bool ai_core::pop_base_machine(int)
{
    TRACE("ai::ai_core::pop_base_machine");

    assert(my_mode != AI_KILLING_MACHINES && "trying to pop a machine while a change is in progress");

    resource_key a2;
    bool result = false;
    if (!this->field_0.empty()) {
        a2 = this->field_0.front();

        {
            auto *head = this->field_0.m_head;
            auto *Prev = head->_Prev;
            if (head->_Prev != head) {
                Prev->_Next->_Prev = Prev->_Prev;
                auto *v5 = Prev->_Prev;
                auto *Next = Prev->_Next;
                v5->_Next = Next;
                operator delete(Prev);
                --this->field_0.m_size;
            }
        }

        result = this->change_base_machine(a2, 2, string_hash{0});
    }

    if (result) {
        auto v21 = a2.m_hash;
        auto *v6 = this->get_actor(0);
        auto id = v6->get_id();
        auto *v18 = id.to_string();
        auto *v8 = v21.to_string();
        debug_print_va("\n--- successful AI machine pop to %s  (ent %s)", v8, v18);
    } else {
        auto v21 = a2.m_hash;
        auto *v9 = this->get_actor(0);
        auto v11 = v9->get_id();
        auto *v18 = v11.to_string();
        auto *v12 = v21.to_string();
        debug_print_va("\n--- failed AI machine pop to %s  (ent %s)", v12, v18);
    }

    {
        int a2 = 0;
        for (auto name : this->field_0) {
            auto v22 = name.m_hash;
            auto *v14 = v22.to_string();
            debug_print_va("    [%d] %s", a2, v14);
            ++a2;
        }
    }

    return result;
}

void ai_core::reset_base_machine(string_hash state)
{
    const auto request_reset = [this, state] {
        if (my_base_machine == nullptr)
            return false;
        change_base_machine(my_base_machine->get_name(), 0, state);
        return true;
    };
    bool requested = request_reset();
    frame_advance(0.0001f);
    for (int attempt = 0; !requested && attempt < 10; ++attempt) {
        requested = request_reset();
        frame_advance(0.0001f);
    }
    for (int index = 0; index < my_info_node_list->m_size; ++index)
        my_info_node_list->m_data[index]->deactivate();
    for (int index = 0; index < my_info_node_list->m_size; ++index)
        my_info_node_list->m_data[index]->activate(this);
}

bool ai_core::change_base_machine(resource_key the_state_graph, int a3, string_hash a4)
{
    TRACE("ai::ai_core::change_base_machine");

    if constexpr (1) {
        if (the_state_graph.get_type() != RESOURCE_KEY_TYPE_AI_STATE_GRAPH) {
            return false;
        }

        if (!this->field_6C->does_base_graph_exist(the_state_graph)) {
            return false;
        }

        if (this->find_state_graph(the_state_graph) == nullptr) {
            return false;
        }

        this->my_mode = static_cast<mode_e>(1);
        this->field_30 = the_state_graph;
        this->field_48 = a4;

        return true;
    } else {
        bool(__fastcall * func)(void *, void *, resource_key, int, string_hash) = CAST(func, 0x006978F0);
        return func(this, nullptr, the_state_graph, a3, a4);
    }
}

void ai_core::create_capsule_alter()
{
    if (field_70 == nullptr) {
        field_70 = new capsule_alter_sys{field_64};
        set_to_default_capsule_alter(field_70, static_cast<conglomerate *>(field_64));
    }
}

void ai_core::adjust_colgeom(bool force)
{
    const int ticks = g_world_ptr->time_manager.field_C;
    if (!force && ticks - field_38 < 8)
        return;
    field_38 = ticks;
    if (field_70 == nullptr)
        create_capsule_alter();
    if (field_70 == nullptr || (field_64->field_4 & 0x4000) == 0)
        return;
    const float distance = g_world_ptr->field_A0.field_0->field_4;
    float nearest = std::numeric_limits<float>::max();
    bool restore = (field_64->field_4 & 0x40000000) != 0 || field_64->get_occluded_last_frame();
    if (!restore) {
        const auto &position = field_64->get_abs_position();
        for (int player = 0; player < g_world_ptr->num_players; ++player) {
            const float squared =
                (g_game_ptr->get_current_view_camera(player)->get_abs_position() - position).length2();
            if (squared < nearest)
                nearest = squared;
        }
        restore = distance * distance < nearest;
    }
    if (restore) {
        if (field_70->field_4 != 0)
            field_70->restore_colgeom();
    } else {
        field_64->get_abs_po();
        field_70->adjust_colgeom(false);
        field_64->get_abs_po();
        if (field_64->has_physical_ifc())
            field_64->physical_ifc()->field_C |= 0x4000;
    }
}

void ai_core::post_entity_mash()
{
    for (auto *node : *my_info_node_list) {
        node->activate(this);
    }
    if (!field_6C->my_locomotion_graphs.empty()) {
        static const string_hash locomotion_hash{"base_locomotion_inode"};
        const auto name = field_50.does_parameter_exist(locomotion_hash) ? field_50.get_pb_hash(locomotion_hash)
                                                                         : field_6C->my_locomotion_graphs.at(0)->m_hash;
        change_locomotion_machine(name);
    }
}

void ai_core::frame_advance_all_core_ais(Float elapsed)
{
    TRACE("ai_core::frame_advance_all_core_ais");

    auto *high = the_ai_core_list_high;
    if (high != nullptr) {
        for (auto it = high->begin(); it != high->end();) {
            auto *core = *it;
            core->frame_advance(elapsed);
            if (core->field_6C != nullptr && core->field_6C->field_44 && (core->field_4C & 1) == 0) {
                it = high->erase(it);
                if (the_ai_core_list_low == nullptr) {
                    the_ai_core_list_low = new _std::list<ai_core *>{};
                }
                the_ai_core_list_low->push_back(core);
            } else {
                ++it;
            }
        }
        if (high->empty()) {
            delete high;
            the_ai_core_list_high = nullptr;
        }
    }

    auto *low = the_ai_core_list_low;
    if (low == nullptr || low->empty()) {
        return;
    }

    auto count = std::max(1, static_cast<int>(static_cast<float>(low->size()) * elapsed.value * 3.0f + 0.5f));
    auto it = low->begin();
    while (count-- > 0 && !low->empty()) {
        if (it == low->end()) {
            it = low->begin();
        }
        auto *core = *it;
        core->frame_advance(elapsed);
        if ((core->field_4C & 1) != 0) {
            it = low->erase(it);
            if (the_ai_core_list_high == nullptr) {
                the_ai_core_list_high = new _std::list<ai_core *>{};
            }
            the_ai_core_list_high->push_back(core);
        } else {
            ++it;
        }
    }
    if (low->empty()) {
        delete low;
        the_ai_core_list_low = nullptr;
    }
}

void ai_core::frame_advance(Float elapsed)
{
    TRACE("ai_core::frame_advance");
    if (field_64 == nullptr) {
        return;
    }
    if (auto *controller = field_64->get_player_controller()) {
        controller->frame_advance(elapsed);
    }
    if ((!field_64->is_in_limbo() && field_64->get_primary_region() != nullptr) || field_64->is_ext_flagged(8u)) {
        if (field_64->is_ext_flagged(0x40000000u) && elapsed > EPSILON) {
            return;
        }
    } else {
        return;
    }

    advance_info_nodes(elapsed);
    if (my_mode == static_cast<mode_e>(1)) {
        if (my_base_machine != nullptr) {
            my_base_machine->request_exit();
        }
        my_mode = AI_KILLING_MACHINES;
    }
    if (my_locomotion_mode == static_cast<mode_e>(1)) {
        if (my_locomotion_machine != nullptr) {
            my_locomotion_machine->request_exit();
        }
        my_locomotion_mode = AI_KILLING_MACHINES;
    }
    if (my_base_machine != nullptr) {
        advance_machine_recursive(my_base_machine, elapsed, false);
    }
    if (my_locomotion_machine != nullptr) {
        advance_machine_recursive(my_locomotion_machine, elapsed, false);
    }
    exit_pending_machines();
    if (my_mode == AI_KILLING_MACHINES && my_base_machine == nullptr) {
        spawn_state_machine_internal(nullptr, field_30, &my_base_machine, field_48);
        field_48 = string_hash{0};
        my_mode = static_cast<mode_e>(0);
    }
    if (my_locomotion_mode == AI_KILLING_MACHINES && my_locomotion_machine == nullptr) {
        auto *animation = static_cast<als_inode *>(get_info_node(als_inode::default_id, false));
        if (animation == nullptr || animation->is_layer_interruptable(static_cast<als::layer_types>(0))) {
            spawn_state_machine_internal(nullptr, field_40->get_graph(), &my_locomotion_machine, string_hash{0});
            if (my_locomotion_machine == nullptr)
                field_44 = 2;
            my_locomotion_mode = static_cast<mode_e>(0);
        }
    }
    if (field_64->colgeom != nullptr && field_64->colgeom->get_type() == 1) {
        adjust_colgeom(field_64->is_hero());
    }
    post_frame_advance();
}

bool ai_core::change_locomotion_machine(const string_hash &name)
{
    auto *node = get_info_node(name, true);
    if (node == field_40) {
        return true;
    }
    if (node == nullptr || !node->is_subclass_of(static_cast<mash::virtual_types_enum>(391))) {
        return false;
    }
    auto *loco = static_cast<loco_inode *>(node);
    const auto &graph = loco->get_graph();
    if (!field_6C->does_locomotion_graph_exist(graph) || find_state_graph(graph) == nullptr) {
        return false;
    }
    if (my_locomotion_machine != nullptr) {
        my_locomotion_mode = static_cast<mode_e>(1);
    }
    field_40 = loco;
    loco->initialize_loco_inode();
    return true;
}

void ai_core::set_allow_facing(bool allow)
{
    if (field_40 != nullptr && field_40->get_graph().is_set()) {
        field_40->allow_facing_change = allow;
    }
}

bool ai_core::stop_movement()
{
    if (my_locomotion_machine != nullptr) {
        const auto mode = my_locomotion_machine->my_curr_mode;
        if (mode != 3 && mode != 5 && mode != 4)
            my_locomotion_machine->request_exit();
        return true;
    }
    if (my_locomotion_mode == 2) {
        my_locomotion_mode = static_cast<mode_e>(0);
        field_44 = 2;
    }
    auto *animation = static_cast<als_inode *>(get_info_node(als_inode::default_id, false));
    if (animation != nullptr) {
        als::param_list parameters;
        parameters.add_param(als::param{0, 0.0f});
        animation->set_desired_params(parameters, static_cast<als::layer_types>(0));
        parameters.clear();
    }
    return false;
}

bool ai_core::set_facing_dir(const vector3d &direction)
{
    auto normalized = direction;
    const float length_squared = normalized.length2();
    if (length_squared > 1.0e-10f)
        normalized *= 1.0f / std::sqrt(length_squared);
    if (field_44 == 75) {
        if (!field_40->allow_facing_change)
            return false;
        field_40->set_facing_dir(normalized);
        return true;
    }
    auto *animation = static_cast<als_inode *>(get_info_node(als_inode::default_id, true));
    if (!animation->is_layer_interruptable(static_cast<als::layer_types>(0)))
        return false;
    als::param_list parameters;
    parameters.add_param(0x1B, normalized);
    animation->set_desired_params(parameters, static_cast<als::layer_types>(0));
    parameters.clear();
    return true;
}

bool ai_core::set_facing_point(const vector3d &point)
{
    return set_facing_dir(point - field_64->get_abs_position());
}

void ai_core::do_machine_exit(ai_state_machine *machine)
{
    const auto name = machine->get_name();
    for (auto &entry : my_machine_list) {
        if (entry.machine->get_name() == name) {
            entry.pending_exit = true;
            return;
        }
    }
}

void ai_core::exit_pending_machines()
{
    for (auto it = my_machine_list.begin(); it != my_machine_list.end();) {
        if (!it->pending_exit) {
            ++it;
            continue;
        }
        auto *machine = it->machine;
        const int message = machine->field_44;
        if (machine == my_base_machine)
            my_base_machine = nullptr;
        if (machine == my_locomotion_machine) {
            field_44 = message == 1 ? 1 : 2;
            my_locomotion_machine = nullptr;
        }
        field_20.push_back(completed_machine{machine->get_name(), message, 1});
        delete machine;
        it = my_machine_list.erase(it);
    }
}

void ai_core::post_frame_advance()
{
    for (auto it = field_20.begin(); it != field_20.end();) {
        if (it->frames_left != 0) {
            --it->frames_left;
            ++it;
        } else {
            it = field_20.erase(it);
        }
    }
}

void ai_core::remove_slave(vhandle_type<actor> actor_handle)
{
    auto *node = static_cast<slave_inode *>(get_info_node(slave_inode::default_id, true));
    for (int i = 0; i < node->records.m_size; ++i) {
        if (node->records_data[i].actor_handle.field_0 == actor_handle.field_0.field_0) {
            std::move(node->records_data + i + 1, node->records_data + node->records.m_size, node->records_data + i);
            --node->records.m_size;
            break;
        }
    }
    if (node->records.m_size == 0) {
        node->field_8->pop_base_machine(5);
    }
}

info_node *ai_core::get_info_node(string_hash the_info_node, bool a3)
{
    if (this->my_info_node_list != nullptr) {
        static info_node searcher{};

        searcher.field_4 = the_info_node;
        auto *v4 = this->my_info_node_list;
        auto v5 = v4->m_size;
        auto **v6 = v4->m_data;
        int index = -1;

        if (binary_search_array_deref(&searcher, v6, v5, &index)) {
            assert(index >= 0);

            assert(this->my_info_node_list->at(index)->get_name() == the_info_node);

            auto *found = this->my_info_node_list->at(static_cast<uint16_t>(index));
            return found;
        }
    }

    if (a3) {
        const char *v8 = this->field_64->field_10.to_string();
        const char *v9 = the_info_node.to_string();
        mString a1{0, "unknown ai info-node name %s, for entity %s", v9, v8};
        sp_log("%s", a1.c_str());
        assert(false && "Unknown AI info-node");
    }

    return nullptr;
}

state_graph *ai_core::find_state_graph(resource_key a2)
{
    TRACE("ai::ai_core::find_state_graph");

    auto *v2 = this->field_6C->field_3C;

    auto *v3 = state_graph_manager::find_state_graph_from_resource(a2, v2);
    return v3;
}

ai_state_machine *ai_core::find_machine(resource_key a2)
{
    TRACE("ai::ai_core::find_machine");

    if constexpr (1) {
        for (auto &entry : this->my_machine_list) {
            auto *machine = entry.machine;
            if (machine->get_name() == a2) {
                return machine;
            }
        }

        return nullptr;
    } else {
        return (ai_state_machine *)THISCALL(0x0069B8F0, this, a2);
    }
}

int ai_core::can_spawn_state_machine(resource_key a2)
{
    TRACE("ai_core::can_spawn_state_machine");

    if constexpr (1) {
        auto *v5 = this->field_6C->field_3C;
        auto v4 = a2;
        if (!state_graph_manager::can_get_graph(v4, v5)) {
            return 2;
        }

        return this->find_machine(a2) != nullptr;
    } else {
        int(__fastcall * func)(void *, void *edx, resource_key a2) = CAST(func, 0x0069E9B0);
        return func(this, nullptr, a2);
    }
}

void ai_core::spawn_state_machine_internal(ai_state_machine *a2, resource_key graph_name,
                                           ai_state_machine **base_machine_ptr, string_hash a5)
{
    TRACE("ai::ai_core::spawn_state_machine_internal");

    assert(can_spawn_state_machine(graph_name) == 0);

    auto *v6 = this->find_state_graph(graph_name);
    if (v6 != nullptr) {
        if (this->find_machine(graph_name) == nullptr) {
            auto *mem = mem_alloc(sizeof(ai_state_machine));

            auto *new_state_machine = new (mem) ai_state_machine{this, v6, a5};

            if (a2 != nullptr) {
                a2->add_as_child(new_state_machine);
            } else {
                *base_machine_ptr = new_state_machine;
            }

            my_machine_list.push_back(machine_entry{new_state_machine, false});
        }
    }
}

void ai_core::advance_info_nodes(Float elapsed)
{
    TRACE("ai::ai_core::advance_info_nodes");
    if (my_info_node_list == nullptr) {
        return;
    }
    for (uint16_t index = 0; index < my_info_node_list->m_size; ++index) {
        auto *node = my_info_node_list->at(index);
        if (node != nullptr && node->does_need_advance()) {
            node->frame_advance(elapsed);
        }
    }
}

void ai_core::advance_machine_recursive(ai_state_machine *machine, Float elapsed, bool interrupted)
{
    TRACE("ai::ai_core::advance_machine_recursive");
    if (machine == nullptr) {
        return;
    }
    for (auto *child : machine->field_1C) {
        advance_machine_recursive(child, elapsed, interrupted);
    }
    machine->process_mode(elapsed, interrupted);
}

}  // namespace ai

void ai_core_patch()
{
    {
        FUNC_ADDRESS(address, &ai::ai_core::get_info_node);
        REDIRECT(0x006A34BA, address);
    }

    {
        FUNC_ADDRESS(address, &ai::ai_core::find_machine);
        //SET_JUMP(0x0069B8F0, address);
    }

    {
        FUNC_ADDRESS(address, &ai::ai_core::push_base_machine);
        //SET_JUMP(0x0069F690, address);
    }

    {
        FUNC_ADDRESS(address, &ai::ai_core::advance_machine_recursive);
        REDIRECT(0x006AF13E, address);
        REDIRECT(0x006B4964, address);
        REDIRECT(0x006B4979, address);
    }

    {
        FUNC_ADDRESS(address, &ai::ai_core::advance_info_nodes);
        REDIRECT(0x006B4920, address);
    }

    {
        FUNC_ADDRESS(address, &ai::ai_core::frame_advance);
        REDIRECT(0x006B4B1A, address);
        REDIRECT(0x006B4C7A, address);
    }

    {
        REDIRECT(0x00558442, ai::ai_core::frame_advance_all_core_ais);
    }
}
