#pragma once

#include "sxcc.h"

#include <memory>

namespace sxcc::language {

enum class access_t { public_member, protected_member, private_member };

struct expression_t {
    enum class kind_t {
        number, string, name, member, index, call, unary, binary,
        assignment, conditional, postfix, bit_cast, aggregate
    } kind = kind_t::name;
    std::string text;
    std::uint32_t bits = 0;
    unsigned line = 0;
    std::vector<std::unique_ptr<expression_t>> operands;
};

struct declaration_t {
    std::string type;
    std::string name;
    unsigned count = 1;
    bool is_array = false;
    std::optional<unsigned> offset;
    bool is_static = false;
    access_t access = access_t::private_member;
};

struct statement_t {
    enum class kind_t {
        block, declaration, expression, conditional, while_loop, do_loop,
        for_loop, return_value, break_loop, continue_loop, jump, label
    } kind = kind_t::block;
    unsigned line = 0;
    std::string name;
    declaration_t declaration;
    std::vector<std::unique_ptr<expression_t>> expressions;
    std::vector<std::unique_ptr<statement_t>> children;
};

struct function_t {
    unsigned index = 0;
    std::string name;
    std::string result = "void";
    std::vector<declaration_t> parameters;
    std::vector<std::unique_ptr<expression_t>> base_arguments;
    std::unique_ptr<statement_t> body;
    bool constructor = false;
    bool entry = false;
    bool destructor = false;
    access_t access = access_t::private_member;
};

struct class_t {
    unsigned index = 0;
    std::vector<declaration_t> fields;
    std::vector<function_t> functions;
    std::vector<std::pair<std::string, std::unique_ptr<expression_t>>> field_initializers;
};

struct initialization_t {
    std::optional<std::string> object_name;
    std::optional<function_t> builder;
};


struct program_t {
    image_t image;
    std::vector<class_t> classes;
    std::vector<native_t> natives;
    std::map<std::string, declaration_t> native_variables;
    std::vector<initialization_t> initializations;
};

program_t parse(std::string_view source);
image_t lower(program_t program, const symbols_t &symbols);

}
