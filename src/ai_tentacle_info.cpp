#include "ai_tentacle_info.h"

#include "ai_tentacle_engine.h"
#include "common.h"
#include "func_wrapper.h"
#include "local_collision.h"
#include "slab_allocator.h"
#include <new>
#include "base_ai_core.h"
#include "debug_render.h"
#include "polytube.h"
#include "vtbl.h"
#include "line_info.h"
#include "wds.h"
#include <algorithm>
#include "custom_math.h"
#include "moved_entities.h"

#include <cmath>

VALIDATE_OFFSET(ai_tentacle_info, tween_positions, 0x48);
VALIDATE_SIZE(ai_tentacle_info, 0xD8u);

VALIDATE_SIZE(polytube_render_info, 0x28);

polytube_render_info::polytube_render_info()
    : texture(string_hash{0}, RESOURCE_KEY_TYPE_NONE), blend_mode(-1),
      radius(0.05f), texture_scale(1.0f), num_sides(0), spline_flags(5), field_1C(0)
{
}

ai_tentacle_info::ai_tentacle_info(ai::ai_core *core)
    : field_0(string_hash{0}, static_cast<resource_key_type>(64)),
      my_ai(core), tentacle(nullptr), field_14(0.0f), field_18(0.0f), field_1C(1.0f),
      tween_timer(0.0f), tween_duration(-1.0f), tween_amount(0.0f),
      field_38(1.0f, 0.0f, 0.0f, 0.0f),
      tween_positions{}, base_node(nullptr), end_node(nullptr), nodes{},
      field_60{}, end_pos{}, field_78(1.0f, 0.0f, 0.0f, 0.0f),
      positions{}, node_po_storage{}, field_98(0.0f), field_9C(1.0f),
      field_A0(0.0f), field_A4(0.5f), field_A8(36), field_B8(0), field_BC(0),
      field_CC(-1.0f), field_D0(0), field_D4(-1)
{
    void *memory = sizeof(polytube_render_info) > slab_allocator::get_max_object_size()
        ? ::operator new(sizeof(polytube_render_info))
        : slab_allocator::allocate(sizeof(polytube_render_info), nullptr);
    render_info = ::new (memory) polytube_render_info;
}

namespace {
void destroy_tentacle_engine(ai_tentacle_engine *engine)
{
    using destroy_fn = void (__fastcall *)(ai_tentacle_engine *, void *, unsigned int);
    reinterpret_cast<destroy_fn>(get_vfunc(engine->m_vtbl, 0))(engine, nullptr, 1);
}
}

ai_tentacle_info::~ai_tentacle_info()
{
    if (render_info != nullptr) {
        if (sizeof(polytube_render_info) <= slab_allocator::get_max_object_size())
            slab_allocator::deallocate(render_info, nullptr);
        else
            ::operator delete(render_info);
        render_info = nullptr;
    }
    for (auto *engine = engines._first_element; engine != nullptr; engine = engine->simple_list_vars._sl_next_element) {
        if (engine->field_10 == field_B8) {
            engines.erase(engine);
            destroy_tentacle_engine(engine);
            break;
        }
    }
    if ((field_A8 & 0x100) != 0)
        kill_all_engines();
    const auto free_object = [](polytube_misc_render_object *object) {
        if (sizeof(polytube_misc_render_object) <= slab_allocator::get_max_object_size())
            slab_allocator::deallocate(object, nullptr);
        else
            ::operator delete(object);
    };
    if (tentacle != nullptr) {
        while (auto *object = tentacle->misc_render_objects._first_element) {
            tentacle->remove_misc_render_object(object);
            free_object(object);
        }
        if ((field_A8 & 0x100) == 0)
            g_world_ptr->ent_mgr.remove_entity(tentacle);
        tentacle = nullptr;
    }
    while (auto *object = misc_render_objects._first_element) {
        misc_render_objects.erase(object);
        free_object(object);
    }
    if (auto *object = field_BC.get_volatile_ptr()) {
        g_world_ptr->ent_mgr.remove_entity(static_cast<entity *>(object));
        field_BC = INVALID_HANDLE;
    }
    engines.clear();
    if (!node_po_storage.field_7)
        delete[] node_po_storage.m_data;
    if (!positions.field_7)
        delete[] positions.m_data;
    if (!nodes.field_7)
        delete[] nodes.m_data;
    if (!tween_positions.field_7)
        delete[] tween_positions.m_data;
}

