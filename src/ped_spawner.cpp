#include "ped_spawner.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "wds.h"
#include "game.h"
#include "mstring.h"
#include "os_developer_options.h"
#include "oldmath_po.h"
#include "variable.h"
#include "vtbl.h"
#include "traffic_path.h"
#include "traffic_path_lane.h"
#include "ai_pedestrian.h"
#include "base_ai_core.h"
#include "als_inode.h"
#include "als_animation_logic_system.h"
#include "state_machine.h"
#include "event.h"
#include "event_manager.h"
#include "slab_allocator.h"
#include "terrain.h"
#include "ai_path.h"
#include "ai_quad_path.h"
#include "ai_quad_path_cell.h"
#include "region.h"
#include "physical_interface.h"
#include "traffic_path.h"
#include "camera.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <new>

VALIDATE_SIZE(ped_spawner, 0x4C);

_std::vector<ped_spawner *> &ped_spawner::ped_spawner_list = var<_std::vector<ped_spawner *>>(0x0096D270);
static auto &peds_initialized = var<bool>(0x0096C9B8);
static auto &special_proc_index = var<int>(0x0096C9C0);
static auto &special_proc_index_0 = var<int>(0x0096C9C4);
static auto &special_proc_timer = var<float>(0x0096C9C8);
static auto &num_peds_spawned = var<int>(0x0096C9CC);
static auto &peds_single_step = var<bool>(0x0096C9D0);
static auto &peds_paused = var<bool>(0x0096C9D1);
static auto &ped_density = var<float>(0x00937FF0);

namespace {
void __fastcall native_ped_spawn(ped_spawner *self, void *, vector3d position,
    vector3d facing, traffic_path_lane *lane, int node, bool first, bool moving)
{
    self->_do_spawn(position, facing, lane, node, first, moving);
}
void __fastcall native_ped_unspawn(ped_spawner *self, void *) { self->_un_spawn(); }

void __fastcall native_ped_critical(ped_spawner *, void *, Float) {}
actor *__fastcall native_ped_actor(ped_spawner *self, void *) { return self->get_my_actor(); }
void __fastcall native_ped_set_actor(ped_spawner *, void *, vhandle_type<actor>) {}
bool __fastcall native_ped_lane(ped_spawner *, void *, traffic_path_lane *) { return true; }
bool __fastcall native_ped_position(ped_spawner *, void *, const vector3d &) { return true; }
void __fastcall native_ped_init(ped_spawner *self, void *, vhandle_type<actor> handle)
{
    self->init_vars(handle);
}
void __fastcall native_ped_reset(ped_spawner *self, void *) { self->reset(); }
void __fastcall native_ped_place(ped_spawner *self, void *, vector3d position, const vector3d &facing)
{
    self->spawn(position, facing);
}
ai::pedestrian_inode *ped_node(actor *owner)
{
    auto *core = owner->get_ai_core();
    return core ? static_cast<ai::pedestrian_inode *>(core->get_info_node(ai::pedestrian_inode::default_id, false))
                : nullptr;
}
}

void *ped_spawner::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_ped_spawn),
        reinterpret_cast<void *>(&native_ped_unspawn),
        reinterpret_cast<void *>(&native_ped_critical),
        reinterpret_cast<void *>(&native_ped_actor),
        reinterpret_cast<void *>(&native_ped_set_actor),
        reinterpret_cast<void *>(&native_ped_lane),
        reinterpret_cast<void *>(&native_ped_position),
        reinterpret_cast<void *>(&native_ped_init),
        reinterpret_cast<void *>(&native_ped_reset),
        reinterpret_cast<void *>(&native_ped_place),
    };
    return table;
}

void *ped_spawner::operator new(std::size_t size)
{
    return size <= slab_allocator::get_max_object_size()
        ? slab_allocator::allocate(size, nullptr) : ::operator new(size);
}

