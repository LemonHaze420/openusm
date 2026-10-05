#include "physics_system_internal.h"
#include "physics_system.h"
#include "rigid_body.h"
#include "rbc_ragdoll.h"
#include "rbc_def_contact.h"
#include "rbc_def_distance.h"
#include "utility.h"
#include "common.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace {
template <class T>
T &field(void *object, unsigned offset)
{
    return *reinterpret_cast<T *>(static_cast<char *>(object) + offset);
}
physics_vec4 add(physics_vec4 a, physics_vec4 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
physics_vec4 sub(physics_vec4 a, physics_vec4 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
physics_vec4 mul(physics_vec4 a, float s)
{
    return {a.x * s, a.y * s, a.z * s, a.w * s};
}
float dot3(physics_vec4 a, physics_vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
physics_vec4 cross(physics_vec4 a, physics_vec4 b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0};
}
physics_vec4 basis(void *matrix, physics_vec4 a)
{
    return add(add(mul(field<physics_vec4>(matrix, 0), a.x), mul(field<physics_vec4>(matrix, 16), a.y)),
               mul(field<physics_vec4>(matrix, 32), a.z));
}
physics_vec4 world_vector(rigid_body *body, physics_vec4 a)
{
    return body->field_144 & 0x10 ? a : basis(body, a);
}
physics_vec4 world_point(rigid_body *body, physics_vec4 a)
{
    return body->field_144 & 0x10 ? a : add(field<physics_vec4>(body, 48), a);
}
physics_vec4 inverse_inertia(rigid_body *body, physics_vec4 a)
{
    return body->field_144 & 2 ? mul(a, body->field_B0) : basis(reinterpret_cast<char *>(body) + 128, a);
}
physics_vec4 body_velocity(rigid_body *body, physics_vec4 anchor, unsigned offset = 208)
{
    return body->field_144 & 0x10
               ? physics_vec4{}
               : add(field<physics_vec4>(body, offset), cross(field<physics_vec4>(body, offset + 16), anchor));
}
physics_pulse_body *pulse_body(rigid_body *body)
{
    return reinterpret_cast<physics_pulse_body *>(body->field_150);
}
template <class T>
T *allocate_pulse(physics_system *world, unsigned list)
{
    auto &current = field<char *>(world, 0x298);
    auto address = (reinterpret_cast<std::uintptr_t>(current) + 3) & ~std::uintptr_t(3);
    auto *result = reinterpret_cast<T *>(address);
    if (reinterpret_cast<char *>(result + 1) > field<char *>(world, 0x294))
        return nullptr;
    current = reinterpret_cast<char *>(result + 1);
    auto *&head = field<T *>(world, list);
    auto *&tail = field<T *>(world, list + 4);
    if (tail)
        tail->next = result;
    else
        head = result;
    tail = result;
    ++field<int>(world, list + 8);
    result->next = nullptr;
    return result;
}
struct contact_tree_node {
    contact_tree_node *left, *right;
    int balance;
    rigid_body *small_body, *large;
    rigid_body_constraint_contact *contact;
};
struct contact_tree {
    contact_tree_node *data;
    int capacity, used;
    contact_tree_node *root;
    void *key;
    contact_tree_node *found;
};
VALIDATE_SIZE(contact_tree_node, 0x18);
VALIDATE_SIZE(contact_tree, 0x18);
VALIDATE_OFFSET(contact_tree, used, 8);
VALIDATE_OFFSET(contact_tree, root, 12);
void rotate_right(contact_tree_node *&root)
{
    auto *child = root->left;
    root->left = child->right;
    child->right = root;
    root->balance += 1 - std::min(child->balance, 0);
    child->balance += 1 + std::max(root->balance, 0);
    root = child;
}
void rotate_left(contact_tree_node *&root)
{
    auto *child = root->right;
    root->right = child->left;
    child->left = root;
    root->balance -= 1 + std::max(child->balance, 0);
    child->balance -= 1 - std::min(root->balance, 0);
    root = child;
}
int compare_pair(rigid_body *small_body, rigid_body *large, contact_tree_node *node)
{
    auto a = reinterpret_cast<std::uintptr_t>(small_body), b = reinterpret_cast<std::uintptr_t>(node->small_body);
    if (a != b)
        return a < b ? -1 : 1;
    a = reinterpret_cast<std::uintptr_t>(large);
    b = reinterpret_cast<std::uintptr_t>(node->large);
    return a == b ? 0 : a < b ? -1 : 1;
}
int insert_tree(contact_tree &tree, contact_tree_node *&root, rigid_body *small_body, rigid_body *large)
{
    if (!root) {
        root = &tree.data[tree.used++];
        *root = {nullptr, nullptr, 0, small_body, large, nullptr};
        tree.found = root;
        return 1;
    }
    int order = compare_pair(small_body, large, root);
    if (!order) {
        tree.found = root;
        return 0;
    }
    if (order < 0) {
        if (!insert_tree(tree, root->left, small_body, large))
            return 0;
        if (--root->balance < -1) {
            if (root->left->balance == 1)
                rotate_left(root->left);
            rotate_right(root);
        }
    } else {
        if (!insert_tree(tree, root->right, small_body, large))
            return 0;
        if (++root->balance > 1) {
            if (root->right->balance == -1)
                rotate_right(root->right);
            rotate_left(root);
        }
    }
    return root->balance;
}
void merge_partitions(rigid_body *first, rigid_body *second)
{
    if (!first || !second)
        return;
    auto *a = reinterpret_cast<rb_partition_node *>(first->field_154);
    auto *b = reinterpret_cast<rb_partition_node *>(second->field_154);
    if (!a || !b || a == b)
        return;
    if (a->body_count < b->body_count)
        std::swap(a, b);
    a->tail->head = b;
    a->tail = b->tail;
    a->body_count += b->body_count;
    for (auto *node = b; node; node = node->head)
        node->body->field_154 = reinterpret_cast<int>(a);
    b->tail = nullptr;
    b->body_count = 0;
}
struct pool_view {
    void *data;
    physics_constraint_link **allocated;
    int capacity, stride, count;
};
pool_view &pool(physics_system *world, unsigned offset)
{
    return field<pool_view>(world, offset);
}
constexpr unsigned pool_offsets[] = {0x1EC, 0x218, 0x22C, 0x240, 0x254, 0x268, 0x27C};
}  // namespace

VALIDATE_SIZE(rb_partition_node, 0x38);
VALIDATE_SIZE(physics_pulse_body, 0x2C);
VALIDATE_SIZE(physics_pulse_scalar, 0x94);
VALIDATE_SIZE(physics_pulse_angular, 0x8C);
VALIDATE_SIZE(physics_pulse_point, 0x130);
VALIDATE_OFFSET(physics_pulse_scalar, body1, 0x88);
VALIDATE_OFFSET(physics_pulse_scalar, cache, 0x90);
VALIDATE_OFFSET(physics_pulse_angular, body1, 0x80);
VALIDATE_OFFSET(physics_pulse_point, body1, 0x124);

rigid_body_constraint_contact **physics_contact_insert(physics_system *world, rigid_body *first, rigid_body *second,
                                                       rigid_body_constraint_contact *contact)
{
    auto &tree = field<contact_tree>(world, 0x1A0);
    if (tree.used == tree.capacity)
        return nullptr;
    if (reinterpret_cast<std::uintptr_t>(first) > reinterpret_cast<std::uintptr_t>(second))
        std::swap(first, second);
    insert_tree(tree, tree.root, first, second);
    auto **slot = &tree.found->contact;
    if (contact)
        *slot = contact;
    return slot;
}
rigid_body_constraint_contact *physics_find_contact(physics_system *world, rigid_body *first, rigid_body *second)
{
    if (reinterpret_cast<std::uintptr_t>(first) > reinterpret_cast<std::uintptr_t>(second))
        std::swap(first, second);
    auto *node = field<contact_tree>(world, 0x1A0).root;
    while (node) {
        int order = compare_pair(first, second, node);
        if (!order)
            return node->contact;
        node = order < 0 ? node->left : node->right;
    }
    return nullptr;
}
rb_partition_node *physics_build_constraint_partitions(physics_system *world)
{
    auto clear = [](rigid_body *body) {
        body->field_154 = body->field_158 = body->field_15C = 0;
    };
    clear(&world->field_34);
    auto &users = pool(world, 0x1C4);
    for (int i = 0; i < users.count; ++i)
        clear(reinterpret_cast<rigid_body *>(users.allocated[i]));
    auto &bodies = pool(world, 0x1D8);
    auto *nodes = field<rb_partition_node *>(world, 0x1B8);
    field<int>(world, 0x1C0) = 0;
    for (int i = 0; i < bodies.count; ++i) {
        auto *body = reinterpret_cast<rigid_body *>(bodies.allocated[i]);
        clear(body);
        auto *node = &nodes[field<int>(world, 0x1C0)++];
        node->body = body;
        node->head = nullptr;
        node->tail = node;
        node->body_count = 1;
        body->field_154 = reinterpret_cast<int>(node);
    }
    for (unsigned type = 0; type < 7; ++type) {
        auto &list = pool(world, pool_offsets[type]);
        for (int i = 0; i < list.count; ++i) {
            auto *constraint = list.allocated[i];
            int count = 1;
            if (!type) {
                auto *contact = reinterpret_cast<rigid_body_constraint_contact *>(constraint);
                count = std::max(field<int>(reinterpret_cast<void *>(contact->field_24), 8),
                                 field<int>(reinterpret_cast<void *>(contact->field_28), 8));
            }
            for (auto *body : {constraint->b1, constraint->b2})
                if (body) {
                    if (!type)
                        body->field_15C += count;
                    else
                        ++body->field_158;
                    if (type == 5 && (field<unsigned>(constraint, 160) & 1))
                        ++body->field_15C;
                }
            merge_partitions(constraint->b1, constraint->b2);
        }
    }
    rb_partition_node *head = nullptr;
    for (int i = 0; i < field<int>(world, 0x1C0); ++i)
        if (nodes[i].body_count > 0) {
            nodes[i].next = head;
            head = &nodes[i];
            for (auto *&constraint : nodes[i].constraints)
                constraint = nullptr;
        }
    for (unsigned type = 0; type < 7; ++type) {
        auto &list = pool(world, pool_offsets[type]);
        for (int i = 0; i < list.count; ++i) {
            auto *constraint = list.allocated[i];
            auto *node = constraint->b1 ? reinterpret_cast<rb_partition_node *>(constraint->b1->field_154) : nullptr;
            if (!node)
                node = reinterpret_cast<rb_partition_node *>(constraint->b2->field_154);
            constraint->next = node->constraints[type];
            node->constraints[type] = constraint;
        }
    }
    return head;
}

namespace {
physics_vec4 relative_velocity(physics_pulse_scalar *pulse, unsigned offset = 208)
{
    auto a = body_velocity(pulse->body1->body, pulse->anchor1, offset);
    auto b = pulse->body2 ? body_velocity(pulse->body2->body, pulse->anchor2, offset) : pulse->angular2;
    return sub(a, b);
}
physics_vec4 relative_change(physics_pulse_scalar *pulse)
{
    auto a = add(pulse->body1->velocity, cross(pulse->body1->angular_velocity, pulse->anchor1));
    if (pulse->body2)
        a = sub(a, add(pulse->body2->velocity, cross(pulse->body2->angular_velocity, pulse->anchor2)));
    return a;
}
physics_vec4 response_direction(physics_pulse_scalar *pulse)
{
    auto result = add(mul(pulse->direction, pulse->body1->inverse_mass), cross(pulse->angular1, pulse->anchor1));
    if (pulse->body2)
        result =
            add(result, add(mul(pulse->direction, pulse->body2->inverse_mass), cross(pulse->angular2, pulse->anchor2)));
    return result;
}
float scalar_position(physics_pulse_scalar *pulse)
{
    auto a = add(field<physics_vec4>(pulse->body1->body, 48), pulse->anchor1);
    auto b = pulse->body2 ? add(field<physics_vec4>(pulse->body2->body, 48), pulse->anchor2) : pulse->anchor2;
    return dot3(sub(a, b), pulse->direction);
}
void apply(physics_pulse_scalar *pulse, float impulse)
{
    auto *a = pulse->body1;
    a->velocity = add(a->velocity, mul(pulse->direction, a->inverse_mass * impulse));
    a->angular_velocity = add(a->angular_velocity, mul(pulse->angular1, impulse));
    if (auto *b = pulse->body2) {
        b->velocity = sub(b->velocity, mul(pulse->direction, b->inverse_mass * impulse));
        b->angular_velocity = sub(b->angular_velocity, mul(pulse->angular2, impulse));
    }
}
void set_scalar(physics_pulse_scalar *pulse, rigid_body *first, physics_vec4 anchor1, rigid_body *second,
                physics_vec4 anchor2, physics_vec4 direction, pulse_sum_cache *cache, physics_vec4 offset = {})
{
    if (pulse_body(first)) {
        pulse->body1 = pulse_body(first);
        pulse->anchor1 = anchor1;
        pulse->body2 = pulse_body(second);
        pulse->anchor2 = pulse->body2 ? anchor2 : world_point(second, anchor2);
        if (!pulse->body2)
            pulse->angular2 = body_velocity(second, anchor2);
    } else {
        pulse->body1 = pulse_body(second);
        pulse->anchor1 = anchor2;
        pulse->body2 = nullptr;
        pulse->angular2 = body_velocity(first, anchor1);
        pulse->anchor2 = world_point(first, anchor1);
        direction = mul(direction, -1);
    }
    pulse->direction = direction;
    pulse->cache = cache;
    pulse->flags = 0;
    pulse->angular1 = inverse_inertia(pulse->body1->body, cross(add(pulse->anchor1, offset), direction));
    pulse->denominator = dot3(cross(pulse->angular1, pulse->anchor1), direction) + pulse->body1->inverse_mass;
    if (pulse->body2) {
        auto moment = cross(pulse->anchor2, direction);
        pulse->angular2 = inverse_inertia(pulse->body2->body, moment);
        pulse->denominator += dot3(moment, pulse->angular2) + pulse->body2->inverse_mass;
    }
}
physics_pulse_scalar *create_scalar(physics_system *world, rigid_body *first, physics_vec4 anchor1, rigid_body *second,
                                    physics_vec4 anchor2, physics_vec4 direction, pulse_sum_cache *cache,
                                    physics_vec4 offset = {})
{
    auto *pulse = allocate_pulse<physics_pulse_scalar>(world, 0x2A8);
    set_scalar(pulse, first, anchor1, second, anchor2, direction, cache, offset);
    return pulse;
}
void setup_unilateral(physics_pulse_scalar *pulse, float elapsed, float minimum, float maximum_speed)
{
    elapsed = std::max(elapsed, 0.0041666669f);
    float target = -(scalar_position(pulse) + minimum - 0.02f) / elapsed;
    if (target < 0)
        target *= 0.3f;
    target = std::max(target, -maximum_speed);
    pulse->position_target = std::min(target, 0.0f);
    pulse->velocity_target = std::max(target, 0.0f);
    pulse->softness = 0;
}
void setup_contact_velocity(physics_pulse_scalar *pulse, float bounce, float maximum_bounce, float elapsed)
{
    float position = scalar_position(pulse);
    float target = -(position - 0.02f) / std::max(elapsed, 0.0041666669f);
    if (target < 0)
        target *= 0.3f;
    target = std::max(target, -5.0f);
    pulse->position_target = std::min(target, 0.0f);
    pulse->velocity_target = std::max(target, 0.0f);
    pulse->softness = 0;
    if (bounce <= 0.00001f || maximum_bounce <= 0.00001f || position < 0)
        return;
    float speed = std::max(dot3(relative_velocity(pulse, 240), pulse->direction),
                           dot3(relative_velocity(pulse), pulse->direction));
    if (speed <= pulse->body1->body->field_134)
        return;
    float rebound = std::min(speed * bounce, maximum_bounce);
    if (pulse->position_target > -0.00001f)
        pulse->velocity_target -= rebound;
    else if (-rebound < pulse->position_target) {
        pulse->position_target = 0;
        pulse->velocity_target = -rebound;
    }
}
void apply(physics_pulse_angular *pulse, float impulse)
{
    pulse->body1->angular_velocity = add(pulse->body1->angular_velocity, mul(pulse->angular1, impulse));
    if (pulse->body2)
        pulse->body2->angular_velocity = sub(pulse->body2->angular_velocity, mul(pulse->angular2, impulse));
}
physics_pulse_angular *create_angular(physics_system *world, rigid_body *first, physics_vec4 anchor1,
                                      rigid_body *second, physics_vec4 anchor2, physics_vec4 direction,
                                      pulse_sum_cache *cache)
{
    auto *pulse = allocate_pulse<physics_pulse_angular>(world, 0x2C0);
    if (pulse_body(first)) {
        pulse->body1 = pulse_body(first);
        pulse->body2 = pulse_body(second);
        pulse->anchor1 = anchor1;
        pulse->anchor2 = anchor2;
        if (!pulse->body2)
            pulse->angular2 = second->field_144 & 0x10 ? physics_vec4{} : field<physics_vec4>(second, 224);
    } else {
        pulse->body1 = pulse_body(second);
        pulse->body2 = nullptr;
        pulse->anchor1 = anchor2;
        pulse->anchor2 = anchor1;
        direction = mul(direction, -1);
        pulse->angular2 = first->field_144 & 0x10 ? physics_vec4{} : field<physics_vec4>(first, 224);
    }
    pulse->direction = direction;
    pulse->cache = cache;
    pulse->flags = 0;
    pulse->angular1 = inverse_inertia(pulse->body1->body, direction);
    pulse->denominator = dot3(pulse->angular1, direction);
    if (pulse->body2) {
        pulse->angular2 = inverse_inertia(pulse->body2->body, direction);
        pulse->denominator += dot3(pulse->angular2, direction);
    }
    return pulse;
}
float angular_position(physics_pulse_angular *pulse)
{
    return -dot3(cross(pulse->anchor1, pulse->anchor2), pulse->direction);
}
float angular_velocity(physics_pulse_angular *pulse)
{
    return dot3(sub(field<physics_vec4>(pulse->body1->body, 224),
                    pulse->body2 ? field<physics_vec4>(pulse->body2->body, 224) : pulse->angular2),
                pulse->direction);
}
float angular_change(physics_pulse_angular *pulse)
{
    return dot3(pulse->body2 ? sub(pulse->body1->angular_velocity, pulse->body2->angular_velocity)
                             : pulse->body1->angular_velocity,
                pulse->direction);
}
void setup_unilateral(physics_pulse_angular *pulse, float elapsed, float maximum_speed)
{
    float target = -(angular_position(pulse) - 0.02f) / std::max(elapsed, 0.0041666669f);
    if (target < 0)
        target *= 0.3f;
    target = std::max(target, -maximum_speed);
    pulse->position_target = std::min(target, 0.0f);
    pulse->velocity_target = std::max(target, 0.0f);
    pulse->softness = 0;
}
physics_vec4 point_change(physics_pulse_point *pulse)
{
    auto a = add(pulse->body1->velocity, cross(pulse->body1->angular_velocity, pulse->anchor1));
    if (pulse->body2)
        a = sub(a, add(pulse->body2->velocity, cross(pulse->body2->angular_velocity, pulse->anchor2)));
    return a;
}
physics_vec4 point_velocity(physics_pulse_point *pulse)
{
    return sub(body_velocity(pulse->body1->body, pulse->anchor1),
               pulse->body2 ? body_velocity(pulse->body2->body, pulse->anchor2) : pulse->angular[1]);
}
void apply(physics_pulse_point *pulse, physics_vec4 impulse)
{
    auto *a = pulse->body1;
    a->velocity = add(a->velocity, mul(impulse, a->inverse_mass));
    a->angular_velocity = add(a->angular_velocity,
                              add(add(mul(pulse->angular[0], impulse.x), mul(pulse->angular[2], impulse.y)),
                                  mul(pulse->angular[4], impulse.z)));
    if (auto *b = pulse->body2) {
        b->velocity = sub(b->velocity, mul(impulse, b->inverse_mass));
        b->angular_velocity = sub(b->angular_velocity,
                                  add(add(mul(pulse->angular[1], impulse.x), mul(pulse->angular[3], impulse.y)),
                                      mul(pulse->angular[5], impulse.z)));
    }
}
void create_point(physics_system *world, rigid_body *first, physics_vec4 anchor1, rigid_body *second,
                  physics_vec4 anchor2, pulse_sum_cache *cache, float elapsed)
{
    auto *pulse = allocate_pulse<physics_pulse_point>(world, 0x2B4);
    pulse->body1 = pulse_body(first);
    pulse->body2 = pulse_body(second);
    pulse->anchor1 = anchor1;
    pulse->anchor2 = pulse->body2 ? anchor2 : world_point(second, anchor2);
    if (!pulse->body2)
        pulse->angular[1] = body_velocity(second, anchor2);
    pulse->cache = cache;
    physics_vec4 columns[3];
    constexpr physics_vec4 axes[] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
    for (int i = 0; i < 3; ++i) {
        pulse->angular[2 * i] = inverse_inertia(first, cross(anchor1, axes[i]));
        columns[i] = add(mul(axes[i], pulse->body1->inverse_mass), cross(pulse->angular[2 * i], anchor1));
        if (pulse->body2) {
            pulse->angular[2 * i + 1] = inverse_inertia(second, cross(anchor2, axes[i]));
            columns[i] = add(columns[i],
                             add(mul(axes[i], pulse->body2->inverse_mass), cross(pulse->angular[2 * i + 1], anchor2)));
        }
    }
    pulse->inverse[0] = cross(columns[1], columns[2]);
    pulse->inverse[1] = cross(columns[2], columns[0]);
    pulse->inverse[2] = cross(columns[0], columns[1]);
    float determinant = dot3(columns[2], pulse->inverse[2]);
    for (auto &row : pulse->inverse)
        row = mul(row, 1.0f / determinant);
    pulse->diagonal = {columns[0].x, columns[1].y, columns[2].z, 0};
    auto position = sub(add(field<physics_vec4>(first, 48), anchor1),
                        pulse->body2 ? add(field<physics_vec4>(second, 48), anchor2) : pulse->anchor2);
    pulse->position_target = mul(position, -0.5f / std::max(elapsed, 0.0041666669f));
    pulse->velocity_target = {};
}
void align_axes(physics_system *world, rigid_body *first, physics_vec4 axis1, rigid_body *second, physics_vec4 axis2,
                physics_vec4 tangent, physics_vec4 bitangent, pulse_sum_cache *cache, float elapsed)
{
    for (int i = 0; i < 2; ++i) {
        auto *pulse = create_angular(world, first, axis1, second, axis2, i ? bitangent : tangent, cache + i);
        pulse->lower = -10000000;
        pulse->upper = 10000000;
        pulse->softness = 0;
        pulse->velocity_target = 0;
        pulse->position_target = -0.5f * angular_position(pulse) / std::max(elapsed, 0.0041666669f);
    }
}
physics_vec4 rotate_between(physics_vec4 first, physics_vec4 second, physics_vec4 value)
{
    auto axis = cross(first, second);
    float length2 = dot3(axis, axis);
    if (length2 < 0.00001f)
        return value;
    float length = std::sqrt(length2);
    axis = mul(axis, 1.0f / length);
    float cosine = dot3(first, second), inverse = 1.0f / std::sqrt(cosine * cosine + length2);
    float sine = length * inverse;
    cosine *= inverse;
    return add(add(mul(value, cosine), mul(cross(axis, value), sine)), mul(axis, dot3(axis, value) * (1 - cosine)));
}
void setup_hinge(rigid_body_constraint_ragdoll *joint, physics_system *world, physics_vec4 reference, physics_vec4 axis,
                 float elapsed)
{
    auto *cache = reinterpret_cast<pulse_sum_cache *>(joint->caches);
    if (joint->flags & 0x10) {
        auto *pulse = create_angular(world,
                                     joint->b1,
                                     reference,
                                     joint->b2,
                                     world_vector(joint->b2, field<physics_vec4>(joint, 0xD0)),
                                     mul(axis, -1),
                                     cache + 6);
        pulse->lower = -10000000;
        pulse->upper = 0;
        setup_unilateral(pulse, elapsed, 5);
    }
    if (joint->flags & 0x20) {
        auto *pulse = create_angular(world,
                                     joint->b1,
                                     reference,
                                     joint->b2,
                                     world_vector(joint->b2, field<physics_vec4>(joint, 0xE0)),
                                     axis,
                                     cache + 7);
        pulse->lower = -10000000;
        pulse->upper = 0;
        setup_unilateral(pulse, elapsed, 5);
    }
}
void prepare_ragdoll(rigid_body_constraint_ragdoll *joint)
{
    if (joint->flags & 0x80) {
        joint->flags |= 0x30;
        for (int i = 0; i < joint->limit_count; ++i)
            joint->flags |= 1u << i;
        return;
    }

    auto predicted = [](rigid_body *body, physics_vec4 axis) {
        return body->field_144 & 0x10 ? axis : basis(reinterpret_cast<char *>(body) + 64, axis);
    };
    auto minimum = predicted(joint->b2, field<physics_vec4>(joint, 0xD0));
    auto maximum = predicted(joint->b2, field<physics_vec4>(joint, 0xE0));
    auto axis = predicted(joint->b2, field<physics_vec4>(joint, 0x90));
    auto reference = predicted(joint->b1, field<physics_vec4>(joint, 0xC0));
    if (joint->flags & 8) {
        for (int i = 0; i < joint->limit_count; ++i) {
            auto limit = predicted(joint->b1, field<physics_vec4>(&joint->limit_data[i], 0));
            if (dot3(axis, limit) > joint->limit_data[i].cosine + 0.05f)
                joint->flags &= ~(1u << i);
            else
                joint->flags |= 1u << i;
        }
        reference = rotate_between(predicted(joint->b1, field<physics_vec4>(joint, 0x80)), axis, reference);
    }
    if (dot3(cross(reference, minimum), axis) < -0.05f)
        joint->flags &= ~0x10u;
    else
        joint->flags |= 0x10;
    if (dot3(cross(reference, maximum), axis) > 0.05f)
        joint->flags &= ~0x20u;
    else
        joint->flags |= 0x20;
}
}  // namespace

void physics_setup_ragdoll(rigid_body_constraint_ragdoll *joint, physics_system *world, float elapsed)
{
    auto *cache = reinterpret_cast<pulse_sum_cache *>(joint->caches);
    create_point(world,
                 joint->b1,
                 world_vector(joint->b1, field<physics_vec4>(joint, 0xC)),
                 joint->b2,
                 world_vector(joint->b2, field<physics_vec4>(joint, 0x1C)),
                 cache,
                 elapsed);
    auto axis1 = world_vector(joint->b1, field<physics_vec4>(joint, 0x80));
    auto axis2 = world_vector(joint->b2, field<physics_vec4>(joint, 0x90));
    if (joint->flags & 0x40) {
        auto direction = sub(field<physics_vec4>(joint->b2, 224), field<physics_vec4>(joint->b1, 224));
        float length = std::sqrt(dot3(direction, direction));
        direction = length <= EPSILON ? axis2 : mul(direction, 1.0f / length);
        auto *pulse = create_angular(world, joint->b1, {}, joint->b2, {}, direction, cache + 3);
        pulse->velocity_target = pulse->position_target = 0;
        if (joint->flags & 0x200) {
            pulse->lower = -10000000;
            pulse->upper = 10000000;
            pulse->softness = 1.0f / (elapsed * joint->damping);
            pulse->denominator += pulse->softness;
        } else {
            pulse->softness = 0;
            pulse->lower = -elapsed * joint->damping;
            pulse->upper = elapsed * joint->damping;
        }
    }
    if (joint->flags & 4) {
        align_axes(world,
                   joint->b1,
                   axis1,
                   joint->b2,
                   axis2,
                   world_vector(joint->b1, field<physics_vec4>(joint, 0xA0)),
                   world_vector(joint->b1, field<physics_vec4>(joint, 0xB0)),
                   cache + 4,
                   elapsed);
        if (joint->flags & 0x30)
            setup_hinge(joint, world, world_vector(joint->b1, field<physics_vec4>(joint, 0xC0)), axis2, elapsed);
    }
    if (joint->flags & 8) {
        if (!(joint->flags & 0x100))
            for (int i = 0; i < joint->limit_count; ++i)
                if (joint->flags & (1u << i)) {
                    auto &limit = joint->limit_data[i];
                    auto axis = world_vector(joint->b1, field<physics_vec4>(&limit, 0));
                    auto tangent = sub(axis2, mul(axis, dot3(axis2, axis)));
                    float length = std::sqrt(dot3(tangent, tangent));
                    if (length >= EPSILON) {
                        auto boundary = add(mul(axis, limit.cosine), mul(tangent, limit.sine / length));
                        auto direction = mul(cross(boundary, axis), 1.0f / limit.sine);
                        auto *pulse =
                            create_angular(world, joint->b1, boundary, joint->b2, axis2, direction, cache + 8 + i);
                        pulse->lower = -10000000;
                        pulse->upper = 0;
                        setup_unilateral(pulse, elapsed, 5);
                    }
                }
        if (joint->flags & 0x30)
            setup_hinge(joint,
                        world,
                        rotate_between(axis1, axis2, world_vector(joint->b1, field<physics_vec4>(joint, 0xC0))),
                        axis2,
                        elapsed);
    }
}

namespace {
struct contact_point {
    contact_point *next;
    physics_vec4 normal, point1, point2, velocity;
    float friction, bounce, maximum_bounce;
    bool velocity_override;
    char padding[3];
    pulse_sum_cache cache[3];
};
struct contact_list {
    contact_point *head, *tail;
    int count;
};
VALIDATE_SIZE(contact_point, 0x6C);
VALIDATE_OFFSET(contact_point, cache, 0x54);
float contact_distance(contact_point *first, contact_point *second)
{
    auto n = sub(first->normal, second->normal), a = sub(first->point1, second->point1),
         b = sub(first->point2, second->point2);
    return dot3(n, n) + dot3(a, a) + dot3(b, b);
}
}  // namespace

void physics_add_contact(rigid_body_constraint_contact *contact, rigid_body *first, rigid_body *,
                         const phys_vector3d &point1, const phys_vector3d &point2, const phys_vector3d &normal,
                         float friction, float bounce, float maximum_bounce, bool)
{
    auto *array = g_physics_system->field_30;
    if (array->count >= array->capacity)
        return;
    auto *point = reinterpret_cast<contact_point *>(&array->points[array->count++]);
    for (auto &cache : point->cache)
        cache.field_0 = -1;
    auto &list = field<contact_list>(reinterpret_cast<void *>(contact->field_28), 0);
    if (list.tail)
        list.tail->next = point;
    else
        list.head = point;
    list.tail = point;
    ++list.count;
    point->next = nullptr;
    point->velocity_override = false;
    auto vec = [](const phys_vector3d &v) {
        return physics_vec4{v.field_0[0], v.field_0[1], v.field_0[2], 0};
    };
    if (contact->b1 == first) {
        point->point1 = vec(point1);
        point->point2 = vec(point2);
        point->normal = vec(normal);
    } else {
        point->point1 = vec(point2);
        point->point2 = vec(point1);
        point->normal = mul(vec(normal), -1);
    }
    point->friction = friction;
    point->bounce = bounce;
    point->maximum_bounce = maximum_bounce;
}
void physics_setup_contact(rigid_body_constraint_contact *contact, physics_system *world, float elapsed)
{
    auto &old = field<contact_list>(reinterpret_cast<void *>(contact->field_24), 0);
    auto &current = field<contact_list>(reinterpret_cast<void *>(contact->field_28), 0);
    for (auto *point = current.head; point; point = point->next) {
        contact_point *nearest = nullptr;
        float best = 10000000;
        for (auto *candidate = old.head; candidate; candidate = candidate->next) {
            float distance = contact_distance(candidate, point);
            if (!nearest || distance < best) {
                nearest = candidate;
                best = distance;
            }
        }
        if (nearest)
            std::memcpy(point->cache, nearest->cache, sizeof(point->cache));
        auto anchor1 = world_vector(contact->b1, point->point1), anchor2 = world_vector(contact->b2, point->point2);
        auto *normal = create_scalar(world, contact->b1, anchor1, contact->b2, anchor2, point->normal, point->cache);
        if (point->velocity_override)
            normal->angular2 = point->velocity;
        normal->lower = -10000000;
        normal->upper = 0;
        setup_contact_velocity(normal, point->bounce, point->maximum_bounce, elapsed);

        if (point->friction < 0.00001f)
            return;
        auto direction = relative_velocity(normal);
        direction = sub(direction, mul(point->normal, dot3(direction, point->normal)));
        float length = std::sqrt(dot3(direction, direction));
        if (length < EPSILON) {
            direction = response_direction(normal);
            direction = sub(direction, mul(point->normal, dot3(direction, point->normal)));
            length = std::sqrt(dot3(direction, direction));
        }
        if (length > EPSILON) {
            auto *friction = create_scalar(
                world, contact->b1, anchor1, contact->b2, anchor2, mul(direction, 1.0f / length), point->cache + 1);
            if (point->velocity_override)
                friction->angular2 = point->velocity;
            friction->position_target = friction->velocity_target = friction->softness = 0;
            friction->friction = point->friction;
            friction->flags |= 1;
            friction->normal = normal;
            friction->lower = friction->upper = 0;
        }
    }
    old = {};
}

void physics_setup_distance(rigid_body_constraint_distance *joint, physics_system *world, float elapsed)
{
    if (!(joint->field_40 & 1))
        return;
    auto anchor1 = world_vector(joint->b1, field<physics_vec4>(joint, 0xC));
    auto anchor2 = world_vector(joint->b2, field<physics_vec4>(joint, 0x1C));
    auto point1 = world_point(joint->b1, anchor1), point2 = world_point(joint->b2, anchor2);
    auto direction = sub(point2, point1);
    float length2 = dot3(direction, direction);
    if (length2 < 0.0000010000001111620804f)
        return;
    direction = mul(direction, 1.0f / std::sqrt(length2));
    auto *maximum = create_scalar(world, joint->b1, anchor1, joint->b2, anchor2, mul(direction, -1), joint->field_44);
    maximum->lower = -10000000;
    maximum->upper = 0;
    setup_unilateral(maximum, elapsed, -joint->m_max_distance, 50);
    maximum->velocity_target += joint->field_38;
    if (joint->m_min_distance > LARGE_EPSILON) {
        auto *minimum = create_scalar(world, joint->b1, anchor1, joint->b2, anchor2, direction, joint->field_44 + 1);
        minimum->lower = -10000000;
        minimum->upper = 0;
        setup_unilateral(minimum, elapsed, joint->m_min_distance, 50);
    }
    float damping = joint->field_3C;
    if (damping <= 0.00001f || length2 < (joint->m_max_distance - 0.1f) * (joint->m_max_distance - 0.1f) ||
        length2 > (joint->m_max_distance + 0.1f) * (joint->m_max_distance + 0.1f))
        return;
    auto tangent = relative_velocity(maximum);
    tangent = sub(tangent, mul(direction, dot3(direction, tangent)));
    float length = std::sqrt(dot3(tangent, tangent));
    if (length < EPSILON) {
        tangent = response_direction(maximum);
        tangent = sub(tangent, mul(direction, dot3(direction, tangent)));
        length = std::sqrt(dot3(tangent, tangent));
    }
    if (length <= EPSILON)
        return;
    auto damping_anchor1 = anchor1, damping_anchor2 = anchor2;
    if (joint->field_40 & 2)
        damping_anchor1 = joint->b1->field_144 & 0x10 ? point2 : sub(point2, field<physics_vec4>(joint->b1, 48));
    else
        damping_anchor2 = joint->b2->field_144 & 0x10 ? point1 : sub(point1, field<physics_vec4>(joint->b2, 48));
    auto *pulse = create_scalar(world,
                                joint->b1,
                                damping_anchor1,
                                joint->b2,
                                damping_anchor2,
                                mul(tangent, 1.0f / length),
                                joint->field_44 + 2);
    pulse->velocity_target = pulse->position_target = 0;
    pulse->softness = 1.0f / (elapsed * damping);
    pulse->denominator += pulse->softness;
    pulse->lower = -10000000;
    pulse->upper = 10000000;
}

namespace {
struct wheel_pulse {
    wheel_pulse *next;
    physics_pulse_scalar normal;
    physics_pulse_scalar *side, *forward;
};
VALIDATE_SIZE(wheel_pulse, 0xA0);
VALIDATE_OFFSET(wheel_pulse, side, 0x98);
float clamp_scalar(physics_pulse_scalar *pulse, float value)
{
    if (pulse->flags & 1) {
        pulse->upper = std::fabs(pulse->normal->impulse) * pulse->friction;
        pulse->lower = -pulse->upper;
    }
    if (value < pulse->lower) {
        pulse->flags |= 2;
        return pulse->lower;
    }
    if (value > pulse->upper) {
        pulse->flags |= 2;
        return pulse->upper;
    }
    pulse->flags &= ~2u;
    return value;
}
void constrain_wheel(wheel_pulse *wheel)
{
    auto *side = wheel->side, *forward = wheel->forward;
    float a = side->friction * wheel->normal.impulse, b = forward->friction * wheel->normal.impulse;
    float product = a * a * b * b,
          ellipse = side->impulse * side->impulse * b * b + forward->impulse * forward->impulse * a * a;
    if (product <= ellipse || product < 0.00001f) {
        forward->flags |= 4;
        float scale = ellipse <= 0.00001f ? 0 : std::sqrt(product / ellipse);
        float side_impulse = side->impulse * scale, forward_impulse = forward->impulse * scale;
        apply(side, side_impulse - side->impulse);
        apply(forward, forward_impulse - forward->impulse);
        side->impulse = side_impulse;
        forward->impulse = forward_impulse;
    } else
        forward->flags &= ~4u;
}
bool scalar_accelerates(physics_pulse_scalar *pulse)
{
    float margin = 0.2f / (pulse->denominator - pulse->softness);
    return margin + pulse->lower < pulse->impulse && pulse->impulse < pulse->upper - margin;
}
void acceleration_scalar(physics_pulse_scalar *pulse, float &squares, float &products, bool eligible)
{
    if (eligible) {
        pulse->flags |= 8;
        float delta = pulse->residual - pulse->previous_residual;
        squares += delta * delta;
        products += delta * pulse->previous_residual;
    } else
        pulse->flags &= ~8u;
}
bool wheel_accelerates(wheel_pulse *wheel)
{
    if (!scalar_accelerates(wheel->side) || !scalar_accelerates(wheel->forward))
        return false;
    float effective = (wheel->normal.denominator - wheel->normal.softness) * wheel->normal.impulse;
    float a = -effective * wheel->side->friction - 0.2f, b = -effective * wheel->forward->friction - 0.2f;
    if (a <= 0 || b <= 0)
        return false;
    float x = (wheel->side->denominator - wheel->side->softness) * wheel->side->impulse;
    float y = (wheel->forward->denominator - wheel->forward->softness) * wheel->forward->impulse;
    return a * a * y * y + b * b * x * x < b * b * a * a;
}
float update_scalar(physics_pulse_scalar *pulse)
{
    pulse->previous_residual = pulse->residual;
    pulse->previous_impulse = pulse->impulse;
    pulse->residual =
        dot3(relative_change(pulse), pulse->direction) + pulse->softness * pulse->impulse - pulse->velocity_target;
    pulse->impulse = clamp_scalar(pulse, pulse->impulse - pulse->residual / pulse->denominator);
    apply(pulse, pulse->impulse - pulse->previous_impulse);
    float error = (pulse->impulse - pulse->previous_impulse) * pulse->denominator;
    return error * error;
}
physics_vec4 point_inverse(physics_pulse_point *pulse, physics_vec4 value)
{
    return add(add(mul(pulse->inverse[0], value.x), mul(pulse->inverse[1], value.y)), mul(pulse->inverse[2], value.z));
}
void solve_iterative(physics_system *world, int iterations, int acceleration_start, float tolerance)
{
    int accelerated = 0, total = 0;
    float error = 100, squares = 0, products = 0;
    while ((total <= iterations && error > tolerance) || accelerated < 1) {
        ++accelerated;
        ++total;
        error = 0;
        if (accelerated > acceleration_start)
            squares = products = 0;
        for (auto *pulse = field<physics_pulse_scalar *>(world, 0x2A8); pulse; pulse = pulse->next) {
            error = std::max(error, update_scalar(pulse));
            if (accelerated > acceleration_start)
                acceleration_scalar(pulse, squares, products, scalar_accelerates(pulse));
        }
        for (auto *pulse = field<physics_pulse_point *>(world, 0x2B4); pulse; pulse = pulse->next) {
            pulse->previous_residual = pulse->residual;
            pulse->previous_impulse = pulse->impulse;
            pulse->residual = sub(point_change(pulse), pulse->velocity_target);
            pulse->impulse = sub(pulse->impulse, point_inverse(pulse, pulse->residual));
            auto delta = sub(pulse->impulse, pulse->previous_impulse);
            apply(pulse, delta);
            error = std::max({error,
                              delta.x * delta.x * pulse->diagonal.x * pulse->diagonal.x,
                              delta.y * delta.y * pulse->diagonal.y * pulse->diagonal.y,
                              delta.z * delta.z * pulse->diagonal.z * pulse->diagonal.z});
            if (accelerated > acceleration_start) {
                auto residual_delta = sub(pulse->residual, pulse->previous_residual);
                squares += dot3(residual_delta, residual_delta);
                products += dot3(residual_delta, pulse->previous_residual);
            }
        }
        for (auto *pulse = field<physics_pulse_angular *>(world, 0x2C0); pulse; pulse = pulse->next) {
            pulse->previous_residual = pulse->residual;
            pulse->previous_impulse = pulse->impulse;
            pulse->residual = angular_change(pulse) + pulse->softness * pulse->impulse - pulse->velocity_target;
            pulse->impulse =
                std::clamp(pulse->impulse - pulse->residual / pulse->denominator, pulse->lower, pulse->upper);
            apply(pulse, pulse->impulse - pulse->previous_impulse);
            float delta = (pulse->impulse - pulse->previous_impulse) * pulse->denominator;
            error = std::max(error, delta * delta);
            if (accelerated > acceleration_start) {
                float margin = 0.2f / (pulse->denominator - pulse->softness);
                if (margin + pulse->lower < pulse->impulse && pulse->impulse < pulse->upper - margin) {
                    pulse->flags |= 1;
                    float residual_delta = pulse->residual - pulse->previous_residual;
                    products += residual_delta * pulse->previous_residual;
                    squares += residual_delta * residual_delta;
                } else
                    pulse->flags &= ~1u;
            }
        }
        for (auto *wheel = field<wheel_pulse *>(world, 0x2CC); wheel; wheel = wheel->next) {
            error = std::max(error, update_scalar(&wheel->normal));
            if (wheel->side) {
                float side_error = update_scalar(wheel->side);
                if (!wheel->forward)
                    error = std::max(error, side_error);
                else {
                    update_scalar(wheel->forward);
                    constrain_wheel(wheel);
                    for (auto *pulse : {wheel->side, wheel->forward}) {
                        float delta = (pulse->impulse - pulse->previous_impulse) * pulse->denominator;
                        error = std::max(error, delta * delta);
                    }
                }
            }
            if (accelerated > acceleration_start) {
                acceleration_scalar(&wheel->normal, squares, products, scalar_accelerates(&wheel->normal));
                if (wheel->side) {
                    bool eligible = wheel->forward ? wheel_accelerates(wheel) : scalar_accelerates(wheel->side);
                    acceleration_scalar(wheel->side, squares, products, eligible);
                    if (wheel->forward)
                        acceleration_scalar(wheel->forward, squares, products, eligible);
                }
            }
        }
        if (accelerated <= acceleration_start || squares < 0.0000001f)
            continue;
        float scale = -(products / squares) - 1;
        if (scale < 1)
            continue;
        for (auto *body = field<physics_pulse_body *>(world, 0x29C); body; body = body->next) {
            body->velocity = {};
            body->angular_velocity = {};
        }
        auto accelerate_scalar = [scale](physics_pulse_scalar *pulse) {
            if (pulse->flags & 8)
                pulse->impulse = (pulse->impulse - pulse->previous_impulse) * scale + pulse->previous_impulse;
            pulse->impulse = clamp_scalar(pulse, pulse->impulse);
            apply(pulse, pulse->impulse);
        };
        for (auto *pulse = field<physics_pulse_scalar *>(world, 0x2A8); pulse; pulse = pulse->next)
            accelerate_scalar(pulse);
        for (auto *pulse = field<physics_pulse_point *>(world, 0x2B4); pulse; pulse = pulse->next) {
            pulse->impulse = add(pulse->previous_impulse, mul(sub(pulse->impulse, pulse->previous_impulse), scale));
            apply(pulse, pulse->impulse);
        }
        for (auto *pulse = field<physics_pulse_angular *>(world, 0x2C0); pulse; pulse = pulse->next) {
            if (pulse->flags & 1)
                pulse->impulse = (pulse->impulse - pulse->previous_impulse) * scale + pulse->previous_impulse;
            pulse->impulse = std::clamp(pulse->impulse, pulse->lower, pulse->upper);
            apply(pulse, pulse->impulse);
        }
        for (auto *wheel = field<wheel_pulse *>(world, 0x2CC); wheel; wheel = wheel->next) {
            accelerate_scalar(&wheel->normal);
            if (wheel->side) {
                accelerate_scalar(wheel->side);
                if (wheel->forward) {
                    accelerate_scalar(wheel->forward);
                    constrain_wheel(wheel);
                }
            }
        }
        accelerated = 0;
    }
}
void solve_pulses(physics_system *world, int visit, int next_visit, float elapsed, int steps)
{
    for (auto *body = field<physics_pulse_body *>(world, 0x29C); body; body = body->next) {
        body->body->advance_forces(elapsed);
        body->velocity = {};
        body->angular_velocity = {};
    }
    auto warm_scalar = [visit, elapsed](physics_pulse_scalar *pulse) {
        pulse->velocity_target -= dot3(relative_velocity(pulse), pulse->direction);
        if (pulse->cache->field_0 != visit)
            pulse->cache->field_4 = 0;
        pulse->impulse = elapsed * field<float>(pulse->cache, 4);
        pulse->impulse = clamp_scalar(pulse, pulse->impulse);
        apply(pulse, pulse->impulse);
    };
    for (auto *pulse = field<physics_pulse_scalar *>(world, 0x2A8); pulse; pulse = pulse->next)
        warm_scalar(pulse);
    for (auto *pulse = field<physics_pulse_point *>(world, 0x2B4); pulse; pulse = pulse->next) {
        pulse->velocity_target = sub(pulse->velocity_target, point_velocity(pulse));
        pulse->impulse = {};
        for (int i = 0; i < 3; ++i) {
            if (pulse->cache[i].field_0 != visit)
                pulse->cache[i].field_4 = 0;
            (&pulse->impulse.x)[i] = elapsed * field<float>(pulse->cache + i, 4);
        }
        apply(pulse, pulse->impulse);
    }
    for (auto *pulse = field<physics_pulse_angular *>(world, 0x2C0); pulse; pulse = pulse->next) {
        pulse->velocity_target -= angular_velocity(pulse);
        if (pulse->cache->field_0 != visit)
            pulse->cache->field_4 = 0;
        pulse->impulse = std::clamp(elapsed * field<float>(pulse->cache, 4), pulse->lower, pulse->upper);
        apply(pulse, pulse->impulse);
    }
    for (auto *wheel = field<wheel_pulse *>(world, 0x2CC); wheel; wheel = wheel->next) {
        warm_scalar(&wheel->normal);
        if (wheel->side) {
            warm_scalar(wheel->side);
            if (wheel->forward) {
                warm_scalar(wheel->forward);
                constrain_wheel(wheel);
            }
        }
    }
    solve_iterative(world, std::max(world->field_14 / steps, 1), std::max(world->field_18 / steps, 2), world->field_1C);
    auto cache_scalar = [next_visit, elapsed](physics_pulse_scalar *pulse) {
        pulse->cache->field_0 = next_visit;
        field<float>(pulse->cache, 4) = pulse->impulse / elapsed;
        pulse->velocity_target += pulse->position_target;
    };
    for (auto *pulse = field<physics_pulse_scalar *>(world, 0x2A8); pulse; pulse = pulse->next)
        cache_scalar(pulse);
    for (auto *pulse = field<physics_pulse_point *>(world, 0x2B4); pulse; pulse = pulse->next) {
        for (int i = 0; i < 3; ++i) {
            pulse->cache[i].field_0 = next_visit;
            field<float>(pulse->cache + i, 4) = (&pulse->impulse.x)[i] / elapsed;
        }
        pulse->velocity_target = add(pulse->velocity_target, pulse->position_target);
    }
    for (auto *pulse = field<physics_pulse_angular *>(world, 0x2C0); pulse; pulse = pulse->next) {
        pulse->cache->field_0 = next_visit;
        field<float>(pulse->cache, 4) = pulse->impulse / elapsed;
        pulse->velocity_target += pulse->position_target;
    }
    for (auto *wheel = field<wheel_pulse *>(world, 0x2CC); wheel; wheel = wheel->next) {
        cache_scalar(&wheel->normal);
        if (wheel->side) {
            cache_scalar(wheel->side);
            if (wheel->forward)
                cache_scalar(wheel->forward);
        }
    }
    solve_iterative(world, std::max(world->field_20 / steps, 1), std::max(world->field_24 / steps, 2), world->field_28);
    bool asleep = true;
    for (auto *body = field<physics_pulse_body *>(world, 0x29C); body; body = body->next) {
        auto *rigid = body->body;
        field<physics_vec4>(rigid, 208) = add(field<physics_vec4>(rigid, 208), body->velocity);
        field<physics_vec4>(rigid, 224) = add(field<physics_vec4>(rigid, 224), body->angular_velocity);
        rigid->integrate(elapsed);
        rigid->update_sleep(elapsed);
        asleep = asleep && (rigid->field_144 & 4);
    }
    for (auto *body = field<physics_pulse_body *>(world, 0x29C); body; body = body->next) {
        if (asleep)
            body->body->field_144 |= 8;
        else
            body->body->field_144 &= ~8u;
    }
}
}  // namespace

namespace {
physics_vec4 predicted_vector(rigid_body *body, physics_vec4 value)
{
    return body->field_144 & 0x10 ? value : basis(reinterpret_cast<char *>(body) + 64, value);
}
void prepare_hinge(physics_constraint_link *joint)
{
    auto reference = predicted_vector(joint->b1, field<physics_vec4>(joint, 0x70));
    auto minimum = predicted_vector(joint->b2, field<physics_vec4>(joint, 0x80));
    auto maximum = predicted_vector(joint->b2, field<physics_vec4>(joint, 0x90));
    auto axis = predicted_vector(joint->b2, field<physics_vec4>(joint, 0x40));
    auto &flags = field<unsigned>(joint, 0xC);
    if (dot3(cross(reference, minimum), axis) < -0.05f)
        flags &= ~1u;
    else
        flags |= 1;
    if (dot3(cross(reference, maximum), axis) > 0.05f)
        flags &= ~2u;
    else
        flags |= 2;
}
void setup_point_constraint(physics_constraint_link *joint, physics_system *world, float elapsed)
{
    create_point(world,
                 joint->b1,
                 world_vector(joint->b1, field<physics_vec4>(joint, 0xC)),
                 joint->b2,
                 world_vector(joint->b2, field<physics_vec4>(joint, 0x1C)),
                 &field<pulse_sum_cache>(joint, 0x2C),
                 elapsed);
}
void setup_hinge_constraint(physics_constraint_link *joint, physics_system *world, float elapsed)
{
    create_point(world,
                 joint->b1,
                 world_vector(joint->b1, field<physics_vec4>(joint, 0x10)),
                 joint->b2,
                 world_vector(joint->b2, field<physics_vec4>(joint, 0x20)),
                 &field<pulse_sum_cache>(joint, 0xA4),
                 elapsed);
    auto axis1 = world_vector(joint->b1, field<physics_vec4>(joint, 0x30));
    auto axis2 = world_vector(joint->b2, field<physics_vec4>(joint, 0x40));
    align_axes(world,
               joint->b1,
               axis1,
               joint->b2,
               axis2,
               world_vector(joint->b1, field<physics_vec4>(joint, 0x50)),
               world_vector(joint->b1, field<physics_vec4>(joint, 0x60)),
               &field<pulse_sum_cache>(joint, 0xC4),
               elapsed);
    float damping = field<float>(joint, 0xA0);
    if (damping > 0.00001f) {
        auto *pulse = create_angular(world, joint->b1, {}, joint->b2, {}, axis2, &field<pulse_sum_cache>(joint, 0xBC));
        pulse->velocity_target = pulse->position_target = pulse->softness = 0;
        pulse->lower = -elapsed * damping;
        pulse->upper = elapsed * damping;
    }
    if (field<unsigned>(joint, 0xC) & 3) {
        auto reference = world_vector(joint->b1, field<physics_vec4>(joint, 0x70));
        for (int i = 0; i < 2; ++i)
            if (field<unsigned>(joint, 0xC) & (1u << i)) {
                auto *pulse = create_angular(world,
                                             joint->b1,
                                             reference,
                                             joint->b2,
                                             world_vector(joint->b2, field<physics_vec4>(joint, 0x80 + 16 * i)),
                                             mul(axis2, i ? 1 : -1),
                                             &field<pulse_sum_cache>(joint, 0xD4 + 8 * i));
                pulse->lower = -10000000;
                pulse->upper = 0;
                setup_unilateral(pulse, elapsed, 5);
            }
    }
}
void setup_actuator(physics_constraint_link *joint, physics_system *world, float elapsed)
{
    for (unsigned offset : {0xACu, 0xB4u, 0xBCu})
        field<float>(joint, offset) *= 0.9f;
    if (!field<bool>(joint, 0xA4))
        return;
    physics_vec4 target[3];
    for (int i = 0; i < 3; ++i)
        target[i] = basis(joint->b1, field<physics_vec4>(joint, 0xC + 16 * i));
    auto velocity = basis(joint->b1, field<physics_vec4>(joint, 0x8C));
    float limit = field<float>(joint, 0xA0) * field<float>(joint, 0x9C) * elapsed;
    for (int i = 0; i < 3; ++i) {
        auto *pulse = create_angular(world,
                                     joint->b1,
                                     target[i],
                                     joint->b2,
                                     field<physics_vec4>(joint->b2, 16 * i),
                                     target[(i + 1) % 3],
                                     &field<pulse_sum_cache>(joint, 0xA8 + 8 * i));
        pulse->lower = -limit;
        pulse->upper = limit;
        pulse->position_target = pulse->softness = 0;
        pulse->velocity_target = -angular_position(pulse) / elapsed - dot3(target[(i + 1) % 3], velocity);
    }
}
void advance_actuator(physics_constraint_link *joint, float elapsed)
{
    auto velocity = field<physics_vec4>(joint, 0x8C);
    float speed = std::sqrt(dot3(velocity, velocity));
    if (speed < 0.00001f)
        return;
    auto axis = mul(velocity, 1.0f / speed);
    float sine = std::sin(speed * elapsed), cosine = std::cos(speed * elapsed);
    physics_vec4 target[3];
    for (int i = 0; i < 3; ++i) {
        auto value = field<physics_vec4>(joint, 0xC + 16 * i);
        target[i] =
            add(add(mul(value, cosine), mul(cross(axis, value), sine)), mul(axis, dot3(axis, value) * (1 - cosine)));
    }
    std::memcpy(reinterpret_cast<char *>(joint) + 0xC, target, sizeof(target));
}
void prepare_wheel(physics_constraint_link *joint)
{
    auto &flags = field<unsigned>(joint, 0xA0);
    if (!(flags & 1))
        return;
    auto suspension = predicted_vector(joint->b1, field<physics_vec4>(joint, 0x3C));
    auto anchor1 =
        add(predicted_vector(joint->b1, field<physics_vec4>(joint, 0x2C)), mul(suspension, field<float>(joint, 0x5C)));
    auto anchor2 = predicted_vector(joint->b2, field<physics_vec4>(joint, 0xC));
    auto point1 = joint->b1->field_144 & 0x10 ? anchor1 : add(field<physics_vec4>(joint->b1, 112), anchor1);
    auto point2 = joint->b2->field_144 & 0x10 ? anchor2 : add(field<physics_vec4>(joint->b2, 112), anchor2);
    if (dot3(sub(point1, point2), suspension) < field<float>(joint, 0x70))
        flags &= ~2u;
    else
        flags |= 2;
}
physics_pulse_scalar *allocate_detached_scalar(physics_system *world)
{
    auto &current = world->field_298;
    auto address = (reinterpret_cast<std::uintptr_t>(current) + 3) & ~std::uintptr_t(3);
    auto *result = reinterpret_cast<physics_pulse_scalar *>(address);
    if (reinterpret_cast<char *>(result + 1) > world->field_294)
        return nullptr;
    current = reinterpret_cast<char *>(result + 1);
    return result;
}
void setup_wheel(physics_constraint_link *joint, physics_system *world, float elapsed)
{
    auto &flags = field<unsigned>(joint, 0xA0);
    flags &= ~4u;
    field<float>(joint, 0x8C) = 0;
    for (unsigned offset : {0xC4u, 0xC8u, 0xCCu})
        field<physics_pulse_scalar *>(joint, offset) = nullptr;
    if (!(flags & 1))
        return;
    auto suspension = world_vector(joint->b1, field<physics_vec4>(joint, 0x3C));
    float radius = field<float>(joint, 0x5C);
    auto anchor1 = add(world_vector(joint->b1, field<physics_vec4>(joint, 0x2C)), mul(suspension, radius));
    auto anchor2 = world_vector(joint->b2, field<physics_vec4>(joint, 0xC));
    auto displacement = sub(world_point(joint->b1, anchor1), world_point(joint->b2, anchor2));
    auto normal = mul(world_vector(joint->b2, field<physics_vec4>(joint, 0x1C)), -1);
    if (flags & 2) {
        auto *pulse = create_scalar(world,
                                    joint->b1,
                                    sub(anchor1, mul(suspension, field<float>(joint, 0x70))),
                                    joint->b2,
                                    anchor2,
                                    normal,
                                    &field<pulse_sum_cache>(joint, 0xA4));
        pulse->lower = -10000000;
        pulse->upper = 0;
        setup_unilateral(pulse, elapsed, 0, 5);
    }
    auto projected = mul(normal, dot3(displacement, normal));
    anchor1 = sub(anchor1, projected);
    anchor2 = sub(anchor2, projected);
    auto offset = mul(suspension, -field<float>(joint, 0x74));
    auto *wheel = allocate_pulse<wheel_pulse>(world, 0x2CC);
    wheel->side = wheel->forward = nullptr;
    field<physics_pulse_scalar *>(joint, 0xC4) = &wheel->normal;
    set_scalar(
        &wheel->normal, joint->b1, anchor1, joint->b2, anchor2, normal, &field<pulse_sum_cache>(joint, 0xAC), offset);
    auto *pulse = &wheel->normal;
    pulse->lower = -10000000;
    pulse->upper = 0;
    float spring = elapsed * field<float>(joint, 0x68);
    float softness = 1.0f / (elapsed * field<float>(joint, 0x6C) + spring * elapsed);
    pulse->softness = softness;
    pulse->position_target = 0;
    pulse->velocity_target = -scalar_position(pulse) * spring * softness;
    pulse->denominator += softness;
    auto side = world_vector(joint->b1, field<physics_vec4>(joint, 0x4C));
    side = sub(side, mul(normal, dot3(side, normal)));
    float length = std::sqrt(dot3(side, side));
    if (length <= 0.001f)
        return;
    side = mul(side, 1.0f / length);
    wheel->side = allocate_detached_scalar(world);
    field<physics_pulse_scalar *>(joint, 0xC8) = wheel->side;
    set_scalar(wheel->side, joint->b1, anchor1, joint->b2, anchor2, side, &field<pulse_sum_cache>(joint, 0xB4), offset);
    wheel->side->velocity_target = wheel->side->position_target = wheel->side->softness = 0;
    auto forward = mul(cross(side, normal), -1);
    field<float>(joint, 0x88) = dot3(forward, relative_velocity(pulse)) / radius;
    int mode = field<int>(joint, 0x9C);
    if ((!mode && field<float>(joint, 0x80) < EPSILON) || (mode == 1 && field<float>(joint, 0x84) < EPSILON)) {
        wheel->side->friction = field<float>(joint, 0x64);
        wheel->side->lower = wheel->side->upper = 0;
        wheel->side->flags |= 1;
        wheel->side->normal = pulse;
        return;
    }
    wheel->side->lower = -10000000;
    wheel->side->upper = 10000000;
    wheel->forward = allocate_detached_scalar(world);
    field<physics_pulse_scalar *>(joint, 0xCC) = wheel->forward;
    set_scalar(
        wheel->forward, joint->b1, anchor1, joint->b2, anchor2, forward, &field<pulse_sum_cache>(joint, 0xBC), offset);
    auto *drive = wheel->forward;
    if (mode) {
        drive->velocity_target = drive->position_target = drive->softness = 0;
        float limit = elapsed * field<float>(joint, 0x84);
        drive->lower = -limit;
        drive->upper = limit;
    } else {
        float speed = field<float>(joint, 0x7C) * field<float>(joint, 0x78);
        drive->position_target = 0;
        drive->velocity_target = speed * radius;
        drive->softness = 1.0f / (field<float>(joint, 0x80) * field<float>(joint, 0x78) * elapsed / (radius * radius));
        drive->denominator += drive->softness;
        drive->lower = speed > EPSILON ? 0 : -10000000;
        drive->upper = speed < -0.0001f ? 0 : 10000000;
    }
    wheel->side->friction = field<float>(joint, 0x64);
    drive->friction = field<float>(joint, 0x60);
    field<float>(joint, 0x8C) = dot3(forward, response_direction(drive));
}
void finish_wheel(physics_constraint_link *joint, float elapsed)
{
    auto flags = field<unsigned>(joint, 0xA0);
    float suspension_offset = 0;
    if (flags & 1) {
        auto suspension = world_vector(joint->b1, field<physics_vec4>(joint, 0x3C));
        auto anchor1 =
            add(world_vector(joint->b1, field<physics_vec4>(joint, 0x2C)), mul(suspension, field<float>(joint, 0x5C)));
        auto anchor2 = world_vector(joint->b2, field<physics_vec4>(joint, 0xC));
        suspension_offset =
            std::min(dot3(sub(world_point(joint->b1, anchor1), world_point(joint->b2, anchor2)), suspension),
                     field<float>(joint, 0x70));
    }
    field<float>(joint, 0x94) = suspension_offset;
    field<float>(joint, 0x98) = field<physics_pulse_scalar *>(joint, 0xC4) ? field<float>(joint, 0xB0) : 0;
    auto *side = field<physics_pulse_scalar *>(joint, 0xC8);
    if (side) {
        auto *forward = field<physics_pulse_scalar *>(joint, 0xCC);
        if (forward) {
            if (forward->flags & 4)
                field<unsigned>(joint, 0xA0) = flags | 4;
            float impulse = clamp_scalar(forward,
                                         forward->impulse + (forward->velocity_target -
                                                             dot3(relative_change(forward), forward->direction) -
                                                             forward->softness * forward->impulse) /
                                                                forward->denominator);
            field<float>(joint, 0x88) += impulse * (field<float>(joint, 0x8C) / field<float>(joint, 0x5C));
        } else if (side->flags & 2)
            field<unsigned>(joint, 0xA0) = flags | 4;
    }
    field<float>(joint, 0x90) += elapsed * field<float>(joint, 0x88);
}
}  // namespace

void physics_execute_constraint_solver(physics_system *world, rb_partition_node *head, int visit, int next_visit)
{
    user_rigid_body *users = nullptr;
    world->field_0 |= 1;
    for (int type = 0; type < 7; ++type)
        for (auto *joint = head->constraints[type]; joint; joint = joint->next) {
            rigid_body *body = joint->b1;
            if (!body || !(body->field_144 & 0x20))
                body = joint->b2;
            if (body && (body->field_144 & 0x20)) {
                bool present = false;
                for (auto *user = users; user; user = field<user_rigid_body *>(user, 0x1B0))
                    if (user == body) {
                        present = true;
                        break;
                    }
                if (!present) {
                    auto *user = static_cast<user_rigid_body *>(body);
                    field<user_rigid_body *>(user, 0x1B0) = users;
                    users = user;
                }
            }
            if (type == 2)
                prepare_hinge(joint);
            if (type == 4)
                prepare_ragdoll(reinterpret_cast<rigid_body_constraint_ragdoll *>(joint));
            if (type == 5)
                prepare_wheel(joint);
        }
    world->field_0 &= ~1u;
    for (auto *user = users; user; user = field<user_rigid_body *>(user, 0x1B0))
        std::memcpy(reinterpret_cast<char *>(user) + 0x16C, &user->field_0, sizeof(matrix4x4));
    float elapsed = head->elapsed / head->steps;
    for (int step = 0; step < head->steps; ++step, ++visit) {
        world->field_298 = world->field_290;
        for (unsigned list : {0x29Cu, 0x2A8u, 0x2B4u, 0x2C0u, 0x2CCu}) {
            field<void *>(world, list) = nullptr;
            field<void *>(world, list + 4) = nullptr;
            field<int>(world, list + 8) = 0;
        }
        for (auto *node = head; node; node = node->head) {
            auto *body = node->body;
            auto *pulse = allocate_pulse<physics_pulse_body>(world, 0x29C);
            body->update_world_inverse_inertia(body->field_0);
            body->field_150 = reinterpret_cast<int>(pulse);
            pulse->body = body;
            pulse->inverse_mass = body->field_130;
        }
        for (int type = 0; type < 7; ++type)
            for (auto *joint = head->constraints[type]; joint; joint = joint->next) {
                switch (type) {
                case 0:
                    physics_setup_contact(reinterpret_cast<rigid_body_constraint_contact *>(joint), world, elapsed);
                    break;
                case 1:
                    setup_point_constraint(joint, world, elapsed);
                    break;
                case 2:
                    setup_hinge_constraint(joint, world, elapsed);
                    break;
                case 3:
                    physics_setup_distance(reinterpret_cast<rigid_body_constraint_distance *>(joint), world, elapsed);
                    break;
                case 4:
                    physics_setup_ragdoll(reinterpret_cast<rigid_body_constraint_ragdoll *>(joint), world, elapsed);
                    break;
                case 5:
                    setup_wheel(joint, world, elapsed);
                    break;
                case 6:
                    setup_actuator(joint, world, elapsed);
                    break;
                }
            }
        solve_pulses(world, visit, step + 1 == head->steps ? next_visit : visit + 1, elapsed, head->steps);
        for (auto *user = users; user; user = field<user_rigid_body *>(user, 0x1B0))
            user->integrate(elapsed);
        for (auto *joint = head->constraints[3]; joint; joint = joint->next)
            field<float>(joint, 0x30) += elapsed * field<float>(joint, 0x38);
        for (auto *joint = head->constraints[6]; joint; joint = joint->next)
            advance_actuator(joint, elapsed);
    }
    for (auto *user = users; user; user = field<user_rigid_body *>(user, 0x1B0))
        user->field_0 = field<matrix4x4>(user, 0x16C);
    for (auto *joint = head->constraints[1]; joint; joint = joint->next) {
        float a = field<float>(joint, 0x30), b = field<float>(joint, 0x38), c = field<float>(joint, 0x40);
        field<float>(joint, 0x44) = a * a + b * b + c * c;
    }
    for (auto *joint = head->constraints[5]; joint; joint = joint->next)
        finish_wheel(joint, head->elapsed);
}

void physics_set_scalar(physics_pulse_scalar *pulse, rigid_body *first, physics_vec4 anchor1, rigid_body *second,
                        physics_vec4 anchor2, physics_vec4 direction, pulse_sum_cache *cache, physics_vec4 offset)
{
    set_scalar(pulse, first, anchor1, second, anchor2, direction, cache, offset);
}
