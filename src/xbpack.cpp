#include "xbpack.h"

#ifdef OPENUSM_XBPACK_MODE

#include "actor_xbpack.h"
#include "ai_interact_resource_handler.h"
#include "ai_state_graph_resource_handler.h"
#include "als_resource_handler.h"
#include "base_ai_resource_handler.h"
#include "combo_system_move.h"
#include "conglom.h"
#include "gab_database_resource_handler.h"
#include "log.h"
#include "mash_virtual_base.h"
#include "ngl.h"
#include "panelfile.h"
#include "panelmeshsection.h"
#include "resource_directory.h"
#include "resource_manager.h"
#include "resource_versions.h"
#include "script_manager.h"
#include "slc_manager.h"
#include "sound_alias_database_resource_handler.h"
#include "variables.h"

#include <cstdint>

namespace
{
constexpr uintptr_t PLATFORM_INITIALIZER = 0x005E10BB;
constexpr uintptr_t EXPECTED_RESOURCE_VERSIONS = 0x00937440;
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
    als_resource_handler_xbpack_patch();
    gab_database_resource_handler_patch();
    sound_alias_database_resource_handler_xbpack_patch();
    slc_manager_patch();
    script_manager_xbpack_patch();
    base_ai_resource_handler_xbpack_patch();
    ai_state_graph_resource_handler_xbpack_patch();
    ai_interact_resource_handler_xbpack_patch();
    mash_virtual_base_xbpack_patch();
    combo_system_move_patch();
    conglomerate_xbpack_patch();
    actor_xbpack_patch();
#endif

    return true;
}
