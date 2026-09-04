#include "simple_classes.h"

#include "log.h"

#include <cassert>

bool sub_69B9D4(const int &a1, int *a2, int size, int *a4)
{
    bool v8 = false;
    int v7 = 0;
    int v6 = size;
    while (v7 < v6) {
        auto v5 = (v6 + v7) / 2;
        if (a2[v5] <= a1) {
            if (a2[v5] >= a1) {
                v8 = true;
                if (a4 != nullptr) {
                    *a4 = v5;
                }

                break;
            }
            v7 = v5 + 1;
        } else {
            v6 = (v6 + v7) / 2;
        }
    }

    if (!v8 && a4 != nullptr) {
        if (v6 == size - 1 && a2[v6] < a1) {
            ++v6;
        }

        *a4 = v6;
    }

    return v8;
}

void binary_search_array_unit_test()
{
    sp_log("Testing binary_search_array.");

    int v8[10];
    v8[0] = 2;
    v8[1] = 4;
    v8[2] = 6;
    v8[3] = 8;
    v8[4] = 10;
    v8[5] = 12;
    v8[6] = 14;
    v8[7] = 16;
    v8[8] = 18;
    v8[9] = 20;

    int find_me = -1;
    int found = -1;
    bool ret_val = false;
    sp_log("Array contents: ");
    for (int i = 0; i < 10; ++i) {
        sp_log("%d ", v8[i]);
    }

    printf("\n\n");
    for (find_me = -1; find_me < 24; ++find_me) {
        ret_val = sub_69B9D4(find_me, v8, 10, &found);
        bool should_have_found_it = (!(find_me % 2) && find_me > 0 && find_me <= 20);
        const char *v1 = (should_have_found_it ? "   should" : "shouldn't");

        if (ret_val) {
            sp_log("Looking for %2d, we %s it (when we %s have) at position %d\n", find_me, "      found", v1, found);
        } else {
            sp_log("Looking for %2d, we %s it (when we %s have) at position %d\n", find_me, "didn't find", v1, found);
        }

        assert(ret_val == should_have_found_it);
        assert(found == (find_me / 2) - 1);
    }

    int v2[1] = {5};
    sp_log("\nArray contents: ");
    for (int i = 0; i < 1; ++i) {
        sp_log("%d ", v2[i]);
    }

    sp_log("\n\n");
    for (find_me = 0; find_me < 10; ++find_me) {
        auto ret_val = sub_69B9D4(find_me, v2, 1, &found);
        if (ret_val) {
            sp_log("Looking for %2d, we %s it at position %d\n", find_me, "      found", found);
        } else {
            sp_log("Looking for %2d, we %s it at position %d\n", find_me, "didn't find", found);
        }

        if (find_me == 5) {
            assert(ret_val);
            assert(found == 0);
        } else {
            assert(!ret_val);
        }
    }
}
