#pragma once

template <typename T>
bool binary_search_array_deref(T *searcher, T **array, int a3, int *index)
{
    bool result = false;
    int v7 = 0;
    int v6 = a3;
    while (v7 < v6) {
        auto idx = (v6 + v7) / 2;
        if (*searcher < *array[idx]) {
            v6 = (v6 + v7) / 2;
        } else {
            if (!(*searcher > *array[idx])) {
                result = true;
                if (index != nullptr) {
                    *index = idx;
                }

                break;
            }

            v7 = idx + 1;
        }
    }

    if (!result && index != nullptr) {
        if (v6 == a3 - 1 && *searcher > *array[v6]) {
            ++v6;
        }

        *index = v6;
    }

    return result;
}
