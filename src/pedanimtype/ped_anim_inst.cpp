#include "ped_anim_inst.h"

#include "common.h"
#include "utility.h"

namespace nalPed {
int &nalPedAnim::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{
        nullptr, func_address(&Process), func_address(&Release), func_address(&CheckVersion), nullptr};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

void nalPedAnim::Process()
{
    auto *base = bit_cast<char *>(this);
    auto &has_data = *bit_cast<int *>(base + 0x64);
    auto &data = *bit_cast<void **>(base + 0x68);
    auto &end = *bit_cast<char **>(base + 0x6C);
    data = has_data != 0 ? base + 0x70 : nullptr;
    end = base + 0x70 + bit_cast<std::intptr_t>(end);
}

void nalPedAnim::Release()
{
    auto *base = bit_cast<char *>(this);
    auto &data = *bit_cast<void **>(base + 0x68);
    auto &end = *bit_cast<char **>(base + 0x6C);
    end -= bit_cast<std::intptr_t>(base + 0x70);
    data = nullptr;
}

bool nalPedAnim::CheckVersion() const
{
    return Version == 0x500;
}
}  // namespace nalPed
