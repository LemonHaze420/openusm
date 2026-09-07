#include "sxcc.h"
#include "string_hash.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <limits>

namespace sxcc {

bytes_t read_file(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw error_t("Cannot open " + path);
    auto length = file.tellg();
    if (length < 0 || static_cast<std::uint64_t>(length) > UINT32_MAX)
        throw error_t("Invalid or oversized input: " + path);
    bytes_t bytes(static_cast<std::size_t>(length));
    file.seekg(0);
    if (!bytes.empty() && !file.read(reinterpret_cast<char *>(bytes.data()), length))
        throw error_t("Cannot read " + path);
    return bytes;
}

void write_file(const std::string &path, std::span<const std::uint8_t> bytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file || (!bytes.empty() && !file.write(reinterpret_cast<const char *>(bytes.data()),
                                               static_cast<std::streamsize>(bytes.size()))))
        throw error_t("Cannot write " + path);
    file.close();
    if (!file)
        throw error_t("Cannot finish " + path);
}

std::string hex(std::uint32_t value)
{
    char buffer[11] = "0x";
    auto result = std::to_chars(buffer + 2, buffer + sizeof(buffer), value, 16);
    return std::string(buffer, result.ptr);
}

std::string quote(std::string_view text)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char c : text) {
        switch (c) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 32 || c >= 127) {
                result += "\\x";
                result += digits[c >> 4];
                result += digits[c & 15];
            } else {
                result += static_cast<char>(c);
            }
        }
    }
    result += '"';
    return result;
}

std::string encode_bytes(std::span<const std::uint8_t> bytes)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(bytes.size() * 2);
    for (auto byte : bytes) {
        result += digits[byte >> 4];
        result += digits[byte & 15];
    }
    return result;
}

bytes_t decode_bytes(std::string_view text)
{
    if (text.size() % 2 != 0)
        throw error_t("Hex byte string must have an even length");
    auto digit = [](char c) -> unsigned {
        if (c >= '0' && c <= '9') return static_cast<unsigned>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<unsigned>(c - 'A' + 10);
        throw error_t("Invalid hex byte string");
    };
    bytes_t result;
    result.reserve(text.size() / 2);
    for (std::size_t i = 0; i < text.size(); i += 2)
        result.push_back(static_cast<std::uint8_t>((digit(text[i]) << 4) | digit(text[i + 1])));
    return result;
}

std::uint32_t name_hash(std::string_view text)
{
    return to_hash(std::string(text).c_str());
}

std::uint32_t read_word(std::span<const std::uint8_t> bytes, std::size_t offset, unsigned size)
{
    if (size > 4 || offset > bytes.size() || size > bytes.size() - offset)
        throw error_t("Truncated serialized field at " + hex(static_cast<std::uint32_t>(offset)));
    std::uint32_t value = 0;
    for (unsigned i = 0; i < size; ++i)
        value |= static_cast<std::uint32_t>(bytes[offset + i]) << (8 * i);
    return value;
}

void put_word(bytes_t &bytes, std::size_t offset, std::uint32_t value, unsigned size)
{
    if (size > 4 || offset > bytes.size() || size > bytes.size() - offset)
        throw error_t("Serialized field exceeds record");
    for (unsigned i = 0; i < size; ++i)
        bytes[offset + i] = static_cast<std::uint8_t>(value >> (8 * i));
}

static unsigned argument_size(opcode_arg_t argument)
{
    unsigned value = static_cast<unsigned>(argument);
    if (value >= std::size(opcode_arg_t_shift) || (value != 0 && opcode_arg_t_shift[value] == 0))
        throw error_t("Unsupported retail PC operand kind " + hex(value));
    return opcode_arg_t_shift[value] * 2;
}

unsigned instruction_t::size() const
{
    return 2 + (size_override ? 2 : 0) + argument_size(argument);
}

unsigned instruction_t::data_size() const
{
    return size_override.value_or(4);
}

