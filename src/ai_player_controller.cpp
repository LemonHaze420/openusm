#include "ai_player_controller.h"

#include "actor.h"
#include "base_ai_core.h"
#include "camera.h"
#include "common.h"
#include "conglom.h"
#include "debugutil.h"
#include "func_wrapper.h"
#include "game.h"
#include "generic_anim_controller.h"
#include "oldmath_po.h"
#include "quaternion.h"
#include "osassert.h"
#include "pole_swing_inode.h"
#include "spiderman_camera.h"
#include "trace.h"

#include <vtbl.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {
ai_player_controller *__fastcall destroy_player_controller(ai_player_controller *self, void *, unsigned char flags)
{
    self->~ai_player_controller();
    if ((flags & 1) != 0) {
        ::operator delete(self);
    }
    return self;
}

void __fastcall update_player_controls(ai_player_controller *self, void *, Float dt, bool force)
{
    self->update_controls(dt, force);
}



bool __fastcall inactive_player_mode(const ai_player_controller *, void *)
{
    return false;
}

template<int Mode>
bool __fastcall player_mode(const ai_player_controller *self, void *)
{
    return self->m_spidey_loco_mode == Mode;
}

bool __fastcall player_wall_mode(const ai_player_controller *self, void *)
{
    return self->m_spidey_loco_mode == 6 || self->m_spidey_loco_mode == 7;
}

float __fastcall player_motion_force(ai_player_controller *self, void *)
{
    return self->get_motion_force();
}

struct player_controller_callbacks {
    ai_player_controller *(__fastcall *destroy)(ai_player_controller *, void *, unsigned char);
    void (__fastcall *update_controls)(ai_player_controller *, void *, Float, bool);
    bool (__fastcall *mode[17])(const ai_player_controller *, void *);
    float (__fastcall *motion_force)(ai_player_controller *, void *);
};

const player_controller_callbacks player_callbacks{
    destroy_player_controller, update_player_controls,
    {inactive_player_mode, inactive_player_mode, inactive_player_mode,
     inactive_player_mode, inactive_player_mode, player_mode<1>, player_mode<1>,
     inactive_player_mode, inactive_player_mode, player_mode<1>,
     inactive_player_mode, player_wall_mode, player_mode<5>, player_mode<2>,
     inactive_player_mode, player_mode<3>, inactive_player_mode},
    player_motion_force
};


vector3d camera_movement_direction;

vector3d project_controller_direction(const vector3d &direction, const vector3d &normal)
{
    auto projected = direction - normal * dot(direction, normal);
    projected.normalize();
    return projected;
}

vector3d rotate_controller_direction(const vector3d &direction, const quaternion &rotation)
{
    const vector3d axis{rotation[1], rotation[2], rotation[3]};
    const auto twice_cross = vector3d::cross(axis, direction) * 2.0f;
    return direction + twice_cross * rotation[0] + vector3d::cross(axis, twice_cross);
}

quaternion controller_normal_rotation(const vector3d &previous, const vector3d &current)
{
    auto axis = vector3d::cross(previous, current);
    const auto length_squared = axis.length2().value;
    if (length_squared < 0.000001f) {
        return {1.0f, 0.0f, 0.0f, 0.0f};
    }
    const auto length = std::sqrt(length_squared);
    axis *= 1.0f / length;
    const auto half_angle = std::asin(std::min(length, 1.0f)) * 0.5f;
    axis *= std::sin(half_angle);
    return {std::cos(half_angle), axis.x, axis.y, axis.z};
}
}

VALIDATE_SIZE(ai_player_controller, 0x424u);

ai_player_controller::ai_player_controller(actor *a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = reinterpret_cast<std::intptr_t>(&player_callbacks);
        for (auto &axis : field_2BC) {
            axis.field_8 = 0.1f;
        }
        field_3E0 = ZEROVEC;
        field_4[1] = static_cast<conglomerate *>(a2);
        field_14 = 0;
        field_4[0] = nullptr;
        m_spidey_loco_mode = static_cast<eHeroLocoMode>(0);
        m_prev_spidey_loco_mode = static_cast<eHeroLocoMode>(0);
        field_3F8 = 0.0f;
        field_3FC = 0.0f;
        field_400 = YVEC;
        m_hero_type = find_hero_type();
        field_3DC = true;
        g_spiderman_camera_ptr()->sub_4B3260(false);
        force_always_camera_relative(false);
        remap_controls();
    } else {
        THISCALL(0x004728D0, this, a2);
    }
}

void ai_player_controller::lock_controls(bool a2)
{
    TRACE("ai_player_controller::lock_controls");

    if (a2 && !this->field_3DC) {
        error("Someone (likely script or UI code) tried to lock controls when they were already locked.");
    }

    debug_print_va("-- controls locked (previous was %s)", field_3DC ? "unlocked" : "locked");

    this->field_3DC = false;
    g_spiderman_camera_ptr()->sub_4B3260(true);
}

