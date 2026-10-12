#include "sxcc.h"
#include "source_parser.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>

namespace sxcc {
namespace {

using table_key_t = std::vector<std::string>;

bytes_t default_record(assembly_record_t kind)
{
    auto bytes = assembly_record_defaults(kind);
    return {bytes.begin(), bytes.end()};
}

bool identifier(std::string_view name)
{
    auto letter = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    };
    return !name.empty() && letter(name.front()) &&
           std::all_of(name.begin() + 1, name.end(), [&](char c) { return letter(c) || (c >= '0' && c <= '9'); });
}

std::uint32_t inferred_parameters(const function_t &function)
{
    std::optional<std::uint32_t> size;
    for (const auto &instruction : function.code) {
        if (instruction.opcode != OP_RET)
            continue;
        if (instruction.argument != OP_ARG_WORD || (size && *size != instruction.value))
            return 0;
        size = instruction.value;
    }
    return size.value_or(0);
}

std::string table_name(const table_key_t &key)
{
    std::string result;
    std::size_t first = 0;
    if (key.front() == "object")
        first = 1;
    else if (key.front() == "initializer") {
        result = "initializers.entry_" + key[1];
        first = 2;
    }
    for (auto i = first; i < key.size(); ++i) {
        if (!result.empty())
            result += '.';
        bool reserved = key.front() == "object" && i == first && (key[i] == "image" || key[i] == "initializers");
        result += identifier(key[i]) && !reserved ? key[i] : quote(key[i]);
    }
    return result;
}

class parser_t : private detail::parser_base_t {
    struct fixup_t {
        unsigned index;
        std::string label;
    };
    struct function_labels_t {
        std::map<std::string, unsigned> labels;
        std::vector<fixup_t> fixups;
    };
    std::vector<function_labels_t> labels;
    std::map<table_key_t, bytes_t> tables;

    instruction_t instruction(std::optional<std::string> &branch_label)
    {
        instruction_t result;
        result.opcode = parse_opcode(token.text);
        advance();
        if (token.text != ";" && token.text != ",") {
            result.argument = parse_argument(token.text);
            advance();
            if (result.argument == OP_ARG_NUM || result.argument == OP_ARG_NUMR) {
                if (accept("bits")) {
                    expect("(");
                    result.value = integer();
                    expect(")");
                } else {
                    bool negative = accept("-");
                    char *end = nullptr;
                    errno = 0;
                    float value = std::strtof(token.text.c_str(), &end);
                    if (token.text.empty() || end != token.text.data() + token.text.size() || !std::isfinite(value) ||
                        (errno == ERANGE && std::fpclassify(value) == FP_ZERO))
                        fail("Invalid or out-of-range num literal; use bits() for explicit IEEE encodings");
                    if (negative)
                        value = -value;
                    std::memcpy(&result.value, &value, sizeof(value));
                    advance();
                }
            } else if ((result.argument == OP_ARG_PCR || result.argument == OP_ARG_WORD) && is_branch(result.opcode) &&
                       !token.quoted && identifier(token.text)) {
                branch_label = token.text;
                advance();
            } else {
                result.value = integer();
                if (result.argument == OP_ARG_WORD || result.argument == OP_ARG_PCR || result.argument == OP_ARG_SPR ||
                    result.argument == OP_ARG_POPO) {
                    if (result.value > UINT16_MAX && result.value < 0xffff8000u)
                        fail("Word operand exceeds 16 bits");
                    result.value &= UINT16_MAX;
                }
            }
        }
        if (accept(","))
            result.size_override = static_cast<std::uint16_t>(small_integer(UINT16_MAX));
        expect(";");
        validate_instruction(result);
        return result;
    }

