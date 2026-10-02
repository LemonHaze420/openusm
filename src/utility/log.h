#pragma once

#if __has_include(<source_location>)
#include <source_location>
using log_source_location = std::source_location;
#else
#include <experimental/source_location>
using log_source_location = std::experimental::source_location;
#endif
#include <string_view>

#include <cstdarg>
#include <cstdio>
#include <iostream>

void __log(const char *file, int line, const char *format, ...);

#define sp_log(fmt, ...)                                                                     \
    {                                                                                        \
        constexpr std::string_view file_name = log_source_location::current().file_name();  \
                                                                                             \
        __log(file_name.data(), log_source_location::current().line(), fmt, ##__VA_ARGS__); \
    }

inline void __log(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("[%s:%d] ", file, line);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}
