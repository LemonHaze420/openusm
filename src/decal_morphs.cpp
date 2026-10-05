#include "entity.h"
#include "entity_base_vhandle.h"
#include "variable.h"
#include "wds.h"

#include "decal_morphs.h"

#include "func_wrapper.h"
#include "line_info.h"
#include "oldmath_po.h"
#include <cmath>
#include <functional>

namespace {
struct morph_state {
    vhandle_type<entity> handle;
    float remaining;
    float duration;
    bool active;
    bool locked;
    char padding[2];
};
auto &states = var<morph_state[30]>(0x0095AD50);

bool find_decal_surface(line_info &line, const vector3d &position, bool include_entities)
{
    const auto &filter = include_entities
        ? static_cast<const local_collision::entfilter_base &>(*local_collision::entfilter_accept_all)
        : static_cast<const local_collision::entfilter_base &>(*local_collision::entfilter_reject_all);
    constexpr vector3d diagonals[]{{0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};
    for (const auto &diagonal : diagonals) {
        for (int reverse = 0; reverse != 2; ++reverse) {
            const auto offset = diagonal * (reverse == 0 ? 1.0f : -1.0f);
            line.clear();
            line.field_0 = position + offset;
            line.field_C = position - offset;
            if (line.check_collision(filter, *local_collision::obbfilter_lineseg_test, nullptr)) {
                const auto normal_offset = line.hit_norm * 0.5f;
                line.clear();
                line.field_0 = position + normal_offset;
                line.field_C = position - normal_offset;
                return line.check_collision(filter, *local_collision::obbfilter_lineseg_test, nullptr);
            }
        }
    }
    return false;
}
}

decal_morphs::decal_morphs() {}

bool decal_morphs::create_decal(string_hash name, vector3d position, float lifetime,
    vector3d direction, entity_base *parent)
{
    line_info surface;
    if (!find_decal_surface(surface, position, parent != nullptr))
        return surface.remove_to_collision_check_queue();
    auto *decal = g_world_ptr->ent_mgr.acquire_entity(name, 0x2000);
    if (decal == nullptr)
        return surface.remove_to_collision_check_queue();
    const auto radius = decal->get_visual_radius();
    const auto normal = surface.hit_norm;
    auto hit = surface.hit_pos;
    for (const auto &state : states) {
        if (!state.active)
            continue;
        auto *existing = state.handle.get_volatile_ptr();
        if (existing != nullptr &&
            (existing->get_abs_po().get_position() - hit).length2().value < 2.0f * radius * radius) {
            g_world_ptr->ent_mgr.release_entity(decal);
            return surface.remove_to_collision_check_queue();
        }
    }
    const auto angle = static_cast<float>(std::acos(dot(normal, YVEC)));
    const auto absolute_angle = std::fabs(angle);
    const auto alignment_axis = vector3d::cross(normal, YVEC);
    const auto tangent_x = absolute_angle <= LARGE_EPSILON
        ? XVEC : vector3d::cross(alignment_axis, normal);
    const auto tangent_z = absolute_angle <= LARGE_EPSILON ? ZVEC : alignment_axis;
    const vector3d corners[]{
        hit + tangent_z * radius - tangent_x * radius,
        hit - tangent_z * radius - tangent_x * radius,
        hit + tangent_z * radius + tangent_x * radius,
        hit - tangent_z * radius + tangent_x * radius};
    const auto &filter = parent != nullptr
        ? static_cast<const local_collision::entfilter_base &>(*local_collision::entfilter_accept_all)
        : static_cast<const local_collision::entfilter_base &>(*local_collision::entfilter_reject_all);
    line_info fitting[4];
    const auto fitting_offset = normal * 0.1f;
    for (int corner = 0; corner != 4; ++corner) {
        fitting[corner].field_0 = corners[corner] + fitting_offset;
        fitting[corner].field_C = corners[corner] - fitting_offset;
        if (!fitting[corner].check_collision(filter, *local_collision::obbfilter_lineseg_test, nullptr)) {
            g_world_ptr->ent_mgr.release_entity(decal);
            return surface.remove_to_collision_check_queue();
        }
    }
    po transform = po_identity_matrix;
    hit = hit + normal * LARGE_EPSILON;
    if (absolute_angle > 0.01 &&
        std::equal_to<float>{}(alignment_axis.length2().value, 0.0f))
        transform.set_rot(XVEC, angle);
    else if (absolute_angle > LARGE_EPSILON)
        transform.set_rot(alignment_axis, angle);
    direction.normalize();
    if (direction != ZEROVEC) {
        const auto forward = transform.slow_xform(-ZVEC);
        const auto turn_angle = static_cast<float>(std::acos(dot(direction, forward)));
        if (std::fabs(turn_angle) > LARGE_EPSILON) {
            auto turn_axis = normal;
            const auto normal_dot = std::fabs(dot(direction, normal));
            if (normal_dot < LARGE_EPSILON ||
                !std::equal_to<float>{}(normal_dot, 1.0f)) {
                if (normal_dot >= LARGE_EPSILON) {
                    const auto tangent = vector3d::cross(normal, forward);
                    direction = (tangent * dot(direction, tangent)).normalized();
                }
                turn_axis = vector3d::cross(direction, forward).normalized();
            }
            if (turn_axis.length2().value > 0.0f) {
                po turn = po_identity_matrix;
                turn.set_rot(turn_axis, turn_angle);
                transform = transform.sub_4BAB00(turn);
            }
        }
    }
    if (parent != nullptr) {
        transform.set_position(hit - parent->get_abs_po().get_position());
        decal->set_parent(parent);
    } else {
        transform.set_position(hit);
    }
    decal->set_abs_po(transform);
    auto best_time = bit_cast<float>(0x47C34F80u);
    int chosen = 0;
    for (int index = 0; index != 30; ++index) {
        if (!states[index].active) {
            chosen = index;
            break;
        }
        if (!states[index].locked && states[index].remaining < best_time) {
            chosen = index;
            best_time = states[index].remaining;
        }
    }
    auto &state = states[chosen];
    if (state.active) {
        if (auto *existing = state.handle.get_volatile_ptr())
            g_world_ptr->ent_mgr.release_entity(existing);
        state.active = false;
    }
    state.handle = vhandle_type<entity>{decal->my_handle};
    state.remaining = lifetime;
    state.duration = lifetime;
    state.active = true;
    state.locked = std::equal_to<float>{}(lifetime, -1.0f);
    return surface.remove_to_collision_check_queue();
}

void decal_morphs::frame_advance(Float elapsed)
{

    for (auto &state : states) {
        if (state.locked) {
            continue;
        }
        if (state.active) {
            state.remaining -= elapsed.value;
        }
        auto *entity_ptr = state.handle.get_volatile_ptr();
        if (state.active && state.remaining < 0.0f) {
            if (entity_ptr != nullptr) {
                g_world_ptr->ent_mgr.release_entity(entity_ptr);
            }
            state.active = false;
        } else if (entity_ptr != nullptr) {
            auto color = entity_ptr->get_render_color();
            const float progress = 1.0f - state.remaining / state.duration;
            const float squared = progress * progress;
            color.field_0[3] = static_cast<uint8_t>(
                (1.0f - squared * squared) * 255.0f + 0.5f);
            entity_ptr->set_render_color(color);
        }
    }
}
