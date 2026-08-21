#include "als_post_kill_rule.h"
#include "common.h"

namespace als {
VALIDATE_SIZE(post_kill_rule, 0x8);
void post_kill_rule::unmash(mash_info_struct *, void *) {}
}  // namespace als