void ai_tentacle_info::kill_all_engines()
{
    while (auto *engine = engines._first_element) {
        engines.erase(engine);
        destroy_tentacle_engine(engine);
    }
}

void ai_tentacle_info::frame_advance(Float time_step)
{
    if (base_node != nullptr)
        field_60 = base_node->get_abs_position();
    else if (tentacle != nullptr && (field_A8 & 0x100) != 0)
        field_60 = tentacle->get_abs_position();
    if (field_1C > 0.0f) {
        if (field_14 < field_18)
            field_14 = std::min(field_14 + time_step.value * field_1C, field_18);
        else if (field_14 > field_18)
            field_14 = std::max(field_14 - time_step.value * field_1C, field_18);
    }
    if (tween_timer <= tween_duration) {
        tween_timer += time_step.value;
        if (tween_timer < tween_duration)
            tween_amount = tween_timer / tween_duration;
        else {
            tween_timer = 0.0f;
            tween_duration = -1.0f;
            tween_amount = 1.0f;
        }
    }
    if (auto *engine = engines._first_element) {
        using advance_fn = bool (__fastcall *)(ai_tentacle_engine *, void *, Float, bool);
        if (reinterpret_cast<advance_fn>(get_vfunc(engine->m_vtbl, 0xC))(engine, nullptr, time_step, false)) {
            engines.erase(engine);
            destroy_tentacle_engine(engine);
        }
    }
    if (tentacle != nullptr && (field_A8 & 0x100) == 0) {
        const auto *owner = my_ai != nullptr ? static_cast<entity *>(my_ai->get_actor(0)) : tentacle;
        tentacle->set_visible(debug_render_get_ival(SKELETONS) == 0 && owner->is_visible(), false);
    }
}

po ai_tentacle_info::get_end_po() const
{
    po v6{identity_matrix};
    this->field_78.to_matrix(v6.m);

    v6[3][0] = this->end_pos[0];
    v6[3][1] = this->end_pos[1];
    v6[3][2] = this->end_pos[2];

    return v6;
}

void ai_tentacle_info::set_position(int index, const vector3d &a1)
{
    assert(index >= 0);
    assert(index < get_num_positions());

    auto &v3 = this->positions.at(index);
    v3 = a1;
}

void ai_tentacle_info::set_code_blend(Float a2, Float a3)
{
    auto v3 = (a2 < 1.0f);
    auto v4 = equal<float>(a2, 1.0f);
    this->field_18 = a2;
    if (v3 || v4) {
        if (a2 < 0.0f) {
            this->field_18 = 0.0;
        }
    } else {
        this->field_18 = 1.0;
    }

    if (a3 >= LARGE_EPSILON) {
        this->field_1C = std::abs(this->field_18 - this->field_14) / a3;
    } else {
        this->field_1C = 0.0;
        this->field_14 = this->field_18;
    }

    if (this->field_18 > EPSILON && this->field_14 >= EPSILON) {
        this->init_code_tween(a3);
    }
}

void ai_tentacle_info::init_code_tween(Float a2)
{
    if (this->field_14 >= EPSILON) {
        if (a2 >= LARGE_EPSILON) {
            assert(this->end_pos.is_valid());

            this->init_positions(true);

            assert(this->end_pos.is_valid());

            assert(tween_positions.size() == positions.size());

            this->tween_positions = this->positions;

            this->field_2C = this->end_pos;

            this->field_38 = this->field_78;
            this->tween_amount = 0.0;
        } else {
            this->tween_duration = -1.0;
            this->tween_amount = 1.0;
        }

        this->tween_timer = 0.0;
        this->tween_duration = a2;
    }
}