void ped_spawner::operator delete(void *storage)
{
    if (sizeof(ped_spawner) <= slab_allocator::get_max_object_size())
        slab_allocator::deallocate(storage, nullptr);
    else
        ::operator delete(storage);
}

ped_spawner::ped_spawner(int index) : spawnable(vhandle_type<entity>{0}),
    field_C(index), field_10(nullptr), field_14(nullptr), field_18(nullptr),
    field_3C(0), field_40(false), field_41(false), field_44(0)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

ped_spawner::~ped_spawner()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    if (field_40 && field_3C.get_volatile_ptr()) {
        if (field_41)
            clear_non_ped_actor(false);
        else
            g_world_ptr->ent_mgr.release_entity(get_my_actor());
    }
    m_vtbl = reinterpret_cast<std::intptr_t>(spawnable::native_vtable());
}

void ped_spawner::init_vars(vhandle_type<actor> handle)
{
    field_3C = handle;
    field_48 = 0;
    field_18 = nullptr;
    reset();
    field_40 = true;
}

void ped_spawner::reset()
{
    field_10 = field_14 = nullptr;
    field_1C = field_20 = 0;
    field_4 = field_5 = true;
    if (field_18)
        exit_intersection();
    field_30 = ZEROVEC;
}

void ped_spawner::_do_spawn(vector3d position, vector3d facing, traffic_path_lane *lane,
    int, bool, bool)
{
    if (get_my_actor()) {
        sub_6BBD30(lane);
        auto callback = reinterpret_cast<void(__fastcall *)(ped_spawner *, void *, vector3d, const vector3d &)>(
            get_vfunc(m_vtbl, 0x24));
        callback(this, nullptr, position, facing);
    }
}

actor *ped_spawner::get_my_actor()
{
    return this->field_3C.get_volatile_ptr();
}

void ped_spawner::spawn(vector3d position, const vector3d &facing)
{
    if (auto *owner = get_my_actor()) {
        event_manager::raise_event(event::RESPAWNED_THIS_FRAME, field_3C.field_0);
        owner->unsuspend(true);
        owner->set_visible(true, false);
        owner->m_timer = 0;
        ++num_peds_spawned;
        auto *node = ped_node(owner);
        if (node) {
            float ground = 0.0f;
            node->calc_elevation(position.y, ground, position, false);
        }
        po placement;
        placement.set_po(facing, YVEC, position);
        owner->set_allow_tunnelling_into_next_frame(true);
        entity_set_abs_po(owner, placement);
        if (node) {
            node->sub_696AF0(position.y);
            auto *layer = node->field_20->field_1C->get_als_layer(static_cast<als::layer_types>(0));
            const vector3d direction = placement.get_z_facing();
            layer->set_desired_param(als::param{27, direction.x});
            layer->set_desired_param(als::param{28, direction.y});
            layer->set_desired_param(als::param{29, direction.z});
            node->restore_hit_pts();
        }
    }
    field_5 = false;
}

actor *ped_spawner::create_ped_actor()
{
    TRACE("ped_spawner::create_ped_actor");

    char v10[32]{};
    sprintf(v10, "PED_%u", this->field_C);

    entity *eb = nullptr;
    if (static_cast<unsigned>(std::rand() * (2.0 / 32768.0))) {
        static string_hash ped_fem_hash{int(to_hash("ped_fem"))};

        string_hash v7{v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_fem_hash, v7, 129);
        this->field_44 = 2;
    } else {
        static string_hash ped_male_hash{int(to_hash("ped_male"))};

        string_hash v7{v10};
        eb = g_world_ptr->ent_mgr.acquire_entity(ped_male_hash, v7, 129);
        this->field_44 = 1;
    }


    if (eb != nullptr) {
        eb->set_visible(false, false);
        if (eb->has_physical_ifc())
            eb->set_collisions_active(false, true);

        return bit_cast<actor *>(eb);
    }

    return nullptr;
}

