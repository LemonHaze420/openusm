#include "ai_std_hero.h"

#include "variable.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_state_jump.h"
#include "ai_state_swing.h"
#include "ai_state_run.h"
#include "pole_swing_state.h"
#include "ai_state_web_zip.h"
#include "ai_std_combat_target.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "colgeom_alter_sys.h"
#include "collide.h"
#include "collision_geometry.h"
#include "combat_inode.h"
#include "combat_state.h"
#include "common.h"
#include "conglom.h"
#include "controller_inode.h"
#include "custom_math.h"
#include "entity.h"
#include "entity_base_vhandle.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "glass_house_inode.h"
#include "glass_house_manager.h"
#include "hit_react_state.h"
#include "interaction_inode.h"
#include "line_info.h"
#include "oldmath_po.h"
#include "param_list.h"
#include "physical_interface.h"
#include "physics_inode.h"
#include "pick_up_state.h"
#include "plr_loco_crawl_state.h"
#include "plr_loco_crawl_transition_state.h"
#include "pole_swing_inode.h"
#include "put_down_state.h"
#include "trace.h"
#include "subdivision_obb.h"
#include "strength_test_inode.h"
#include "terrain.h"
#include "throw_state.h"
#include "utility.h"
#include "vector2d.h"
#include "vtbl.h"
#include "state_machine.h"
#include "wds.h"
#include "damage_interface.h"
#include "input_mgr.h"
#include "rumble_manager.h"
#include "ai_plr_loco_crawling.h"
#include "movement_info.h"
#include <cstring>
#include "attach_state.h"
#include "interaction_state.h"

#include <cmath>
#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(hero_inode::internal, 0x124);
VALIDATE_SIZE(hero_inode, 0x24C);
VALIDATE_OFFSET(hero_inode, field_70, 0x70);
VALIDATE_OFFSET(hero_inode, field_88, 0x88);
VALIDATE_OFFSET(hero_inode, field_1B0, 0x1B0);
VALIDATE_OFFSET(hero_inode, field_20C, 0x20C);
VALIDATE_OFFSET(hero_inode, field_23C, 0x23C);

namespace {
void __fastcall hero_destruct(hero_inode *self, void *)
{
    self->field_88.field_20.remove_to_collision_check_queue();
    self->field_88.field_7C.field_0.remove_to_collision_check_queue();
    self->field_1B0.remove_to_collision_check_queue();
    self->_destruct_mashed_class();
}
void __fastcall hero_unmash(hero_inode *self, void *, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}
void *__fastcall hero_delete(hero_inode *self, void *, unsigned flags)
{
    self->~hero_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall hero_type(hero_inode *, void *)
{
    return 384;
}
bool __fastcall hero_subclass(hero_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall hero_needs_advance(hero_inode *, void *)
{
    return true;
}
void __fastcall hero_advance(hero_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
void __fastcall hero_activate(hero_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall hero_deactivate(hero_inode *self, void *)
{
    self->_deactivate();
}
int __fastcall hero_size(hero_inode *, void *)
{
    return sizeof(hero_inode);
}
}  // namespace

void *hero_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 12> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(&hero_destruct);
        result[1] = reinterpret_cast<void *>(&hero_unmash);
        result[2] = reinterpret_cast<void *>(&hero_delete);
        result[3] = reinterpret_cast<void *>(&hero_type);
        result[4] = reinterpret_cast<void *>(&hero_subclass);
        result[6] = reinterpret_cast<void *>(&hero_needs_advance);
        result[7] = reinterpret_cast<void *>(&hero_advance);
        result[8] = reinterpret_cast<void *>(&hero_activate);
        result[9] = reinterpret_cast<void *>(&hero_deactivate);
        result[11] = reinterpret_cast<void *>(&hero_size);
        return result;
    }();
    return table.data();
}

hero_inode::internal::internal() : field_0(0) {}

hero_inode::hero_inode()
    : info_node(), field_1C(false), field_4C(0), field_7C(false), field_1AC(false), field_1AD(false),
      field_20C{static_cast<crawl_transition_type_enum>(0), false, false, 0.0f, 0.0f, ZEROVEC, ZEROVEC, 0.0f},
      field_23C{0}, field_240(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
}

hero_inode::internal::internal(from_mash_in_place_constructor *constructor)
    : field_10(constructor), field_20(constructor), field_7C(constructor)
{
    field_0 = 0;
}

hero_inode::hero_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), field_58(constructor), field_64(constructor), field_88(constructor),
      field_1B0(constructor),
      field_20C{static_cast<crawl_transition_type_enum>(0), false, false, 0.0f, 0.0f, ZEROVEC, ZEROVEC, 0.0f}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
    field_23C = {0};
}

void hero_inode::_unmash(mash_info_struct *info, void *context)
{
    info_node::_unmash(info, context);
}

void hero_inode::_deactivate()
{
    cleanup_collision_lists();
    if (field_248 != 0) {
        auto *owned = reinterpret_cast<void *>(field_248);
        using deleting_destructor = void(__fastcall *)(void *, void *, unsigned int);
        const auto table = *reinterpret_cast<std::intptr_t *>(owned);
        reinterpret_cast<deleting_destructor>(get_vfunc(table, 0))(owned, nullptr, 1);
    }
}

void hero_inode::rumble_and_damage(float fall_height)
{
    const float medium = field_8->field_50.get_pb_float(string_hash{"fall_medium_height"});
    const float large = field_8->field_50.get_pb_float(string_hash{"fall_large_height"});
    const bool medium_fall = fall_height >= medium && fall_height < large;
    if (!medium_fall && fall_height < large)
        return;
    if (auto *rumble = input_mgr::instance->rumble_ptr) {
        rumble->start_vibration(medium_fall ? 0.5f : 1.0f, medium_fall ? 0.4f : 0.5f, 0.0f, 0.0f, 1, 0.3f);
    }
    const string_hash no_attack{0};
    field_C->damage_ifc()->apply_damage(nullptr,
                                        fall_height - medium,
                                        1,
                                        field_C->get_abs_position(),
                                        vector3d{0.0f, 1.0f, 0.0f},
                                        0,
                                        no_attack,
                                        no_attack,
                                        no_attack,
                                        false,
                                        ZEROVEC,
                                        17,
                                        false);
}

