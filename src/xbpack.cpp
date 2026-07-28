#include "xbpack.h"

#ifdef OPENUSM_XBPACK_MODE

#include "actor_xbpack.h"
#include "ai_interact_resource_handler.h"
#include "ai_state_graph_resource_handler.h"
#include "als_resource_handler.h"
#include "base_ai_resource_handler.h"
#include "combo_system.h"
#include "combo_system_move.h"
#include "colmesh.h"
#include "conglom.h"
#include "core_ai_resource.h"
#include "gab_database_resource_handler.h"
#include "log.h"
#include "mash_virtual_base.h"
#include "ngl.h"
#include "panelfile.h"
#include "panelmeshsection.h"
#include "parse_generic_mash.h"
#include "resource_directory.h"
#include "resource_manager.h"
#include "resource_versions.h"
#include "script_manager.h"
#include "script_lib_debug_menu.h"
#include "slc_manager.h"
#include "sound_alias_database_resource_handler.h"
#include "utility.h"
#include "variables.h"
#include "wds.h"

#ifdef OPENUSM_XBPACK_V10
#include <nal_generic.h>
#include <nal_skeleton.h>
#endif

#include <cstdint>
#include <cstring>

namespace
{
constexpr uintptr_t PLATFORM_INITIALIZER = 0x005E10BB;
constexpr uintptr_t EXPECTED_RESOURCE_VERSIONS = 0x00937440;

#ifdef OPENUSM_XBPACK_V10
constexpr uintptr_t PO_VTABLES[] = {
    0x008BDA58,
    0x008BDAB8,
    0x008BDB18,
    0x008BDB78,
};

constexpr uintptr_t PO_ALIGN = 0x10;
constexpr uintptr_t PO_SIZE = 0x20;

uint8_t *align_po(void *data)
{
    auto address = reinterpret_cast<uintptr_t>(data);
    address = (address + PO_ALIGN - 1) & ~(PO_ALIGN - 1);
    return reinterpret_cast<uint8_t *>(address);
}

int __fastcall po_size(void *, void *)
{
    return PO_SIZE;
}

void __fastcall layout_po(
    void *, void *, const nalGeneric::nalComponentInfo *info,
    void **cursor, void **)
{
    *cursor = align_po(*cursor) + PO_SIZE * info->field_28;
}

void __fastcall advance_po(
    void *, void *, const nalGeneric::nalComponentInfo *info,
    void **cursor)
{
    *cursor = align_po(*cursor) + PO_SIZE * info->field_28;
}

void __fastcall copy_po(
    void *, void *, const nalGeneric::nalComponentInfo *info,
    void **dst_cursor, const void **src_cursor)
{
    auto *dst = align_po(*dst_cursor);
    auto *src = align_po(const_cast<void *>(*src_cursor));
    auto size = PO_SIZE * info->field_28;

    std::memcpy(dst, src, size);
    *dst_cursor = dst + size;
    *src_cursor = src + size;
}

void __fastcall init_signal_pose(
    void *, void *, const nalGeneric::nalComponentInfo *info,
    void **cursor)
{
    auto *data = static_cast<uint8_t *>(*cursor);

    for (int i = 0; i < info->field_28; ++i) {
        *data++ = 0;
    }

    *cursor = data;
}
#endif
}

#endif

bool install_xbpack_support()
{
#ifdef OPENUSM_XBPACK_MODE
    auto *platform = reinterpret_cast<uint8_t *>(PLATFORM_INITIALIZER);
    if (*platform != NL_PLATFORM_PC && *platform != NL_PLATFORM_XBOX) {
        sp_log("Unexpected g_platform initializer at 0x%08X: 0x%02X",
               static_cast<unsigned>(PLATFORM_INITIALIZER),
               *platform);
        return false;
    }

    *platform = NL_PLATFORM_XBOX;
    g_platform = NL_PLATFORM_XBOX;
    *reinterpret_cast<resource_versions *>(EXPECTED_RESOURCE_VERSIONS) =
        expected_resource_versions(NL_PLATFORM_XBOX);

    resource_manager_xbpack_patch();
    resource_directory_xbpack_patch();
    ngl_xbpack_patch();
    PanelMeshSection_patch();
    PanelFile_xbpack_patch();
    slc_manager_patch();
    script_manager_xbpack_patch();
    wds_xbpack_patch();

    if constexpr (xbpack::v10) {
#ifdef OPENUSM_XBPACK_V10
        for (auto vtable : PO_VTABLES) {
            set_vfunc(vtable + 0x04, &po_size);
            set_vfunc(vtable + 0x10, &layout_po);
            set_vfunc(vtable + 0x2C, &advance_po);
            set_vfunc(vtable + 0x30, &advance_po);
            set_vfunc(vtable + 0x34, &copy_po);
        }

        set_vfunc(0x00881284, &init_signal_pose);
        SET_JUMP(0x005F6DA0, inverse_kinematics::LegsIK_BuildBoneMatrices_v10);
#endif

        *reinterpret_cast<uint8_t *>(0x0053D010) = 0xE9;
        *reinterpret_cast<uint32_t *>(0x0053D011) = 0x0053D02F - 0x0053D015;

        construct_debug_menu_lib();
        REDIRECT(0x00509DAC, parse_generic_mash_init);
        cg_mesh_patch();
        combo_system_patch();
        core_ai_resource_patch();
    }

    als_resource_handler_xbpack_patch();
    ai_state_graph_resource_handler_xbpack_patch();
    mash_virtual_base_xbpack_patch();
    combo_system_move_patch();
    conglomerate_xbpack_patch();
    actor_xbpack_patch();

    if constexpr (!xbpack::v10) {
        gab_database_resource_handler_patch();
        sound_alias_database_resource_handler_xbpack_patch();
        base_ai_resource_handler_xbpack_patch();
        ai_interact_resource_handler_xbpack_patch();
    }
#endif

    return true;
}
