#pragma once

#include "semantic.h"

namespace sxcc {

struct value_ref_t {
    unsigned value = 0;
    unsigned offset = 0;
    bool operator==(const value_ref_t &) const = default;
};

struct lifted_address_t {
    enum class kind_t { local, shared, instance, game_variable, shared_variable } kind = kind_t::local;
    unsigned owner = 0;
    unsigned offset = 0;
    unsigned width = 4;
    std::optional<value_ref_t> receiver;
    std::optional<value_ref_t> index;
};
struct lifted_value_t {
    enum class kind_t {
        parameter, uninitialized, merge, constant, string, class_value,
        signal, operation, native_call, script_call, load, indexed_store
    } kind = kind_t::uninitialized;
    unsigned instruction = UINT32_MAX;
    unsigned width = 4;
    std::string type;
    std::uint32_t immediate = 0;
    std::vector<std::vector<value_ref_t>> arguments;
    std::optional<lifted_address_t> address;
};


struct lifted_effect_t {
    enum class kind_t { value, store, branch, return_value, thread, callback, signal, termination } kind = kind_t::value;
    unsigned instruction = 0;
    unsigned value = UINT32_MAX;
    std::optional<lifted_address_t> address;
    std::vector<std::vector<value_ref_t>> arguments;
};

struct lifted_block_t {
    unsigned begin = 0;
    unsigned end = 0;
    std::vector<unsigned> successors;
    std::vector<value_ref_t> incoming;
    std::vector<value_ref_t> outgoing;
    std::vector<lifted_effect_t> effects;
    std::optional<value_ref_t> incoming_array_index;
    std::optional<value_ref_t> outgoing_array_index;
    value_ref_t incoming_signal_parent;
    value_ref_t outgoing_signal_parent;
};

struct lifted_function_t {
    unsigned object = 0;
    unsigned function = 0;
    bool initializer = false;
    std::vector<lifted_value_t> values;
    std::vector<lifted_block_t> blocks;
    std::vector<std::string> diagnostics;
};



std::vector<lifted_function_t> lift_values(const image_t &image, const symbols_t &symbols,
                                         const stack_analysis_t &stack);

}
