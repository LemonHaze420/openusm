#include "debug_user_render.h"

#include "mstring.h"
#include "render_text.h"
#include "vector2di.h"
#include "vector3d.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

static constexpr auto NUM_DEBUG_STRINGS = 28;

struct ColorString {
    mString m_string;
    color32 m_color;
};

ColorString s_debug_strings[28]{};

void insertDebugString(int index, const mString &a1, color32 color)
{
    assert(index >= 0);

    assert(index < NUM_DEBUG_STRINGS);

    s_debug_strings[index].m_string = a1;
    s_debug_strings[index].m_color = color;
}

void sub_CB97D0(int arg0, int arg4, color32 color, float a4, float a5, const char *a2, ...)
{
    va_list va;
    va_start(va, a2);

    char dest[2048];

    memset(dest, 0, 2048);

    vsprintf(dest, a2, va);

    mString v10{dest};

    vector2di v7{arg0, arg4};
    render_text(v10, v7, color, a4, a5);

    va_end(va);
}

void renderDebugStrings()
{
    for (int i = 0; i < NUM_DEBUG_STRINGS; ++i) {
        if (!s_debug_strings[i].m_string.empty()) {
            const auto *v0 = s_debug_strings[i].m_string.c_str();
            sub_CB97D0(20, 16 * i + 16, s_debug_strings[i].m_color, LARGE_EPSILON, 1.0f, v0);
        }
    }
}
