#pragma once

#include <algorithm>
#include <set>
#include <string>
#include <string_view>

namespace sxcc::pretty {

inline std::string entity_local_name(std::string_view path, std::set<std::string> &occupied)
{
    const auto separator = path.find_last_of("\\/");
    std::string candidate(path.substr(separator == path.npos ? 0 : separator + 1));
    static constexpr std::string_view keywords[]{
        "alignas",     "alignof",   "and",        "and_eq",    "asm",      "auto",         "bitand",
        "bitor",       "bool",      "break",      "case",      "catch",    "char",         "char8_t",
        "char16_t",    "char32_t",  "class",      "compl",     "concept",  "const",        "consteval",
        "constexpr",   "constinit", "const_cast", "continue",  "co_await", "co_return",    "co_yield",
        "decltype",    "default",   "delete",     "do",        "double",   "dynamic_cast", "else",
        "enum",        "explicit",  "export",     "extern",    "false",    "float",        "for",
        "friend",      "goto",      "if",         "inline",    "int",      "long",         "mutable",
        "namespace",   "new",       "noexcept",   "not",       "not_eq",   "nullptr",      "operator",
        "or",          "or_eq",     "private",    "protected", "public",   "register",     "reinterpret_cast",
        "requires",    "return",    "short",      "signed",    "sizeof",   "static",       "static_assert",
        "static_cast", "struct",    "switch",     "template",  "this",     "thread_local", "throw",
        "true",        "try",       "typedef",    "typeid",    "typename", "union",        "unsigned",
        "using",       "virtual",   "void",       "volatile",  "wchar_t",  "while",        "xor",
        "xor_eq",      "bit_cast",  "thread",     "callback"};
    const auto letter = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    };
    const bool valid =
        !candidate.empty() && letter(candidate.front()) && std::all_of(candidate.begin(), candidate.end(), [&](char c) {
            return letter(c) || (c >= '0' && c <= '9') || c == '_';
        });

    const bool edge =
        candidate.starts_with("edge") && candidate.size() > 4 && candidate[4] >= '0' && candidate[4] <= '9';
    if (!valid || candidate.find("__") != candidate.npos || edge || occupied.count(candidate) ||
        std::find(std::begin(keywords), std::end(keywords), candidate) != std::end(keywords)) {
        unsigned fallback = 0;
        do
            candidate = "entity_local_v" + std::to_string(fallback++);
        while (occupied.count(candidate));
    }
    occupied.insert(candidate);
    return candidate;
}

}