void ai_player_controller::unlock_controls(bool a2)
{
    if (a2 && this->field_3DC) {
        error("Someone (likely script or UI code) tried to unlock controls when they were already unlocked.");
    }

    debug_print_va("-- controls unlocked (previous was %s)", field_3DC ? "unlocked" : "locked");

    this->field_3DC = true;
    g_spiderman_camera_ptr()->sub_4B3260(false);
}

void ai_player_controller::set_spidey_loco_mode(eHeroLocoMode a2)
{
    if (a2 != this->m_spidey_loco_mode) {
        this->m_prev_spidey_loco_mode = this->m_spidey_loco_mode;
    }

    this->m_spidey_loco_mode = a2;
}

hero_type_enum ai_player_controller::find_hero_type() const
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *core = this->field_4[1]->get_ai_core();
        if (core == nullptr) {
            return hero_type_enum::UNDEFINED;
        }

        static constexpr const char *machine_names[] = {"SPIDEY", "VENOM", "PARKER"};
        for (std::size_t index = 0; index < std::size(machine_names); ++index) {
            resource_key key {string_hash {machine_names[index]}, RESOURCE_KEY_TYPE_AI_STATE_GRAPH};
            if (core->find_machine(key) != nullptr) {
                return static_cast<hero_type_enum>(index + 1);
            }
        }
        return hero_type_enum::UNDEFINED;
    } else {
        return static_cast<hero_type_enum>(THISCALL(0x00449390, this));
    }
}

anchor_storage_class ai_player_controller::get_poleswing_anchor() const
{
    auto *v2 = this->field_4[1]->get_ai_core();
    auto *v3 = (ai::pole_swing_inode *)v2->get_info_node(ai::pole_swing_inode::default_id, true);

    auto a2 = v3->field_1C;
    return a2;
}

int ai_player_controller::get_spidey_loco_mode() const
{
    return this->m_spidey_loco_mode;
}

void ai_player_controller::set_player_num(int a2)
{
    this->field_14 = a2;
    this->remap_controls();
}

void ai_player_controller::clear_controls()
{
    debug_print_va("-- controls cleared");
    this->remap_controls();
    this->gb_jump.clear_flags();
    this->gb_swing_raw.clear_flags();
    this->gb_attack.clear_flags();
    this->gb_attack_secondary.clear_flags();
    this->gb_grab.clear_flags();
    this->gb_range.clear_flags();
    this->field_150.clear_flags();
    this->field_254.clear_flags();
    this->field_288.clear_flags();
    this->gb_camera_center.clear_flags();
    this->field_1B8.clear_flags();
    this->gb_swing.clear_flags();
}

void ai_player_controller::remap_controls()
{
    if constexpr (STANDALONE_SYSTEM) {
        auto v2 = input_mgr::instance->field_58;
        this->gb_jump.set_id(v2);

        this->gb_swing_raw.set_id(v2);
        this->gb_attack.set_id(v2);
        this->gb_attack_secondary.set_id(v2);
        this->gb_grab.set_id(v2);
        this->gb_range.set_id(v2);
        this->field_150.set_id(v2);
        this->field_254.set_id(v2);
        this->field_288.set_id(v2);
        this->gb_camera_center.set_id(v2);
        this->field_1B8.set_id(v2);
        this->gb_swing.set_id(v2);
        this->field_220.set_id(v2);

        this->field_2BC[0].set_id(v2);
        this->field_2BC[1].set_id(v2);
        this->field_2BC[2].set_id(v2);
        this->field_2BC[3].set_id(v2);
        this->field_2BC[4].set_id(v2);
        this->field_2BC[5].set_id(v2);

        this->gb_jump.set_control(static_cast<game_control_t>(96));
        this->gb_swing_raw.set_control(static_cast<game_control_t>(101));

        this->gb_swing_raw.set_modifier(game_button{static_cast<game_control_t>(104)});

        this->gb_swing_raw.set_trigger_type(0);

        this->gb_attack.set_control((game_control_t)97);
        this->gb_attack_secondary.set_control((game_control_t)99);
        this->gb_grab.set_control((game_control_t)98);
        this->gb_range.set_control((game_control_t)104);

        this->gb_range.field_2C = 0.1f;

        this->field_150.set_control((game_control_t)100);
        this->field_254.set_control((game_control_t)104);
        this->field_288.set_control((game_control_t)101);

        this->gb_camera_center.set_primary(game_button{(game_control_t)100});
        this->gb_camera_center.set_modifier(game_button{(game_control_t)103});

        this->gb_camera_center.set_trigger_type(1);

        this->field_1B8.set_control((game_control_t)101);
        this->field_1B8.field_2C = 0.1f;

        this->gb_swing.set_control((game_control_t)101);
        this->field_220.set_control((game_control_t)114);

        this->field_2BC[0].set_control(106);
        this->field_2BC[1].set_control(107);
        this->field_2BC[2].set_control(110);
        this->field_2BC[3].set_control(111);
        this->field_2BC[4].set_control(108);
        this->field_2BC[5].set_control(109);
    } else {
        THISCALL(0x00468FE0, this);
    }
}

