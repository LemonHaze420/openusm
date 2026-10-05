#include "ai_state_car.h"

#include "base_ai_core.h"
#include "common.h"
#include "func_wrapper.h"
#include "traffic.h"
#include "traffic_inode.h"
#include "conglom.h"
#include "event.h"
#include "event_manager.h"
#include "info_node_desc_list.h"
#include "info_node_descriptor.h"
#include "vtbl.h"
#include <cstdlib>
#include <array>
#include <algorithm>
#include <cfloat>

VALIDATE_SIZE(ai::drive_car_state, 0x44);

VALIDATE_SIZE(ai::ai_car_inode, 0x3C);

namespace ai {

int &ai_car_inode::cars_occupied = var<int>(0x00958068);
int &ai_car_inode::searches_in_progress = var<int>(0x00958060);
int &ai_car_inode::cars_selected = var<int>(0x00958064);

namespace {
unsigned __fastcall drive_type(drive_car_state *, void *)
{
    return 259;
}
bool __fastcall drive_subclass(drive_car_state *, void *, mash::virtual_types_enum type)
{
    return type == 330 || type == 536 || type == 535 || type == 567 || type == 573;
}
void __fastcall drive_activate(drive_car_state *self, void *, ai_state_machine *machine, const mashed_state *state,
                               const mashed_state *previous, const param_block *params,
                               base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
void __fastcall drive_deactivate(drive_car_state *self, void *, const mashed_state *next)
{
    self->deactivate(next);
}
void __fastcall drive_nodes(drive_car_state *self, void *, info_node_desc_list &nodes)
{
    self->get_info_node_list(nodes);
}
state_trans_action *__fastcall drive_check(drive_car_state *self, void *, state_trans_action *out, Float time)
{
    *out = self->_check_transition(time);
    return out;
}
state_trans_action *__fastcall drive_default(drive_car_state *self, void *, state_trans_action *out)
{
    *out = self->_get_default_return_code();
    return out;
}
int __fastcall drive_direction(drive_car_state *self, void *, traffic_path_road **roads)
{
    return self->get_next_direction(roads);
}

bool &seat_occupied(traffic_inode *node, int seat)
{
    switch (seat) {
    case 0:
        return node->field_C4;
    case 1:
        return node->field_C5;
    case 2:
        return node->field_C6;
    case 3:
        return node->field_C7;
    default:
        return node->field_C8;
    }
}

traffic_inode *car_inode(entity *car)
{
    return static_cast<traffic_inode *>(car->get_ai_core()->get_info_node(traffic_inode::default_id, true));
}

void refresh_seats(traffic_inode *node)
{
    const bool occupied = node->field_C4 || node->field_C5 || node->field_C6 || node->field_C7 || node->field_C8;
    if (node->traffic_ptr != nullptr) {
        if (occupied) {
            node->traffic_ptr->field_201 = true;
            node->traffic_ptr->set_destroyable(!node->traffic_ptr->field_201 && !node->traffic_ptr->field_203);
        } else {
            node->traffic_ptr->set_destroyable(true);
        }
    }
}

void clear_get_out(ai_car_inode *self)
{
    self->my_param_block.set_pb_int(string_hash{int(to_hash("get_out_now"))}, 0, true);
}

void __fastcall car_destruct(ai_car_inode *self, void *)
{
    self->_destruct_mashed_class();
}
void *__fastcall car_delete(ai_car_inode *self, void *, unsigned char flags)
{
    self->~ai_car_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(*self));
    return self;
}
unsigned __fastcall car_type(ai_car_inode *, void *)
{
    return 258;
}
bool __fastcall car_needs_advance(ai_car_inode *, void *)
{
    return true;
}
void __fastcall car_advance(ai_car_inode *self, void *, Float time)
{
    self->_frame_advance(time);
}
int __fastcall car_size(ai_car_inode *, void *)
{
    return sizeof(ai_car_inode);
}
}  // namespace

void *ai_car_inode::native_vtable()
{
    static const std::array<void *, 12> table = [] {
        std::array<void *, 12> result{};
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.begin());
        result[0] = reinterpret_cast<void *>(&car_destruct);
        result[2] = reinterpret_cast<void *>(&car_delete);
        result[3] = reinterpret_cast<void *>(&car_type);
        result[6] = reinterpret_cast<void *>(&car_needs_advance);
        result[7] = reinterpret_cast<void *>(&car_advance);
        result[11] = reinterpret_cast<void *>(&car_size);
        return result;
    }();
    return const_cast<void **>(table.data());
}

