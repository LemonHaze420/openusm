#pragma once

#include "config.h"
#include "log.h"
#include "resource_key.h"

struct resource_pack_location;
struct resource_key;

template <typename T0, typename T1>
int compare_deref(T0 &, T1 &);

//0x0055DEA0
extern int compare_resource_key_resource_pack_location(const resource_key &a1, const resource_pack_location &a2);

template <typename T0, typename T1>
bool binary_search_array_cmp(T0 *a1, T1 *a2, int start_index, int count, int *p_index, int (*fn_compare)(T0 &, T1 &))
{
    //sp_log("%d", count);
    bool result = false;
    while (start_index < count) {
        int mid_index = (start_index + count) / 2;
        int res_comp = fn_compare(*a1, a2[mid_index]);
        if (res_comp > -1) {
            if (res_comp < 1) {
                result = true;

                if (p_index != nullptr) {
                    *p_index = mid_index;
                }

                break;
            }

            start_index = mid_index + 1;
        } else {
            count = (count + start_index) / 2;
        }
    }

    if (!result && p_index != nullptr) {
        *p_index = count;
    }

    return result;
}