void ai_tentacle_info::init_positions(bool blend)
{
    const auto interpolate = [](const vector3d &start, const vector3d &end, float amount) {
        if (amount < EPSILON)
            return start;
        if (amount > 1.0f - EPSILON)
            return end;
        return start + (end - start) * amount;
    };
    if (nodes.size() != 0) {
        const bool code_blend = blend && field_14 >= EPSILON;
        for (int i = 0; i < nodes.size(); ++i) {
            const auto &node_position = nodes[i]->get_abs_position();
            if (code_blend) {
                vector3d code_position = positions[i];
                if (tween_timer < tween_duration)
                    code_position = interpolate(tween_positions[i], code_position, tween_amount);
                positions[i] = interpolate(node_position, code_position, field_14);
            } else {
                positions[i] = node_position;
            }
        }
        if (end_node != nullptr) {
            const po &node_pose = end_node->get_abs_po();
            if (code_blend) {
                vector3d code_position = end_pos;
                quaternion code_rotation = field_78;
                if (tween_timer < tween_duration) {
                    code_position = interpolate(field_2C, code_position, tween_amount);
                    code_rotation = slerp(field_38, code_rotation, tween_amount);
                }
                end_pos = interpolate(end_node->get_abs_position(), code_position, field_14);
                field_78 = slerp(quaternion{node_pose.m}, code_rotation, field_14);
            } else {
                end_pos = end_node->get_abs_position();
                field_78 = quaternion{node_pose.m};
            }
        }
    } else if ((field_A8 & 0x100) != 0 && tentacle != nullptr) {
        const po &tube_pose = tentacle->get_abs_po();
        const int count = tentacle->get_num_control_pts();
        for (int i = 0; i < count; ++i) {
            const vector3d point = tube_pose.slow_xform(tentacle->get_control_pt(i));
            if (i == 0)
                field_60 = point;
            else if (i == count - 1)
                end_pos = point;
            else
                positions[i - 1] = point;
        }
    }
}

vector3d ai_tentacle_info::correct_tentacle_pos(line_info &line, bool &previous_collision,
                                               vector3d &previous_direction, vector3d &previous_normal)
{
    vector3d result = line.field_C;
    if ((field_A8 & 4) == 0 ||
        !line.check_collision(*local_collision::entfilter_entity_no_capsules,
                              *local_collision::obbfilter_lineseg_test, nullptr)) {
        previous_collision = false;
        return result;
    }
    const vector3d contact = line.hit_pos + line.hit_norm * std::max(render_info->radius, 0.1f);
    vector3d direction = line.field_C - line.field_0;
    const float normal_projection = dot(direction, line.hit_norm);
    if (normal_projection <= 0.0f)
        direction -= line.hit_norm * normal_projection;
    vector3d contact_direction = contact - line.field_0;
    const float contact_projection = dot(contact_direction, line.hit_norm);
    if (contact_projection < 0.0f)
        contact_direction -= line.hit_norm * contact_projection;
    const float remaining = std::max(direction.length() - contact_direction.length(), 0.0f);
    direction.set_length(remaining);
    result = contact + direction;
    if (previous_collision && dot(line.hit_norm, previous_normal) > 0.0f) {
        vector3d offset = result - line.field_0;
        const float projection = dot(offset, previous_direction);
        if (projection <= 0.0f) {
            offset -= previous_direction * projection;
            result = line.field_0 + offset;
        }
    }
    previous_collision = true;
    previous_direction = result - line.field_0;
    previous_direction.normalize();
    previous_normal = line.hit_norm;
    return result;
}

int ai_tentacle_info::push_engine(ai_tentacle_engine *eng)
{
    assert(eng != nullptr);

    this->engines.push_front(eng);
    return (eng != nullptr ? eng->field_10 : 0);
}

void ai_tentacle_info::apply_render_info()
{
    if (tentacle == nullptr)
        return;
    if (render_info->texture.m_hash != string_hash{0})
        tentacle->set_material(render_info->texture.m_hash);
    tentacle->set_render_color(color32{static_cast<uint32_t>(render_info->blend_mode)});
    if (std::not_equal_to<float>{}(tentacle->tube_radius, render_info->radius)) {
        tentacle->tube_radius = render_info->radius;
        tentacle->field_78 = 0;
    }
    tentacle->tiles_per_meter = render_info->texture_scale;
    if (tentacle->num_sides != render_info->num_sides) {
        tentacle->num_sides = render_info->num_sides;
        tentacle->field_78 = 0;
    }
    tentacle->field_108 = render_info->field_1C;
    tentacle->the_spline.build(render_info->spline_flags, tentacle->the_spline.field_38);
}

