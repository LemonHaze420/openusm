#include "chuck_str.h"

#include <cassert>
#include <cstdio>
#include <cstring>

unsigned int chuck_ftoa(Float a1, int precision, char *result, int result_size) {
    assert(result_size > 0);
    assert(result != nullptr);
    assert(precision >= 0 && precision < 10);

    char v1[36];

    snprintf(v1, 30u, "%%.%df", precision);
    snprintf(result, result_size, v1, a1);
    result[result_size - 1] = 0;
    return strlen(result);
}

int chuck_itoa(int a1, char *a2, int result_size) {
    int output_pos = 0;
    auto v7 = a1;
    if (a1 < 0) {
        v7 = -a1;
    }

    do {
        assert(output_pos < result_size - 1);
        a2[output_pos++] = v7 % 10 + '0';
        v7 /= 10;
    } while (v7);

    if (a1 < 0) {
        assert(output_pos < result_size - 1);
        a2[output_pos++] = '-';
    }

    auto *v6 = &a2[output_pos - 1];
    for (auto i = a2; i < v6; ++i) {
        auto v4 = *i;
        *i = *v6;
        *v6-- = v4;
    }

    assert(output_pos > 0 && output_pos < result_size);
    a2[output_pos] = 0;
    return output_pos;
}

static constexpr auto RIDICULOUS_SIZE = 4096;

char *chuck_strcpy(char *dest, const char *src, unsigned int dest_buffer_length) {
    assert(dest_buffer_length > 0);
    assert(dest_buffer_length <= RIDICULOUS_SIZE);

    for (auto i = 0u; i < dest_buffer_length - 1; ++i) {
        dest[i] = src[i];
        if (!src[i]) {
            break;
        }
    }

    dest[dest_buffer_length - 1] = '\0';
    return dest;
}

char *chuck_strcat(char *dest, const char *src, unsigned int dest_buffer_length) {
    assert(dest_buffer_length > 0);
    assert(dest_buffer_length <= RIDICULOUS_SIZE);
    assert(strlen(dest) + strlen(src) < dest_buffer_length - 1);

    strncat(dest, src, dest_buffer_length);
    dest[dest_buffer_length - 1] = 0;
    return dest;
}
