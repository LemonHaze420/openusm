#include <nal_skeleton.h>

#include <func_wrapper.h>
#include <vtbl.h>

#include <trace.h>

#include <nal_list.h>
#include <nal_anim_comp.h>
#include <nal_system.h>
#include <tl_instance_bank.h>


nalBasePose* nalBaseSkeleton::VirtualGetDefaultPose()
{
    nalBasePose * (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x24));
    return func(this);
}

nalBasePose* nalBaseSkeleton::VirtualCreatePose()
{
    nalBasePose * (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x28));
    return func(this);
}

void nalBaseSkeleton::VirtualDestroyPose(nalBasePose *a2)
{
    void (__fastcall *func)(void *, void *edx, nalBasePose *) = CAST(func, get_vfunc(this->m_vtbl, 0x2C));
    func(this, nullptr, a2);
}

void sub_826190(nalBasePose &dst, Float a2, nalBasePose &src0, nalBasePose &src1)
{
    assert(dst.GetSkeleton() == src0.GetSkeleton() && dst.GetSkeleton() == src1.GetSkeleton()
        && "attempting to blend incompatible skeletons");

    auto *v6 = dst.GetSkeleton();
    v6->VirtualBlend(&dst, a2, &src0, &src1);
}

void * nalConstructSkeleton(void *a1)
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