void ai_tentacle_info::create_tentacle(polytube *tube)
{
    if (tentacle != nullptr || ((field_A8 & 0x20) == 0 && tube == nullptr))
        return;
    if (tube != nullptr) {
        tentacle = tube;
        field_A8 |= 0x120;
        const auto count = static_cast<uint16_t>(tube->get_num_control_pts() - 2);
        base_node = nullptr;
        end_node = nullptr;
        const auto resize_positions = [count](mashable_vector<vector3d> &points) {
            if (!points.field_7)
                delete[] points.m_data;
            points.m_size = count;
            points.m_data = new vector3d[count];
            for (int i = 0; i < count; ++i)
                points.m_data[i] = ZEROVEC;
        };
        resize_positions(positions);
        resize_positions(tween_positions);
        init_positions(true);
        set_code_blend(1.0f, 0.0f);
        return;
    }
    field_A8 &= ~0x100;
    void *memory = sizeof(polytube) <= slab_allocator::get_max_object_size()
        ? slab_allocator::allocate(sizeof(polytube), nullptr) : ::operator new(sizeof(polytube));
    tentacle = ::new (memory) polytube(make_unique_entity_id(), 0);
    g_world_ptr->ent_mgr.add_dynamic_instanced_entity(tentacle);
    tentacle->set_force_start(true);
    entity_base *parent = my_ai != nullptr ? static_cast<entity_base *>(my_ai->get_actor(0)) : tentacle;
    if (base_node != nullptr)
        parent = base_node->m_parent != nullptr ? base_node->m_parent : base_node;
    if (parent != tentacle)
        tentacle->set_parent(parent);
    tentacle->set_abs_po(po{identity_matrix});
    const int count = positions.size() + 2;
    tentacle->reserve_control_pts(count);
    for (int i = 0; i < count; ++i)
        tentacle->add_control_pt(ZEROVEC);
    tentacle->set_visible(true, false);
    apply_render_info();
}