void ped_spawner::sub_6BBD30(traffic_path_lane *lane)
{
    if (field_10 == lane || (lane != nullptr && !lane->is_valid(nullptr)))
        return;
    if (field_10 != nullptr && field_10->is_valid(nullptr)) {
        field_10->remove_ai_from_lane(field_3C);
        field_14 = field_10;
    } else {
        field_14 = nullptr;
    }
    if (lane != nullptr)
        lane->add_ai_to_lane(field_3C);
    field_10 = lane;
}

void ped_spawner::exit_intersection()
{
    if (get_my_actor() == nullptr || field_18 == nullptr)
        return;
    field_18->remove_ai_from_intersection(field_3C, 0);
    auto *list = field_18->get_ai_list();
    bool has_pedestrian = false;
    if (list != nullptr) {
        for (int index = 0; index < list->num_ais; ++index) {
            auto *occupant = index < 20 ? list->ais[index].get_volatile_ptr() : nullptr;
            if (occupant != nullptr && (occupant->field_4 & 0x800) == 0) {
                has_pedestrian = true;
                break;
            }
        }
    }
    if (!has_pedestrian)
        field_18->release_semaphore(false);
    field_18 = nullptr;
}

bool ped_spawner::can_do_special_processing() const
{
    if (special_proc_index > special_proc_index_0)
        return field_C <= static_cast<unsigned>(special_proc_index_0)
            || field_C >= static_cast<unsigned>(special_proc_index);
    return field_C >= static_cast<unsigned>(special_proc_index)
        && field_C <= static_cast<unsigned>(special_proc_index_0);
}

void ped_spawner::sub_6C2EA0(Float elapsed)
{
    auto *owner = get_my_actor();
    if (!owner)
        return;
    auto critical = reinterpret_cast<void(__fastcall *)(ped_spawner *, void *, Float)>(get_vfunc(m_vtbl, 8));
    critical(this, nullptr, elapsed);
    if (!field_4)
        return;
    sub_6B9B60(elapsed);
    if (field_5 || !can_do_special_processing())
        return;
    auto *camera = g_game_ptr->get_current_view_camera(0);
    const vector3d delta = owner->get_abs_position() - camera->get_abs_position();
    const float distance = delta.xz_length2();
    if (distance > 3025.0f) {
        un_spawn();
        return;
    }
    auto *node = ped_node(owner);
    const int maximum = static_cast<int>(ped_spawner_list.size() * static_cast<double>(ped_density));
    if (distance > 400.0f || num_peds_spawned > maximum || (node && node->field_D1)) {
        const auto forward = camera->get_abs_po().get_z_facing();
        if (delta.x * forward.x + delta.y * forward.y + delta.z * forward.z < 0.0f)
            un_spawn();
    }
}

void ped_spawner::sub_6B9B60(Float elapsed)
{
    spawnable::sub_6B9B60(elapsed);
}

void ped_spawner::_un_spawn()
{
    if (auto *owner = get_my_actor()) {
        if (auto *node = ped_node(owner)) {
            if (!node->my_param_block.get_optional_pb_int(string_hash("allow_unspawn"), 1, nullptr))
                return;
            node->reset();
            owner->get_ai_core()->stop_movement();
        }
        owner->set_visible(false, false);
        if (auto *node = ped_node(owner)) {
            node->reset();
            owner->get_ai_core()->stop_movement();
        }
        --num_peds_spawned;
    }
    sub_6BBD30(nullptr);
    reset();
    field_5 = true;
    ++field_48;
    if (field_41)
        clear_non_ped_actor(true);
}

void ped_spawner::clear_non_ped_actor(bool replace)
{
    if (field_18)
        exit_intersection();
    sub_6BBD30(nullptr);
    field_1C = field_20 = 0;
    if (auto *owner = get_my_actor()) {
        if (auto *node = ped_node(owner))
            node->set_ped_spawner(nullptr);
        field_3C = vhandle_type<actor>{0};
    }
    if (replace) {
        if (auto *owner = create_ped_actor()) {
            init_vars(vhandle_type<actor>{owner->my_handle});
            if (auto *node = ped_node(owner)) {
                node->set_ped_spawner(this);
                node->reset();
            }
        }
    }
    field_41 = false;
}

