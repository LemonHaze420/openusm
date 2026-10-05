#include "traffic.h"

#include "camera.h"
#include "base_ai_core.h"
#include "traffic_inode.h"
#include "common.h"
#include "event.h"
#include "event_manager.h"
#include "func_wrapper.h"
#include "game.h"
#include "parking_marker.h"
#include "poi.h"
#include "region.h"
#include "terrain.h"
#include "trace.h"
#include "traffic_path_graph.h"
#include "traffic_path_lane.h"
#include "traffic_signal_mgr.h"
#include "variables.h"
#include "vehicle.h"
#include "wds.h"
#include "ai_player_controller.h"
#include "ai_voice_box_inode.h"
#include "damage_interface.h"
#include "physical_interface.h"
#include "slab_allocator.h"
#include "vtbl.h"
#include "hierarchical_entity_proximity_map.h"
#include "subdivision_visitor.h"

#include <algorithm>
#include <cstdlib>
#include <new>
#include <cmath>

VALIDATE_OFFSET(traffic, field_1C4, 0x1C4);
VALIDATE_SIZE(traffic, 0x22Cu);

static bool &g_traffic_paused = var<bool>(0x0096CA0C);

static int &g_traffic_testscript = var<int>(0x0096CA14);

static int &g_traffic_single_step = var<int>(0x0096CA10);

static const vector3d HELLFORCARS{0.0, -99.0, 0.0};

bool &traffic::traffic_enabled = var<bool>(0x00937FF4);

float &traffic::traffic_density = var<float>(0x00937FF8);

float &traffic::parking_density = var<float>(0x00937FFC);

bool &traffic::traffic_initialized = var<bool>(0x0096C9E8);

int &traffic::lane_changes_this_frame = var<int>(0x0096CA18);
int &traffic::spawned_this_frame = var<int>(0x0096C9F4);
int &traffic::unspawned_this_frame = var<int>(0x0096C9F8);

_std::vector<traffic *> &traffic::traffic_list = var<_std::vector<traffic *>>(0x0096D2A8);

int &traffic::last_traffic_id = var<int>(0x0096CA08);

float (&stru_937FAC)[8] = var<float[8]>(0x00937FAC);

std::array<int, 5> &traffic::old_drivers = var<std::array<int, 5>>(0x0096CA40);

std::array<int, 5> &traffic::new_drivers = var<std::array<int, 5>>(0x0096D288);

int &traffic::living_cars = var<int>(0x0096C9EC);
int &traffic::parked_cars = var<int>(0x0096C9F0);
int &traffic::visible_cars = var<int>(0x0096C9FC);

traffic *&traffic::manual_car = var<traffic *>(0x0096C9D8);

traffic *&traffic::getaway_car = var<traffic *>(0x0096C9E0);

traffic *&traffic::emergency_car = var<traffic *>(0x0096C9E4);

traffic *&traffic::field_96C9DC = var<traffic *>(0x0096C9DC);

_std::vector<traffic *> *&traffic::ai_occupied_cars = var<_std::vector<traffic *> *>(0x0096CA00);

static float &stru_937FA4 = var<float>(0x00937FA4);

#if STANDALONE_SYSTEM

static const bool native_traffic_defaults = [] {
    traffic::traffic_enabled = true;
    traffic::traffic_density = 1.0f;
    traffic::parking_density = 0.5f;
    stru_937FA4 = 40.0f;
    stru_937FAC[0] = 1.0f;
    stru_937FAC[1] = 3.0f;
    var<int>(0x00938000) = 50;
    return true;
}();
#endif

namespace {
struct parking_marker_visitor : subdivision_visitor {
    const vector3d &point;
    float closest_squared;
    parking_marker *closest = nullptr;

    parking_marker_visitor(const vector3d &position, float radius) : point(position), closest_squared(radius * radius)
    {
        static const native_vtable table{visit_marker, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&table);
    }

    static int visit_marker(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &self = static_cast<parking_marker_visitor &>(base);
        auto *marker = reinterpret_cast<parking_marker *>(const_cast<subdivision_node *>(&node));
        if (marker->field_5C == entity::visit_key || (marker->field_4 & 0x2000))
            return 0;
        const float distance = (marker->get_abs_position() - self.point).xz_length2();
        if (distance < self.closest_squared) {
            self.closest = marker;
            self.closest_squared = distance;
        }
        return 0;
    }
};

void __fastcall native_traffic_spawn(traffic *self, void *, vector3d position, vector3d facing, traffic_path_lane *lane,
                                     int node, bool first, bool moving)
{
    self->_do_spawn(position, facing, lane, node, first, moving);
}
void __fastcall native_traffic_unspawn(traffic *self, void *)
{
    self->_un_spawn();
}
void __fastcall native_traffic_critical(traffic *self, void *, Float time)
{
    self->_critical_processing(time);
}
actor *__fastcall native_traffic_actor(traffic *self, void *)
{
    return self->field_C.field_54;
}
void __fastcall native_traffic_set_actor(traffic *self, void *, vhandle_type<entity> handle)
{
    self->set_actor(handle);
}
bool __fastcall native_traffic_lane(traffic *self, void *, traffic_path_lane *lane)
{
    return self->_is_viable_lane(lane);
}
bool __fastcall native_traffic_position(traffic *self, void *, const vector3d &position)
{
    return self->_is_viable_pos(position);
}
traffic *vehicle_owner(vehicle *self)
{
    return reinterpret_cast<traffic *>(reinterpret_cast<char *>(self) - offsetof(traffic, field_C));
}
actor *__fastcall native_traffic_vehicle_actor(vehicle *self, void *)
{
    return vehicle_owner(self)->field_C.field_54;
}
void __fastcall native_traffic_vehicle_reset(vehicle *self, void *)
{
    vehicle_owner(self)->reset();
}
void __fastcall native_traffic_vehicle_set_actor(vehicle *self, void *, vhandle_type<entity> handle)
{
    vehicle_owner(self)->set_actor(handle);
}
void __fastcall native_traffic_vehicle_out_of_world(vehicle *self, void *)
{
    auto *owner = vehicle_owner(self);
    if (owner->field_4)
        owner->_un_spawn();
}
void __cdecl traffic_damaged(event *, entity_base_vhandle handle, void *)
{
    if (auto *car = traffic::get_traffic_from_entity(vhandle_type<entity>{handle}))
        car->damage_callback(0, car->get_my_actor());
}
void __cdecl traffic_destroyed(event *, entity_base_vhandle handle, void *)
{
    auto *core = handle.get_volatile_ptr()->get_ai_core();
    if (auto *car = traffic::get_traffic_from_entity(vhandle_type<entity>{handle})) {
        car->field_C.stop_engine_sounds();
        car->field_C.stop_horn();
        car->field_C.field_4 = false;
        if (car->field_158 && core) {
            auto *node = static_cast<ai::traffic_inode *>(core->get_info_node(ai::traffic_inode::default_id, false));
            if (node)
                node->remove_from_traffic_system(false);
        }
    }
}
}  // namespace