    function_t function()
    {
        function_t result;
        result.name = string();
        result.fullname_hash = name_hash(result.name);
        result.name_hash = name_hash(result.name.substr(0, result.name.find('(')));
        result.code_order = static_cast<unsigned>(labels.size());
        result.preserved = default_record(assembly_record_t::function);
        labels.emplace_back();
        auto &pending = labels.back();
        expect("{");
        while (!accept("}")) {
            if (accept("label")) {
                if (token.quoted || !identifier(token.text))
                    fail("Expected label identifier");
                auto name = token.text;
                advance();
                expect(":");
                if (!pending.labels.emplace(name, static_cast<unsigned>(result.code.size())).second)
                    fail("Duplicate label");
                continue;
            }
            if (accept("goto")) {
                if (token.quoted || !identifier(token.text))
                    fail("Expected branch label");
                pending.fixups.push_back({static_cast<unsigned>(result.code.size()), token.text});
                advance();
                expect(";");
                result.code.push_back({OP_BRA, OP_ARG_PCR, 0, {}, 0});
            } else {
                std::optional<std::string> branch_label;
                result.code.push_back(instruction(branch_label));
                if (branch_label)
                    pending.fixups.push_back({static_cast<unsigned>(result.code.size() - 1), std::move(*branch_label)});
            }
        }
        result.parameters_size = inferred_parameters(result);
        return result;
    }

    object_t object()
    {
        object_t result;
        result.name = string();
        result.name_hash = name_hash(result.name);
        result.flags = image.objects.empty() ? 3 : 2;
        result.preserved = default_record(assembly_record_t::object);
        if (accept("extends"))
            result.parent = string();
        expect("{");
        while (!accept("}")) {
            expect("function");
            result.functions.push_back(function());
        }
        return result;
    }

    initializer_t initializer()
    {
        initializer_t result;
        result.preserved = default_record(assembly_record_t::initializer);
        expect("(");
        result.name_hash = integer();
        expect(",");
        result.offset = integer();
        expect(",");
        result.kind = static_cast<std::int32_t>(integer());
        expect(",");
        result.value = integer();
        expect(",");
        result.object = integer();
        expect(")");
        if (accept("{")) {
            expect("function");
            result.builder = function();
            expect("}");
        } else
            expect(";");
        return result;
    }

    std::string name_component()
    {
        if (!token.quoted && !identifier(token.text))
            fail("Expected data table name");
        auto result = token.text;
        advance();
        return result;
    }

    void data()
    {
        expect("{");
        while (!accept("}")) {
            const bool image_scope = !token.quoted && token.text == "image";
            const bool initializer_scope = !token.quoted && token.text == "initializers";
            auto first = name_component();
            table_key_t key;
            if (image_scope)
                key = {"image"};
            else if (initializer_scope) {
                expect(".");
                auto entry = name_component();
                if (!entry.starts_with("entry_"))
                    fail("Expected initializer entry_N");
                auto index = std::string_view(entry).substr(6);
                unsigned value = 0;
                auto parsed = std::from_chars(index.data(), index.data() + index.size(), value);
                if (index.empty() || parsed.ec != std::errc{} || parsed.ptr != index.data() + index.size())
                    fail("Invalid initializer table index");
                key = {"initializer", std::to_string(value)};
            } else
                key = {"object", std::move(first)};
            while (accept("."))
                key.push_back(name_component());
            expect(":");
            bytes_t bytes;
            while (accept(".")) {
                const auto directive = name_component();
                if (directive == "zero") {
                    auto count = integer();
                    if (count > 64u * 1024u * 1024u - bytes.size())
                        fail("Data table exceeds allocation limit");
                    bytes.resize(bytes.size() + count);
                } else if (directive == "long" || directive == "byte") {
                    const unsigned width = directive == "long" ? 4 : 1;
                    do {
                        auto value = integer();
                        if (width == 1 && value > UINT8_MAX)
                            fail("Byte directive exceeds 8 bits");
                        if (bytes.size() > 64u * 1024u * 1024u - width)
                            fail("Data table exceeds allocation limit");
                        auto offset = bytes.size();
                        bytes.resize(offset + width);
                        put_word(bytes, offset, value, width);
                    } while (accept(","));
                } else
                    fail("Unknown data directive ." + directive);
                accept(";");
            }
            if (!tables.emplace(std::move(key), std::move(bytes)).second)
                fail("Duplicate data table");
        }
    }

    bool take(const table_key_t &key, bytes_t &destination)
    {
        auto found = tables.find(key);
        if (found == tables.end())
            return false;
        destination = std::move(found->second);
        tables.erase(found);
        return true;
    }

