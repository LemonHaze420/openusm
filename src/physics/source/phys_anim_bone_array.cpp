#include "phys_anim_bone_array.h"

#include "common.h"
#include "conglom.h"
#include "entity_base.h"
#include "oldmath_po.h"
#include <cmath>
#include "string_hash.h"
#include <cstdint>

VALIDATE_SIZE(phys_anim_bone_array, 0x1104);
VALIDATE_OFFSET(phys_anim_bone_array, bones, 0x5A0);
VALIDATE_OFFSET(phys_anim_bone_array, saved_poses, 0x10F0);
VALIDATE_OFFSET(phys_anim_bone_array, owner, 0x1100);

void phys_anim_bone_array::copy_back_bones_recurse(entity_base *parent)
{
    int index = -1;
    if (parent == nullptr) {
        owner->dirty_family(true);
        parent = owner;
    }
    copy_back_bones(parent, index);
}

void phys_anim_bone_array::copy_back_bones(entity_base *parent, int &index)
{
    if (index == -1) {
        for (auto *child = parent->m_child; child != nullptr; child = child->field_28)
            if (child->field_4 & (0x8000u | 4u))
                child->dirty_model_po_family();
    }
    for (auto *child = parent->m_child; child != nullptr; child = child->field_28) {
        if (child->field_40 == 0xFF)
            continue;
        const auto &entry = bones[++index];
        if (entry.rigid_body_index == -1) {
            if (entry.saved_pose_index != -1) {
                if (child->field_8 & 0x08000000u)
                    child->compute_rel_po_from_model();
                *child->my_rel_po = saved_poses[entry.saved_pose_index].relative;
                if (child->field_8 & 0x08000000u)
                    child->compute_rel_po_from_model();
                po::compose(*child->my_abs_po, *parent->my_abs_po, *child->my_rel_po);
            }
        } else if (index != 0) {
            if (child->field_8 & 0x08000000u)
                child->compute_rel_po_from_model();
            po::compose_ortho(*child->my_rel_po, *parent->my_abs_po, *child->my_abs_po);
            if (child->field_8 & 0x08000000u)
                child->compute_rel_po_from_model();
            child->my_abs_po->set_position(
                parent->my_abs_po->slow_xform(child->my_rel_po->get_position()));
        }
        child->field_8 &= ~0x10000000u;
        copy_back_bones(child, index);
    }
}

void phys_anim_bone_array::update_owner_matrix(const vector3d &position)
{
    auto *root_bone = bones[0].bone;
    const auto &root = *root_bone->my_abs_po;
    const auto down = -root.get_x_facing();
    const auto back = -root.get_z_facing();
    vector3d forward;
    if (std::fabs(down.y) >= 0.7f)
        forward = vector3d(back.x, 0.0f, back.z);
    else if (back.y < 0.0f)
        forward = vector3d(down.x, 0.0f, down.z);
    else
        forward = vector3d(-down.x, 0.0f, -down.z);
    forward.normalize();
    po restored;
    restored.set_po(vector3d(forward.z, 0.0f, -forward.x),
                    vector3d(0.0f, 1.0f, 0.0f), forward, position);
    *owner->my_abs_po = restored;
    if (owner->m_parent != nullptr) {
        if (owner->field_8 & 0x08000000u)
            owner->compute_rel_po_from_model();
        po::full_inv_multiply(*owner->my_rel_po, *owner->m_parent->my_abs_po,
                              *owner->my_abs_po);
    } else {
        if (owner->field_8 & 0x08000000u)
            owner->compute_rel_po_from_model();
        if (owner->my_rel_po != owner->my_abs_po) {
            if (owner->field_8 & 0x08000000u)
                owner->compute_rel_po_from_model();
            *owner->my_rel_po = *owner->my_abs_po;
        }
    }
    if (root_bone->field_8 & 0x08000000u)
        root_bone->compute_rel_po_from_model();
    po::full_inv_multiply(*root_bone->my_rel_po, *owner->my_abs_po, *root_bone->my_abs_po);
    owner->field_8 = (owner->field_8 & ~0x10000000u) | 0x40u;
}