void ped_spawner::init()
{
    TRACE("ped_spawner::init");

    if (peds_initialized) {
        return;
    }
    ped_density = 1.0f;
    ped_spawner_list.clear();
    for (int index = 0; index < 10; ++index) {
        auto *spawner = new ped_spawner{index};
        next_ped_spawner = spawner;
        auto *ped_actor = spawner->create_ped_actor();
        next_ped_spawner = nullptr;
        if (ped_actor == nullptr) {
            delete spawner;
            break;
        }
        spawner->init_vars(vhandle_type<actor>{ped_actor->get_my_vhandle()});
        ped_spawner_list.push_back(spawner);
    }
    special_proc_index = 0;
    special_proc_index_0 = 0;
    special_proc_timer = 0.0f;
    peds_initialized = true;
    const vector3d forward{0.0f, 0.0f, 1.0f};
    const vector3d up{0.0f, 1.0f, 0.0f};
    const vector3d position{-1234.0f, -1234.0f, -1234.0f};
    spawnable::last_camera_po.set_po(forward, up, position);
}

void ped_spawner::cleanup()
{
    TRACE("ped_spawner::cleanup");
    if (!peds_initialized) {
        return;
    }
    for (auto *spawner : ped_spawner_list) {
        delete spawner;
    }
    ped_spawner_list.clear();
    peds_initialized = false;
    num_peds_spawned = 0;
}