    void record(const table_key_t &key, bytes_t &destination, assembly_record_t kind)
    {
        if (take(key, destination) && destination.size() != assembly_record_defaults(kind).size())
            fail("Wrong record size for " + table_name(key));
    }

    void scalar(const table_key_t &key, std::uint32_t &destination)
    {
        bytes_t value;
        if (!take(key, value))
            return;
        if (value.size() != 4)
            fail("Expected one .long in " + table_name(key));
        destination = read_word(value, 0);
    }

    void slots(const table_key_t &key, bytes_t &destination, std::size_t count)
    {
        if (take(key, destination) && destination.size() != count * 4)
            fail("Slot count disagrees with declarations in " + table_name(key));
    }

    void function_data(function_t &function, table_key_t key, std::uint32_t default_flags)
    {
        key.push_back("hashes");
        bytes_t hashes;
        if (take(key, hashes)) {
            if (hashes.size() != 8)
                fail("Function hashes require two .long values");
            function.fullname_hash = read_word(hashes, 0);
            function.name_hash = read_word(hashes, 4);
        }
        key.back() = "parameter_bytes";
        scalar(key, function.parameters_size);
        function.flags = default_flags;
        key.back() = "flags";
        scalar(key, function.flags);
        key.back() = "record";
        record(key, function.preserved, assembly_record_t::function);
    }

    void apply_data(std::vector<function_t *> &functions)
    {
        record({"image", "mash"}, image.mash_preserved, assembly_record_t::mash);
        record({"image", "record"}, image.preserved, assembly_record_t::image);
        slots({"image", "object_slots"}, image.object_slots, image.objects.size());
        slots({"image", "string_slots"}, image.string_slots, image.strings.size());
        std::set<std::string> object_names;
        for (auto &object : image.objects) {
            if (!object_names.insert(object.name).second)
                fail("Duplicate object label " + object.name);
            scalar({"object", object.name, "hash"}, object.name_hash);
            scalar({"object", object.name, "flags"}, object.flags);
            scalar({"object", object.name, "instance_bytes"}, object.instance_size);
            std::uint32_t destructor = static_cast<std::uint32_t>(object.destructor);
            scalar({"object", object.name, "destructor"}, destructor);
            object.destructor = static_cast<std::int32_t>(destructor);
            record({"object", object.name, "record"}, object.preserved, assembly_record_t::object);
            take({"object", object.name, "static_data"}, object.static_data);
            slots({"object", object.name, "function_slots"}, object.function_slots, object.functions.size());
            std::set<std::string> function_names;
            for (auto &function : object.functions) {
                if (!function_names.insert(function.name).second)
                    fail("Duplicate function label " + function.name);
                functions.at(function.code_order) = &function;
                function_data(function,
                              {"object", object.name, function.name},
                              VM_EXECUTABLE_FLAG_FROM_MASH |
                                  ((object.flags & SCRIPT_OBJECT_FLAG_GLOBAL) ? VM_EXECUTABLE_FLAG_STATIC : 0));
            }
        }
        for (std::size_t i = 0; i < image.initializers.size(); ++i) {
            auto &initializer = image.initializers[i];
            record({"initializer", std::to_string(i), "record"}, initializer.preserved, assembly_record_t::initializer);
            if (initializer.builder) {
                functions.at(initializer.builder->code_order) = &*initializer.builder;
                function_data(
                    *initializer.builder, {"initializer", std::to_string(i), "builder"}, VM_EXECUTABLE_FLAG_FROM_MASH);
            }
        }
        bytes_t order;
        if (take({"image", "code_order"}, order)) {
            if (order.size() != functions.size() * 4)
                fail("Code order count disagrees with functions");
            std::set<unsigned> seen;
            for (std::size_t i = 0; i < functions.size(); ++i) {
                auto index = read_word(order, i * 4);
                if (index >= functions.size() || !seen.insert(index).second)
                    fail("Code order is not a permutation");
                functions[i]->code_order = index;
            }
        }
        if (take({"image", "object_order"}, order)) {
            if (order.size() != image.objects.size() * 4)
                fail("Object order count disagrees with objects");
            for (std::size_t i = 0; i < image.objects.size(); ++i)
                image.object_order.push_back(read_word(order, i * 4));
        } else {
            for (unsigned i = 0; i < image.objects.size(); ++i)
                image.object_order.push_back(i);
            std::stable_sort(image.object_order.begin(), image.object_order.end(), [&](auto a, auto b) {
                return image.objects[a].name_hash < image.objects[b].name_hash;
            });
        }
        bytes_t padding;
        if (take({"image", "padding"}, padding)) {
            std::size_t offset = 0;
            while (offset < padding.size()) {
                if (padding.size() - offset < 4)
                    fail("Truncated padding boundary size");
                auto size = read_word(padding, offset);
                offset += 4;
                if (size > padding.size() - offset)
                    fail("Truncated padding boundary bytes");
                image.padding.emplace_back(padding.begin() + offset, padding.begin() + offset + size);
                offset += size;
            }
        }
        if (!tables.empty())
            fail("Unknown data table " + table_name(tables.begin()->first));
    }