bool sub_68A0B0(int a1)
{
    bool result;
    switch (a1) {
    case 4:
    case 6:
    case 7:
    case 8:
        result = true;
        break;
    default:
        result = false;
        break;
    }

    return result;
}

bool hero_inode::jump_can_go_to(string_hash a2)
{
    TRACE("ai::hero_inode::jump_can_go_to", a2.to_string());

    if constexpr (1) {
        auto *v4 = this->field_8;
        auto *info_node = bit_cast<als_inode *>(v4->get_info_node(als_inode::default_id, true));
        if (this->field_7C) {
            return false;
        }

        if (a2 == web_zip_state::default_id) {
            auto v7 = this->field_50;
            if (v7 != 13 && v7 != 12) {
                return true;
            }

            auto *v8 = this->field_8;
            auto *v9 = bit_cast<physics_inode *>(v8->get_info_node(physics_inode::default_id, true));
            auto vel = v9->field_1C->get_velocity();
            vector2d v10 = {vel.x, vel.z};
            auto len = v10.length();
            return len * 0.2f < -vel.y;
        }

        if ((a2 == plr_loco_crawl_transition_state::default_id || a2 == plr_loco_crawl_state::default_id) &&
            sub_68A0B0(this->field_50)) {
            auto v2 = this->field_70;
            if (v2 < 0.30000001f) {
                return false;
            }
        }

        static string_hash loco_allow_aerial_hit_react_id{int(to_hash("loco_allow_aerial_hit_react"))};

        auto v19 = 0;
        auto *v11 = this->field_8;
        auto optional_pb_int = v11->field_50.get_optional_pb_int(loco_allow_aerial_hit_react_id, v19, nullptr);

        if (a2 == hit_react_state::default_id && !optional_pb_int) {
            return false;
        }

        if (a2 == put_down_state::default_id) {
            return false;
        }

        if (a2 == swing_state::default_id) {
            static string_hash cat_id_jump_to_swing{int(to_hash("Jump_To_Swing"))};

            auto v15 = 0.0f;
            auto cat_id = info_node->get_category_id(static_cast<als::layer_types>(0));
            auto *v13 =
                bit_cast<als::state_machine *>(info_node->field_1C->get_als_layer(static_cast<als::layer_types>(0)));
            auto &anim_handle = v13->get_anim_handle();
            if (anim_handle.get_anim_ptr()) {
                auto &v14 = v13->get_anim_handle();
                v15 = v14.get_anim_norm_time();
            }

            return cat_id != cat_id_jump_to_swing || v15 >= 1.0f;
        } else {
            auto *v16 = info_node->field_1C->get_als_layer(static_cast<als::layer_types>(0));
            return v16->is_interruptable();
        }
    } else {
        return THISCALL(0x006A6E70, this, a2);
    }
}

void hero_inode::_frame_advance(Float time_step)
{
    auto *player_controller = get_actor()->get_player_controller();
    auto *physics = static_cast<physics_inode *>(field_8->get_info_node(physics_inode::default_id, true));
    if (!field_1C) {
        setup_hero_capsule(field_C);
        field_1C = true;
    }

    const auto position = physics->field_C->get_abs_po().get_position();
    field_C->get_primary_region();
    static constexpr float max_ground_dist = 10.0f;
    const auto end = position - UP * max_ground_dist;
    vector3d hit, normal;
    if (find_intersection(position,
                          end,
                          *local_collision::entfilter_entity_no_capsules,
                          *local_collision::obbfilter_lineseg_test,
                          &hit,
                          &normal,
                          nullptr,
                          nullptr,
                          nullptr,
                          false)) {
        field_244 = position.y - hit.y;
    } else {
        field_244 = max_ground_dist;
    }

    float swing_hold = 0.0f;
    const auto &swing_button = player_controller->get_gb_swing_raw();
    if (swing_button.is_pressed())
        swing_hold = (swing_button.is_flagged(0x20) ? 0.0f : swing_button.field_1C) + 0.05f;

    field_238 += time_step;
    if (player_controller->get_motion_force() > EPSILON)
        field_238 = 0.0f;

    auto *animation = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    als::param_list params;
    params.add_param(als::param{17, swing_hold});
    params.add_param(als::param{19, field_238});
    params.add_param(als::param{70, field_244});
    animation->field_1C->get_als_layer(static_cast<als::layer_types>(0))->set_desired_params(params);
    cleanup_collision_lists();
}

void hero_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    field_54 = field_50;
    field_50 = static_cast<eJumpType>(1);
    nearby_crawl_collidables = nullptr;
    nearby_swing_collidables = nullptr;
    field_7C = false;
    field_1C = false;
    field_23C = {0};
    field_20 = static_cast<als_inode *>(core->get_info_node(als_inode::default_id, true));
    field_28 = static_cast<physics_inode *>(core->get_info_node(physics_inode::default_id, true));
    field_24 = static_cast<controller_inode *>(core->get_info_node(controller_inode::default_id, true));
    field_2C = static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, true));
    field_30 = static_cast<base_full_target_inode *>(core->get_info_node(combat_target_inode::default_id, true));
    field_34 = static_cast<interaction_inode *>(core->get_info_node(interaction_inode::default_id, true));
    field_3C = static_cast<web_zip_inode *>(core->get_info_node(web_zip_inode::default_id, true));
    field_40 = static_cast<swing_inode *>(core->get_info_node(swing_inode::default_id, false));
    field_44 = static_cast<glass_house_inode *>(core->get_info_node(glass_house_inode::default_id, true));
    auto *strength = static_cast<strength_test_inode *>(core->get_info_node(strength_test_inode::default_id, true));
    field_248 = 0;
    field_38 = strength;
}

