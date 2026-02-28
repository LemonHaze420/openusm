#include <nal_list.h>

#include <common.h>
#include <log.h>
#include <trace.h>
#include <vtbl.h>

#include <character_anim_inst.h>
#include <character_pose_skel.h>
#include <nal_generic_component.h>
#include <panel_pose_skel.h>
#include <panel_anim_inst.h>
#include <ped_skel_pose.h>
#include <ped_anim_inst.h>
#include <spidey_signal.h>
#include <camera_skel_pose.h>
#include <camera_anim_inst.h>
#include <variables.h>

#include <nal_system.h>

VALIDATE_SIZE(nalInitListAnimType, 0x30);

#if !STANDALONE_SYSTEM

void * & nalInitList::head = var<void *>(0x00977154);

#else

void * & nalInitList::head = []() -> auto & {
    static void * g_head {};
    return g_head;
}();

#define make_class(Type, Str)                               \
    static nalComponentInitList InitListComponent_##Type {  \
        Str,                                                \
        &Component_##Type                                   \
    }

make_class(spideySignal, "Spidey_Signal");
make_class(nalComponentRLE8Int1, "NAL_RLE8Int1");

#undef make_class

namespace nalChar {
    int & nalCharSkeleton::vtbl_ptr = []() -> auto & {
        static nalCharSkeleton skel {};
        return skel.m_vtbl;
    }();
}

static nalInitListAnimType InitListAnimType_nalCharAnim {
    "Character",
    nalChar::nalCharAnim::vtbl_ptr,
    nalChar::nalCharSkeleton::vtbl_ptr
};

static nalInitListAnimType InitListAnimType_nalPanelAnim {
    "Panel",
    nalPanel::nalPanelAnim::vtbl_ptr,
    nalPanel::nalPanelSkeleton::vtbl_ptr
};

static nalInitListAnimType InitListAnimType_nalGenericAnim {
    "generic",
    nalGeneric::nalGenericAnim::vtbl_ptr,
    nalGeneric::nalGenericSkeleton::vtbl_ptr
};

static nalInitListAnimType InitListAnimType_nalCamAnim {
        "Camera",
        nalCam::nalCamAnim::vtbl_ptr,
        nalCam::nalCamSkeleton::vtbl_ptr
};

#define make_class(Type, Base, Str) \
    struct Type : Base {            \
    };                              \
    struct Type##Data {             \
    };                              \
                                    \
    static nalComponent<Base, Type##Data, Type> Component_##Type {}; \
                                                                     \
    static nalComponentInitList InitListComponent_##Type {           \
        Str,                                                         \
        &Component_##Type                                            \
    }

make_class(nalComponentEntropyQuat, nalComponentQuatBase, "NAL_EntropyQuaternion");

make_class(nalComponentEntropyFloat1, nalComponentFloat1Base, "NAL_EntropyFloat1");
make_class(nalComponentEntropyFloat3, nalComponentFloat3Base, "NAL_EntropyFloat3");

make_class(nalComponentPO, nalComponentPOBase, "NAL_PositionOrientation");
make_class(nalComponentEntropyPO, nalComponentPOBase, "NAL_EntropyPositionOrientation");
make_class(nalComponentEntropyTrajectoryPO, nalComponentPOBase, "NAL_EntropyTrajectoryPositionOrientation");

#undef make_class

#endif

nalInitList::nalInitList() {
    this->field_4 = head;
    head = this;
}

tlInstanceBank::Node * nalInitList::Register() {
    tlInstanceBank::Node * (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    return func(this);
}

nalInitListAnimType::nalInitListAnimType(const char *str,
        std::intptr_t anim_vtbl_ptr_arg,
        std::intptr_t skel_vtbl_ptr_arg)
    : nalInitList(), field_8(str), anim_vtbl_ptr(anim_vtbl_ptr_arg),
    skel_vtbl_ptr(skel_vtbl_ptr_arg)
{
    static vtbl g_vtbl = {
        func_address(&_Register)
    };

    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
}

tlInstanceBank::Node *nalInitListAnimType::_Register() {
    sp_log("0x%08X %s", field_8.m_hash, field_8.to_string());

    return nalTypeInstanceBank.Insert(this->field_8, this);
}

nalComponentInitList::nalComponentInitList(const char *str, void *a3)
    : nalInitList(), field_8(str), field_C(a3)
{
    if constexpr (1) {
        static vtbl g_vtbl = {
            func_address(&_Register)
        };

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00880958;
    }
}

tlInstanceBank::Node * nalComponentInitList::_Register() {
    TRACE("nalComponentInitList::_Register");

    tlFixedString v3 {this->field_8};

    //sp_log("%d %s", v3.m_hash, v3.to_string());

    return nalComponentInstanceBank.Insert(v3, this->field_C);
}

void nalInitListInit()
{
    TRACE("nalInitListInit");

    for (nalInitList *v0 = static_cast<decltype(v0)>(nalInitList::head);
            v0 != nullptr;
            v0 = static_cast<decltype(v0)>(v0->field_4)
        )
    {
        v0->Register();
    }
}

void sub_8530C0()
{
#if !STANDALONE_SYSTEM
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalCharAnim {
            "Character",
            nalChar::nalCharAnim::vtbl_ptr,
            nalChar::nalCharSkeleton::vtbl_ptr
    };
#endif
}

void sub_85E010()
{
#if !STANDALONE_SYSTEM
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalPanelAnim {
            "Panel",
            nalPanel::nalPanelAnim::vtbl_ptr,
            nalPanel::nalPanelSkeleton::vtbl_ptr
    };
#endif
}

void sub_864790()
{
#if !STANDALONE_SYSTEM
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalGenericAnim {
            "generic",
            nalGeneric::nalGenericAnim::vtbl_ptr,
            nalGeneric::nalGenericSkeleton::vtbl_ptr
    };
#endif
}

void sub_8531A0()
{
#if !STANDALONE_SYSTEM
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalPedAnim {
            "Ped",
            nalPed::nalPedAnim::vtbl_ptr,
            nalPed::nalPedSkeleton::vtbl_ptr
    };
#endif
}

void sub_853260()
{
#if !STANDALONE_SYSTEM
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalCamAnim {
            "Camera",
            nalCam::nalCamAnim::vtbl_ptr,
            nalCam::nalCamSkeleton::vtbl_ptr
    };
#endif
}

void nalInitList_patch()
{
    //static variables
    {
        SET_JUMP(0x008530C0, sub_8530C0);

        SET_JUMP(0x0085E010, sub_85E010);

        SET_JUMP(0x00864790, sub_864790);

        SET_JUMP(0x008531A0, sub_8531A0);

        SET_JUMP(0x00853260, sub_853260);
    }

    REDIRECT(0x00783E28, nalInitListInit);
}