void ped_spawner::advance_peds(Float elapsed)
{
    TRACE("ped_spawner::advance_peds");

    if (peds_single_step) {
        peds_single_step = false;
    } else if (peds_paused) {
        return;
    }

    if (!os_developer_options::instance->get_flag(mString{"ENABLE_PEDESTRIANS"})) {
        if (peds_initialized) {
            cleanup();
        }
        return;
    }
    if (!peds_initialized) {
        if (g_game_ptr->get_current_view_camera(0) == nullptr) {
            return;
        }
        init();
    }

    const int target_count = static_cast<int>(
        static_cast<double>(ped_spawner_list.size()) * ped_density);
    if (num_peds_spawned < target_count) {
        if ((g_world_ptr->time_manager.field_C & 1) == 0) {
            populate_quad_paths();
        }
        if (num_peds_spawned < target_count) {
            populate_lanes();
        }
    }
    for (auto *spawner : ped_spawner_list) {
        if (!spawner->get_my_actor())
            spawner->clear_non_ped_actor(true);
        spawner->sub_6C2EA0(elapsed);
    }

    special_proc_timer += elapsed.value;
    constexpr float interval = 0.011111111f;
    if (special_proc_timer > interval) {
        const int steps = static_cast<int>(static_cast<double>(special_proc_timer) * 90.0);
        special_proc_index = (special_proc_index_0 + 1) % 10;
        special_proc_index_0 = (special_proc_index_0 + steps) % 10;
        special_proc_timer = static_cast<float>(special_proc_timer - static_cast<double>(steps) * interval);
    }
}
namespace {
vector3d quad_path_test_position()
{
    auto *camera = g_game_ptr->get_current_view_camera(0);
    auto position = camera->get_abs_position();
    if (auto *hero = g_world_ptr->get_hero_ptr(0))
        position.y = hero->get_abs_position().y - hero->physical_ifc()->ground_elevation;
    auto forward = camera->get_abs_po().get_z_facing();
    forward.y = 0.0f;
    forward.normalize();
    const float angle = static_cast<float>(std::rand()) / 32768.0f * 6.2831854820251465f;
    const vector3d direction{std::cos(angle), 0.0f, std::sin(angle)};
    const float cosine = std::clamp(direction.x * forward.x + direction.z * forward.z, -1.0f, 1.0f);
    const float random = static_cast<float>(std::rand()) / 32768.0f;
    const float radius = std::acos(cosine) >= 0.7853981852531433f
        ? random * 10.0f + 5.0f : random * 20.0f + 40.0f;
    return position + direction * radius;
}

ai_quad_path_cell *spawner_quad_cell(const ped_spawner *spawner)
{
    auto *reg = reinterpret_cast<region *>(spawner->field_20);
    return spawner->field_1C && reg && (reg->flags & 0x10)
        ? reinterpret_cast<ai_quad_path_cell *>(spawner->field_1C) : nullptr;
}

void seed_quad_path_cell(ai_quad_path_cell *cell, region *reg)
{
    auto *camera = g_game_ptr->get_current_view_camera(0);
    const auto camera_position = camera->get_abs_position();
    int occupants = 0;
    for (auto *spawner : ped_spawner::ped_spawner_list)
        if (!spawner->field_5 && spawner_quad_cell(spawner) == cell && ++occupants == 3)
            break;
    const int capacity = 3 - occupants;
    const int start = static_cast<int>(static_cast<float>(std::rand()) / 8192.0f);
    vector3d positions[3];
    int count = 0;
    for (int corner = 0; corner < 4 && count < capacity; ++corner) {
        const auto midpoint = cell->get_midpoint();
        const auto offset = cell->field_0[(start + corner) % 4] - midpoint;
        if (offset.length2() <= 1.0f)
            continue;
        const auto candidate = midpoint + offset * 0.67f;
        auto toward = candidate - camera_position;
        const float distance = toward.length2();
        toward.y = 0.0f;
        toward.normalize();
        auto forward = camera->get_abs_po().get_z_facing();
        forward.y = 0.0f;
        forward.normalize();
        if (toward.x * forward.x + toward.z * forward.z >= 0.707f && distance <= 35.0f)
            continue;
        bool crowded = false;
        for (auto *spawner : ped_spawner::ped_spawner_list) {
            if (!spawner->field_5 && spawner_quad_cell(spawner) == cell) {
                if (auto *actor = spawner->get_my_actor())
                    if (static_cast<int>((actor->get_abs_position() - candidate).length2()) < 16) {
                        crowded = true;
                        break;
                    }
            }
        }
        for (int index = 0; index < count && !crowded; ++index)
            crowded = (candidate - positions[index]).length2() < 16.0f;
        if (!crowded && !pedestrian_seed_blocked(*camera, candidate))
            positions[count++] = candidate;
    }
    int index = 0;
    for (auto *spawner : ped_spawner::ped_spawner_list) {
        if (index == count)
            break;
        if (spawner->field_5) {
            auto position = positions[index++];
            if (spawner->get_my_actor()) {
                spawner->field_1C = reinterpret_cast<int>(cell);
                spawner->field_20 = reinterpret_cast<int>(reg);
                position.y += 2.0f;
                spawner->spawn(position, vector3d{1.0f, 0.0f, 0.0f});
            }
        }
    }
}
}

void ped_spawner::populate_quad_paths()
{
    const int target = static_cast<int>(static_cast<double>(ped_spawner_list.size()) * ped_density);
    if (num_peds_spawned >= target)
        return;
    const auto position = quad_path_test_position();
    vector3d projected = ZEROVEC;
    ai_quad_path *path = nullptr;
    ai_quad_path_cell *cell = nullptr;
    if (!ai_path::find_closest_point_on_path_to_point(position, Float{2.0f}, &projected, &path, &cell))
        return;
    if (cell->field_4D & 1)
        seed_quad_path_cell(cell, path->field_18);
    for (int edge = 0; edge < 4 && num_peds_spawned < target; ++edge)
        for (int index = 0; index < cell->neighbor_counts[edge] && num_peds_spawned < target; ++index) {
            auto *neighbor = cell->get_edge_neighbor(edge, index);
            if (neighbor && (neighbor->field_4D & 1))
                seed_quad_path_cell(neighbor, path->field_18);
        }
}

