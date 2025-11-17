#include <nal_skeleton.h>

#include <func_wrapper.h>
#include <vtbl.h>

#include <trace.h>

#include <nal_list.h>
#include <nal_system.h>
#include <tl_instance_bank.h>


#include <cmath>
#include "vector3d.h"
#include "vector4d.h"
#include "matrix4x4.h"

void *nalConstructSkeleton(void *a1)
{
    TRACE("nalConstructSkeleton");

    if constexpr (0)
    {
        struct {
            std::intptr_t m_vtbl;
            int Version;
            tlFixedString field_8;
            tlHashString field_28;
            int field_48;
            int field_4C;
            int field_50;

        } *skel = static_cast<decltype(skel)>(a1);

#ifdef TARGET_XBOX
        tlFixedString str = *bit_cast<tlFixedString *>(&skel->field_28);
#else
        const tlHashString &str = skel->field_28;
#endif

        auto *instance = nalTypeInstanceBank.Search(str);
        assert(instance != nullptr && "unable to find skeleton type in type instance bank");

        auto *v1 = static_cast<nalInitListAnimType *>(instance->field_20);

        auto vtbl = v1->skel_vtbl_ptr;
        skel->m_vtbl = vtbl;
        sp_log("0x%08X", vtbl);

        bool (__fastcall *CheckVersion)(void *) = CAST(CheckVersion, get_vfunc(skel->m_vtbl, 0x10));
        if ( !CheckVersion(a1) )
        {
#ifdef TARGET_XBOX
            auto v3 = skel->Version;
            auto *v5 = skel->field_8.to_string();
            sp_log("Unsupported skeleton version %x (%s).\n", v3, v5);
            assert(0);
#endif
        }

        void (__fastcall *Process)(void *) = CAST(Process, get_vfunc(skel->m_vtbl, 0x8));
        Process(a1);

        skel->field_50 = 0;
        return a1;

    } else {
        return (void *) CDECL_CALL(0x0078DC80, a1);
    }
}

namespace inverse_kinematics {

    // let distance d = ||T - P|| and precomputed coefficients,
    //    cos0 = a0 * d + b0 / d
    //    cos1 = a1 * d + b1 / d
    //    sin_i = sqrt(1 - cos_i^2)
    void __cdecl solve_two_bone_angles(
        matrix4x4* hinge,
        vector3d* root,
        vector3d* target,
        float b0a_len,
        float b1a_len,
        float b0b_len,
        float b1b_len,
        vector3d* proj_point,
        vector3d* bone_axis_dir,
        float* sin0,
        float* cos0,
        float* sin1,
        float* cos1)
    {
        vector3d tmpProj;
        ProjectPointOntoLineXform(&tmpProj, root, hinge);
        *proj_point = tmpProj;

        vector3d diff;
        diff.x = target->x - proj_point->x;
        diff.y = target->y - proj_point->y;
        diff.z = target->z - proj_point->z;

        // ||T - P||
        float bend_radius = std::sqrt(
            diff.x * diff.x +
            diff.y * diff.y +
            diff.z * diff.z);

        float inv_radius = 1.0f / bend_radius;

        // (T - P) / ||T - P||
        bone_axis_dir->x = diff.x * inv_radius;
        bone_axis_dir->y = diff.y * inv_radius;
        bone_axis_dir->z = diff.z * inv_radius;

        float cos0_raw = bend_radius * b0a_len + inv_radius * b0b_len;
        float cos1_raw = bend_radius * b1a_len + inv_radius * b1b_len;

        // clamp -1,1
        float c0 = cos0_raw;
        if (c0 > 1.0f)
            c0 = 1.0f;
        else if (c0 < -1.0f)
            c0 = -1.0f;
        *cos0 = c0;

        float c1 = cos1_raw;
        if (c1 > 1.0f)
            c1 = 1.0f;
        else if (c1 < -1.0f)
            c1 = -1.0f;
        *cos1 = c1;

        // sin = sqrt(1 - cos^2)
        *sin0 = std::sqrt(1.0f - (*cos0) * (*cos0));
        *sin1 = std::sqrt(1.0f - (*cos1) * (*cos1));
    }

