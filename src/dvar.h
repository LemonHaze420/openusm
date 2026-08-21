#pragma once

#include "mstring.h"

#include <map>

struct debug_variable_t {
    mString field_0;

    debug_variable_t(const char *a1, float a2);

    void add_value(const mString &a1);

    mString &get_value() const;

    operator float() const;
};

extern std::map<mString, mString> g_dvars;
