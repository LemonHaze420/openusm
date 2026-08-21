#include "quick_anchor_info.h"

#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "log.h"
#include "utility.h"

#include <cassert>
#include <cmath>
#include <vector>

VALIDATE_SIZE(quick_anchor_info, 0x30);

bool quick_anchor_info::operator<(const quick_anchor_info &a2)
{
    if (std::isnan(a2.field_24)) {
        return true;
    }

    if (std::isnan(this->field_24)) {
        return false;
    }

    return std::isless(this->field_24, a2.field_24);
}

void sub_48C340(quick_anchor_info *a1, quick_anchor_info *a2, quick_anchor_info *a3)
{
    CDECL_CALL(0x0048C340, a1, a2, a3);
}

void sub_48EFB0(quick_anchor_info *begin, quick_anchor_info *end)
{
    if constexpr (1) {
        if (begin != end) {
            auto *v2 = begin + 1;
            if (&begin[1] != end) {
                auto *v3 = begin + 2;
                do {
                    if (begin->field_24 < v3[-1].field_24) {
                        auto *v4 = v2;
                        for (auto *i = v2;; v4 = i) {
                            --i;

                            if (i->field_24 < v3[-1].field_24) {
                                break;
                            }
                        }

                        if (v4 != v2 && v2 != v3) {
                            sub_48C340(v4, v2, v3);
                        }

                    } else if (begin != v2 && v2 != v3) {
                        sub_48C340(begin, v2, v3);
                    }

                    ++v2;
                    ++v3;
                } while (v2 != end);
            }
        }

    } else {
        CDECL_CALL(0x0048EFB0, begin, end);
    }
}

void sub_48D790(quick_anchor_info *a1, int a2, int size, quick_anchor_info a4)
{
    CDECL_CALL(0x0048D790, a1, a2, size, a4);
}

void sub_48E470(quick_anchor_info *begin, quick_anchor_info *end)
{
    if constexpr (1) {
        auto size = end - begin;

        auto v2 = size / 2;
        if (v2 > 0) {
            quick_anchor_info tmp;

            for (auto *i = &begin[v2];; --i) {
                --v2;

                tmp = i[-1];

                sub_48D790(begin, v2, size, tmp);
                if (v2 <= 0) {
                    break;
                }
            }
        }

    } else {
        CDECL_CALL(0x0048E470, begin, end);
    }
}

void sub_48F6D0(quick_anchor_info *a1, quick_anchor_info *a2)
{
    CDECL_CALL(0x0048F6D0, a1, a2);
}

using pair_t = typename std::pair<quick_anchor_info *, quick_anchor_info *>;

pair_t *create_pair(pair_t *out, quick_anchor_info *a2, quick_anchor_info *a3)
{
    return (pair_t *)CDECL_CALL(0x0048EC10, out, a2, a3);
}

template <>
void sort<quick_anchor_info>(quick_anchor_info *begin, quick_anchor_info *end, int size)
{
    //sp_log("%d", size);

    assert(begin <= end);

    std::sort(begin, end);

    if constexpr (0) {
        for (int i = 0; i < size; ++i) {
            auto *it = begin + i;
            sp_log("%d %f", i, it->field_24);
        }
    }

    return;

    if constexpr (1) {
        int v5;

        pair_t pair;

        while (1) {
            v5 = end - begin;

            if (v5 <= 32 || size <= 0) {
                break;
            }

            create_pair(&pair, begin, end);
            size = size / 2 / 2 + size / 2;
            if ((pair.first - begin) >= end - pair.second) {
                sort(pair.second, end, size);
                end = pair.first;
            } else {
                sort(begin, pair.first, size);
                begin = pair.second;
            }
        }

        if (v5 <= 32) {
            if (v5 > 1) {
                sub_48EFB0(begin, end);
            }

        } else {
            if (end - begin > 1) {
                sub_48E470(begin, end);
            }

            sub_48F6D0(begin, end);
        }

    } else {
        CDECL_CALL(0x0048FBB0, begin, end, size);
    }
}

void quick_anchor_info_patch()
{
    {
        auto *address = sort<quick_anchor_info>;

        REDIRECT(0x00486B4F, address);
    }
}