bool hero_inode::is_a_crawl_state(string_hash a1, bool a2)
{
    auto result = (a1 == plr_loco_crawl_state::default_id);
    if (a2) {
        if (a1 == plr_loco_crawl_transition_state::default_id || result) {
            result = true;
        }
    }

    return result;
}

bool hero_inode::ought_to_jump_off_wall(line_info &a2)
{
#if STANDALONE_SYSTEM
    if (!ought_to_stick_to_wall(a2, false))
        return false;
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    static const string_hash facing_arc{"jump_off_wall_facing_arc"};
    const float arc = field_8->field_50.get_pb_float(facing_arc);
    const auto axis = controller->get_axis(static_cast<controller_inode::eControllerAxis>(0));
    return -std::cos(arc * 0.017453292f * 0.5f) >= dot(axis, a2.hit_norm);
#else
    return (bool)THISCALL(0x006A6280, this, &a2);
#endif
}

bool hero_inode::ought_to_stick_to_wall(line_info &a2, bool a3)
{
    if constexpr (1) {
        auto *v4 = this->get_actor();
        entity *v20 = nullptr;
        subdivision_node_obb_base *v21 = nullptr;

        auto &v5 = v4->get_abs_po();

        auto v6 = this->field_C->get_render_scale()[1] * 0.5f;
        auto v27 = UP * v6;

        a2.field_0 = v5.get_position() + v27;

        vector3d a1;
        vector3d a5;
        if (!find_sphere_intersection(a2.field_0,
                                      2.0,
                                      *local_collision::entfilter_entity_no_capsules,
                                      *local_collision::obbfilter_sphere_test,
                                      &a1,
                                      &a5,
                                      &v20,
                                      &v21) ||
            !glass_house_manager::is_point_in_glass_house(a1) || a5[1] > 0.73242188f) {
            return false;
        }

        a2.field_C = a1;

        auto a2a = (v20 ? v20->get_my_handle() : 0);

        a2.hit_entity = {a2a};

        a2.hit_norm = a5;

        a2.hit_pos = a1;

        a2.m_obb = v21;
        a2.collision = true;

        auto *v15 = this->field_C->physical_ifc();
        auto *v16 = v15->field_84.get_volatile_ptr();
        entity *v17 = v20;
        entity *v18 = v16;

        bool result = true;
        if (is_noncrawlable_surface(a2) || have_relative_movement(v18, v17)) {
            result = false;
        }

        return result;
    } else {
        return (bool)THISCALL(0x0069FAF0, this, &a2, a3);
    }
}

bool hero_inode::accept_crawl_spot(vector3d position, vector3d normal)
{
    vector3d tangent = vector3d::cross(YVEC, normal);
    const float tangent_squared = tangent.length2();
    if (tangent_squared > 1.0e-10f)
        tangent *= 1.0f / std::sqrt(tangent_squared);
    vector3d bitangent = vector3d::cross(normal, tangent);
    const float bitangent_squared = bitangent.length2();
    if (bitangent_squared > 1.0e-10f)
        bitangent *= 1.0f / std::sqrt(bitangent_squared);
    line_info line;
    for (const vector3d &offset : {tangent, -tangent, bitangent, -bitangent}) {
        line.field_0 = position + offset + normal;
        line.field_C = position + offset - normal;
        if (!line.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr) &&
            !is_noncrawlable_surface(line))
            return false;
        if (dot(line.hit_norm, normal) < 0.9f)
            return false;
    }
    return true;
}

namespace {
template <class T>
void store_corner(corner_info &corner, unsigned offset, const T &value)
{
    std::memcpy(reinterpret_cast<char *>(&corner) + offset, &value, sizeof(value));
}
vector4d corner_plane(const line_info &line)
{
    return {line.hit_norm.x, line.hit_norm.y, line.hit_norm.z, -dot(line.hit_norm, line.hit_pos)};
}
void set_corner_plane(hero_inode *hero, corner_info &corner)
{
    const auto plane = corner_plane(corner.field_0);
    store_corner(corner, 0x98, plane);
    store_corner(corner, 0x88, plane);
    hero->field_88.field_10 = plane;
    std::memcpy(&hero->field_88.field_4, &corner.field_0.hit_pos, sizeof(vector3d));
}
bool corner_surface_forbidden(const line_info &line)
{
    if (auto *entity = line.hit_entity.get_volatile_ptr()) {
        if ((entity->field_4 & 0x800) != 0 || (entity->field_8 & 0x8000) != 0)
            return true;
        if (auto *geometry = entity->get_colgeom())
            if (geometry->get_type() == collision_geometry::CAPSULE)
                return true;
    }
    return line.m_obb != nullptr && (line.m_obb->flags & 0x40) != 0;
}
void find_corner(hero_inode *hero, corner_info &out)
{
    const auto &pose = hero->field_C->get_abs_po();
    const vector3d up = pose.get_y_facing();
    const vector3d forward = pose.get_z_facing();
    const vector3d right = pose.get_x_facing();
    const vector3d foot = pose.get_position() - up * hero->field_C->get_floor_offset();
    const vector4d ground_plane{up.x, up.y, up.z, -dot(foot, up)};
    const float forward_factor[5]{1.0f, 1.0f, 1.0f, 0.0f, 0.0f};
    const float side_factor[5]{0.0f, -1.0f, 1.0f, -1.0f, 1.0f};
    float nearest = 10000.0f;
    out.field_0.clear();
    store_corner(out, 0x84, 0.0f);
    for (int index = 0; index < 5; ++index) {
        const vector3d direction = forward * forward_factor[index] + right * side_factor[index];
        const vector3d horizontal = direction * 2.0f;
        line_info wall, high;
        const auto check = [](line_info &line) {
            return line.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
        };
        high.field_0 = foot + up * 0.2f;
        high.field_C = high.field_0 + horizontal;
        if (check(high) && !corner_surface_forbidden(high)) {
            high.field_0 = foot + up * 1.4f;
            high.field_C = high.field_0 + horizontal;
            if (!check(high) && !is_noncrawlable_surface(high)) {
                out.field_0.clear();
                return;
            }
        }
        wall.field_0 = foot + up * 1.2f;
        wall.field_C = wall.field_0 + horizontal;
        if (!check(wall) && !corner_surface_forbidden(wall)) {
            wall.field_0 = wall.field_C;
            wall.field_C = wall.field_0 - up * 1.5f;
            if (!check(wall) && !is_noncrawlable_surface(wall)) {
                wall.field_0 = foot - up * 0.1f + horizontal;
                wall.field_C = wall.field_0 - horizontal;
                if (check(wall) && !is_noncrawlable_surface(wall)) {
                    line_info lower;
                    lower.field_0 = foot - up * 0.6f + horizontal;
                    lower.field_C = lower.field_0 - direction * 3.0f;
                    if (!check(lower) || is_noncrawlable_surface(lower))
                        store_corner(out, 0x84, 3.0f);
                    else
                        store_corner(out,
                                     0x84,
                                     (lower.field_0 - lower.hit_pos).length() - (wall.field_0 - wall.hit_pos).length());
                }
            }
        }
        if (wall.collision && !corner_surface_forbidden(wall)) {
            const vector3d normal = wall.hit_norm;
            const float wall_w = -dot(normal, wall.hit_pos);
            const vector3d axis = vector3d::cross(up, normal);
            const float axis_squared = axis.length2();
            const double alignment = dot(up, normal);
            if (alignment < 0.8f && axis_squared >= 0.0001f) {
                const double denominator = 1.0 - alignment * alignment;
                const vector3d line_origin =
                    up * static_cast<float>((alignment * wall_w - ground_plane.w) / denominator) +
                    normal * static_cast<float>((alignment * ground_plane.w - wall_w) / denominator);
                const vector3d probe = foot + forward * 0.5f;
                const double parameter = dot(probe - line_origin, axis) / axis_squared;
                const vector3d closest = line_origin + axis * static_cast<float>(parameter);
                const float distance = (closest - probe).length();
                if (distance < nearest) {
                    nearest = distance;
                    out.field_0.copy(wall);
                    store_corner(out, 0x68, axis);
                    store_corner(out, 0x5C, line_origin);
                    store_corner(out, 0x98, ground_plane);
                    store_corner(out, 0x88, corner_plane(wall));
                    store_corner(out, 0x74, closest);
                    store_corner(out, 0x80, distance);
                }
            } else {
                wall.clear();
            }
        }
        if (index == 0 && !wall.collision) {
            out.field_0.clear();
            return;
        }
    }
}
}  // namespace