    void resolve_labels(function_t &function, const function_labels_t &pending)
    {
        std::uint64_t offset = 0;
        for (auto &instruction : function.code) {
            validate_instruction(instruction);
            if (offset > UINT32_MAX)
                fail("Function exceeds 32-bit code size");
            instruction.offset = static_cast<unsigned>(offset);
            offset += instruction.size();
        }
        for (const auto &fixup : pending.fixups) {
            auto found = pending.labels.find(fixup.label);
            if (found == pending.labels.end())
                fail("Undefined branch label " + fixup.label);
            auto target = found->second == function.code.size() ? offset : function.code[found->second].offset;
            auto &instruction = function.code[fixup.index];
            auto displacement = static_cast<std::int64_t>(target) - instruction.offset - instruction.size();
            if (displacement < INT16_MIN || displacement > INT16_MAX)
                fail("Branch displacement exceeds retail word range");
            instruction.value = static_cast<std::uint16_t>(displacement);
        }
    }

public:
    explicit parser_t(std::string_view source) : parser_base_t(source) {}

    image_t parse()
    {
        expect("sxcc");
        if (integer() != 2)
            fail("Expected SXCC assembly version 2; disassemble the original PCSX again");
        expect(";");
        expect("target");
        expect("pc");
        expect(";");
        expect("script");
        image.name = string();
        expect("{");
        image.mash_preserved = default_record(assembly_record_t::mash);
        image.preserved = default_record(assembly_record_t::image);
        while (!accept("}")) {
            if (accept("object"))
                image.objects.push_back(object());
            else if (accept("initialize"))
                image.initializers.push_back(initializer());
            else if (accept("strings")) {
                expect("{");
                while (!accept("}")) {
                    auto text = string();
                    bytes_t bytes(text.begin(), text.end());
                    if (!accept("unterminated"))
                        bytes.push_back(0);
                    image.strings.push_back(std::move(bytes));
                    expect(";");
                }
            } else if (accept("data"))
                data();
            else
                fail("Expected object, initializer, strings, or data");
        }
        if (!token.text.empty())
            fail("Unexpected text after script");
        std::vector<function_t *> functions(labels.size());
        apply_data(functions);
        for (std::size_t i = 0; i < functions.size(); ++i)
            resolve_labels(*functions[i], labels[i]);
        validate_image(image);
        return std::move(image);
    }
};

std::string format_num(std::uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    if (!std::isfinite(value))
        return "bits(" + hex(bits) + ")";
    char buffer[64];
    auto count = std::snprintf(
        buffer, sizeof(buffer), "%.*g", std::numeric_limits<float>::max_digits10, static_cast<double>(value));
    if (count < 0 || static_cast<std::size_t>(count) >= sizeof(buffer))
        throw error_t("Cannot format num literal");
    return std::string(buffer, static_cast<std::size_t>(count));
}

void emit_function(std::ostream &out, const function_t &function, std::string_view indent)
{
    out << indent << "function " << quote(function.name) << " {\n";
    std::set<unsigned> labels;
    unsigned offset = 0;
    for (const auto &instruction : function.code) {
        if (is_branch(instruction.opcode) &&
            (instruction.argument == OP_ARG_PCR || instruction.argument == OP_ARG_WORD))
            labels.insert(static_cast<unsigned>(static_cast<std::int64_t>(offset) + instruction.size() +
                                                static_cast<std::int16_t>(instruction.value)));
        offset += instruction.size();
    }
    offset = 0;
    for (const auto &instruction : function.code) {
        if (labels.count(offset))
            out << indent << "    label L" << hex(offset).substr(2) << ":\n";
        out << indent << "    " << opcode_name(instruction.opcode);
        if (instruction.argument != OP_ARG_NULL) {
            out << ' ' << argument_name(instruction.argument) << ' ';
            if (instruction.argument == OP_ARG_NUM || instruction.argument == OP_ARG_NUMR)
                out << format_num(instruction.value);
            else if (is_branch(instruction.opcode) &&
                     (instruction.argument == OP_ARG_PCR || instruction.argument == OP_ARG_WORD))
                out << 'L'
                    << hex(static_cast<unsigned>(static_cast<std::int64_t>(offset) + instruction.size() +
                                                 static_cast<std::int16_t>(instruction.value)))
                           .substr(2);
            else if (instruction.argument == OP_ARG_WORD || instruction.argument == OP_ARG_PCR ||
                     instruction.argument == OP_ARG_SPR || instruction.argument == OP_ARG_POPO)
                out << static_cast<std::int16_t>(instruction.value);
            else
                out << hex(instruction.value);
        }
        if (instruction.size_override)
            out << ", " << *instruction.size_override;
        out << ";\n";
        offset += instruction.size();
    }
    if (labels.count(offset))
        out << indent << "    label L" << hex(offset).substr(2) << ":\n";
    out << indent << "}\n";
}

class data_writer_t {
    std::ostream &out;

public:
    explicit data_writer_t(std::ostream &output) : out(output) {}

