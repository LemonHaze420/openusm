#pragma once

#include "lifting.h"

#include <tuple>

namespace sxcc {

struct recovered_parameter_t {
    unsigned offset = 0;
    unsigned width = 0;
    std::string type;
};

struct recovered_function_t {
    std::vector<recovered_parameter_t> parameters;
    std::string result;
};


using storage_key_t = std::tuple<unsigned, unsigned, unsigned, unsigned>;

struct recovered_types_t {
    std::vector<recovered_function_t> functions;
    std::vector<std::optional<std::string>> parents;


    std::map<storage_key_t, std::string> storage;
};


recovered_types_t recover_types(const image_t &image, const symbols_t &symbols, const stack_analysis_t &stack,
                                std::vector<lifted_function_t> &functions);

}
