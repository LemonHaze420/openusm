#include "dangler.h"

#include "common.h"
#include "slab_allocator.h"
#include "local_collision.h"
#include "polytube.h"

#include <algorithm>
#include <cfloat>

#include <new>

VALIDATE_SIZE(dangler, 0x24);
VALIDATE_SIZE(dangler::dangler_particle, 0x3C);

dangler::dangler()
    : field_4(0), damping(0.0f), length(0.0f), gravity(0.0f, -10.0f, 0.0f), constraint_iterations(4),
      collision_enabled(false), field_21(true)
{
    using particle_vector = _std::vector<dangler_particle>;
    void *memory = sizeof(particle_vector) <= slab_allocator::get_max_object_size()
                       ? slab_allocator::allocate(sizeof(particle_vector), nullptr)
                       : ::operator new(sizeof(particle_vector));
    particles = new (memory) particle_vector;
}

dangler::~dangler()
{
    if (particles != nullptr) {
        using particle_vector = _std::vector<dangler_particle>;
        particles->~particle_vector();
        if (sizeof(particle_vector) <= slab_allocator::get_max_object_size()) {
            slab_allocator::deallocate(particles, nullptr);
        } else {
            ::operator delete(particles);
        }
    }
}

void *dangler::operator new(size_t size)
{
    return size <= slab_allocator::get_max_object_size() ? slab_allocator::allocate(size, nullptr)
                                                         : ::operator new(size);
}

void dangler::operator delete(void *memory)
{
    if (sizeof(dangler) <= slab_allocator::get_max_object_size()) {
        slab_allocator::deallocate(memory, nullptr);
    } else {
        ::operator delete(memory);
    }
}


int dangler::init_dangle(const vector3d &start, const vector3d &end, const vector3d *intermediate,
                         int intermediate_count, float total_length, const vector3d &velocity, char first_flags)
{
    field_4 = intermediate_count + 2;
    length = total_length > 0.0f ? total_length : 0.0f;
    particles->clear();
    particles->reserve(field_4);
    for (int i = 0; i < field_4; ++i) {
        dangler_particle particle{};
        particle.field_0 = i == 0 ? first_flags : 0;
        particle.position = i == 0 ? start : i <= intermediate_count ? intermediate[i - 1] : end;
        particle.velocity = velocity * (static_cast<float>(i) / (field_4 - 1));
        particle.pinned = i == 0;
        if (i != 0) {
            particle.rest_length = total_length > 0.0f ? total_length / (field_4 - 1)
                                                       : (particle.position - particles->back().position).length();
            if (total_length <= 0.0f)
                length += particle.rest_length;
        }
        particles->push_back(particle);
    }
    damping = 0.0f;
    return field_4;
}

void dangler::init_line(const vector3d &start, const vector3d &end, int segments, const vector3d &velocity,
                        char first_flags)
{
    field_4 = segments + 1;
    length = (end - start).length();
    particles->clear();
    particles->reserve(field_4);
    for (int i = 0; i < field_4; ++i) {
        const float fraction = static_cast<float>(i) / segments;
        dangler_particle particle{};
        particle.field_0 = i == 0 ? first_flags : 0;
        particle.position = start * (1.0f - fraction) + end * fraction;
        particle.velocity = velocity * fraction;
        particle.rest_length = i == 0 ? 0.0f : length / segments;
        particle.pinned = i == 0;
        particles->push_back(particle);
    }
    damping = 0.0f;
}

void dangler::init_polytube(polytube *tube, const vector3d &velocity, char first_flags)
{
    field_4 = tube->get_num_control_pts();
    length = 0.0f;
    particles->clear();
    particles->reserve(field_4);
    for (int i = 0; i < field_4; ++i) {
        const int control_index = field_4 - i - 1;
        dangler_particle particle{};
        particle.field_0 = i == 0 ? first_flags : 0;
        particle.position = tube->get_control_pt(control_index);
        particle.velocity = velocity * (static_cast<float>(control_index) / (field_4 - 1));
        particle.pinned = i == 0;
        if (i != 0) {
            particle.rest_length = (particle.position - particles->back().position).length();
            length += particle.rest_length;
        }
        particles->push_back(particle);
    }
    damping = 0.0f;
}


void dangler::frame_advance(Float dt)
{
    for (auto &particle : *particles) {
        if (particle.pinned)
            continue;
        if (particle.preserve_previous_position)
            particle.preserve_previous_position = false;
        else
            particle.previous_position = particle.position;
        particle.velocity += (particle.acceleration + gravity) * dt;
        particle.position += particle.velocity * dt;
    }
    for (int iteration = 0; iteration < constraint_iterations; ++iteration) {
        for (size_t i = 1; i < particles->size(); ++i) {
            auto &previous = (*particles)[i - 1];
            auto &current = (*particles)[i];
            vector3d direction = previous.position - current.position;
            const float distance = direction.length();
            const float half_error = (distance - current.rest_length) * 0.5f;
            if (distance > 0.00001f)
                direction *= 1.0f / distance;
            const vector3d correction = direction * half_error;
            if (!previous.pinned)
                previous.position -= correction * (current.pinned ? 2.0f : 1.0f);
            if (!current.pinned)
                current.position += correction * (previous.pinned ? 2.0f : 1.0f);
        }
    }
    const float inverse_dt = 1.0f / dt;
    for (auto &particle : *particles) {
        particle.velocity = (particle.position - particle.previous_position) * inverse_dt;
        if (damping > EPSILON) {
            const float speed = particle.velocity.length();
            if (speed > EPSILON)
                particle.velocity -= particle.velocity * (std::min(dt * damping, speed) / speed);
        }
    }
    if (collision_enabled) {
        vector3d minimum(FLT_MAX);
        vector3d maximum(-FLT_MAX);
        for (const auto &particle : *particles) {
            minimum = vector3d::min(minimum, particle.position);
            maximum = vector3d::max(maximum, particle.position);
        }
        const vector3d center = (minimum + maximum) * 0.5f;
        const float radius = std::min((center - minimum).length(), 25.0f);
        auto *primitives = local_collision::query_sphere(
            center, radius, *local_collision::entfilter_accept_all, *local_collision::obbfilter_sphere_test, {});
        for (auto &particle : *particles) {
            vector3d point, normal;
            if (!particle.pinned && local_collision::get_closest_sphere_intersection(
                                        primitives, particle.position, 0.1f, &point, &normal, nullptr)) {
                const vector3d normal_velocity = normal * dot(normal, particle.velocity);
                const vector3d tangent_velocity = particle.velocity - normal_velocity;
                particle.position = point + normal * 0.1f;
                particle.velocity = tangent_velocity * 0.9f - normal_velocity * 0.2f;
            }
        }
        local_collision::destroy_primitive_list(&primitives);
    }
}


void dangler::build_polytube(polytube *tube)
{
    for (size_t i = 0; i < particles->size(); ++i)
        tube->set_abs_control_pt(static_cast<int>(i), (*particles)[i].position);
    tube->rebuild_helper();
}