ai_car_inode::ai_car_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    field_20 = field_24 = field_28 = vhandle_type<entity>{};
    field_1C = false;
    field_1D = field_1E = false;
    clear_get_out(this);
    field_30 = false;
    field_1F = true;
    field_34 = nullptr;
    field_2C = 5;
    field_38 = 1.0f;
}

ai_car_inode::ai_car_inode(from_mash_in_place_constructor *constructor) : info_node(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    field_20 = field_24 = field_28 = vhandle_type<entity>{};
    field_1C = false;
    field_1D = field_1E = false;
    clear_get_out(this);
    field_30 = false;
    field_1F = true;
    field_34 = nullptr;
    field_2C = 5;
    field_38 = 1.0f;
}

void ai_car_inode::_destruct_mashed_class()
{
    cancel_search();
    set_inside_car(false);
    clear_car();
    info_node::_destruct_mashed_class();
}

void ai_car_inode::cancel_search_internal()
{
    if (auto *car = traffic::get_traffic_from_entity(field_24))
        --car->ai_potential_car_counter;
    field_20 = field_24 = vhandle_type<entity>{};
    --searches_in_progress;
}

void ai_car_inode::cancel_search()
{
    if (field_20.get_volatile_ptr() != nullptr)
        cancel_search_internal();
}

bool ai_car_inode::can_use_car(const traffic *car, int seat, int *selected_seat, bool ignore_stealable) const
{
    if (car == nullptr || (!car->field_200 && !ignore_stealable))
        return false;
    auto *vehicle = static_cast<conglomerate *>(car->field_C.field_50.get_volatile_ptr());
    auto *node = car_inode(vehicle);
    if ((vehicle->field_4 & 0x204) != 0x204)
        return false;
    const bool full = node->field_C4 && node->field_C6 &&
                      (node->animation_vehicle_type != 1 || (node->field_C7 && node->field_C8)) &&
                      (node->animation_vehicle_type != 2 || (node->field_C5 && node->field_C7 && node->field_C8));
    if (full)
        return false;
    static const std::array<string_hash, 5> door_nodes{string_hash{int(to_hash("ai_door_df_node"))},
                                                       string_hash{int(to_hash("ai_trunk_node"))},
                                                       string_hash{int(to_hash("ai_door_pf_node"))},
                                                       string_hash{int(to_hash("ai_door_pr_node"))},
                                                       string_hash{int(to_hash("ai_door_dr_node"))}};
    static const std::array<string_hash, 5> seat_nodes{string_hash{int(to_hash("driver_node"))},
                                                       string_hash{int(to_hash("trunk"))},
                                                       string_hash{int(to_hash("pass_pf_node"))},
                                                       string_hash{int(to_hash("pass_pr_node"))},
                                                       string_hash{int(to_hash("pass_dr_node"))}};
    auto available = [&](int index) {
        return !seat_occupied(node, index) && vehicle->get_member(door_nodes[index], true) != nullptr &&
               vehicle->get_member(seat_nodes[index], true) != nullptr;
    };
    if (seat >= 5) {
        for (seat = 0; seat < 5 && !available(seat); ++seat) {}
        if (seat == 5)
            return false;
    } else if (!available(seat)) {
        return false;
    }
    if (selected_seat != nullptr)
        *selected_seat = seat;
    return true;
}

void ai_car_inode::set_selected_car(entity *selected, int seat)
{
    cancel_search();
    clear_car();
    clear_get_out(this);
    if (selected == nullptr)
        return;
    auto *car = traffic::get_traffic_from_entity(vhandle_type<entity>{selected->my_handle.field_0});
    if (!can_use_car(car, 5, nullptr, true))
        return;
    auto *node = car_inode(selected);
    if (!seat_occupied(node, seat)) {
        seat_occupied(node, seat) = true;
        refresh_seats(node);
    }
    field_30 = true;
    field_28 = vhandle_type<entity>{selected->my_handle.field_0};
    field_2C = seat;
    ++car->ai_potential_car_counter;
    ++cars_selected;
}

