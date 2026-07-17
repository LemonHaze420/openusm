#pragma once

// ... for now, while this madness is still ongoing ...
// load v14 is possible but need verify v10+v14
// v10 is ?
// v14 + pc should be possible

#if defined(TARGET_XBOX) || defined(OPENUSM_XBPACK_MODE)
#define OPENUSM_XBOX_MASH_FORMAT 1
#else
#define OPENUSM_XBOX_MASH_FORMAT 0
#endif