void ai_tentacle_info::update_spline()
{
    const auto interpolate = [](const vector3d &a, const vector3d &b, float amount) {
        if (amount < EPSILON)
            return a;
        if (amount > 1.0f - EPSILON)
            return b;
        return a + (b - a) * amount;
    };
    if (tentacle == nullptr || tentacle->is_visible() || debug_render_get_ival(SKELETONS) != 0) {
        const bool store_poses = (field_A8 & 0x80) == 0 && nodes.size() != 0;
        const bool apply_poses = (field_A8 & 0x80) != 0 && nodes.size() != 0;
        const bool render_poses = (field_A8 & 0x40) != 0 && nodes.size() != 0;
        if (base_node != nullptr)
            field_60 = base_node->get_abs_position();
        else if (tentacle != nullptr && (field_A8 & 0x100) != 0)
            field_60 = tentacle->get_abs_position();
        if (field_14 >= EPSILON || (field_A8 & 4) != 0) {
            line_info line;
            line.field_0 = base_node != nullptr ? base_node->get_abs_position() : field_60;
            bool collision = false;
            vector3d previous_direction, previous_normal;
            entity *owner = my_ai != nullptr ? static_cast<entity *>(my_ai->get_actor(0)) : tentacle;
            const bool ragdoll = (owner->field_8 & 0x40000) != 0;
            int pose_index = 0;
            if (nodes.size() != 0) {
                if (store_poses)
                    node_po_storage[pose_index++] = base_node->get_abs_po();
                for (int i = 0; i < nodes.size(); ++i) {
                    if (!ragdoll) {
                        vector3d code_position = positions[i];
                        if (tween_timer < tween_duration)
                            code_position = interpolate(tween_positions[i], code_position, tween_amount);
                        line.field_C = interpolate(nodes[i]->get_abs_position(), code_position, field_14);
                        line.field_0 = correct_tentacle_pos(line, collision, previous_direction, previous_normal);
                        entity_set_abs_position(nodes[i], line.field_0);
                    }
                    if (store_poses)
                        node_po_storage[pose_index++] = nodes[i]->get_abs_po();
                }
            } else {
                const int step = (field_A8 & 2) != 0 ? -1 : 1;
                int index = step < 0 && tentacle != nullptr ? tentacle->get_num_control_pts() - 2 : 1;
                for (int i = 0; i < positions.size(); ++i, index += step) {
                    line.field_C = positions[i];
                    line.field_0 = correct_tentacle_pos(line, collision, previous_direction, previous_normal);
                    if (tentacle != nullptr)
                        tentacle->set_control_pt(index, line.field_0);
                }
            }
            po end_pose = get_end_po();
            if (!ragdoll || end_node == nullptr) {
                if (end_node != nullptr) {
                    vector3d code_position = end_pos;
                    quaternion rotation = field_78;
                    if (tween_timer < tween_duration) {
                        code_position = interpolate(field_2C, code_position, tween_amount);
                        rotation = slerp(field_38, rotation, tween_amount);
                    }
                    rotation = slerp(quaternion{end_node->get_abs_po().m}, rotation, field_14);
                    rotation.to_matrix(end_pose.m);
                    end_pose.set_position(interpolate(end_node->get_abs_position(), code_position, field_14));
                }
                line.field_C = end_pose.get_position();
                end_pose.set_position(ZEROVEC);
                end_pose.sub_48D840();
                line.field_0 = correct_tentacle_pos(line, collision, previous_direction, previous_normal);
                end_pose.set_position(line.field_0);
                if (end_node != nullptr)
                    entity_set_abs_po(end_node, end_pose);
                else
                    end_pos = end_pose.get_position();
            }
            if (store_poses) {
                node_po_storage[pose_index] = end_node != nullptr ? end_node->get_abs_po() : end_pose;
                const auto align_pose = [](po &pose, const po &previous, const vector3d &target) {
                    const vector3d position = pose.get_position();
                    vector3d direction = target - position;
                    if (direction.length2() <= EPSILON) {
                        pose = previous;
                    } else {
                        direction.normalize();
                        const float cosine = dot(direction, previous.get_z_facing());
                        if (cosine < -0.99f) {
                            pose.set_po(-previous.get_z_facing(), previous.get_y_facing(), position);
                        } else if (cosine >= 0.99f) {
                            pose = previous;
                        } else {
                            vector3d axis = vector3d::cross(direction, previous.get_z_facing());
                            axis.normalize();
                            po rotation{identity_matrix};
                            rotation.set_rot(axis, bounded_acos(cosine));
                            pose = previous;
                            pose.set_position(ZEROVEC);
                            pose.set_from_ptr_to_po_world(ptr_to_po{&pose.m, &rotation.m});
                        }
                    }
                    pose.set_position(position);
                };
                for (int i = 0; i < node_po_storage.size(); ++i) {
                    if (i == 0) {
                        align_pose(node_po_storage[i], base_node->get_abs_po(), node_po_storage[i + 1].get_position());
                        if (apply_poses && !ragdoll)
                            entity_set_abs_po(base_node, node_po_storage[i]);
                    } else if (i - 1 < nodes.size()) {
                        align_pose(node_po_storage[i], node_po_storage[i - 1], node_po_storage[i + 1].get_position());
                        if (apply_poses && !ragdoll)
                            entity_set_abs_po(nodes[i - 1], node_po_storage[i]);
                    } else if (i == node_po_storage.size() - 1) {
                        const po &previous = node_po_storage[i - 1];
                        align_pose(node_po_storage[i], previous,
                                   node_po_storage[i].get_position() + previous.get_z_facing());
                        if (apply_poses && !ragdoll && end_node != nullptr)
                            entity_set_abs_po(end_node, end_pose);
                    }
                }
                for (int i = 0; i < node_po_storage.size(); ++i)
                    node_po_storage[i].set_from_ptr_to_po_world(
                        ptr_to_po{&node_po_storage[i].m, &tentacle->get_abs_po().inverse()->m});
            }
            if (tentacle != nullptr) {
                tentacle->field_7B = render_poses;
                tentacle->field_7C = store_poses ? node_po_storage.m_data : nullptr;
            }
        } else if (tentacle != nullptr) {
            tentacle->field_7B = render_poses;
            tentacle->field_7C = nullptr;
        }
        entity_base *owner = my_ai != nullptr ? static_cast<entity_base *>(my_ai->get_actor(0)) : tentacle;
        if (base_node != nullptr)
            owner = base_node->m_parent != nullptr ? base_node->m_parent : base_node;
        const po inverse = *owner->get_abs_po().inverse();
        const int step = (field_A8 & 2) != 0 ? -1 : 1;
        int index = step < 0 && tentacle != nullptr ? tentacle->get_num_control_pts() - 1 : 0;
        vector3d previous = inverse.slow_xform(base_node != nullptr ? base_node->get_abs_position() : field_60);
        if (tentacle != nullptr)
            tentacle->set_control_pt(index, previous);
        index += step;
        field_98 = 0.0f;
        const int count = nodes.size() != 0 ? nodes.size() : positions.size();
        for (int i = 0; i < count; ++i, index += step) {
            const vector3d point = inverse.slow_xform(nodes.size() != 0
                ? nodes[i]->get_abs_position() : tentacle->get_control_pt(index));
            if (tentacle != nullptr)
                tentacle->set_control_pt(index, point);
            field_98 += (point - previous).length();
            previous = point;
        }
        const vector3d endpoint = inverse.slow_xform(end_node != nullptr ? end_node->get_abs_position() : end_pos);
        if (tentacle != nullptr)
            tentacle->set_control_pt(index, endpoint);
        field_98 += (endpoint - previous).length();
        if (tentacle != nullptr) {
            if (field_A0 > EPSILON) {
                float tiles = render_info->texture_scale;
                const float difference = field_98 - field_9C;
                if (std::abs(difference) > EPSILON) {
                    const float amount = std::min(std::abs(difference / field_A0), 1.0f);
                    const float target = difference > 0.0f
                        ? render_info->field_20 : render_info->field_24;
                    tiles += (target - tiles) * amount;
                }
                tentacle->tiles_per_meter = tiles;
            }
            if (tentacle->the_spline.need_rebuild)
                tentacle->the_spline.rebuild_helper();
            moved_entities::add_moved(vhandle_type<entity>{entity_base_vhandle{tentacle->my_handle}});
        }
    }
    if (tentacle != nullptr && std::abs(field_CC) > EPSILON && field_D0 > 0) {
        const float length = tentacle->the_spline.curve_length(-1.0f);
        if (length > EPSILON) {
            int count = field_CC > 0.0f ? static_cast<int>(length / field_CC) : field_D0 + 1;
            const bool capped = count > field_D0;
            count = std::min(count, field_D0);
            while (tentacle->misc_render_objects.m_size < static_cast<uint32_t>(count)) {
                auto *object = misc_render_objects._first_element;
                misc_render_objects.erase(object);
                object->color = color32{static_cast<uint32_t>(field_D4)};
                object->enabled = true;
                tentacle->add_misc_render_object(object);
            }
            while (tentacle->misc_render_objects.m_size > static_cast<uint32_t>(count)) {
                auto *object = tentacle->misc_render_objects._first_element;
                tentacle->remove_misc_render_object(object);
                object->enabled = false;
                misc_render_objects.push_back(object);
            }
            const float interval = capped ? 1.0f / field_D0 : field_CC / length;
            float percent = interval;
            for (auto *object = tentacle->misc_render_objects._first_element; object != nullptr;
                 object = object->simple_list_vars._sl_next_element, percent += interval)
                object->percent = percent;
        }
    }
}