bool ai_car_inode::start_search(bool occupied_first)
{
    if (field_20.get_volatile_ptr() != nullptr)
        return false;
    clear_car();
    field_24 = vhandle_type<entity>{};
    if (occupied_first)
        field_1F = true;
    if (field_1F && traffic::ai_occupied_cars != nullptr && !traffic::ai_occupied_cars->empty()) {
        field_20 = traffic::ai_occupied_cars->front()->field_C.field_50;
    } else if (!traffic::traffic_list.empty()) {
        field_1F = false;
        field_20 = traffic::traffic_list.front()->field_C.field_50;
    } else {
        field_20 = vhandle_type<entity>{};
    }
    if (field_20.get_volatile_ptr() != nullptr)
        ++searches_in_progress;
    return true;
}

void ai_car_inode::_frame_advance(Float)
{
    if (field_20.get_volatile_ptr() == nullptr)
        return;
    auto *cars = field_1F ? traffic::ai_occupied_cars : &traffic::traffic_list;
    if (cars == nullptr || cars->empty()) {
        if (field_1F && !traffic::traffic_list.empty()) {
            field_1F = false;
            field_20 = traffic::traffic_list.front()->field_C.field_50;
        } else {
            field_20 = vhandle_type<entity>{};
        }
        return;
    }
    auto find_cursor = [&] {
        return std::find_if(cars->begin(), cars->end(), [&](traffic *car) {
            return car->field_C.field_50.field_0 == field_20.field_0;
        });
    };
    auto cursor = find_cursor();
    if (cursor == cars->end()) {
        cancel_search();
        start_search(false);
        cursor = find_cursor();
    }
    for (int visited = 0; cursor != cars->end() && visited < 2; ++cursor, ++visited) {
        auto *car = *cursor;
        if (!can_use_car(car, 5, nullptr, false))
            continue;
        const auto position = field_C->get_abs_position();
        const float distance = (car->field_C.field_50.get_volatile_ptr()->get_abs_position() - position).length2();
        auto *previous = traffic::get_traffic_from_entity(field_24);
        const float previous_distance =
            can_use_car(previous, 5, nullptr, false)
                ? static_cast<float>(
                      (previous->field_C.field_50.get_volatile_ptr()->get_abs_position() - position).length2())
                : FLT_MAX;
        if (distance < previous_distance) {
            if (previous != nullptr)
                --previous->ai_potential_car_counter;
            field_24 = car->field_C.field_50;
            ++car->ai_potential_car_counter;
        }
    }
    field_20 = cursor != cars->end() ? (*cursor)->field_C.field_50 : vhandle_type<entity>{};
    if (field_20.get_volatile_ptr() != nullptr)
        return;
    auto *best = traffic::get_traffic_from_entity(field_24);
    if (!field_1F)
        cancel_search_internal();
    int seat = 0;
    while (seat < 5 && !can_use_car(best, seat, nullptr, false))
        ++seat;
    if (seat == 5) {
        if (!field_1F || traffic::traffic_list.empty()) {
            field_20 = vhandle_type<entity>{};
            field_1F = false;
            clear_car();
        } else {
            field_1F = false;
            field_20 = traffic::traffic_list.front()->field_C.field_50;
        }
        return;
    }
    set_selected_car(best->field_C.field_50.get_volatile_ptr(), seat);
    if (field_1F)
        cancel_search_internal();
}

void *drive_car_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 19> result;
        std::copy_n(static_cast<void **>(launch_layer_state::native_vtable()), 18, result.data());
        result[3] = bit_cast<void *>(&drive_type);
        result[4] = bit_cast<void *>(&drive_subclass);
        result[6] = bit_cast<void *>(&drive_activate);
        result[7] = bit_cast<void *>(&drive_deactivate);
        result[9] = bit_cast<void *>(&drive_nodes);
        result[11] = bit_cast<void *>(&drive_check);
        result[14] = bit_cast<void *>(&drive_default);
        result[18] = bit_cast<void *>(&drive_direction);
        return result;
    }();
    return table.data();
}

