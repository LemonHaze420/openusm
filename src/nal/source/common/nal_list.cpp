#include <nal_list.h>

#include <common.h>
#include <log.h>
#include <trace.h>
#include <vtbl.h>

#include <character_anim_inst.h>
#include <character_pose_skel.h>
#include <panel_pose_skel.h>
#include <panel_anim_inst.h>
#include <ped_skel_pose.h>
#include <ped_anim_inst.h>

#include <nal_system.h>

VALIDATE_SIZE(nalInitListAnimType, 0x30);

void * & nalInitList::head = var<void *>(0x00977154);

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
    static vtbl g_vtbl = {
        func_address(&_Register)
    };

    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
}

tlInstanceBank::Node *nalComponentInitList::_Register() {
    tlFixedString v3{this->field_8};

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
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalCharAnim {
            "Character",
            nalChar::nalCharAnim::vtbl_ptr,
            nalChar::nalCharSkeleton::vtbl_ptr
    };
}

void sub_85E010()
{
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalPanelAnim {
            "Panel",
            nalPanel::nalPanelAnim::vtbl_ptr,
            nalPanel::nalPanelSkeleton::vtbl_ptr
    };
}

void sub_864790()
{
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalGenericAnim {
            "generic",
            nalGeneric::nalGenericAnim::vtbl_ptr,
            nalGeneric::nalGenericSkeleton::vtbl_ptr
    };
}

void sub_8531A0()
{
    [[maybe_unused]] static nalInitListAnimType InitListAnimType_nalPedAnim {
            "Ped",
            nalPed::nalPedAnim::vtbl_ptr,
            nalPed::nalPedSkeleton::vtbl_ptr
    };
}

void nalInitList_patch()
{
    SET_JUMP(0x008530C0, sub_8530C0);

    SET_JUMP(0x0085E010, sub_85E010);

    SET_JUMP(0x00864790, sub_864790);

    REDIRECT(0x00783E28, nalInitListInit);
}