game_button *ai_player_controller::get_gb_jump()
{
    return &this->gb_jump;
}

game_button *ai_player_controller::get_gb_attack()
{
    return &this->gb_attack;
}

game_button *ai_player_controller::get_gb_attack_secondary()
{
    return &this->gb_attack_secondary;
}

game_button *ai_player_controller::get_gb_grab()
{
    return &this->gb_grab;
}

game_button *ai_player_controller::get_gb_range()
{
    return &this->gb_range;
}

game_button *ai_player_controller::get_gb_camera_center()
{
    return &this->gb_camera_center;
}

game_button &ai_player_controller::get_gb_swing_raw()
{
    return this->gb_swing;
}

vector3d ai_player_controller::convert_left_stick_from_camera_space_to_world_space(bool a3)
{
    if constexpr (!STANDALONE_SYSTEM) {
        vector3d result;
        THISCALL(0x00457C20, this, &result, a3);
        return result;
    }

    auto *cam = g_game_ptr->get_current_view_camera(0);
    if (cam == nullptr) {
        camera_movement_direction = ZEROVEC;
        return camera_movement_direction;
    }

    float horizontal = field_2BC[1].field_10;
    float vertical = -field_2BC[0].field_10;
    float magnitude = std::sqrt(horizontal * horizontal + vertical * vertical);
    if (magnitude <= EPSILON) {
        field_40C = std::sqrt(field_3F8 * field_3F8 + field_3FC * field_3FC);
        field_400 = ZEROVEC;
        field_3F8 = 0.0f;
        field_3FC = 0.0f;
        camera_movement_direction = ZEROVEC;
        return camera_movement_direction;
    }
    if (magnitude > 1.0f) {
        horizontal /= magnitude;
        vertical /= magnitude;
        magnitude = 1.0f;
    }
    const float dx = horizontal - field_3F8;
    const float dy = vertical - field_3FC;
    field_40C = std::sqrt(dx * dx + dy * dy);

    auto *hero = field_4[1];
    vector3d normal = hero->get_abs_po().get_y_facing();
    if (m_spidey_loco_mode == SWINGING || m_spidey_loco_mode == POLE_SWING) {
        normal = YVEC;
    } else if ((m_spidey_loco_mode == CRAWLING || m_spidey_loco_mode == 7) && hero->anim_ctrl != nullptr) {
        auto *animation = static_cast<generic_anim_controller *>(hero->anim_ctrl);
        po local_floor;
        auto floor_pose = reinterpret_cast<void (__fastcall *)(generic_anim_controller *, void *, po *)>(
            get_vfunc(animation->m_vtbl, 0x90));
        floor_pose(animation, nullptr, &local_floor);
        po world_floor;
        world_floor.set_from_ptr_to_po_world({&local_floor.m, &hero->get_abs_po().m});
        normal = world_floor.get_y_facing();
    } else {
        normal.normalize();
    }

    auto forward = hero->get_abs_position() - cam->get_abs_position();
    forward.normalize();
    auto camera_up = YVEC;
    if ((m_spidey_loco_mode == CRAWLING || m_spidey_loco_mode == 7) && std::abs(normal.y) < 0.1f) {
        forward = YVEC;
        camera_up = normal;
    }
    forward = project_controller_direction(forward, camera_up);
    auto right = vector3d::cross(camera_up, forward);
    right.normalize();

    if (field_400.length2() >= EPSILON) {
        const auto rotation = controller_normal_rotation(field_400, normal);
        if (std::not_equal_to<float>{}(rotation[0], 1.0f)) {
            camera_movement_direction = rotate_controller_direction(camera_movement_direction, rotation);
        }
    }
    if (camera_movement_direction.length2() >= EPSILON
        && field_3F8 * field_3F8 + field_3FC * field_3FC >= EPSILON
        && horizontal * horizontal + vertical * vertical >= EPSILON) {
        const float old_length = std::sqrt(field_3F8 * field_3F8 + field_3FC * field_3FC);
        const float new_length = std::sqrt(horizontal * horizontal + vertical * vertical);
        const float sine = std::clamp((field_3FC * horizontal - vertical * field_3F8)
                                     / (old_length * new_length), -1.0f, 1.0f);
        if (std::abs(sine) >= 0.000001f) {
            const float half_angle = std::asin(sine) * 0.5f;
            const auto axis = normal * std::sin(half_angle);
            camera_movement_direction = rotate_controller_direction(
                camera_movement_direction, {std::cos(half_angle), axis.x, axis.y, axis.z});
        }
    }
    camera_movement_direction = project_controller_direction(camera_movement_direction, normal);
    camera_movement_direction.set_length(magnitude);
    auto desired = project_controller_direction(right, normal) * horizontal
                 + project_controller_direction(forward, normal) * vertical;
    auto difference = desired - camera_movement_direction;
    const float max_change = std::max(field_3DD ? 1.0f : 0.025f, 1.75f * field_40C);
    if (difference.length2() > max_change * max_change) {
        difference.set_length(max_change);
    }
    desired = camera_movement_direction + difference;
    camera_movement_direction.set_length(magnitude);
    desired.normalize();
    field_3F8 = horizontal;
    field_3FC = vertical;
    field_400 = normal;
    camera_movement_direction = desired;
    return camera_movement_direction;
}

