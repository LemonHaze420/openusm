#include "damage_morphs.h"

#include "actor.h"
#include "camera.h"
#include "common.h"
#include "conglom.h"
#include "damage_interface.h"
#include "game.h"
#include "memory.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_morph.h"
#include "variables.h"
#include "oldmath_po.h"

#include <algorithm>

VALIDATE_SIZE(balanced_tree::tree_node, 0x20);
VALIDATE_SIZE(damage_morph_memory_pool::allocation, 0xC);
VALIDATE_OFFSET(damage_interface, morph_regions, 0x4C);
VALIDATE_OFFSET(damage_interface, morph_source, 0xAC);
VALIDATE_OFFSET(damage_interface, morph_mesh, 0xCC);
VALIDATE_OFFSET(damage_interface, morph_registration_id, 0xD4);

#if !STANDALONE_SYSTEM
int &damage_morphs::allocations_intercept_reference_count = var<int>(0x0095A760);
damage_morph_memory_pool &damage_morphs::write_combine_pool = var<damage_morph_memory_pool>(0x0095ABA4);
damage_morph_memory_pool &damage_morphs::normal_pool = var<damage_morph_memory_pool>(0x00921AC8);
balanced_tree &damage_morphs::registration_tree = var<balanced_tree>(0x0095AB98);
#else
namespace {
int allocation_intercepts{};
damage_morph_memory_pool write_pool{0};
damage_morph_memory_pool metadata_pool{0x1400};
balanced_tree mesh_registrations{};
}
int &damage_morphs::allocations_intercept_reference_count = allocation_intercepts;
damage_morph_memory_pool &damage_morphs::write_combine_pool = write_pool;
damage_morph_memory_pool &damage_morphs::normal_pool = metadata_pool;
balanced_tree &damage_morphs::registration_tree = mesh_registrations;
#endif

namespace {
struct mesh_registration {
    vhandle_type<actor> subject;
    nglMesh *original;
    nglMesh *copy;
};
VALIDATE_SIZE(mesh_registration, 0xC);

using tree_node = balanced_tree::tree_node;
int height(tree_node *node) { return node == nullptr ? 0 : node->height; }
void update_height(tree_node *node)
{
    node->height = 1 + std::max(height(node->left), height(node->right));
}
void replace_child(balanced_tree &tree, tree_node *old, tree_node *replacement)
{
    if (old->parent == nullptr)
        tree.root = replacement;
    else if (old->parent->left == old)
        old->parent->left = replacement;
    else
        old->parent->right = replacement;
    if (replacement != nullptr)
        replacement->parent = old->parent;
}
tree_node *rotate_left(balanced_tree &tree, tree_node *node)
{
    auto *right = node->right;
    replace_child(tree, node, right);
    node->right = right->left;
    if (node->right != nullptr)
        node->right->parent = node;
    right->left = node;
    node->parent = right;
    update_height(node);
    update_height(right);
    return right;
}
tree_node *rotate_right(balanced_tree &tree, tree_node *node)
{
    auto *left = node->left;
    replace_child(tree, node, left);
    node->left = left->right;
    if (node->left != nullptr)
        node->left->parent = node;
    left->right = node;
    node->parent = left;
    update_height(node);
    update_height(left);
    return left;
}
void rebalance(balanced_tree &tree, tree_node *node)
{
    while (node != nullptr) {
        update_height(node);
        const auto balance = height(node->left) - height(node->right);
        if (balance > 1) {
            if (height(node->left->right) > height(node->left->left))
                rotate_left(tree, node->left);
            node = rotate_right(tree, node);
        } else if (balance < -1) {
            if (height(node->right->left) > height(node->right->right))
                rotate_right(tree, node->right);
            node = rotate_left(tree, node);
        }
        node = node->parent;
    }
}
mesh_registration *find_registration(int id)
{
    int value;
    return damage_morphs::registration_tree.retrieve(id, &value)
        ? reinterpret_cast<mesh_registration *>(value) : nullptr;
}
void set_actor_mesh(actor *subject, nglMesh *mesh)
{
    subject->field_90.set_mesh(mesh);
    if (mesh != nullptr)
        subject->field_4 |= 0x100;
    else
        subject->field_4 &= ~0x100;
}


int nearest_morph_region(actor *subject, damage_interface &damage)
{
    if ((subject->field_4 & 4) == 0)
        return 0;
    auto *group = static_cast<conglomerate *>(subject);
    auto *member = group->get_member(damage.morph_regions[0].member.m_hash, true);
    if (member == nullptr)
        return 0;
    const auto impact = damage.field_104.field_4;
    auto distance = (member->get_abs_position() - impact).length2();
    int nearest = 0;
    for (int region = 1; region < 6 && damage.morph_regions[region].member.is_set(); ++region) {
        member = group->get_member(damage.morph_regions[region].member.m_hash, true);
        if (member == nullptr)
            break;
        const auto candidate = (member->get_abs_position() - impact).length2();
        if (candidate < distance) {
            distance = candidate;
            nearest = region;
        }
    }
    return nearest;
}
}