drive_car_state::drive_car_state()
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00879DD8;
}

drive_car_state::drive_car_state(from_mash_in_place_constructor *constructor) : launch_layer_state(constructor)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00879DD8;
}

void drive_car_state::activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                               const param_block *params, activate_flag_e flags)
{
    launch_layer_state::activate(machine, state, previous, params, flags);
    field_40 = 0;
    event_manager::raise_event(event::AI_GET_IN_CAR_FINISHED, get_core()->get_actor(0)->my_handle);
    auto *node = static_cast<ai_car_inode *>(get_core()->get_info_node(ai_car_inode::default_id, true));


    const string_hash assigned_car{int(to_hash("assigned_car"))};
    auto *assignment = node->my_param_block.param_array != nullptr
                           ? node->my_param_block.param_array->common_find_data(assigned_car)
                           : nullptr;
    const vhandle_type<entity> assigned{assignment != nullptr ? assignment->m_union.i : 0};
    if (auto *car = assigned.get_volatile_ptr()) {
        if (!node->field_30 || (node->field_28.get_volatile_ptr() != nullptr &&
                                node->field_28.get_volatile_ptr()->my_handle.field_0 != assigned.field_0.field_0)) {
            const int seat =
                node->my_param_block.get_optional_pb_int(string_hash{int(to_hash("assigned_seat"))}, 5, nullptr);
            if (seat == 5) {
                auto *traffic_car = traffic::get_traffic_from_entity(assigned);
                for (int candidate = 0; candidate < 5; ++candidate) {
                    if (node->can_use_car(traffic_car, candidate, nullptr, true)) {
                        node->set_selected_car(car, candidate);
                        break;
                    }
                }
            } else {
                node->set_selected_car(car, seat);
            }
            node->field_1E = true;
            node->my_param_block.param_array->common_find_data(assigned_car)->m_union.i = 0;
        }
    }

    if (node->field_28.get_volatile_ptr() != nullptr && node->inside_car()) {
        node->field_34 = nullptr;
        if (auto *car = traffic::get_traffic_from_entity(node->field_28)) {
            node->field_34 = this;
            car->set_ai_controller(node);
            if (!node->field_1E && car->get_damage_done() == 0)
                car->set_hit_points(200);
        }

        if (node->field_1D) {
            node->field_1D = false;
            if (auto *car = traffic::get_traffic_from_entity(node->field_28)) {
                if (car->field_218-- == 1 && (car->field_15C == 12 || car->field_15C == 13))
                    car->field_15C = car->field_160;
            }
        }
        if (node->field_2C == 1)
            field_40 = 1;
    }
}

void drive_car_state::deactivate(const mashed_state *next)
{
    auto *node = static_cast<ai_car_inode *>(get_core()->get_info_node(ai_car_inode::default_id, true));
    if (node->field_28.get_volatile_ptr() != nullptr && node->inside_car()) {
        node->field_34 = nullptr;
        if (auto *car = traffic::get_traffic_from_entity(node->field_28))
            car->set_ai_controller(nullptr);
    }
    launch_layer_state::deactivate(next);
}

void drive_car_state::get_info_node_list(info_node_desc_list &nodes)
{
    nodes.add_entry(info_node_descriptor{ai_car_inode::default_id, 258});
}

int drive_car_state::get_next_direction(traffic_path_road **roads) const
{
    float weights[3]{roads[0] ? 1.0f : 0.0f, roads[1] ? 1.0f : 0.0f, roads[2] ? 1.0f : 0.0f};
    const float total = weights[0] + weights[1] + weights[2];
    if (total <= 0.0f)
        return 0;
    const float choice = static_cast<double>(std::rand()) * (1.0f / RAND_MAX) * total;
    double cumulative = 0.0;
    for (int index = 0; index < 3; ++index) {
        if (weights[index] > 0.0f) {
            cumulative += weights[index];
            if (choice <= cumulative)
                return index == 0 ? -1 : index;
        }
    }
    return 0;
}

