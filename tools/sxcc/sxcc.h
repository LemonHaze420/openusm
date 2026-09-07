#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstddef>
#include <cstdint>
#include <array>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <optional>
#include <iosfwd>
#include "opcodes.h"

namespace sxcc {

using bytes_t = std::vector<std::uint8_t>;

struct error_t : std::runtime_error {
    using std::runtime_error::runtime_error;
};

constexpr bool is_branch(opcode_t opcode)
{
    return opcode == OP_BF || opcode == OP_BRA || opcode == OP_FEQZB;
}

struct instruction_t {
    opcode_t opcode = OP_NOP;
    opcode_arg_t argument = OP_ARG_NULL;
    std::uint32_t value = 0;
    std::optional<std::uint16_t> size_override;
    std::uint32_t offset = 0;

    unsigned size() const;
    unsigned data_size() const;
};




struct function_t {
    std::string name;
    std::uint32_t name_hash = 0;
    std::uint32_t fullname_hash = 0;
    std::uint32_t parameters_size = 0;
    std::uint32_t flags = 4;
    std::uint32_t code_order = 0;
    bytes_t preserved;
    std::vector<instruction_t> code;
};

struct object_t {
    std::string name;
    std::uint32_t name_hash = 0;
    std::uint32_t instance_size = 0;
    std::int32_t destructor = -1;
    std::uint32_t flags = 2;
    bytes_t preserved;
    bytes_t static_data;
    bytes_t function_slots;
    std::vector<function_t> functions;
    std::optional<std::string> parent;
    std::string parent_evidence;
};

struct initializer_t {
    std::uint32_t name_hash = 0;
    std::uint32_t offset = 0;
    std::int32_t kind = -3;
    std::uint32_t value = 0;
    std::uint32_t object = 0;
    bytes_t preserved;
    std::optional<function_t> builder;
};

struct image_t {
    std::string name;
    bytes_t mash_preserved;
    bytes_t preserved;
    bytes_t object_slots;
    bytes_t string_slots;
    std::vector<object_t> objects;
    std::vector<std::uint32_t> object_order;
    std::vector<bytes_t> strings;
    std::vector<initializer_t> initializers;


    std::vector<bytes_t> padding;
};

struct native_t {
    unsigned class_id = 0;
    unsigned function_id = 0;
    std::string class_name;
    std::string signature;
    std::optional<std::string> result;
    std::vector<std::string> parameters;
    unsigned parameter_bytes = 0;
    std::optional<unsigned> result_bytes;
    std::uint32_t implementation = 0;
    bool variadic = false;
};

struct symbols_t {
    std::map<std::uint32_t, std::string> names;
    std::map<std::pair<unsigned, unsigned>, native_t> natives;
    std::map<std::string, unsigned> type_sizes;
    std::map<std::string, unsigned> class_ids;
    std::map<std::string, std::uint32_t> type_evidence;
    std::map<std::string, std::string> parents;
    std::map<std::string, std::pair<unsigned, unsigned>> variables;
};

struct block_t {
    unsigned begin = 0;
    unsigned end = 0;
    std::vector<unsigned> successors;
    std::vector<unsigned> predecessors;
};

struct function_analysis_t {
    std::vector<block_t> blocks;
    std::map<unsigned, unsigned> block_at;
    std::vector<std::string> diagnostics;
};

bytes_t read_file(const std::string &path);
void write_file(const std::string &path, std::span<const std::uint8_t> bytes);
std::string hex(std::uint32_t value);
std::string quote(std::string_view text);
std::string encode_bytes(std::span<const std::uint8_t> bytes);
bytes_t decode_bytes(std::string_view text);
std::uint32_t name_hash(std::string_view text);
std::uint32_t read_word(std::span<const std::uint8_t> bytes, std::size_t offset, unsigned size = 4);
void put_word(bytes_t &bytes, std::size_t offset, std::uint32_t value, unsigned size = 4);

std::vector<instruction_t> decode_code(std::span<const std::uint8_t> bytes);
bytes_t encode_code(std::span<const instruction_t> code);
void validate_instruction(const instruction_t &instruction);
std::string opcode_name(opcode_t opcode);
std::string argument_name(opcode_arg_t argument);
opcode_t parse_opcode(std::string_view name);
opcode_arg_t parse_argument(std::string_view name);
image_t decode_image(std::span<const std::uint8_t> bytes);
bytes_t encode_image(const image_t &image);
void validate_image(const image_t &image);
void resolve_symbols(image_t &image, const symbols_t &symbols);
symbols_t load_symbols(const std::string &dictionary);
function_analysis_t analyze_function(const function_t &function);
std::string emit_assembly(const image_t &image);
image_t assemble_source(std::string_view source);
void emit_source_header(std::ostream &out, const image_t &image);
void emit_source_record(std::ostream &out, const bytes_t &record, std::string_view name, std::string_view indent);
std::string emit_decompiled_source(const image_t &image, const symbols_t &symbols);

}
