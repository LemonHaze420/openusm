#include "ped_skel_pose.h"

#include "common.h"
#include "utility.h"

namespace nalPed {
int &nalPedSkeleton::vtbl_ptr = []() -> int & {
    static void *g_vtbl[]{
        nullptr, nullptr, func_address(&Process), func_address(&Release), func_address(&CheckVersion)};
    static int g_vtbl_ptr = bit_cast<int>(static_cast<void *>(g_vtbl));
    return g_vtbl_ptr;
}();

void nalPedSkeleton::Process()
{
    *bit_cast<nalPedSkeleton **>(bit_cast<char *>(this) + 0x60) = this;
}

void nalPedSkeleton::Release()
{
    *bit_cast<nalPedSkeleton **>(bit_cast<char *>(this) + 0x60) = nullptr;
}

bool nalPedSkeleton::CheckVersion() const
{
    return Version == 0x500;
}
}  // namespace nalPed