bool hero_inode::get_closest_corner(corner_info *corner, crawl_request_type request)
{
    const int type = request.field_0;
    field_88.field_0 = type;
    if (type == 1 || type == 7) {
        find_corner(this, *corner);
        return corner->field_0.collision;
    }
    if (type == 4) {
        auto *zip = static_cast<web_zip_inode *>(field_8->get_info_node(web_zip_inode::default_id, true));
        corner->field_0.copy(zip->field_1C);
        set_corner_plane(this, *corner);
        return true;
    }
    if (type == 6) {
        auto *swing = static_cast<swing_inode *>(field_8->get_info_node(swing_inode::default_id, true));
        corner->field_0.hit_norm = swing->field_70;
        corner->field_0.hit_pos = swing->field_58 + swing->field_70;
        corner->field_0.collision = true;
        set_corner_plane(this, *corner);
        field_1AC = true;
        return true;
    }
    if (type != 2 && type != 3)
        return corner->field_0.collision;
    field_1AC = false;
    bool accepted = ought_to_stick_to_wall(corner->field_0, false);
    if (!accepted)
        return false;
    auto *physics = static_cast<physics_inode *>(field_8->get_info_node(physics_inode::default_id, true));
    if (!accept_crawl_spot(corner->field_0.hit_pos, corner->field_0.hit_norm)) {
        if (type == 2)
            return false;
        accepted = false;
    }
    if (type == 2) {
        field_8->get_info_node(controller_inode::default_id, true);
        const auto &pose = physics->get_abs_po();
        const float up_alignment = dot(pose.get_y_facing(), corner->field_0.hit_norm);
        const float forward_alignment = dot(pose.get_z_facing(), corner->field_0.hit_norm);
        accepted = (forward_alignment < -0.1f && corner->field_0.hit_norm.y > -0.2f) || -up_alignment > 0.8f ||
                   up_alignment > 0.8f;
        if (!accepted) {
            field_1AC = false;
            return false;
        }
    }
    set_corner_plane(this, *corner);
    field_1AC = accepted;
    return accepted;
}

bool hero_inode::run_is_eligible(string_hash a2)
{
    TRACE("hero_inode::run_is_eligible");

    if constexpr (1) {
        auto *v3 = this->field_8;
        auto *the_phys_inode = bit_cast<physics_inode *>(v3->get_info_node(physics_inode::default_id, true));
        this->field_4C = 0;
        if (auto *phys_ifc = the_phys_inode->field_1C;
            phys_ifc->is_effectively_standing() && phys_ifc->allow_manage_standing()) {
            if (a2 != jump_state::default_id) {
                this->field_58 = the_phys_inode->get_abs_position();
            }

            return true;
        }

        return false;

    } else {
        return THISCALL(0x006A7770, this, a2);
    }
}

static string_hash loco_allow_crawl_id{int(to_hash("loco_allow_crawl"))};

bool hero_inode::crawl_is_eligible(string_hash a2, bool a3)
{
    TRACE("hero_inode::crawl_is_eligible");

    if constexpr (1) {
        auto &v4 = this->field_8->field_50;
        if (v4.get_pb_int(loco_allow_crawl_id) == 0) {
            return false;
        }

        auto is_eligible = this->crawl_is_eligible_internals(a2, a3);
        if (is_eligible) {
            this->field_1B0 = this->field_88.field_7C.field_0;
        }

        return is_eligible;
    } else {
        return (bool)THISCALL(0x006B0EB0, this, a2, a3);
    }
}