void *traffic::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_traffic_spawn),
        reinterpret_cast<void *>(&native_traffic_unspawn),
        reinterpret_cast<void *>(&native_traffic_critical),
        reinterpret_cast<void *>(&native_traffic_actor),
        reinterpret_cast<void *>(&native_traffic_set_actor),
        reinterpret_cast<void *>(&native_traffic_lane),
        reinterpret_cast<void *>(&native_traffic_position),
    };
    return table;
}

void *traffic::native_vehicle_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_traffic_vehicle_actor),
        reinterpret_cast<void *>(&native_traffic_vehicle_reset),
        reinterpret_cast<void *>(&native_traffic_vehicle_set_actor),
        reinterpret_cast<void *>(&native_traffic_vehicle_out_of_world),
    };
    return table;
}

traffic::traffic(vhandle_type<entity> handle)
    : spawnable(handle), field_C(handle), field_140(nullptr), field_158(false), field_170(0), field_174(0),
      field_180(false), field_1BC(false), field_1BD(false), field_1BE(false), field_1BF(false), field_1C0(0),
      field_1C9(false), field_1E4(0), field_200(false), field_201(false), field_202(true), field_203(false),
      field_21C(nullptr), field_228(0)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    field_C.m_vtbl = reinterpret_cast<std::intptr_t>(native_vehicle_vtable());
    field_C.field_54->field_4 |= 0x800;
    reset();
    traffic_list.push_back(this);
    field_1C4 = 3;
    if (traffic_initialized || !traffic_enabled) {
        field_C.set_collidable(true);
        field_C.set_visible(true);
        set_standing(false);
    }
    field_210 = event_manager::add_callback(event::DAMAGED, field_C.field_50.field_0, traffic_damaged, this, false);
    field_214 = event_manager::add_callback(event::DESTROYED, field_C.field_50.field_0, traffic_destroyed, this, false);
}

traffic *traffic::create_traffic_from_entity(vhandle_type<entity> handle)
{
    ++var<int>(0x0096CA04);
    void *storage = sizeof(traffic) <= slab_allocator::get_max_object_size()
                        ? slab_allocator::allocate(sizeof(traffic), nullptr)
                        : ::operator new(sizeof(traffic));
    return storage ? new (storage) traffic(handle) : nullptr;
}

void traffic::set_actor(vhandle_type<entity> handle)
{
    field_C.set_actor(handle);
}

void traffic::detach_current_lane()
{
    if (field_140) {
        auto *owner = field_C.get_my_actor();
        const vhandle_type<actor> handle{owner ? owner->my_handle : entity_base_vhandle{0}};
        if (field_140->is_valid(nullptr) && handle.get_volatile_ptr()) {
            field_140->remove_ai_from_lane(handle);
            field_140->update_lane_indexes();
        }
        field_140 = nullptr;
    }
}

void traffic::reset()
{
    set_ai_car_occupied(false);
    ai_potential_car_counter = 0;
    field_C.reset();
    field_1C4 = 0;
    field_228 = 0;
    field_18C = 0;
    field_190 = nullptr;
    previous_lane = nullptr;
    detach_current_lane();
    field_144 = field_148 = nullptr;
    field_15C = 1;
    field_4 = field_5 = true;
    field_164 = std::rand() * (1.0f / RAND_MAX) + 1.0f;
    field_168 = field_16C = 0;
    field_170.field_0 = 0;
    field_174 = 0;
    field_1C9 = false;
    field_17C = static_cast<traffic_path_intersection::eDirection>(0);
    field_1BE = true;
    field_1E8 = nullptr;
    field_194 = 1.0f;
    field_198 = field_19C = 0.0f;
    field_1A0 = 0.0f;
    field_1A4 = 1.0f;
    field_1A8 = 0.0f;
    field_1AC = field_1B0 = 0.0f;
    field_1B4 = 1.0f;
    field_178 = 0.0f;
    field_1BC = field_1BD = false;
    field_1B8 = std::rand() * (1.0f / RAND_MAX) + 0.5f;
    field_1BF = false;
    field_180 = false;
    field_184 = 0;
    field_188 = nullptr;
    field_1CC = 0;
    field_1D0 = 1.0f;
    field_1D4 = FARAWAY;
    field_1E0 = 0.0f;
    field_1E4.field_0 = 0;
    field_1FC = 0.2f;
    field_200 = field_201 = true;
    field_202 = field_203 = false;
    set_hit_points(var<int>(0x00938000));
    set_damage_done(0);
    field_218 = 0;
    field_1C8 = true;
    if (auto *owner = field_C.get_my_actor()) {
        owner->field_8 &= ~0xFu;
        owner->on_fade_distance_changed_internal(0);
    }
    field_20C = 0;
}

void traffic::set_standing(bool enabled)
{
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_physical_ifc()) {
        owner->physical_ifc()->set_allow_manage_standing(enabled);
        owner->physical_ifc()->enable(enabled);
    }
}

void traffic::set_hit_points(int hit_points)
{
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_damage_ifc()) {
        auto &health = owner->damage_ifc()->field_1FC.field_0;
        health[2] = static_cast<float>(hit_points);
        if (health[2] < health[1])
            std::swap(health[1], health[2]);
        health[0] = std::max(health[1], std::min(static_cast<float>(hit_points), health[2]));
    } else {
        field_204 = hit_points;
    }
}

void traffic::set_damage_done(int amount)
{
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_damage_ifc()) {
        auto &health = owner->damage_ifc()->field_1FC.field_0;
        health[0] = std::max(health[1], std::min(health[2] - amount, health[2]));
    } else {
        field_208 = amount;
    }
}