void phys_anim_bone_array::copy_bones(entity_base *parent)
{
    for (auto *child = parent->m_child; child; child = child->field_28) {
        if (child->field_40 == 0xFF)
            continue;
        assert(bone_count < 90);
        bones[bone_count++].bone = child;
        copy_bones(child);
    }
}

void phys_anim_bone_array::assign_parents(entity_base *parent, int rigid_parent, int &index)
{
    for (auto *child = parent->m_child; child; child = child->field_28) {
        if (child->field_40 == 0xFF)
            continue;
        auto &entry = bones[++index];
        entry.field_8 = rigid_parent;
        assign_parents(child, entry.rigid_body_index == -1
            ? rigid_parent : entry.rigid_body_index, index);
    }
}

bool phys_anim_bone_array::assign_saved_poses(entity_base *parent, int &index)
{
    const int parent_index = index;
    if (parent_index != -1)
        bones[parent_index].saved_pose_index = -1;
    auto save = [this](phys_anim_bone_entry &entry) {
        if (entry.saved_pose_index == -1) {
            assert(field_10F4 < 30);
            entry.saved_pose_index = field_10F4++;
        }
    };
    for (auto *child = parent->m_child; child; child = child->field_28) {
        if (child->field_40 == 0xFF)
            continue;
        const int child_index = ++index;
        if (assign_saved_poses(child, index) && parent_index != -1)
            save(bones[parent_index]);
        if (parent_index != -1 && bones[parent_index].rigid_body_index != -1)
            save(bones[child_index]);
    }
    if (parent_index == -1)
        return true;
    auto &entry = bones[parent_index];
    if (entry.rigid_body_index != -1)
        save(entry);
    return entry.rigid_body_index != -1 || entry.saved_pose_index != -1;
}

void phys_anim_bone_array::attach_physics_bones()
{
    copy_bones(owner);
    static const string_hash names[] = {
        string_hash{"BIP01 PELVIS"}, string_hash{"BIP01 HEAD"},
        string_hash{"BIP01 L UPPERARM"}, string_hash{"BIP01 L FOREARM"},
        string_hash{"BIP01 R UPPERARM"}, string_hash{"BIP01 R FOREARM"},
        string_hash{"BIP01 L THIGH"}, string_hash{"BIP01 L CALF"},
        string_hash{"BIP01 R THIGH"}, string_hash{"BIP01 R CALF"}};
    for (int i = 0; i < bone_count; ++i) {
        auto &entry = bones[i];
        entry.rigid_body_index = entry.field_8 = -1;
        for (int body = 0; body < 10; ++body)
            if (entry.bone->field_10 == names[body])
                entry.rigid_body_index = body;
    }
    int index = -1;
    assign_parents(owner, 0, index);
    index = -1;
    assign_saved_poses(owner, index);
}

void phys_anim_bone_array::save_poses(po *absolute, po *relative)
{
    for (int i = 0; i < bone_count; ++i) {
        auto *bone = bones[i].bone;
        absolute[i] = bone->get_abs_po();
        if (bone->field_8 & 0x08000000u)
            bone->compute_rel_po_from_model();
        relative[i] = *bone->my_rel_po;
    }
}

void phys_anim_bone_array::restore_poses(const po *absolute, const po *relative)
{
    for (int i = 0; i < bone_count; ++i) {
        auto *bone = bones[i].bone;
        *bone->my_abs_po = absolute[i];
        if (bone->field_8 & 0x08000000u)
            bone->compute_rel_po_from_model();
        *bone->my_rel_po = relative[i];
        bone->field_8 &= ~0x10000000u;
    }
}