bool hero_inode::oldcrawl_is_eligible(string_hash state, bool require_button)
{
    static const string_hash allow_oldcrawl{static_cast<int>(to_hash("loco_allow_oldcrawl"))};
    return field_8->field_50.get_pb_int(allow_oldcrawl) != 0 && crawl_is_eligible_internals(state, require_button);
}

void hero_inode::set_surface_info(const line_info &a2)
{
    this->field_1B0.copy(a2);
}

void hero_inode::update_wall_run_als_params()
{
    if constexpr (1) {
        auto *v2 = this->field_8;

        auto *v3 = (als_inode *)v2->get_info_node(als_inode::default_id, true);
        auto *v4 = this->get_actor();
        auto *v5 = v3;

        auto *v6 = this->get_actor();

        float v8 = -calculate_xz_angle_relative_to_local_po(v6->get_abs_po(), YVEC, v4->get_abs_po().get_z_facing());

        constexpr float flt_882080 = PI / 4.0;
        constexpr float flt_8A48CC = -flt_882080;

        v8 = std::clamp(v8, flt_8A48CC, flt_882080);

        float v12 = (v8 + flt_882080) / half_PI;

        als::param_list list;
        list.add_param({18, v12});

        auto *v9 = v5->field_1C->get_als_layer(static_cast<als::layer_types>(0));
        v9->set_desired_params(list);

    } else {
        THISCALL(0x006A67E0, this);
    }
}

void hero_inode::update_crawl_als_params()
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x006A63D0, this);
        return;
    }
    auto *input = static_cast<controller_inode *>(get_core()->get_info_node(controller_inode::default_id, true));
    auto *animation = static_cast<als_inode *>(get_core()->get_info_node(als_inode::default_id, true));
    auto *owner = get_actor();
    auto *layer = animation->get_als_layer(static_cast<als::layer_types>(0));
    if (!layer->is_active())
        return;
    vector3d up{field_88.field_10.x, field_88.field_10.y, field_88.field_10.z};
    if (up.length2() < 0.1f)
        up = owner->get_abs_po().get_y_facing();
    vector3d forward;
    float motion_force = 0.0f;
    const auto &pose = owner->get_abs_po();
    if (input->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0))) {
        const auto basis = is_colinear(pose.get_z_facing(), up, 0.01f) ? pose.get_y_facing() : pose.get_z_facing();
        forward = sub_444A60(basis, up);
        forward.normalize();
    } else {
        forward = sub_444A60(input->get_axis(static_cast<controller_inode::eControllerAxis>(0)), up);
        if (forward.length2() <= 0.1f)
            forward = is_colinear(pose.get_z_facing(), up, 0.01f) ? pose.get_y_facing() : pose.get_z_facing();
        forward.normalize();
        motion_force = owner->m_player_controller->get_motion_force();
    }
    up.normalize();
    als::param_list desired;
    desired.add_param({0, motion_force});
    desired.add_param(27, forward);
    desired.add_param(24, up);
    desired.add_param(42, field_1B0.hit_norm);
    desired.add_param(39, field_1B0.hit_pos);
    desired.add_param({6, 1.0f});
    layer->set_desired_params(desired);
}

bool hero_inode::run_can_go_to(string_hash a2)
{
    TRACE("hero_inode::run_can_go_to");

    if constexpr (1) {
        auto *v3 = this->field_8;

        v3->get_info_node(physics_inode::default_id, true);

        auto *v4 = this->field_8;

        auto *v5 = (als_inode *)v4->get_info_node(als_inode::default_id, true);
        if (a2 == hit_react_state::default_id) {
            return true;
        }

        if (a2 == throw_state::default_id &&
            (a2 = v5->get_category_id(static_cast<als::layer_types>(0)), a2 != cat_id_idle_walk_run())) {
            return false;
        }

        auto *v7 = v5->field_1C->get_als_layer(static_cast<als::layer_types>(0));
        return v7->is_interruptable();

    } else {
        return THISCALL(0x006A76D0, this, a2);
    }
}

void hero_inode::engage_water_exit()
{
    TRACE("hero_inode::engage_water_exit");

    THISCALL(0x006944D0, this);
}

bool hero_inode::jump_is_eligible(string_hash state)
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    auto *physics = static_cast<physics_inode *>(field_8->get_info_node(physics_inode::default_id, true));
    auto *als = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    static const string_hash allow_jump{static_cast<int>(to_hash("loco_allow_jump"))};
    static const string_hash allow_super_jump{static_cast<int>(to_hash("loco_allow_super_jump"))};
    static const string_hash allow_swing{static_cast<int>(to_hash("loco_allow_swing"))};
    if (field_8->field_50.get_pb_int(allow_jump) == 0)
        return false;
    const bool super_jump = field_8->field_50.get_pb_int(allow_super_jump) == 1;
    const auto choose = [this](int type) {
        set_jump_type(static_cast<eJumpType>(type), false);
        return true;
    };
    if (state == web_zip_state::default_id) {
        auto *zip = static_cast<web_zip_inode *>(field_8->get_info_node(web_zip_inode::default_id, true));
        if (zip->m_zip_type == 0)
            choose(12);
        else if (zip->m_zip_type == 2)
            choose(13);
        return true;
    }
    if (state == pole_swing_state::default_id)
        return choose(19);
    if (state == attach_state::default_id)
        return choose(0);
    const bool allow_swing_jump = field_8->field_50.get_pb_int(allow_swing) != 0;
    if (is_a_crawl_state(state, false)) {
        if ((allow_swing_jump || super_jump) &&
            controller->get_button(static_cast<controller_inode::eControllerButton>(11)).is_triggered()) {
            if (!super_jump)
                return choose(6);
            if (controller->get_axis(static_cast<controller_inode::eControllerAxis>(0)).length2() <= EPSILON)
                return choose(16);

            const auto &pose = field_C->get_abs_po();
            line_info forward, wall;
            forward.field_0 = pose.get_position();
            forward.field_C = forward.field_0 + pose.get_z_facing() * 7.0f;
            forward.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            bool clearance = false;
            if (!forward.collision) {
                wall.field_0 = forward.field_C;
                wall.field_C = wall.field_0 - pose.get_y_facing() * 1.5f;
                wall.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
                clearance = wall.collision;
            }
            return choose(clearance ? 17 : 18);
        }
        if (!controller->get_button(static_cast<controller_inode::eControllerButton>(7)).is_triggered())
            return false;
        const auto animation = als->get_state_id(static_cast<als::layer_types>(0));
        if (animation == string_hash{to_hash("Wall_Run")} || animation == string_hash{to_hash("Wall_Run_To_Crawl")})
            return choose(7);
        if (ought_to_stick_to_wall(field_1B0, false)) {
            if (dot(-YVEC, field_C->get_abs_po().get_z_facing()) < std::cos(0.7853981852531433))
                return choose(
                    controller->get_axis(static_cast<controller_inode::eControllerAxis>(0)).length2() <= EPSILON ? 6
                                                                                                                 : 8);
            return choose(5);
        }
        return true;
    }
    if (physics->field_1C->is_effectively_standing() || !physics->field_1C->allow_manage_standing()) {
        const bool triggered =
            controller->get_button(static_cast<controller_inode::eControllerButton>(11)).is_triggered();
        if (triggered &&
            (super_jump || als->get_category_id(static_cast<als::layer_types>(0)) == cat_id_idle_walk_run())) {
            if (super_jump)
                return choose(15);
            choose(9);

            return false;
        }
        if (controller->get_button(static_cast<controller_inode::eControllerButton>(7)).is_triggered())
            return choose(1);
        return false;
    }
    if (state == swing_state::default_id)
        return choose(11);
    if (state == interaction_state::default_id)
        return choose(20);
    return choose(0);
}