int traffic::get_hit_points()
{
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_damage_ifc()) {
        const auto &health = owner->damage_ifc()->field_1FC.field_0;
        return static_cast<int>(health[2] - health[0]);
    }
    return field_208;
}

int traffic::get_damage_done()
{
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_damage_ifc())
        return static_cast<int>(owner->damage_ifc()->field_1FC.field_0[2]);
    return field_204;
}

void traffic::screeching_halt()
{
    if (field_15C == 12)
        return;
    if (field_15C != 13)
        field_160 = field_15C;
    field_15C = 12;
    field_C.field_F4 = 1000.0f;
    field_1D0 = static_cast<double>(std::rand()) * (1.0f / RAND_MAX) * 2.0 - 1.0;
}

void traffic::distract(float duration)
{
    if (!field_1CC && field_15C == 3) {
        field_1CC = static_cast<int>(duration * 8.0f);
        field_1D0 = -field_1D0;
    }
}

void traffic::damage_callback(int, entity *)
{
    if (get_damage_done() - get_hit_points() <= 0)
        return;
    if (get_damage_done() - get_hit_points() > 0) {
        if (!field_1CC && field_15C == 3)
            distract(1.0f);
    } else {
        set_damage_done(get_damage_done());
        screeching_halt();
        event_manager::raise_event(event::TRAFFIC_CAR_KILLED, field_C.get_my_actor()->my_handle);
        if (auto *hero = g_world_ptr->get_hero_ptr(0))
            event_manager::raise_event(event::TRAFFIC_CAR_KILLED, hero->my_handle);
        field_200 = false;
        if (field_96C9DC == this) {
            field_203 = false;
            field_96C9DC = nullptr;
        }
    }
    field_C.set_damage_level(static_cast<int>(static_cast<float>(get_hit_points()) / (get_damage_done() + 1) * 6.0), 0);
    field_C.set_damage_level(static_cast<int>(static_cast<float>(get_hit_points()) / (get_damage_done() + 1) * 6.0), 1);
}

void traffic::play_car_toss_voice()
{
    if (vhandle_type<entity>{field_228}.get_volatile_ptr() || field_C.bodytype > 1)
        return;
    auto *voice = static_cast<ai::voice_box_inode *>(
        field_C.get_my_actor()->get_ai_core()->get_info_node(ai::voice_box_inode::default_id, false));
    if (voice && voice->can_gab()) {
        voice->say_gab(string_hash("cartoss"), false, 0, nullptr);
        field_C.field_28 = 10.0f;
        field_C.field_30 = false;
    }
}

bool traffic::set_destroyable(bool enabled)
{
    if (field_201 == enabled)
        return field_201;
    auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0));
    const int hero_type = hero ? hero->m_player_controller->m_hero_type : 0;
    auto *owner = field_C.get_my_actor();
    if (owner && owner->has_damage_ifc() && hero_type != 2) {
        auto *damage = owner->damage_ifc();
        if (enabled) {
            damage->field_1F8 &= ~0x40000;
            damage->field_1FC.sub_48BFB0(1.0f);
        } else {
            damage->field_1F8 |= 0x40000;
        }
    }
    field_201 = enabled;
    return field_201;
}

void traffic::set_destroyed_elsewhere(traffic *car)
{
    if (car->field_158 && car->field_C.bodytype < VEHICLE_MODEL_MAX) {
        if (auto *model = vehicle::models()[car->field_C.bodytype])
            --model->refcount;
    }
    car->field_158 = false;
}

void traffic::destroy()
{
    if (field_1C9) {
        if (auto *car = get_traffic_from_entity_slow(field_C.field_50)) {
            car->field_1C9 = false;
            car->set_driver_type(3);
        }
        field_1C9 = false;
    }
    if (field_96C9DC == this) {
        field_203 = false;
        field_96C9DC = nullptr;
    }
    field_C.stop_horn();
    clear_previous_lane();
    release_turn();
    delete field_1E8;
    field_1E8 = nullptr;
    detach_current_lane();
}

void traffic::destroy_traffic(traffic *car)
{
    if (!car)
        return;
    if (field_96C9DC == car) {
        car->field_203 = false;
        field_96C9DC = nullptr;
    }
    car->set_driver_type(3);
    car->field_4 = true;
    set_destroyed_elsewhere(car);
    car->destroy();
    car->field_158 = false;
    car->set_actor(vhandle_type<entity>{0});
    car->_un_spawn();
    car->~traffic();
    if (sizeof(traffic) <= slab_allocator::get_max_object_size())
        slab_allocator::deallocate(car, nullptr);
    else
        ::operator delete(car);
}

void traffic::unspawn_parked()
{
    vhandle_type<entity> marker{field_228};
    if (auto *owner = marker.get_volatile_ptr()) {
        owner->set_active(false);
        field_228 = 0;
    }
    field_C.set_collidable(false);
    if (auto *owner = field_C.get_my_actor())
        owner->set_visible(false, false);
    detach_current_lane();
    reset();
    if (auto *owner = field_C.get_my_actor())
        owner->remove_from_regions();
    field_5 = true;
}

void traffic::_un_spawn()
{
    field_C.set_collidable(false);
    if (vhandle_type<entity>{field_228}.get_volatile_ptr()) {
        unspawn_parked();
        return;
    }
    if (field_96C9DC == this) {
        field_203 = false;
        field_96C9DC = nullptr;
    }
    field_C.stop_horn();
    field_C.set_collidable(false);
    clear_previous_lane();
    release_turn();
    delete field_1E8;
    field_1E8 = nullptr;
    if (auto *owner = field_C.get_my_actor())
        owner->set_visible(false, false);
    detach_current_lane();
    reset();
    if (auto *owner = field_C.get_my_actor()) {
        owner->remove_from_regions();
        field_C.stop_engine_sounds();
        field_C.stop_horn();
        field_C.field_4 = true;
    }
    field_5 = true;
}

void traffic::sub_6DA3B0(Float a2, Float a3, Float a4)
{
    auto &v4 = this->field_14C;
    v4[0] = a2;
    v4[1] = a3;
    v4[2] = a4;
}