po ai_tentacle_info::get_abs_end_po() const
{
    if (end_node == nullptr)
        return get_end_po();
    vector3d code_position = end_pos;
    quaternion code_rotation = field_78;
    const auto interpolate = [](const vector3d &start, const vector3d &end, float amount) {
        if (amount < EPSILON)
            return start;
        if (amount > 1.0f - EPSILON)
            return end;
        return start + (end - start) * amount;
    };
    if (tween_timer < tween_duration) {
        code_position = interpolate(field_2C, code_position, tween_amount);
        code_rotation = slerp(field_38, code_rotation, tween_amount);
    }
    po result{identity_matrix};
    const quaternion rotation = slerp(quaternion{end_node->get_abs_po().m}, code_rotation, field_14);
    rotation.to_matrix(result.m);
    result.set_position(interpolate(end_node->get_abs_position(), code_position, field_14));
    return result;
}

void ai_tentacle_info::create_line(const vector3d &end, const vector3d *facing)
{
    const vector3d start = base_node != nullptr ? base_node->get_abs_position() : field_60;
    vector3d direction = end - start;
    const float length = direction.length();
    direction *= 1.0f / length;
    const float interval = length / (positions.size() + 1);
    for (int i = 0; i < positions.size(); ++i)
        positions[i] = start + direction * ((i + 1) * interval);
    end_pos = end;
    vector3d forward = facing != nullptr ? *facing
        : end_pos - (positions.size() != 0 ? positions[positions.size() - 1] : start);
    const float forward_length = forward.length();
    if (forward_length >= EPSILON) {
        forward *= 1.0f / forward_length;
    } else {
        forward = get_end_po().get_z_facing();
        if (forward.length() < EPSILON)
            forward = ZVEC;
    }
    po end_pose = get_abs_end_po();
    vector3d up = end_pose.get_y_facing();
    if (is_colinear(up, forward, 0.01f))
        up = end_pose.get_x_facing();
    end_pose.set_po(up, forward, end_pos);
    end_pose.set_po(end_pose.get_y_facing(), end_pose.get_z_facing(), end_pose.get_position());
    field_78 = quaternion{end_pose.m};
    end_pos = end_pose.get_position();
}