damage_morph_memory_pool::damage_morph_memory_pool(int size)
    : allocation_list(nullptr), list_end(nullptr), field_8(size), memory_pool(nullptr),
      field_10(0), field_14(0), field_18(0)
{
}

void damage_morph_memory_pool::init()
{
    assert(memory_pool == nullptr);
    memory_pool = arch_memalign(4, field_8);
    field_10 = reinterpret_cast<uintptr_t>(memory_pool) + field_8;
    field_14 = reinterpret_cast<uintptr_t>(memory_pool);
    field_18 = field_10;
}

void *damage_morph_memory_pool::memalloc(int alignment, int size)
{
    auto start = field_14;
    auto end = start + size + alignment;
    if (end >= field_10) {
        start = reinterpret_cast<uintptr_t>(memory_pool);
        end = start + size + alignment;
    }
    field_14 = end;
    auto *memory = reinterpret_cast<void *>(start + alignment - start % alignment);
    auto *entry = new (arch_memalign(4, sizeof(allocation))) allocation{memory, true, nullptr};
    if (allocation_list != nullptr)
        list_end->next = entry;
    else
        allocation_list = entry;
    list_end = entry;
    return memory;
}

uintptr_t damage_morph_memory_pool::memfree(void *memory)
{
    auto *entry = allocation_list;
    while (entry != nullptr && entry->memory != memory)
        entry = entry->next;
    if (entry != nullptr)
        entry->live = false;
    while (allocation_list != nullptr && !allocation_list->live) {
        auto *released = allocation_list;
        allocation_list = released->next;
        mem_freealign(released);
    }
    if (allocation_list == nullptr)
        list_end = nullptr;
    return allocation_list == nullptr ? 0 : reinterpret_cast<uintptr_t>(allocation_list->memory);
}

bool balanced_tree::retrieve(int key, int *value)
{
    auto *node = root;
    while (node != nullptr && node->key != key)
        node = key < node->key ? node->left : node->right;
    if (node == nullptr)
        return false;
    *value = node->value;
    return true;
}

void balanced_tree::add(int key, int value)
{
    auto *node = new (arch_memalign(4, sizeof(tree_node))) tree_node{key, value, nullptr,
        nullptr, nullptr, nullptr, newest, 1};
    if (newest != nullptr)
        newest->next = node;
    else
        oldest = node;
    newest = node;
    auto **child = &root;
    tree_node *parent = nullptr;
    while (*child != nullptr) {
        parent = *child;
        child = key < parent->key ? &parent->left : &parent->right;
    }
    *child = node;
    node->parent = parent;
    rebalance(*this, parent);
}

bool balanced_tree::remove(int key)
{
    auto *node = root;
    while (node != nullptr && node->key != key)
        node = key < node->key ? node->left : node->right;
    if (node == nullptr)
        return false;
    if (node->previous != nullptr)
        node->previous->next = node->next;
    else
        oldest = node->next;
    if (node->next != nullptr)
        node->next->previous = node->previous;
    else
        newest = node->previous;
    tree_node *rebalance_from = node->parent;
    if (node->left != nullptr && node->right != nullptr) {
        auto *successor = node->right;
        while (successor->left != nullptr)
            successor = successor->left;
        if (successor->parent != node) {
            rebalance_from = successor->parent;
            replace_child(*this, successor, successor->right);
            successor->right = node->right;
            successor->right->parent = successor;
        } else {
            rebalance_from = successor;
        }
        replace_child(*this, node, successor);
        successor->left = node->left;
        successor->left->parent = successor;
        update_height(successor);
    } else {
        replace_child(*this, node, node->left != nullptr ? node->left : node->right);
    }
    mem_freealign(node);
    rebalance(*this, rebalance_from);
    return true;
}

void damage_morphs::init_memory_pools()
{
    normal_pool.init();
    write_combine_pool.init();
}

bool damage_morphs::intercepting_allocations()
{
    return allocations_intercept_reference_count > 0;
}

bool damage_morphs::is_subject_off_screen(actor *subject)
{
    auto *camera = g_game_ptr->get_current_view_camera(0);
    if (camera == nullptr)
        return false;
    const auto &camera_pose = camera->get_abs_po();
    const auto facing = camera_pose.get_z_facing();
    auto direction = subject->get_abs_position() - camera->get_abs_position();
    direction.normalize();
    return dot(facing, direction) < 0.25f;
}

