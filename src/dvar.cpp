#include "dvar.h"

#include <cassert>

std::map<mString, mString> g_dvars{};

debug_variable_t::debug_variable_t(const char *a1, float a2) : field_0(a1)
{
    mString val{a2};
    this->add_value(val);
}

void debug_variable_t::add_value(const mString &a1)
{
    auto it = g_dvars.find(this->field_0);
    if (it == g_dvars.end()) {
        std::pair<mString, mString> v3{this->field_0, a1};
        g_dvars.insert(v3);
    }
}

mString &debug_variable_t::get_value() const
{
    auto a1 = g_dvars.find(this->field_0);

    auto end = g_dvars.end();
    if (a1 == end) {
        assert("Uninitialized dvar" && 0);
    }

    return a1->second;
}

debug_variable_t::operator float() const
{
    auto &v1 = this->get_value();
    return v1.to_float();
}
