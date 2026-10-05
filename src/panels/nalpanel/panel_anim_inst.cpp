#include "panel_anim_inst.h"

#include "common.h"
#include "utility.h"

namespace nalPanel {
int &nalPanelAnim::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{nullptr,
                          func_address(&nalPanelAnim::Process),
                          func_address(&nalPanelAnim::Release),
                          func_address(&nalPanelAnim::CheckVersion),
                          nullptr,
                          func_address(&nalComp::nalCompAnim::_GetPerAnimDataFromComponentIx),
                          func_address(&nalComp::nalCompAnim::_GetPerAnimUserDataInt),
                          func_address(&nalComp::nalCompAnim::_UnMash),
                          func_address(&nalComp::nalCompAnim::_ReMash)};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

void nalPanelAnim::Process()
{
    _UnMash(this);
}

void nalPanelAnim::Release()
{
    _ReMash(this);
}

bool nalPanelAnim::CheckVersion() const
{
    return Version == 0x300;
}
}  // namespace nalPanel