bool hero_inode::crawl_can_go_to(string_hash a2, string_hash a3)
{
    TRACE("hero_inode::crawl_can_go_to");

    if constexpr (1) {
        auto *v3 = this->field_8;
        auto *the_als_inode = bit_cast<als_inode *>(v3->get_info_node(als_inode::default_id, true));
        auto v5 = (a2 == jump_state::default_id || a2 == combat_state::default_id || a2 == web_zip_state::default_id ||
                   a2 == hit_react_state::default_id || a2 == pick_up_state::default_id ||
                   a2 == plr_loco_crawl_state::default_id);

        bool result = false;
        if (a3 == plr_loco_crawl_state::default_id && v5) {
            result = (v5 && the_als_inode->is_layer_interruptable(static_cast<als::layer_types>(0)));
        }

        return result;
    } else {
        return THISCALL(0x006A6340, this, a2, a3);
    }
}

void hero_inode::set_jump_type(eJumpType a2, bool a3)
{
    this->field_54 = static_cast<eJumpType>(a3 ? 21 : this->field_50);
    this->field_50 = a2;
}

void hero_inode::cleanup_crawl_collision_list()
{
    if (this->nearby_crawl_collidables != nullptr) {
        local_collision::destroy_primitive_list(&this->nearby_crawl_collidables);
    }

    assert(nearby_crawl_collidables == nullptr);
}

void hero_inode::cleanup_swing_collision_list()
{
    if (this->nearby_swing_collidables != nullptr) {
        local_collision::destroy_primitive_list(&this->nearby_swing_collidables);
    }

    assert(nearby_swing_collidables == nullptr);
}

void hero_inode::cleanup_collision_lists()
{
    cleanup_crawl_collision_list();
    cleanup_swing_collision_list();
}

hero_type_enum hero_inode::get_hero_type()
{
    auto *hero = static_cast<actor *>(g_world_ptr->get_hero_ptr(0));
    return hero != nullptr ? static_cast<hero_type_enum>(hero->m_player_controller->m_hero_type) : UNDEFINED;
}

bool hero_inode::crawl_is_eligible_internals(string_hash state, bool require_button)
{
    auto *controller = static_cast<controller_inode *>(field_8->get_info_node(controller_inode::default_id, true));
    auto *physics = static_cast<physics_inode *>(field_8->get_info_node(physics_inode::default_id, true));
    if (!glass_house_manager::is_point_in_glass_house(physics->field_C->get_abs_position()))
        return false;
    clear_curr_ground();
    const bool pressed =
        !require_button || controller->get_button(static_cast<controller_inode::eControllerButton>(1)).is_pressed();
    const auto accept_corner = [this](const corner_info &corner) {
        field_20C.field_0 = static_cast<crawl_transition_type_enum>(0);
        field_88.field_7C = corner;
        return true;
    };
    if (state == swing_state::default_id || state == swing_state::auto_crawl_attach_id) {
        corner_info corner;
        const int request = state == swing_state::auto_crawl_attach_id ? 2 : 6;
        if (pressed && get_closest_corner(&corner, static_cast<crawl_request_type>(request))) {
            physics->field_1C->set_velocity(ZEROVEC, false);
            return accept_corner(corner);
        }
        return false;
    }
    auto *als = static_cast<als_inode *>(field_8->get_info_node(als_inode::default_id, true));
    if (state == web_zip_state::default_id) {
        corner_info corner;
        if (get_closest_corner(&corner, static_cast<crawl_request_type>(4)) &&
            als->is_layer_interruptable(static_cast<als::layer_types>(0)))
            return accept_corner(corner);
    }
    if (state == jump_state::default_id ||
        als->get_category_id(static_cast<als::layer_types>(0)) == string_hash{to_hash("Jump_Air")}) {
        corner_info corner;
        const int request = field_50 == static_cast<eJumpType>(17) || field_50 == static_cast<eJumpType>(18) ? 3 : 2;
        if (pressed && get_closest_corner(&corner, static_cast<crawl_request_type>(request)))
            return accept_corner(corner);
        return false;
    }
    if (controller->is_axis_neutral(static_cast<controller_inode::eControllerAxis>(0)) ||
        state != run_state::default_id || !pressed)
        return false;
    corner_info corner;
    if (!get_closest_corner(&corner, static_cast<crawl_request_type>(1)))
        return false;
    const bool allow_crawl = field_8->field_50.get_pb_int(loco_allow_crawl_id) != 0;
    vector4d plane;
    std::memcpy(&plane, reinterpret_cast<const char *>(&corner) + 0x88, sizeof(plane));
    float offset;
    std::memcpy(&offset, reinterpret_cast<const char *>(&corner) + 0x80, sizeof(offset));
    if (dot(vector3d{plane.x, plane.y, plane.z}, field_C->get_abs_po().get_z_facing()) < 0.0f && offset < 0.8f) {
        field_88.field_7C = corner;
        field_20C.field_1C = corner.field_0.hit_pos;
        field_20C.field_10 = corner.field_0.hit_norm;
        field_20C.field_8 = 90.0f;
        field_20C.field_C = 0.0f;
        field_20C.field_0 = static_cast<crawl_transition_type_enum>(6);
        field_20C.field_28 = field_C->m_player_controller->m_hero_type == VENOM ? 1.3f : 0.5f;
        compute_curr_ground_plane(static_cast<force_recompute_enum>(1), 2.5f);
        return true;
    }
    if (allow_crawl) {
        const auto *surface =
            check_exterior_transition(field_C, field_20C, als, get_hero_type() != VENOM, false, false);
        field_88.field_7C.field_0.copy(*surface);
        if (field_88.field_7C.field_0.collision) {
            field_1B0.copy(field_88.field_7C.field_0);
            const auto &normal = field_88.field_7C.field_0.hit_norm;
            field_88.field_10 = vector4d{normal.x, normal.y, normal.z, -dot(normal, field_88.field_7C.field_0.hit_pos)};
            field_20C.field_0 = static_cast<crawl_transition_type_enum>(7);
            return true;
        }
    }
    return false;
}