void phys_anim_bone_array::prepare_physics_pose()
{
    static const string_hash end_names[] = {
        string_hash{"BIP01 HEAD"}, string_hash{"BIP01 NECK"},
        string_hash{"BIP01 NECK1"}, string_hash{"BIP01 SPINE"},
        string_hash{"BIP01 SPINE1"}, string_hash{"BIP01 SPINE2"},
        string_hash{"BIP01 R HAND"}, string_hash{"BIP01 R FOOT"},
        string_hash{"BIP01 L HAND"}, string_hash{"BIP01 L FOOT"}};
    static const std::uint32_t rotations[10][4] = {
        {0,3193573466u,3045472189u,1064968380u},
        {0,0,0,1065353216u}, {0,1048871917u,897988541u,1064781549u},
        {906377149u,969976488u,897988541u,1065353216u},
        {0,0,0,1065353216u}, {0,0,0,1065353216u},
        {3207915385u,3162284025u,3162297984u,1060441183u},
        {3171571892u,1026766961u,3179994860u,1065288892u},
        {1060431737u,3162284025u,1014814336u,1060441183u},
        {1024088244u,1026766961u,1032511212u,1065288892u}};
    for (int i = 0; i < bone_count; ++i) {
        auto &entry = bones[i];
        auto *bone = entry.bone;
        if (bone->field_8 & 0x08000000u)
            bone->compute_rel_po_from_model();
        if (entry.saved_pose_index != -1)
            saved_poses[entry.saved_pose_index].original_rotation =
                quaternion{*reinterpret_cast<matrix4x4 *>(bone->my_rel_po)};
        for (int end = 0; end < 10; ++end) {
            if (bone->field_10 != end_names[end])
                continue;
            const auto *r = reinterpret_cast<const float *>(rotations[end]);
            const quaternion rotation{r[3], r[0], r[1], r[2]};
            const auto position = bone->my_rel_po->get_position();
            rotation.to_matrix(*reinterpret_cast<matrix4x4 *>(bone->my_rel_po));
            bone->my_rel_po->set_position(position);
            bone->dirty_family(false);
            if (bone->field_4 & (0x8000u | 4u))
                bone->dirty_model_po_family();
            bone->po_changed();
            break;
        }
    }
    for (int i = 0; i < bone_count; ++i) {
        auto &entry = bones[i];
        if (entry.saved_pose_index == -1)
            continue;
        if (entry.bone->field_8 & 0x08000000u)
            entry.bone->compute_rel_po_from_model();
        auto &saved = saved_poses[entry.saved_pose_index];
        saved.relative = *entry.bone->my_rel_po;
        saved.physics_rotation = quaternion{*reinterpret_cast<matrix4x4 *>(entry.bone->my_rel_po)};
    }
}

void phys_anim_bone_array::copy_back_tween_recurse(float fraction)
{
    int index = -1;
    owner->dirty_family(true);
    copy_back_tween(owner, index, fraction);
}

void phys_anim_bone_array::copy_back_tween(entity_base *parent, int &index, float fraction)
{
    if (index == -1)
        for (auto *child = parent->m_child; child; child = child->field_28)
            if (child->field_4 & (0x8000u | 4u))
                child->dirty_model_po_family();
    for (auto *child = parent->m_child; child; child = child->field_28) {
        if (child->field_40 == 0xFF) {
            child->dirty_family(false);
            continue;
        }
        const int current = ++index;
        const auto &entry = bones[current];
        if ((entry.rigid_body_index == -1 && entry.saved_pose_index != -1) ||
            (entry.rigid_body_index != -1 && current != 0)) {
            if (child->field_8 & 0x08000000u)
                child->compute_rel_po_from_model();
            if (entry.rigid_body_index != -1)
                po::compose_ortho(*child->my_rel_po, *parent->my_abs_po, *child->my_abs_po);
            const auto &saved = saved_poses[entry.saved_pose_index];
            const quaternion target = entry.rigid_body_index == -1
                ? saved.physics_rotation
                : quaternion{*reinterpret_cast<matrix4x4 *>(child->my_rel_po)};
            const auto position = child->my_rel_po->m[3];
            slerp(saved.original_rotation, target, fraction).to_matrix(
                *reinterpret_cast<matrix4x4 *>(child->my_rel_po));
            child->my_rel_po->m[3] = position;
            po::compose(*child->my_abs_po, *parent->my_abs_po, *child->my_rel_po);
        }
        child->field_8 &= ~0x10000000u;
        copy_back_tween(child, index, fraction);
    }
}