char sub_6BA070()
{
    for (auto &model : vehicle::models())
        model = nullptr;
    const bool car = vehicle::add_model(1, mString{"vehicles\\Entities\\vcl_modcar"}, stru_937FAC[1]);
    const bool taxi = vehicle::add_model(0, mString{"vehicles\\Entities\\vcl_checkercab"}, stru_937FAC[0]);
    return car | taxi;
}

traffic::~traffic()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    field_C.m_vtbl = reinterpret_cast<std::intptr_t>(native_vehicle_vtable());
    event_manager::remove_callback(field_210, event::DAMAGED, field_C.field_50.field_0);
    event_manager::remove_callback(field_214, event::DESTROYED, field_C.field_50.field_0);
    field_4 = true;
    set_driver_type(0);
    _un_spawn();
    sub_6CD2D0(this);
}

void traffic::initialize_traffic()
{
    if (traffic_initialized) {
        return;
    }

    stru_937FA4 = 10.0f;
    flt_937FA8 = 90.0f;
    if (spawnable::spawnable_lanes == nullptr) {
        spawnable::spawnable_lanes = new _std::vector<traffic_path_graph::laneInfoStruct>{};
        spawnable::spawnable_lanes->reserve(30u);
    }
    if (sub_6BA070()) {
        for (int index = 29; index >= 0; --index)
            create_new_traffic(index);
    }

    traffic_initialized = true;
    spawnable::last_camera_po.set_po(ZVEC, YVEC, vector3d{-1234.0f, -1234.0f, -1234.0f});
}

void traffic::sub_6CD2D0(traffic *entry)
{
    bool found = false;
    for (unsigned index = 0; index < traffic_list.size(); ++index) {
        if (traffic_list[index] == entry)
            found = true;
        if (found && index + 1 < traffic_list.size())
            traffic_list[index] = traffic_list[index + 1];
    }
    if (found && !traffic_list.empty())
        traffic_list.pop_back();
}

bool sub_6DA670(traffic_path_lane *a1, traffic_path_lane *a2)
{
    if (a1 == a2) {
        return true;
    }

    if (a1 != nullptr) {
        if (a2 != nullptr) {
            if (a1->is_valid(nullptr) && a2->is_valid(nullptr) && a1->my_road != a2->my_road) {
                auto *next_intersection = a1->get_next_intersection(1);
                auto *v4 = a2->sub_5C8460();
                auto *v5 = a1->sub_5C8460();
                auto *v6 = a2->get_next_intersection(1);
                if (next_intersection != nullptr) {
                    if (v4 != nullptr && v5 != nullptr && v6 != nullptr && v4 == next_intersection && v6 != v5)
                        return 1;
                }
            }
        }
    }

    return false;
}

void traffic::sub_6DACB0(entity_base_vhandle a2)
{
    if (a2 == this->get_my_actor()->get_my_vhandle()) {
        this->field_170.field_0 = 0;
    } else {
        this->field_170.field_0 = a2.field_0;
    }
}

traffic *traffic::car_behind()
{
    vhandle_type<entity> v5{};
    int v3;
    auto *v2 = this->field_140;
    if (v2 != nullptr && (v3 = this->field_16C, v3 < v2->get_num_ais() - 1) &&
        (v5.field_0 = this->field_140->get_ai_by_index(v3 + 1), v5.get_volatile_ptr() != nullptr)) {
        return get_traffic_from_entity(v5);
    }

    return nullptr;
}

vector3d traffic::pop_dest()
{
    auto v5 = this->field_1EC.back();
    this->field_1EC.pop_back();

    return v5;
}