std::string opcode_name(opcode_t opcode)
{
    auto value = static_cast<unsigned>(opcode);
    if (value >= std::size(opcode_t_str) || opcode_t_str[value][0] == '\0')
        throw error_t("Unsupported retail PC opcode " + hex(value));
    return opcode_t_str[value] + 3;
}

std::string argument_name(opcode_arg_t argument)
{
    argument_size(argument);
    return opcode_arg_t_str[static_cast<unsigned>(argument)] + 7;
}

opcode_t parse_opcode(std::string_view name)
{
    for (unsigned i = 0; i < std::size(opcode_t_str); ++i)
        if (opcode_t_str[i][0] != '\0' && name == opcode_t_str[i] + 3)
            return static_cast<opcode_t>(i);
    throw error_t("Unknown opcode: " + std::string(name));
}

opcode_arg_t parse_argument(std::string_view name)
{
    for (unsigned i = 0; i < std::size(opcode_arg_t_shift); ++i)
        if ((i == 0 || opcode_arg_t_shift[i] != 0) && name == opcode_arg_t_str[i] + 7)
            return static_cast<opcode_arg_t>(i);
    throw error_t("Unknown operand kind: " + std::string(name));
}

void validate_instruction(const instruction_t &instruction)
{
    opcode_name(instruction.opcode);
    argument_size(instruction.argument);
    if (instruction.argument == OP_ARG_NULL && instruction.value != 0)
        throw error_t("A no-operand instruction cannot encode a payload");
    if (is_branch(instruction.opcode) && instruction.argument != OP_ARG_PCR &&
        instruction.argument != OP_ARG_WORD && instruction.argument != OP_ARG_NUM && instruction.argument != OP_ARG_NUMR)
        throw error_t("Branch operand is not a fixed displacement");
    if (argument_size(instruction.argument) == 2 && instruction.value > UINT16_MAX)
        throw error_t("Word operand exceeds 16 bits");
}

std::vector<instruction_t> decode_code(std::span<const std::uint8_t> bytes)
{
    std::vector<instruction_t> code;
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        instruction_t instruction;
        instruction.offset = static_cast<std::uint32_t>(offset);
        auto word = read_word(bytes, offset, 2);
        offset += 2;
        instruction.opcode = static_cast<opcode_t>(word >> 8);
        instruction.argument = static_cast<opcode_arg_t>(word & OP_ARGTYPE_MASK);
        if ((word & OP_DSIZE_FLAG) != 0) {
            instruction.size_override = static_cast<std::uint16_t>(read_word(bytes, offset, 2));
            offset += 2;
        }
        auto width = argument_size(instruction.argument);
        if (width == 2)
            instruction.value = read_word(bytes, offset, 2);
        else if (width == 4)
            instruction.value = (read_word(bytes, offset, 2) << 16) | read_word(bytes, offset + 2, 2);
        offset += width;
        validate_instruction(instruction);
        code.push_back(instruction);
    }
    return code;
}

bytes_t encode_code(std::span<const instruction_t> code)
{
    bytes_t bytes;
    std::uint64_t total = 0;
    for (const auto &instruction : code) {
        validate_instruction(instruction);
        total += instruction.size();
    }
    if (total > UINT32_MAX)
        throw error_t("Bytecode exceeds retail address space");
    bytes.resize(static_cast<std::size_t>(total));
    std::size_t offset = 0;
    for (const auto &instruction : code) {
        auto word = (static_cast<unsigned>(instruction.opcode) << 8) |
                    static_cast<unsigned>(instruction.argument) |
                    (instruction.size_override ? OP_DSIZE_FLAG : 0);
        put_word(bytes, offset, word, 2);
        offset += 2;
        if (instruction.size_override) {
            put_word(bytes, offset, *instruction.size_override, 2);
            offset += 2;
        }
        auto width = argument_size(instruction.argument);
        if (width == 2)
            put_word(bytes, offset, instruction.value, 2);
        else if (width == 4) {
            put_word(bytes, offset, instruction.value >> 16, 2);
            put_word(bytes, offset + 2, instruction.value & UINT16_MAX, 2);
        }
        offset += width;
    }
    return bytes;
}

}
