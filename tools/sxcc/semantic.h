#pragma once

#include "sxcc.h"

namespace sxcc {

struct stack_function_t {
    unsigned object = 0;
    unsigned function = 0;
    bool initializer = false;
    std::optional<unsigned> result_bytes;
    std::vector<std::optional<int>> heights;
    std::vector<std::string> diagnostics;


    bool discards_exit_stack = false;
    std::vector<std::optional<unsigned>> array_widths = {};
    bool noreturn = false;
    unsigned storage_bytes = 0;
    function_analysis_t control_flow = {};
};

struct stack_analysis_t {
    std::vector<stack_function_t> functions;
    std::map<std::pair<unsigned, unsigned>, unsigned> native_results;
};



stack_analysis_t analyze_stack(const image_t &image, const symbols_t &symbols);

}