void hero_inode::clear_curr_ground()
{
    this->field_88.field_10[0] = 0.0;
    this->field_88.field_10[1] = 0.0;
    this->field_88.field_10[2] = 0.0;
    this->field_88.field_10[3] = 0.0;

    this->field_88.field_20.clear();
}

bool hero_inode::compute_curr_ground_plane(force_recompute_enum a2, Float a3)
{
    TRACE("hero_inode::compute_curr_ground_plane");

    if constexpr (STANDALONE_SYSTEM) {
        auto func = [](const line_info &self) -> vector4d {
            auto v3 = -dot(self.hit_norm, self.hit_pos);
            vector4d a2{self.hit_norm, v3};
            return a2;
        };

        bool v3 = false;
        if (a2 == 1 || bit_cast<vector3d *>(&this->field_88.field_10)->length2() < 0.000099999997) {
            auto *v8 = this->get_actor();

            auto &abs_po = v8->get_abs_po();
            auto v42 = -abs_po.get_y_facing() * a3;

            line_info a1{};
            a1.clear();

            auto *v10 = this->get_actor();
            auto &v11 = v10->get_abs_po();

            auto v45 = v11.get_y_facing() * 0.1f;

            auto *v12 = this->get_actor();
            auto v13 = v12->get_abs_position();

            a1.field_0 = v13 - v45;

            a1.field_C = a1.field_0 + v42;
            a1.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            if (!a1.collision || is_noncrawlable_surface(a1)) {
                a1.clear();
                auto *v19 = this->get_actor();
                auto &v20 = v19->get_abs_po();

                auto *v21 = this->get_actor();
                auto &v22 = v21->get_abs_po();
                vector3d v23 = v22.get_y_facing() + v20.get_z_facing();

                auto *v26 = this->get_actor();
                auto v40 = v23 * EPSILON;
                auto v28 = v26->get_abs_position();

                auto *v29 = this->get_actor();
                a1.field_0 = v28 - v40;

                vector3d v51 = a1.field_0 + v42;
                a1.field_C = v51;

                auto v31 = v29->get_abs_po().get_z_facing();
                vector3d v52 = -1.0f * (v31 * 2.5f);
                a1.field_C += v52;
                if (!a1.check_collision(*local_collision::entfilter_entity_no_capsules,
                                        *local_collision::obbfilter_lineseg_test,
                                        nullptr) ||
                    is_noncrawlable_surface(a1)) {
                    this->clear_curr_ground();
                } else {
                    auto v34 = func(a1);
                    this->field_88.field_10 = v34;
                    this->field_88.field_20 = a1;
                    v3 = true;
                }
            } else {
                auto v17 = func(a1);
                this->field_88.field_10 = v17;
                this->field_88.field_20 = a1;
                v3 = true;
            }
        }

        return v3;
    } else {
        bool(__fastcall * func)(void *, void *edx, force_recompute_enum a2, Float a3) = CAST(func, 0x00698970);
        return func(this, nullptr, a2, a3);
    }
}

static const string_hash bip01_pelvis{int(to_hash("BIP01 PELVIS"))};

static const string_hash bip01_l_foot{int(to_hash("BIP01 L FOOT"))};

static const string_hash bip01_r_foot{int(to_hash("BIP01 R FOOT"))};

void shrink_capsule_for_slanted_surfaces(actor *act)
{
    if constexpr (1) {
        assert(act->is_a_conglomerate());

        auto *v1 = act->get_ai_core();
        v1->create_capsule_alter();
        auto *capsule_alter = act->get_ai_core()->field_70;
        assert(capsule_alter != nullptr);

        capsule_alter->set_mode((capsule_alter_sys::eAlterMode)3);

        conglomerate *conglm_ptr = CAST(conglm_ptr, act);
        auto *v3 = conglm_ptr->get_bone(bip01_pelvis, true);
        capsule_alter->set_base_avg_node(1, v3, 1.0);
        capsule_alter->set_base_avg_node(2, nullptr, 0.0);

        auto *ctrl = act->m_player_controller;
        assert(ctrl != nullptr);

        if (ctrl->m_hero_type == 2) {
            capsule_alter->set_avg_radius(0.5);
        } else {
            capsule_alter->set_avg_radius(0.30000001);
        }

    } else {
        CDECL_CALL(0x0068A5F0, act);
    }
}