state_trans_action drive_car_state::_check_transition(Float a3)
{
    auto result = this->_get_default_return_code();
    auto *ai_core = this->get_core();
    auto *info_node = (ai_car_inode *)ai_core->get_info_node(ai_car_inode::default_id, true);
    auto *selected_car = info_node->get_selected_car();
    if (selected_car != nullptr && info_node->inside_car() && selected_car->is_visible()) {
        if (info_node->car_is_dead()) {
            result = this->process_message(a3, static_cast<state_trans_messages>(1));
        }
    } else {
        info_node->clear_car();
        result = this->process_message(a3, static_cast<state_trans_messages>(2));
    }

    return result;
}

state_trans_action drive_car_state::_get_default_return_code() const
{
    auto result = state_trans_action{
        static_cast<state_trans_actions>(4), string_hash{0}, static_cast<state_trans_messages>(75), nullptr};

    return result;
}

entity *ai_car_inode::get_selected_car()
{
    assert(this->search_finished());

    if (this->field_28.get_volatile_ptr() == nullptr) {
        return nullptr;
    } else {
        return this->field_28.get_volatile_ptr();
    }
}

bool ai_car_inode::search_finished() const
{
    return (this->field_20.get_volatile_ptr() == nullptr);
}

int ai_car_inode::get_next_direction(traffic_path_road **roads)
{
    if (field_34 == nullptr)
        return 0;
    using direction_fn = int(__fastcall *)(drive_car_state *, void *, traffic_path_road **);
    return reinterpret_cast<direction_fn>(get_vfunc(field_34->m_vtbl, 0x48))(field_34, nullptr, roads);
}

bool ai_car_inode::car_is_dead() const
{
    if (my_param_block.get_optional_pb_int(string_hash{int(to_hash("get_out_now"))}, 0, nullptr) != 0)
        return true;
    if (field_1E)
        return false;
    auto *car = traffic::get_traffic_from_entity(field_28);
    return car == nullptr || ((car->field_15C == 12 || car->field_15C == 13) && car->field_C.field_C8 < 0.0001f &&
                              car->get_damage_done() - car->get_hit_points() <= 0);
}

void ai_car_inode::clear_car()
{
    set_inside_car(false);
    if (auto *selected = field_28.get_volatile_ptr()) {
        if (auto *car = traffic::get_traffic_from_entity(field_28)) {
            auto *node = car_inode(selected);
            seat_occupied(node, field_2C) = false;
            refresh_seats(node);
            --car->ai_potential_car_counter;
        }
        --cars_selected;
    }
    field_30 = false;
    clear_get_out(this);
    field_28 = vhandle_type<entity>{};
    field_2C = 5;
    field_1E = false;
}

void ai_car_inode::set_inside_car(bool inside)
{
    assert(!inside || this->get_selected_car() != nullptr);

    bool v9 = this->inside_car();
    auto *selected_car = field_28.get_volatile_ptr();
    if (selected_car != nullptr) {
        this->field_1C = inside;
        auto *the_traffic = traffic::get_traffic_from_entity((vhandle_type<entity>)selected_car->my_handle.field_0);
        if (the_traffic != nullptr) {
            auto v6 = this->field_1C;
            the_traffic->set_ai_car_occupied(v6);
            if (!this->field_1E) {
                if (inside) {
                    the_traffic->set_driver_type(2);
                } else {
                    the_traffic->set_driver_type(0);
                }
            }

            if (the_traffic->is_ai_car_occupied()) {
                if (!v9) {
                    this->field_38 = the_traffic->field_1B8;
                }
            } else if (v9) {
                the_traffic->field_1B8 = this->field_38;
            }
        }
    } else {
        this->field_1C = false;
    }

    bool v8 = this->field_1C;
    if (!v8) {
        this->field_34 = nullptr;
    }

    if (v9) {
        if (!v8) {
            --ai_car_inode::cars_occupied;
        }
    } else if (v8) {
        ++ai_car_inode::cars_occupied;
    }
}

}  // namespace ai