void ped_spawner::populate_lanes()
{
    TRACE("ped_spawner::populate_lanes");

    auto *camera = g_game_ptr->get_current_view_camera(0);
    _std::vector<traffic_path_lane *> lanes;
    lanes.reserve(10);
    for (const auto &information : *spawnable_lanes) {
        auto *lane = information.field_4;
        if (traffic_path_lane::lane_is_valid(lane) && lane->my_road
            && lane->my_road->total_in_lanes < 8 && lane->my_road->total_out_lanes < 8
            && lane->get_type() == 1)
            lanes.push_back(lane);
    }
    for (unsigned index = 1; index < lanes.size(); ++index)
        std::swap(lanes[index], lanes[std::rand() % (index + 1)]);
    const int target = static_cast<int>(static_cast<double>(ped_spawner_list.size()) * ped_density);
    if (target <= 0)
        return;
    int remaining = static_cast<int>(lanes.size());
    for (auto *lane : lanes) {
        if ((remaining & 1) == 0) {
            const int requested = std::min(2, target - num_peds_spawned);
            if (requested > 0)
                lane->seed_with_pedestrians(*camera, requested);
        }
        --remaining;
    }
}

ped_spawner *ped_spawner::assign_non_ped_actor(vhandle_type<actor> handle)
{
    auto *owner = handle.get_volatile_ptr();
    if (!owner)
        return nullptr;
    ped_spawner *behind = nullptr;
    ped_spawner *farthest = nullptr;
    ped_spawner *selected = nullptr;
    float farthest_squared = 0.0f;
    for (auto *spawner : ped_spawner_list) {
        if (spawner->field_41)
            continue;
        auto *current = spawner->get_my_actor();
        if (spawner->field_5 || !current) {
            selected = spawner;
            break;
        }
        if (!behind) {
            const auto forward = g_game_ptr->get_current_view_camera(0)->get_abs_po().get_z_facing();
            const vector3d delta = current->get_abs_position() - owner->get_abs_position();
            if (forward.x * delta.x + forward.y * delta.y + forward.z * delta.z < 0.0f) {
                behind = spawner;
            } else if (delta.length2() > farthest_squared) {
                farthest_squared = delta.length2();
                farthest = spawner;
            }
        }
    }
    if (!selected)
        selected = behind ? behind : farthest;
    if (!selected)
        return nullptr;
    if (selected->field_18)
        selected->exit_intersection();
    selected->sub_6BBD30(nullptr);
    selected->field_1C = selected->field_20 = 0;
    if (auto *current = selected->get_my_actor())
        g_world_ptr->ent_mgr.release_entity(current);
    selected->init_vars(handle);
    selected->field_44 = 0;
    selected->field_41 = true;
    selected->field_5 = false;
    vector3d direction;
    auto *lane = g_world_ptr->the_terrain->traffic_ptr->get_closest_or_farthest_lane(
        true, owner->get_abs_position(), ZEROVEC, &direction,
        static_cast<traffic_path_lane::eLaneType>(1), false, nullptr);
    if (lane)
        selected->sub_6BBD30(lane);
    return selected;
}

void ped_spawner_patch()
{
    {
        FUNC_ADDRESS(address, &ped_spawner::_do_spawn);
        //SET_JUMP(0x006BBDA0, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::spawn);
        //set_vfunc(0x008A5B94, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::create_ped_actor);
        SET_JUMP(0x006C30A0, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::sub_6C2EA0);
        REDIRECT(0x006D1C1D, address);
    }

    {
        FUNC_ADDRESS(address, &ped_spawner::sub_6B9B60);
        REDIRECT(0x006C2EE7, address);
    }

    REDIRECT(0x006D1B92, &ped_spawner::init);

    REDIRECT(0x006D1B74, &ped_spawner::cleanup);

    REDIRECT(0x006D86DA, &ped_spawner::advance_peds);

    REDIRECT(0x006D1BEA, &ped_spawner::populate_lanes);
}