void extend_capsule_for_jump(actor *act)
{
    assert(act->is_a_conglomerate());

    ai_core *v1 = act->get_ai_core();
    v1->create_capsule_alter();
    auto *capsule_alter = act->get_ai_core()->field_70;
    capsule_alter->set_mode((capsule_alter_sys::eAlterMode)3);

    conglomerate *conglom_ptr = CAST(conglom_ptr, act);
    auto *v3 = conglom_ptr->get_bone(bip01_l_foot, true);
    capsule_alter->set_base_avg_node(0, v3, 1.75);

    auto *v4 = conglom_ptr->get_bone(bip01_r_foot, true);
    capsule_alter->set_base_avg_node(1, v4, 1.75);

    auto *v5 = conglom_ptr->get_bone(bip01_pelvis, true);
    capsule_alter->set_base_avg_node(2, v5, 1.0);
    capsule_alter->set_base_avg_node(3, nullptr, 0.0);
}

}  // namespace ai

bool have_relative_movement(entity *first, entity *second)
{
    const auto moved = [](const po &delta) {
        return (delta.slow_xform(XVEC) - XVEC).length2() + (delta.slow_xform(YVEC) - YVEC).length2() +
                   (delta.slow_xform(ZVEC) - ZVEC).length2() >
               0.001f;
    };
    const auto moving_delta = [&moved](entity *entity) -> const po * {
        if (entity == nullptr || !entity->is_an_actor())
            return nullptr;
        auto *actor = static_cast<::actor *>(entity);
        if (!actor->is_frame_delta_valid())
            return nullptr;
        const auto *delta = &actor->get_movement_info()->field_0;
        return moved(*delta) ? delta : nullptr;
    };
    const auto *first_delta = moving_delta(first);
    const auto *second_delta = moving_delta(second);
    if (first_delta == nullptr || second_delta == nullptr)
        return first_delta != second_delta;
    const auto *inverse = second_delta->inverse();
    const ptr_to_po composition{&first_delta->m, &inverse->m};
    po relative;
    relative.set_from_ptr_to_po_world(composition);
    return moved(relative);
}

bool get_axis_correction_delta(const vector3d &a1, const vector3d &a2, float a3, vector3d *corrected_hit_pos)
{
    assert(corrected_hit_pos != nullptr);

    auto v25 = a3 * a2 + a1;
    auto v7 = -a3;
    auto arg4a = v7;

    vector3d a5, a6;
    auto check1 = find_intersection(a1,
                                    v25,
                                    *local_collision::entfilter_entity_no_capsules,
                                    *local_collision::obbfilter_lineseg_test,
                                    &a5,
                                    &a6,
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    false);
    auto v9 = v7 * a2[0];

    vector3d v27;
    v27[1] = v7 * a2[1];
    v27[2] = v7 * a2[2];

    v25[0] = v9 + a1[0];
    v25[1] = v27[1] + a1[1];
    v25[2] = v27[2] + a1[2];

    vector3d v29, v33;

    auto check2 = find_intersection(a1,
                                    v25,
                                    *local_collision::entfilter_entity_no_capsules,
                                    *local_collision::obbfilter_lineseg_test,
                                    &v29,
                                    &v33,
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    false);

    if (!check1 && !check2) {
        auto v11 = a2 * a3;
        auto v26 = v11 + a1;
        auto v13 = a2 * 0.15000001;

        vector3d v24 = a1 - v13;
        check1 = g_world_ptr->the_terrain->find_region(v24, nullptr) &&
                 find_intersection(v24,
                                   v26,
                                   *local_collision::entfilter_entity_no_capsules,
                                   *local_collision::obbfilter_lineseg_test,
                                   &a5,
                                   &a6,
                                   nullptr,
                                   nullptr,
                                   nullptr,
                                   false);

        auto v15 = arg4a * a2;

        v26 = v15 + a1;

        v24 = a2 * 0.15000001 + a1;
        check2 = g_world_ptr->the_terrain->find_region(v24, nullptr) &&
                 find_intersection(v24,
                                   v26,
                                   *local_collision::entfilter_entity_no_capsules,
                                   *local_collision::obbfilter_lineseg_test,
                                   &v29,
                                   &v33,
                                   nullptr,
                                   nullptr,
                                   nullptr,
                                   false);
    }

    if (check1 && check2) {
        return false;
    }

    if (check1 != check2) {
        if (check1) {
            auto v22 = a2 * a3;
            v27 = a5 - v22;

            *corrected_hit_pos = v27;
        } else {
            assert(check2);

            auto v19 = a2 * a3;
            auto v27 = v19 + v29;

            *corrected_hit_pos = v27;
        }
        *corrected_hit_pos -= a1;
    }

    return true;
}

bool is_noncrawlable_surface(line_info &surface)
{
    if constexpr (!STANDALONE_SYSTEM) {
        bool (*func)(line_info *) = CAST(func, 0x0068A9D0);
        return func(&surface);
    }
    if (auto *hit = surface.hit_entity.get_volatile_ptr()) {
        if (hit->is_flagged(0x800u) || hit->is_ext_flagged(0x8000u))
            return true;
        if (auto *geometry = static_cast<entity *>(hit)->get_colgeom(); geometry && geometry->get_type() == 1)
            return true;
    }
    return surface.m_obb && surface.m_obb->is_flagged(0x40);
}


void hero_inode_patch()
{
    {
        FUNC_ADDRESS(address, &ai::hero_inode::_activate);
        set_vfunc(0x0087DAC4, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::run_can_go_to);
        REDIRECT(0x00488980, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::_frame_advance);
        set_vfunc(0x0087DAC0, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::crawl_is_eligible);
        //SET_JUMP(0x006B0EB0, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::run_is_eligible);
        SET_JUMP(0x006A7770, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::crawl_can_go_to);
        SET_JUMP(0x006A6340, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::engage_water_exit);
        REDIRECT(0x00478EAA, address);
    }

    {
        FUNC_ADDRESS(address, &ai::hero_inode::jump_can_go_to);
        SET_JUMP(0x006A6E70, address);
    }
}