    void __cdecl build_chain_transforms(
        float chain_scale,
        float sin0,
        float cos0,
        float sin1,
        float cos1,
        vector3d* origin,
        vector3d* bone_axis_dir,
        vector4d* bend_dir,
        float chain_sin0,
        float chain_cos0,
        matrix4x4* joint0,
        matrix4x4* joint1)
    {
        const vector3d bone = *bone_axis_dir;
        const vector3d bend{
            bend_dir->x,
            bend_dir->y,
            bend_dir->z
        };

        // N = normalize(bone x bend)
        vector3d normal = vector3d::cross(bone, bend).normalized();
        vector3d tangent = vector3d::cross(normal, bone);

        // pack basis
        vector4d axisX{ bone.x,   bone.y,   bone.z,   0.0f };
        vector4d axisY{ tangent.x,tangent.y,tangent.z,0.0f };
        vector4d axisZ{ normal.x, normal.y, normal.z, 0.0f };

        vector4d origin4{
            origin->x,
            origin->y,
            origin->z,
            1.0f
        };

        matrix4x4 hinge_space;
        hinge_space.compose_from_basis(&axisX, &axisY, &axisZ, &origin4);

        // j0

        const float neg_chain_sin0 = -chain_sin0;

        vector4d j0_x{ // r00
            cos0,
            sin0 * chain_cos0,
            sin0 * chain_sin0,
            0.0f
        };

        vector4d j0_y{ // r01
            -sin0,
            cos0 * chain_cos0,
            cos0 * chain_sin0,
            0.0f
        };

        vector4d j0_z{ // r02
            0.0f,
            neg_chain_sin0,
            chain_cos0,
            0.0f
        };

        vector4d j0_pos{ 0,0,0,1 }; // r03

        matrix4x4 joint0_local;
        joint0_local.compose_from_basis(&j0_x, &j0_y, &j0_z, &j0_pos);

        local_to_world(joint0, &joint0_local, &hinge_space);

        // j1

        float off_sin0 = chain_scale * sin0;

        vector4d j1_pos{ // r03
            chain_scale * cos0,
            chain_cos0 * off_sin0,
            chain_sin0 * off_sin0,
            1.0f
        };

        vector4d j1_z{ // same as j0_z
            0.0f,
            neg_chain_sin0,
            chain_cos0,
            0.0f
        };

        vector4d j1_y{ // r01
            sin1,
            cos1 * chain_cos0,
            cos1 * chain_sin0,
            0.0f
        };

        vector4d j1_x{ // r00
            cos1,
            -(sin1 * chain_cos0),
            -(sin1 * chain_sin0),
            0.0f
        };

        matrix4x4 joint1_local;
        joint1_local.compose_from_basis(&j1_x, &j1_y, &j1_z, &j1_pos);

        local_to_world(joint1, &joint1_local, &hinge_space);
    }

    inline void flip_chain_basis(matrix4x4* m) {
        vector4d r0 = m->arr[0],
            r1 = m->arr[1],
            r2 = m->arr[2];

        auto negate = [](vector4d v) {
            v.x = -v.x;
            v.y = -v.y;
            v.z = -v.z;
            v.w = -v.w;
            return v;
        };

        r0 = negate(r0);
        r1 = negate(r1);
        r2 = negate(r2);

        std::swap(r1, r2);

        m->arr[0] = r0;
        m->arr[1] = r1;
        m->arr[2] = r2;
    }

    void __cdecl solve_two_bone(
        matrix4x4* j0,
        matrix4x4* j1,
        matrix4x4* line_xform,
        vector3d* root,
        matrix4x4* effector,
        ik_bone_chain_t* chain,
        GetBendDirection_t GetBendDirection)
    {
        vector3d target{ effector->arr[3].x,effector->arr[3].y, effector->arr[3].z };
        float    sin0 = 0.0f;
        float    sin1 = 0.0f;
        float    cos0 = 1.0f;
        float    cos1 = 1.0f;
        vector3d proj_point{};
        vector3d bone_axis_dir{};

        inverse_kinematics::solve_two_bone_angles(
            line_xform,
            root,
            &target,
            chain->b0a_len,
            chain->b0b_len,
            chain->b1a_len,
            chain->b1b_len,
            &proj_point,
            &bone_axis_dir,
            &sin0,
            &cos0,
            &sin1,
            &cos1);

        vector3d tmp_bend{};
        vector3d* bend_dir = GetBendDirection(&tmp_bend, line_xform, effector, bone_axis_dir.x, bone_axis_dir.y, bone_axis_dir.z);
        vector4d bone_bend_dir{ bend_dir->x, bend_dir->y, bend_dir->z, 0.0f };

        inverse_kinematics::build_chain_transforms(
            chain->chain_scale,
            sin0,
            cos0,
            sin1,
            cos1,
            &proj_point,
            &bone_axis_dir,
            &bone_bend_dir,
            0.0f,   // chain_sin0
            1.0f,   // chain_cos0
            j0,
            j1);

        flip_chain_basis(j0);
        flip_chain_basis(j1);
    }

    void __cdecl solve_two_bone_with_twist(
        matrix4x4* joint0,
        matrix4x4* joint1,
        matrix4x4* hinge,
        vector3d* root,
        matrix4x4* effector,
        ik_bone_chain_t* chain,
        GetBendDirection_t GetBendDirection,
        float                   twistAngle)
    {
        vector3d target{ effector->arr[3].x, effector->arr[3].y, effector->arr[3].z };

        float    sin0 = 0.0f;
        float    sin1 = 0.0f;
        float    cos0 = 1.0f;
        float    cos1 = 1.0f;
        vector3d proj_point{};
        vector3d axis_dir{}; // hinge space

        inverse_kinematics::solve_two_bone_angles(
            hinge,
            root,
            &target,
            chain->b0a_len,
            chain->b0b_len,
            chain->b1a_len,
            chain->b1b_len,
            &proj_point,
            &axis_dir,
            &sin0,
            &cos0,
            &sin1,
            &cos1);

        vector3d tmp{};
        vector3d* bend_vec = GetBendDirection(&tmp, hinge, effector, axis_dir.x, axis_dir.y, axis_dir.z);
        vector4d bend_dir{ bend_vec->x, bend_vec->y, bend_vec->z, 0.0f };

        // apply twist
        float chain_cos0 = std::cos(twistAngle);
        float chain_sin0 = std::sin(twistAngle);

        inverse_kinematics::build_chain_transforms(
            chain->chain_scale,
            sin0,
            cos0,
            sin1,
            cos1,
            &proj_point,
            &axis_dir,
            &bend_dir,
            chain_sin0,
            chain_cos0,
            joint0,
            joint1);

        flip_chain_basis(joint0);
        flip_chain_basis(joint1);
    }
}