int damage_morphs::register_mesh_copy(actor *subject)
{
    static int next_id = 0;
    const auto id = next_id++;
    auto *entry = new mesh_registration{{subject->my_handle}, subject->get_mesh(), nullptr};
    registration_tree.add(id, reinterpret_cast<int>(entry));
    ++allocations_intercept_reference_count;
    entry->copy = nglCreateMeshClone(entry->original);
    entry->copy->Name = entry->original->Name;
    for (uint32_t section = 0; section < entry->original->NSections; ++section) {
        if (entry->original->Sections[section].Section->VertexDef != nullptr)
            nglMakeSectionUnique(entry->copy, section);
    }
    --allocations_intercept_reference_count;
    set_actor_mesh(subject, entry->copy);
    return id;
}

bool damage_morphs::unregister_mesh_copy(int id)
{
    auto *entry = find_registration(id);
    if (entry == nullptr)
        return false;
    if (auto *subject = entry->subject.get_volatile_ptr())
        set_actor_mesh(subject, entry->original);
    ++allocations_intercept_reference_count;
    nglDestroyMesh(entry->copy);
    --allocations_intercept_reference_count;
    delete entry;
    return registration_tree.remove(id);
}

void *damage_morphs::memalloc(int alignment, int size, bool write_combine)
{
    auto &pool = write_combine ? write_combine_pool : normal_pool;
    const auto required = alignment + size;
    for (;;) {
        const auto current = pool.field_14;
        const auto oldest = pool.field_18;
        const auto free_space = current >= oldest
            ? oldest - reinterpret_cast<uintptr_t>(pool.memory_pool) : oldest - current;
        if ((current >= oldest && pool.field_10 - current > static_cast<uintptr_t>(required)) ||
            free_space > static_cast<uintptr_t>(required))
            return pool.memalloc(alignment, size);
        if (registration_tree.oldest == nullptr)
            return nullptr;
        unregister_mesh_copy(registration_tree.oldest->key);
    }
}

void damage_morphs::memfree(void *memory)
{
    auto *entry = normal_pool.allocation_list;
    while (entry != nullptr && (entry->memory != memory || !entry->live))
        entry = entry->next;
    auto &pool = entry != nullptr ? normal_pool : write_combine_pool;
    pool.field_18 = pool.memfree(memory);
    if (pool.field_18 == 0) {
        pool.field_14 = reinterpret_cast<uintptr_t>(pool.memory_pool);
        pool.field_18 = pool.field_10;
    }
}


void damage_morphs::instance_frame_advance(actor *subject)
{
    auto *damage = subject->damage_ifc();
    if (damage->morph_source[0] == '\0')
        return;
    auto *morph = subject->get_morph(tlFixedString{damage->morph_source}, false);
    if (morph == nullptr)
        return;
    const auto health = static_cast<int>(damage->field_1FC.field_0[0]);
    int new_damage = static_cast<int>(damage->field_1FC.field_0[2]) - health;
    int active_count = 0;
    for (const auto &region : damage->morph_regions)
        new_damage -= region.accumulated_damage;
    if (new_damage != 0)
        damage->morph_regions[nearest_morph_region(subject, *damage)].accumulated_damage += new_damage;
    for (const auto &region : damage->morph_regions)
        active_count += region.accumulated_damage >= region.threshold;
    if (active_count == 0)
        return;
    auto *registration = find_registration(damage->morph_registration_id);
    if (health <= 0) {
        if (registration != nullptr)
            unregister_mesh_copy(damage->morph_registration_id);
        return;
    }
    if ((subject->field_4 & 4) != 0) {
        auto *member = static_cast<conglomerate *>(subject)->get_member(damage->morph_mesh.m_hash, true);
        if (member == nullptr || !member->is_an_actor())
            return;
        subject = static_cast<actor *>(member);
    }
    auto *mesh = subject->get_mesh();
    if (mesh == nullptr)
        return;
    if (registration != nullptr) {
        if (new_damage == 0)
            return;
    } else {
        if (is_subject_off_screen(subject))
            return;
        damage->morph_registration_id = register_mesh_copy(subject);
        registration = find_registration(damage->morph_registration_id);
        mesh = subject->get_mesh();
    }
    nglCopyMesh(mesh, registration->original);
    nglMorphFrame frames[6];
    nglMorphEntry entries[6];
    int count = 0;
    for (uint32_t frame = 1; frame <= 6; ++frame) {
        const auto &region = damage->morph_regions[frame - 1];
        if (region.accumulated_damage < region.threshold)
            continue;
        if (frame > static_cast<uint32_t>(morph->NFrames))
            return;
        new (&frames[count]) nglMorphFrame{morph->Frames + frame};
        entries[count] = {1.0f, &frames[count]};
        ++count;
    }
    nglBlendMorphs(mesh, count, entries);
}