    void table(const table_key_t &key, std::span<const std::uint8_t> bytes)
    {
        out << "        " << table_name(key) << ":\n";
        std::size_t offset = 0;
        while (offset < bytes.size()) {
            auto end = offset;
            while (end < bytes.size() && bytes[end] == 0)
                ++end;
            if (end - offset >= 4) {
                out << "            .zero " << end - offset << '\n';
                offset = end;
            } else if (bytes.size() - offset >= 4) {
                out << "            .long " << hex(read_word(bytes, offset)) << '\n';
                offset += 4;
            } else {
                out << "            .byte " << hex(bytes[offset++]) << '\n';
            }
        }
    }

    void scalar(const table_key_t &key, std::uint32_t value, std::uint32_t default_value)
    {
        if (value == default_value)
            return;
        out << "        " << table_name(key) << ":\n            .long " << hex(value) << '\n';
    }

    void record(const table_key_t &key, const bytes_t &value, assembly_record_t kind)
    {
        auto defaults = assembly_record_defaults(kind);
        if (value.empty()) {
            if (kind == assembly_record_t::function) {
                bytes_t zeros(defaults.size());
                table(key, zeros);
            }
        } else if (!std::equal(value.begin(), value.end(), defaults.begin(), defaults.end()))
            table(key, value);
    }

    void slots(const table_key_t &key, const bytes_t &value)
    {
        if (std::any_of(value.begin(), value.end(), [](auto byte) { return byte != 0; }))
            table(key, value);
    }

    void function(const function_t &function, table_key_t key, std::uint32_t flags)
    {
        key.push_back("hashes");
        if (name_hash(function.name) != function.fullname_hash ||
            name_hash(function.name.substr(0, function.name.find('('))) != function.name_hash) {
            out << "        " << table_name(key) << ":\n            .long " << hex(function.fullname_hash) << ", "
                << hex(function.name_hash) << '\n';
        }
        key.back() = "parameter_bytes";
        scalar(key, function.parameters_size, inferred_parameters(function));
        key.back() = "flags";
        scalar(key, function.flags, flags);
        key.back() = "record";
        record(key, function.preserved, assembly_record_t::function);
    }
};

}

image_t assemble_source(std::string_view source)
{
    return parser_t(source).parse();
}