bool traffic::is_not_chase_lane(traffic_path_lane *a1)
{
    if (!traffic_list.empty()) {
        if (old_drivers[2] + old_drivers[3] > 0) {
            for (auto &v4 : traffic_list) {
                if (v4 != nullptr) {
                    auto v5 = v4->field_1C4;
                    if (v5 == 2 || v5 == 1) {
                        if (sub_6DA670(v4->get_current_lane(), a1) || sub_6DA670(v4->field_148, a1)) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void traffic::update_destination(Float a2)
{
    auto v3 = this->field_15C;
    if (this->field_1BC) {
        switch (v3) {
        case 1: {
            this->field_C.manage_vehicle_height(false);
            auto *v6 = this->field_140;
            if (v6 != nullptr) {
                auto v7 = this->field_168 + 1;
                this->field_190 = nullptr;
                this->field_168 = v7;
                auto v8 = v6->get_node(v7);
                this->sub_6DA3B0(v8[0], v8[1], v8[2]);
                this->field_1BC = false;
                auto *v9 = this->get_my_actor();
                if (v9->get_floor_offset() > EPSILON) {
                    this->field_15C = 3;
                }

                this->field_1BE = false;
                auto v10 = this->car_ahead();
                if (v10 != nullptr) {
                    auto v11 = v10->field_C.field_C8;
                    a2 = this->field_C.field_C8;
                    auto v26 = v11;
                    float v12 = v26;
                    if (v11 >= a2) {
                        v12 = a2;
                    }

                    this->field_C.field_C8 = v12;
                }
            }
            break;
        }
        case 2:
        case 3:
            goto LABEL_28;
        case 4: {
            this->clear_previous_lane();
            --this->field_168;
        LABEL_28:
            this->release_turn();
            auto *v15 = this->field_140;
            auto v16 = this->field_168 + 1;
            this->field_168 = v16;
            if (v16 >= v15->total_nodes) {
                auto *next_intersection = v15->get_next_intersection(1);
                if (next_intersection->has_stopsign(false)) {
                    this->field_178 = 0.0099999998;
                    this->field_15C = 8;
                } else if (!this->start_turn()) {
                    this->field_15C = 9;
                }
            } else {
                this->field_15C = 3;
                auto v17 = v15->get_node(v16);
                this->sub_6DA3B0(v17[0], v17[1], v17[2]);
                this->field_1BC = false;
            }
            break;
        }
        case 5:
            if (this->more_dests()) {
                auto v14 = this->pop_dest();
                this->sub_6DA3B0(v14[0], v14[1], v14[2]);
                this->field_1BC = false;
            }

            if (!this->more_dests()) {
                this->field_174 = 0;
                this->field_15C = 3;
            }

            break;
        case 6: {
            auto *the_traffic = traffic::get_traffic_from_entity(vhandle_type<entity>{this->field_174});
            if (the_traffic != this->car_behind() || this->is_not_chase_lane(this->field_140) ||
                !this->is_fully_pulled_over()) {
                if (this->more_dests()) {
                    auto v14 = this->pop_dest();
                    this->sub_6DA3B0(v14[0], v14[1], v14[2]);
                    this->field_1BC = false;
                }

                if (!this->more_dests()) {
                    this->field_174 = 0;
                    this->field_15C = 3;
                }
            }
            break;
        }
        case 8: {
            this->release_turn();
            if (this->field_1C4 || std::abs(this->field_C.field_C8) < EPSILON) {
                this->field_15C = 9;
            } else {
                this->field_178 += a2;
            }
            break;
        }
        case 9: {
            this->release_turn();
            this->field_140->get_next_intersection(1);
            if (!this->start_turn()) {
                this->field_178 = 0.0099999998;
            }
            break;
        }
        case 10: {
            if (!this->field_144) {
                this->field_15C = 9;
            } else {
                this->field_1BC = 0;
                auto *v4 = this->field_144;
                this->field_15C = 11;

                auto v5 = 0;
                if (v4->total_nodes == 0) {
                    v5 = v4->total_nodes - 1;
                }

                this->sub_6DA3B0(v4->nodes[v5][0], v4->nodes[v5][1], v4->nodes[v5][2]);
            }
            break;
        }
        case 11: {
            this->start_lane(this->field_144, 1);
            break;
        }
        case 12: {
            this->release_turn();

            this->field_1BC = true;
            auto v20 = this->field_C.sub_6DA250();
            this->sub_6DA3B0(v20[0], v20[1], v20[2]);
            break;
        }
        case 13: {
            this->release_turn();

            this->field_1BC = true;
            auto v20 = this->field_C.sub_6DA250();
            this->sub_6DA3B0(v20[0], v20[1], v20[2]);
            break;
        }
        default:
            break;
        }
    } else if ((v3 == 3 || (v3 == 2 && this->field_1C4)) && !this->field_1BE) {
        this->check_lane_change();
    }

    auto v21 = this->field_140;
    if (v21 != nullptr && v21->is_valid(nullptr)) {
        auto v22 = this->field_15C;
        if (v22 != 11 && v22 != 10) {
            auto v23 = this->field_16C;
            if (v23 <= 0) {
                this->sub_6DACB0(0);
            } else {
                auto ai_by_index = this->field_140->get_ai_by_index(v23 - 1);
                this->sub_6DACB0(ai_by_index);
            }
        }
    } else {
        int a2 = 0;
        if (this->get_my_actor()->my_handle.field_0) {
            *(float *)&this->field_170.field_0 = a2;
        } else {
            this->field_170.field_0 = 0;
        }
    }
}

bool traffic::start_lane(traffic_path_lane *lane, bool release)
{
    if (release)
        release_turn();
    field_1BE = false;
    if (field_140 && field_140->is_valid(nullptr))
        field_140->get_next_intersection(1)->remove_ai_from_intersection(
            vhandle_type<actor>{get_my_actor()->get_my_vhandle()}, 0);
    if (!lane)
        return false;
    set_current_lane(lane, -1, true);
    field_144 = lane;
    const auto position = field_C.get_abs_position();
    const auto first = lane->get_node(0);
    const auto second = lane->get_node(1);
    field_17C = static_cast<traffic_path_intersection::eDirection>(0);
    field_148 = nullptr;
    field_1BC = false;
    if (!release && (position - second).xz_length2() > 1.0f) {
        field_168 = 0;
        field_14C = first + (second - first).normalized();
        field_15C = 2;
    } else {
        field_168 = 1;
        field_15C = 3;
        field_14C = lane->get_node(field_168);
    }
    sub_6DACB0(lane->get_ai_for_actor());
    return true;
}


void traffic::clear_previous_lane()
{
    auto *v2 = this->previous_lane;
    if (v2 != nullptr && v2->is_valid(nullptr)) {
        auto v3 = this->get_my_actor()->get_my_vhandle();
        this->previous_lane->get_ai_index(vhandle_type<actor>{v3});
        this->previous_lane->remove_ai_from_lane(vhandle_type<actor>{v3});
        this->previous_lane->update_lane_indexes();
    }

    this->previous_lane = nullptr;
}

traffic *traffic::car_ahead()
{
    entity_base_vhandle handle{0};
    if (field_140 && field_16C > 0)
        handle = field_140->get_ai_by_index(field_16C - 1);
    else if (field_144 && field_144 != field_140)
        handle = field_144->get_ai_by_index(field_144->get_num_ais() - 1);
    return handle.get_volatile_ptr() ? get_traffic_from_entity(vhandle_type<entity>{handle}) : nullptr;
}


void traffic::test_script()
{
    if constexpr (0) {
    } else {
        CDECL_CALL(0x006D6350);
    }
}

void sub_6BA2A0(vhandle_type<entity> a1, uint32_t a2)
{
    if (a2 <= 7) {
        auto *v2 = vehicle::models()[a2];
        if (v2 != nullptr) {
            v2->sub_6B9F30(a1);
        }
    }
}

//FIXME
void traffic::terminate_traffic()
{
    if (!traffic_initialized) {
        return;
    }

    bool v13 = false;
    bool v14 = true;

    while (v14) {
        v14 = false;
        for (auto &the_traffic : traffic_list) {
            if (the_traffic != nullptr) {
                if (the_traffic->field_4 && the_traffic->field_158) {
                    if (!the_traffic->field_5) {
                        if (the_traffic->get_current_lane() != nullptr) {
                            the_traffic->un_spawn();
                        }
                    }

                    the_traffic->field_158 = false;
                    auto *e = the_traffic->get_my_actor();
                    auto bodytype = the_traffic->field_C.bodytype;
                    the_traffic = nullptr;
                    assert(e != nullptr);

                    if (e != nullptr) {
                        sub_6BA2A0(vhandle_type<entity>{e->get_my_handle()}, bodytype);
                    }

                    v14 = true;
                    continue;
                }

                sp_log("traffic was disabled with mission cars in world - this may cause problems");
                v13 = true;
            }
        }
    }

    if (!v13) {
        vehicle::terminate_vehicles();
        traffic_list.clear();
    }

    traffic_initialized = false;
}

void traffic::enable_traffic(bool a1, bool a2)
{
    TRACE("traffic::enable_traffic");

    if constexpr (1) {
        traffic_enabled = a1;
        if (a1 && !traffic_initialized) {
            initialize_traffic();
        }

        if (!a1 && traffic_initialized) {
            terminate_traffic();
        }
    } else {
        CDECL_CALL(0x006D33C0, a1, a2);
    }
}

static constexpr vector3d stru_9381D4{0.0, -99.0, 0.0};

vhandle_type<entity> sub_6C6790(uint32_t vehicle_type, int a3)
{
    vhandle_type<entity> result;
    if (vehicle_type <= 7 && vehicle::models()[vehicle_type] != nullptr) {
        vehicle::cur_vehicle_type = vehicle_type;
        auto v1 = vehicle::models()[vehicle_type]->create(a3);
        vehicle::cur_vehicle_type = -1;
        result.field_0 = {v1};
    } else {
        result.field_0 = {0};
    }

    return result;
}

void traffic::create_new_traffic(int a1)
{
    auto vehtype = (a1 >= 0 ? vehicle::pick_model(a1) : vehicle::pick_random_model());

    auto v3 = last_traffic_id++;
    auto v5 = sub_6C6790(vehtype, v3);

    if (v5.get_volatile_ptr() != nullptr) {
        auto *t = get_traffic_from_entity_slow(v5);

        assert(t != nullptr);
        assert(t->field_C.bodytype == static_cast<uint32_t>(vehtype));

        t->field_C.bodytype = vehtype;

        t->field_158 = true;
        t->field_1C4 = 0;

        auto *v7 = t->get_my_actor();
        v7->set_visible(false, false);
        t->field_C.sub_6BAED0(stru_9381D4);
    }
}

traffic *traffic::get_traffic_from_entity_slow(vhandle_type<entity> a1)
{
    if (!traffic_list.empty()) {
        for (auto &t : traffic_list) {
            if (t != nullptr) {
                if (t->get_my_actor()->my_handle == a1) {
                    return t;
                }
            }
        }
    }

    return nullptr;
}

void traffic::get_closest_point_on_lane_with_facing(vector3d *a1, vector3d *a2, bool a3)
{
    if constexpr (1) {
        auto *v3 = g_world_ptr->the_terrain->get_region_info_for_point(*a1);
        if (v3 != nullptr) {
            for (auto &reg : (*v3)) {
                auto *v6 = reg->get_traffic_path_graph();
                if (reg->is_interior()) {
                    continue;
                }

                if (v6 != nullptr) {
                    vector3d a5;
                    auto *lane = v6->get_closest_or_farthest_lane(
                        true,
                        *a1,
                        ZEROVEC,
                        &a5,
                        static_cast<traffic_path_lane::eLaneType>(static_cast<int>(!a3)),
                        false,
                        nullptr);
                    if (lane != nullptr) {
                        int idx = 0;
                        lane->get_node_before_point(a5, &idx);

                        vector3d node1 = lane->get_node(idx);
                        vector3d node2 = lane->get_node(idx + 1);

                        if (idx < 1 || idx < lane->get_num_nodes() - 2) {
                            node1 = lane->get_node(idx);
                            node2 = lane->get_node(idx + 1);
                        } else {
                            node1 = lane->get_node(idx - 1);
                            node2 = lane->get_node(idx);
                        }

                        assert((node2 - node1).xz_length2() > EPSILON);

                        auto v17 = (node2 - node1).normalized();

                        *a2 = v17;
                        *a1 = a5;
                        return;
                    }
                }
            }
        }

        sp_log("Something went wrong with the get_closest_point_on_lane_with_facing() function");
    } else {
        CDECL_CALL(0x006D0910, a1, a2, a3);
    }
}

bool traffic::is_unanimated_car(actor *a1)
{
    if constexpr (1) {
        bool result = false;
        if ((a1->field_4 & 0x800) != 0) {
            auto *v1 = traffic::get_traffic_from_entity({a1->my_handle});
            if (v1 == nullptr || !v1->field_202) {
                result = true;
            }
        }

        return result;
    } else {
        return (bool)CDECL_CALL(0x004ADE20, a1);
    }
}

bool traffic::is_ai_car_occupied() const
{
    if (ai_occupied_cars == nullptr) {
        return false;
    }

    for (auto &the_traffic : (*ai_occupied_cars)) {
        if (the_traffic == this) {
            assert(this->is_ai_potential_car());

            return true;
        }
    }

    return false;
}


bool sub_6DA550(const vector3d &a1, Float a2)
{
    auto *current_view_camera = g_game_ptr->get_current_view_camera(0);
    if (current_view_camera != nullptr) {
        auto abs_pos = current_view_camera->get_abs_position();

        auto abs_po = current_view_camera->get_abs_po();
        auto z_facing = abs_po.get_z_facing();

        auto v9 = a1 - abs_pos;

        if (a2 > v9.xz_length2()) {
            auto v8 = dot(v9, z_facing);
            if (v8 > 0.0) {
                return true;
            }
        }
    }

    return false;
}

bool sub_6DA630(const vhandle_type<parking_marker> &a1)
{
    auto *marker = a1.get_volatile_ptr();
    auto abs_pos = marker->get_abs_position();

    return sub_6DA550(abs_pos, 50.0 * 50.0);
}

void traffic::_critical_processing(Float a2)
{
    bool v3 = false;
    bool v4 = false;
    if (spawned_this_frame < 1) {
        v3 = true;
    }

    if (parking_density * traffic_density * 30.0f > parked_cars) {
        v4 = true;
    }

    if (!traffic_enabled || (traffic_density * 30.0f) <= (parked_cars + living_cars) ||
        (traffic_density * 15.0f) <= visible_cars) {
        v3 = false;
    }

    if (this->field_4 && v3) {
        if (this->is_ai_potential_car()) {
            return;
        }

        if (this->field_5) {
            parking_marker *open_parking_marker = nullptr;
            parking_marker *v7 = nullptr;
            if (this->field_C.bodytype == 1 && v4 &&
                (open_parking_marker = find_open_parking_marker(), (v7 = open_parking_marker) != nullptr) &&
                !sub_6DA630(vhandle_type<parking_marker>{v7->get_my_vhandle()})) {
                this->spawn_at_marker(open_parking_marker);
            } else if (parking_density < 1.0f) {
                traffic_path_graph *a3 = nullptr;
                traffic_path_graph *a4 = nullptr;
                int a5 = 0;
                auto *new_spawn_pos = this->get_new_spawn_pos(
                    static_cast<traffic_path_lane::eLaneType>(0), &this->field_1D4, &a3, &a4, &a5);

                if (new_spawn_pos == nullptr || poi_manager::near_violence_poi(this->field_1D4)) {
                    ++unspawned_this_frame;
                } else {
                    this->field_14C = this->prepare_for_spawn(new_spawn_pos, this->field_1D4, a5);
                    ++spawned_this_frame;
                }

                this->field_1D4 = FARAWAY;
            }
        }
    }

    if (!this->is_ai_potential_car() && this->field_4) {
        this->sub_6B9B60(a2);
    }
}


bool traffic::_is_viable_pos(const vector3d &position)
{
    auto *region = g_world_ptr->get_the_terrain()->find_region(position, nullptr);
    if (!region || !region->is_loaded())
        return false;
    if (old_drivers[2] > 0) {
        for (auto *car : traffic_list) {
            if (car && car->field_1C4 == 2 && (car->field_C.get_abs_position() - position).xz_length2() < 625.0f)
                return false;
        }
    }
    return true;
}

void traffic::spawn_at_marker(parking_marker *marker)
{
    marker->set_active(true);
    field_228 = marker->my_handle.field_0;
    auto *owner = field_C.get_my_actor();
    owner->set_allow_tunnelling_into_next_frame(true);
    entity_set_abs_po(owner, marker->get_abs_po());
    event_manager::raise_event(event::RESPAWNED_THIS_FRAME, get_my_actor()->my_handle);
    field_C.set_collidable(true);
    field_C.set_visible(true);
    field_C.pick_body_and_color();
    field_C.sub_6BA920(0);
    field_C.field_C8 = 0.0f;
    field_15C = 14;
    field_5 = false;
    get_my_actor()->invalidate_frame_delta();
    get_my_actor()->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    field_C.manage_engine_sounds(EPSILON, false);
}

void traffic::set_traffic_density(Float density)
{
    assert(density >= 0.0f);
    assert(density <= 1.0f);

    traffic_density = density;
}

void traffic::set_ai_controller(ai::ai_car_inode *a2)
{
    this->field_220 = a2;
}

void traffic::set_ai_car_occupied(bool a2)
{
    if (a2) {
        assert(this->is_ai_potential_car());

        if (!this->is_ai_car_occupied()) {
            if (ai_occupied_cars == nullptr) {
                ai_occupied_cars = new _std::vector<traffic *>{};
            }

            ai_occupied_cars->push_back(this);
        }

        assert(this->is_ai_car_occupied());
    } else {
        this->set_ai_controller(nullptr);

        if (ai_occupied_cars != nullptr) {
            for (auto &the_traffic : (*ai_occupied_cars)) {
                if (the_traffic == this) {
                    assert(this->is_ai_potential_car());
                    break;
                }
            }

            if (ai_occupied_cars->empty()) {
                if (ai_occupied_cars != nullptr) {
                    delete ai_occupied_cars;
                }

                ai_occupied_cars = nullptr;
            }
        }

        assert(!this->is_ai_car_occupied());
    }
}

void sub_6BC0A0(traffic *a1)
{
    if (traffic::getaway_car != a1) {
        traffic::getaway_car = a1;
        spawnable::spawn_spacing = 2.0;
        if (a1 == nullptr) {
            spawnable::spawn_spacing = 1.0;
        }
    }
}

void sub_6BC060(traffic *a1)
{
    if (traffic::manual_car != a1) {
        if (traffic::manual_car != nullptr) {
            traffic::manual_car->field_1C4 = 0;
        }

        traffic::manual_car = a1;
        if (a1 != nullptr) {
            a1->field_1C4 = -1;
            traffic::manual_car->field_4 = false;
        }
    }
}

void sub_6BC0D0(traffic *a1)
{
    if (traffic::emergency_car != a1) {
        traffic::emergency_car = a1;
    }
}

void traffic::set_driver_type(int a2)
{
    if (this->field_1C4 != a2) {
        if (this == manual_car) {
            sub_6BC060(nullptr);
        }

        if (this == getaway_car) {
            sub_6BC0A0(nullptr);
        }

        if (this == emergency_car) {
            sub_6BC0D0(nullptr);
        }

        this->field_1C4 = a2;
        switch (a2) {
        case -1:
            sub_6BC060(this);
            break;
        case 0:
        case 3:
            return;
        case 1:
            sub_6BC0D0(this);
            break;
        case 2:
            sub_6BC0A0(this);
            break;
        default:
            assert(0 && "unknown driver type");
            return;
        }
    }
}


traffic *traffic::get_traffic_from_entity(vhandle_type<entity> handle)
{
    auto *entity_ptr = handle.get_volatile_ptr();
    if (!entity_ptr)
        return nullptr;
    auto *core = entity_ptr->get_ai_core();
    if (!core)
        return nullptr;
    auto *node = static_cast<ai::traffic_inode *>(core->get_info_node(ai::traffic_inode::default_id, false));
    return node ? node->traffic_ptr : nullptr;
}

void traffic::set_traffic_model_usage(int modelid, Float a2)
{
    assert(modelid >= VEHICLE_MODEL_TAXI && modelid < VEHICLE_MODEL_MAX);
    if (modelid < 8) {
        stru_937FAC[modelid] = a2;
    }
}

bool traffic::add_traffic_model(int a1, mString &a2)
{
    float v4 = 1.0;
    mString v3 = a2;
    return vehicle::add_model(a1, v3, v4);
}

static float &flt_96C9D4 = var<float>(0x0096C9D4);

static void sub_6C3430()
{
    CDECL_CALL(0x006C3430);
}

static void sub_6C3490(Float a1)
{
    if (flt_96C9D4 > 0.0f) {
        auto v1 = flt_96C9D4 - a1;
        flt_96C9D4 = v1;
        if (v1 <= 0.0f) {
            sub_6C3430();
        }
    }
}

void traffic::advance_traffic(Float a1)
{
    //update_global_cheats();
    if (g_traffic_single_step) {
        --g_traffic_single_step;
    } else if (g_traffic_paused) {
        return;
    }

    if (traffic_enabled) {
        sub_6C3490(a1);

        if (spawned_this_frame && stru_937FA4 < 40.0f) {
            stru_937FA4 += 1.0f;
        }

        old_drivers = new_drivers;

        new_drivers = {};

        living_cars = 0;
        parked_cars = 0;
        visible_cars = 0;

        if (g_traffic_testscript) {
            --g_traffic_testscript;
            test_script();
        }

        traffic_signal_mgr::frame_advance(a1);
        if (!traffic_initialized) {
            if (nullptr == g_game_ptr->get_current_view_camera(0)) {
                return;
            }

            initialize_traffic();
        }

        lane_changes_this_frame = 0;
        spawned_this_frame = 0;
        unspawned_this_frame = 0;
    }
}

parking_marker *traffic::find_open_parking_marker()
{
    auto *camera = g_game_ptr->get_current_view_camera(0);
    const vector3d point = camera->get_abs_position();
    auto *region = camera->get_primary_region();
    if (!region || !region->collision_proximity_map)
        return nullptr;
    parking_marker_visitor visitor(point, flt_937FA8);
    region->parking_proximity_map->traverse_sphere(point, flt_937FA8, &visitor);
    return visitor.closest;
}


void traffic::set_current_lane(traffic_path_lane *lane, int index, bool remove_previous)
{
    if (field_140 == lane)
        return;
    auto *owner = get_my_actor();
    vhandle_type<actor> handle{owner ? owner->get_my_vhandle() : entity_base_vhandle{0}};
    if (field_140 && field_140->is_valid(nullptr) && handle.get_volatile_ptr()) {
        if (remove_previous) {
            field_140->remove_ai_from_lane(handle);
            field_140->update_lane_indexes();
        } else {
            previous_lane = field_140;
        }
    }
    field_140 = lane;
    if (lane && lane->is_valid(nullptr) && handle.get_volatile_ptr()) {
        if (lane->get_ai_index(handle) == -1)
            field_16C = index <= -1 ? lane->add_ai_to_lane(handle) : lane->add_ai_to_lane(handle, index);
        else
            lane->update_lane_indexes();
    }
}


void traffic::update_facing_lane()
{
    if (field_190 == field_140 && field_18C == field_168)
        return;
    const int end =
        field_168 < 1 ? 1 : (field_168 < field_140->get_num_nodes() - 1 ? field_168 : field_140->get_num_nodes() - 1);
    const auto forward = (field_140->get_node(end) - field_140->get_node(end - 1)).normalized();
    const vector3d right{forward.z, 0.0f, -forward.x};
    const vector3d up{-forward.y * forward.x, forward.z * forward.z + forward.x * forward.x, -forward.y * forward.z};
    field_194 = right.x;
    field_198 = right.y;
    field_19C = right.z;
    field_1A0 = up.x;
    field_1A4 = up.y;
    field_1A8 = up.z;
    field_1AC = forward.x;
    field_1B0 = forward.y;
    field_1B4 = forward.z;
    field_190 = field_140;
    field_18C = field_168;
}


actor *traffic::actor_ahead()
{
    entity_base_vhandle handle{0};
    if (field_16C > 0)
        handle = field_140->get_ai_by_index(field_16C - 1);
    else if (field_144 && field_144 != field_140)
        handle = field_144->get_ai_by_index(field_144->get_num_ais() - 1);
    return vhandle_type<actor>{handle}.get_volatile_ptr();
}

bool traffic::is_halting() const
{
    return field_15C == 12 || field_15C == 13;
}

bool traffic::is_halted() const
{
    return is_halting() && field_C.field_C8 < EPSILON;
}

bool traffic::is_destroyed_halt() const
{
    return is_halted() && field_208 - field_204 <= 0;
}


void traffic::check_obstacle_point(const vector3d &position, float radius, bool &stop, bool &slow, bool &clear,
                                   bool check_angle)
{
    auto delta = position - field_C.get_abs_position();
    delta.y = 0.0f;
    if (check_angle) {
        auto facing = get_my_actor()->get_abs_po().get_z_facing();
        delta.normalize();
        facing.normalize();
        if (dot(facing, delta) > std::cos(0.21816616f))
            return;
    }
    const float distance = delta.xz_length2();
    const float speed_scale = field_C.field_C8 * 0.2f;
    if (speed_scale >= 1.0f)
        radius *= speed_scale;
    if (distance <= radius * radius)
        stop = true;
    else if (distance > 6.0f * radius * radius)
        clear = true;
    else
        slow = true;
}


void traffic::check_obstacle(entity *other, bool &stop, bool &slow, bool &clear, bool check_angle)
{
    if (!other)
        return;
    float scale = old_drivers[2] && !field_1C4 && field_15C != 6 ? 3.0f : 1.5f;
    if (!(field_8 & 1))
        scale *= 2.0f;
    const float radius = (get_my_actor()->get_colgeom_radius() + other->get_colgeom_radius()) * scale + field_164;
    check_obstacle_point(other->get_abs_position(), radius, stop, slow, clear, check_angle);
    if (stop || slow)
        field_1C0 = other->get_my_vhandle().field_0;
}


bool traffic::point_in_front(const vector3d &position)
{
    auto delta = position - field_C.get_abs_position();
    delta.y *= 0.5f;
    const float scale = field_C.field_C8 * 0.1f;
    const float radius = 8.0f * (scale >= 1.0f ? scale : 1.0f);
    if (delta.xz_length2() > radius * radius)
        return false;
    auto facing = get_my_actor()->get_abs_po().get_z_facing();
    delta.normalize();
    facing.normalize();
    return dot(facing, delta) > std::cos(0.34906587f);
}

bool traffic::player_in_front()
{
    auto *player = g_world_ptr->get_hero_ptr(0);
    return player && !var<traffic *>(0x0096C9DC) && point_in_front(player->get_abs_position());
}

void traffic_patch()
{
    {
        FUNC_ADDRESS(address, &traffic::_do_spawn);
        //set_vfunc(0x008A5CF8, address);
    }

    {
        FUNC_ADDRESS(address, &traffic::set_current_lane);
        REDIRECT(0x006D9095, address);
    }

    //SET_JUMP(0x006D33C0, traffic::enable_traffic);
}