vector3d ai_player_controller::compute_left_stick_from_camera()
{
    vector3d result;

    if constexpr (1) {
        auto *cam = g_game_ptr->get_current_view_camera(0);
        if (cam != nullptr) {
            auto v8 = this->field_2BC[1].field_10;
            auto v7 = this->field_2BC[0].field_10;

            vector3d look, up;
            cam->get_look_and_up(&look, &up);

            vector3d v11 = vector3d::cross(up, look);

            look[1] = 0.0;
            look.normalize();

            v11[1] = 0.0;
            v11.normalize();
            if (std::abs(v8) < 0.1f) {
                v8 = 0.0;
            }

            if (std::abs(v7) < 0.1f) {
                v7 = 0.0;
            }

            v11 = v11 * v8 - look * v7;
            v11.normalize();

            result = v11;
        } else {
            result = ZEROVEC;
        }

        return result;

    } else {
        THISCALL(0x004495D0, this, &result);

        return result;
    }
}

void ai_player_controller::update_controls(Float a2, bool a3)
{
    TRACE("ai_player_controller::update_controls");

    if (a3 || (this->field_3DC && is_a_controllable_mode(this->m_spidey_loco_mode))) {
        this->gb_jump.update(a2);
        this->gb_swing_raw.update(a2);
        this->gb_attack.update(a2);

        this->gb_attack_secondary.update(a2);
        this->gb_grab.update(a2);
        this->gb_range.update(a2);
        this->field_150.update(a2);
        this->field_254.update(a2);
        this->field_288.update(a2);
        this->gb_camera_center.update(a2);
        this->field_1B8.update(a2);
        this->gb_swing.update(a2);

        this->field_2BC[0].update(a2);
        this->field_2BC[1].update(a2);
        this->field_2BC[2].update(a2);
        this->field_2BC[3].update(a2);
        this->field_2BC[4].update(a2);
        this->field_2BC[5].update(a2);
    }

    this->field_220.update(a2);
}

void ai_player_controller::frame_advance(Float a2)
{
    TRACE("ai_player_controller::frame_advance");

    if (this->field_4[0] == nullptr) {
        this->field_4[0] = CAST(this->field_4[0], g_game_ptr->current_game_camera);

        auto *hero_camera = this->field_4[0];
        assert(hero_camera != nullptr);
    }

    auto update = reinterpret_cast<void (__fastcall *)(ai_player_controller *, void *, Float, bool)>(
        get_vfunc(m_vtbl, 0x4));
    update(this, nullptr, a2, false);

    this->field_3E0 = this->convert_left_stick_from_camera_space_to_world_space(false);

    this->field_3EC = this->compute_left_stick_from_camera();
}

float ai_player_controller::get_motion_force()
{
    if constexpr (STANDALONE_SYSTEM) {
        const float vertical = field_2BC[0].field_2D ? 0.0f : field_2BC[0].field_10;
        float horizontal = field_2BC[1].field_2D ? 0.0f : field_2BC[1].field_10;
        auto reversed = reinterpret_cast<bool (__fastcall *)(const ai_player_controller *, void *)>(
            get_vfunc(m_vtbl, 0x48));
        if (reversed(this, nullptr)) {
            horizontal = -horizontal;
        }
        if (std::equal_to<float>{}(vertical, 0.0f) && std::equal_to<float>{}(horizontal, 0.0f)) {
            return 0.0f;
        }
        return std::min(std::sqrt(horizontal * horizontal + vertical * vertical), 1.0f);
    } else {
        float __fastcall (*func)(void *self) = CAST(func, 0x00449880);
        return func(this);
    }
}

void ai_player_controller_patch()
{
    {
        FUNC_ADDRESS(address, &ai_player_controller::frame_advance);
        SET_JUMP(0x00468E80, address);
    }

    {
        FUNC_ADDRESS(address, &ai_player_controller::lock_controls);
        REDIRECT(0x00741839, address);
    }
}