std::string emit_assembly(const image_t &image)
{
    std::ostringstream out;
    out << "sxcc 2;\ntarget pc;\nscript " << quote(image.name) << " {\n";
    for (const auto &object : image.objects) {
        out << "    object " << quote(object.name);
        if (object.parent)
            out << " extends " << quote(*object.parent);
        out << " {\n";
        for (const auto &function : object.functions)
            emit_function(out, function, "        ");
        out << "    }\n\n";
    }
    for (const auto &initializer : image.initializers) {
        out << "    initialize(" << hex(initializer.name_hash) << ", " << initializer.offset << ", " << initializer.kind
            << ", " << hex(initializer.value) << ", " << initializer.object << ')';
        if (initializer.builder) {
            out << " {\n";
            emit_function(out, *initializer.builder, "        ");
            out << "    }\n";
        } else
            out << ";\n";
    }
    out << "\n    strings {\n";
    for (const auto &bytes : image.strings) {
        bool terminated = !bytes.empty() && bytes.back() == 0;
        auto size = bytes.size() - (terminated ? 1 : 0);
        out << "        " << quote(std::string_view(reinterpret_cast<const char *>(bytes.data()), size));
        if (!terminated)
            out << " unterminated";
        out << ";\n";
    }
    out << "    }\n\n    data {\n";
    data_writer_t data(out);
    data.record({"image", "mash"}, image.mash_preserved, assembly_record_t::mash);
    data.record({"image", "record"}, image.preserved, assembly_record_t::image);
    data.slots({"image", "object_slots"}, image.object_slots);
    data.slots({"image", "string_slots"}, image.string_slots);
    std::vector<unsigned> natural_order;
    for (unsigned i = 0; i < image.objects.size(); ++i)
        natural_order.push_back(i);
    std::stable_sort(natural_order.begin(), natural_order.end(), [&](auto a, auto b) {
        return image.objects[a].name_hash < image.objects[b].name_hash;
    });
    if (natural_order != image.object_order) {
        out << "        image.object_order:\n";
        for (auto index : image.object_order)
            out << "            .long " << index << '\n';
    }
    std::vector<unsigned> code_order;
    for (std::size_t i = 0; i < image.objects.size(); ++i) {
        const auto &object = image.objects[i];
        data.scalar({"object", object.name, "hash"}, object.name_hash, name_hash(object.name));
        data.scalar({"object", object.name, "flags"}, object.flags, i == 0 ? 3 : 2);
        data.scalar({"object", object.name, "instance_bytes"}, object.instance_size, 0);
        data.scalar({"object", object.name, "destructor"}, static_cast<std::uint32_t>(object.destructor), UINT32_MAX);
        data.record({"object", object.name, "record"}, object.preserved, assembly_record_t::object);
        if (!object.static_data.empty())
            data.table({"object", object.name, "static_data"}, object.static_data);
        data.slots({"object", object.name, "function_slots"}, object.function_slots);
        for (const auto &function : object.functions) {
            code_order.push_back(function.code_order);
            data.function(function,
                          {"object", object.name, function.name},
                          VM_EXECUTABLE_FLAG_FROM_MASH |
                              ((object.flags & SCRIPT_OBJECT_FLAG_GLOBAL) ? VM_EXECUTABLE_FLAG_STATIC : 0));
        }
    }
    for (std::size_t i = 0; i < image.initializers.size(); ++i) {
        const auto &initializer = image.initializers[i];
        data.record(
            {"initializer", std::to_string(i), "record"}, initializer.preserved, assembly_record_t::initializer);
        if (initializer.builder) {
            code_order.push_back(initializer.builder->code_order);
            data.function(
                *initializer.builder, {"initializer", std::to_string(i), "builder"}, VM_EXECUTABLE_FLAG_FROM_MASH);
        }
    }
    bool reordered = false;
    for (std::size_t i = 0; i < code_order.size(); ++i)
        reordered |= code_order[i] != i;
    if (reordered) {
        out << "        image.code_order:\n";
        for (auto index : code_order)
            out << "            .long " << index << '\n';
    }
    if (std::any_of(image.padding.begin(), image.padding.end(), [](const auto &bytes) {
            return std::any_of(bytes.begin(), bytes.end(), [](auto byte) { return byte != 0xe3; });
        })) {
        out << "        image.padding:\n";
        for (const auto &padding : image.padding) {
            out << "            .long " << padding.size() << '\n';
            for (auto byte : padding)
                out << "            .byte " << hex(byte) << '\n';
        }
    }
    out << "    }\n}\n";
    return out.str();
}

}
