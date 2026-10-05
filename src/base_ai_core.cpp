#include <algorithm>
#include <cmath>

#include "base_ai_core.h"
#include "core_ai_resource.h"

#include "actor.h"
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
#include "memory.h"
#include "mstring.h"
#include "resource_manager.h"
#include "trace.h"
#include "traffic_inode.h"
#include "utility.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(ai_core, 0x74u);

_std::list<ai_core *> *&ai_core::the_ai_core_list_high = var<_std::list<ai_core *> *>(0x0096BE24);

_std::list<ai_core *> *&ai_core::the_ai_core_list_low = var<_std::list<ai_core *> *>(0x0096BE28);

void *&ai_core::next_ai_core_list_low_iter = var<void *>(0x0096C110);

ai_core::ai_core(core_ai_resource *a2, const param_block *a3, actor *a4)
{
    if constexpr (1) {
        this->field_0 = {};
        this->my_base_machine = nullptr;
        this->my_locomotion_machine = nullptr;
        this->my_machine_list = {};
        this->field_20 = {};
        this->my_mode = static_cast<mode_e>(0);
        this->field_30 = {};
        this->my_locomotion_mode = static_cast<mode_e>(0);
        this->field_40 = nullptr;
        this->field_44 = 1;
        this->field_48 = {};
        this->field_4C = 0;
        this->field_50 = {};
        auto *v5 = a2;
        this->field_64 = a4;

        static constexpr int dword_937CF0 = 8;
        auto v6 = dword_937CF0;
        this->field_6C = v5;
        this->field_70 = nullptr;
        auto v7 = g_world_ptr->time_manager.field_C;

        auto func = [](int begin, int end) -> int {
            assert(begin < end);

            return begin + ((end * rand()) / 32768.0f);
        };

        this->field_38 = func(0, v6) + v7;
        this->field_48 = {0};
        this->field_50.copy_from_pb_override(v5->field_0);
        this->field_50.copy_from_pb_override(*a3);
        this->field_5C = true;
        auto v8 = v5->field_40;
        if (v8 != 0) {
            this->my_info_node_list =
                static_cast<mVector<info_node> *>(arch_memalign(16u, v8));
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
                next_ai_core_list_low_iter = *the_ai_core_list_low->begin();
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
    if constexpr (1) {
        if (this->field_70 == nullptr) {
            this->field_70 = new capsule_alter_sys{this->field_64};

            auto *my_actor = this->field_64;
            assert(my_actor->is_a_conglomerate());

            set_to_default_capsule_alter(this->field_70, bit_cast<conglomerate *>(my_actor));
        }
    } else {
        THISCALL(0x00687C50, this);
    }
}

void ai_core::post_entity_mash()
{
    THISCALL(0x006A36E0, this);
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

    auto count = std::max(1, static_cast<int>(
        static_cast<float>(low->size()) * elapsed.value * 3.0f + 0.5f));
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
    if (field_64 == nullptr ||
        (field_64->is_in_limbo() && !field_64->is_flagged(8u))) {
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
}

bool ai_core::change_locomotion_machine(const string_hash &a2)
{
    return (bool)THISCALL(0x006A34A0, this, &a2);
}

void ai_core::set_allow_facing(bool a2)
{
    THISCALL(0x0068FC00, this, a2);
}

bool ai_core::stop_movement()
{
    return (bool)THISCALL(0x006A3590, this);
}

void ai_core::do_machine_exit(ai_state_machine *a2)
{
    THISCALL(0x0069B940, this, a2);
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
        for (auto &v3 : this->my_machine_list) {
            auto name = v3->get_name();
            if (name == a2) {
                return v3;
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

            auto *v9 = this->my_machine_list.m_head;
            auto *v10 = v9->_Prev;
            auto *v11 = &this->my_machine_list;

            decltype(v9)(__fastcall * sub_5E3BE0)(void *, void *, void *a1, void *a2, ai_state_machine **a3) =
                CAST(sub_5E3BE0, 0x005E3BE0);

            auto *v12 = sub_5E3BE0(v11, nullptr, v9, v10, &new_state_machine);

            void(__fastcall * sub_6B76F0)(void *, void *, unsigned int a2) = CAST(sub_6B76F0, 0x006B76F0);

            sub_6B76F0(v11, nullptr, 1u);
            v9->_Prev = v12;
            v12->_Prev->_Next = v12;
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
