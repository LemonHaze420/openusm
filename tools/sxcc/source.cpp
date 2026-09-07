#include "sxcc.h"
#include "script_executable.h"
#include "script_object.h"
#include "vm_executable.h"
#include "parse_generic_mash.h"
#include "source_parser.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <charconv>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <sstream>

namespace sxcc {
namespace {


class parser_t : private detail::parser_base_t {
    unsigned next_order = 0;
    struct fixup_t {
        unsigned index;
        std::string label;
    };


    instruction_t instruction(std::optional<std::string> &branch_label)
    {
        instruction_t result;
        result.opcode = parse_opcode(token.text);
        advance();
        if (token.text != ";" && token.text != "@") {
            result.argument = parse_argument(token.text);
            advance();
            if (result.argument == OP_ARG_NUM || result.argument == OP_ARG_NUMR) {
                if (accept("bits")) {
                    expect("("); result.value = integer(); expect(")");
                } else {
                    bool negative = accept("-");
                    char *end = nullptr;
                    errno = 0;
                    float value = std::strtof(token.text.c_str(), &end);
                    if (token.text.empty() || end != token.text.data() + token.text.size() ||
                        !std::isfinite(value) || (errno == ERANGE && std::fpclassify(value) == FP_ZERO))
                        fail("Invalid or out-of-range num literal; use bits() for explicit IEEE encodings");
                    if (negative) value = -value;
                    std::memcpy(&result.value, &value, sizeof(value));
                    advance();
                }
            } else if ((result.argument == OP_ARG_PCR || result.argument == OP_ARG_WORD) &&
                       is_branch(result.opcode) && !token.quoted && !token.text.empty() &&
                       ((token.text.front() >= 'A' && token.text.front() <= 'Z') ||
                        (token.text.front() >= 'a' && token.text.front() <= 'z') || token.text.front() == '_')) {
                branch_label = token.text;
                advance();
            } else {
                result.value = integer();
                if (result.argument == OP_ARG_WORD || result.argument == OP_ARG_PCR || result.argument == OP_ARG_SPR || result.argument == OP_ARG_POPO) {
                    if (result.value > UINT16_MAX && result.value < 0xffff8000u) fail("Word operand exceeds 16 bits");
                    result.value &= UINT16_MAX;
                }
            }
        }
        if (accept("@")) {
            expect("size"); expect("(");
            result.size_override = static_cast<std::uint16_t>(small_integer(UINT16_MAX));
            expect(")");
        }
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
        result.code_order = next_order++;
        while (accept("@")) {
            if (accept("hash")) {
                expect("("); result.fullname_hash = integer(); expect(","); result.name_hash = integer(); expect(")");
            } else if (accept("parameters")) {
                expect("("); result.parameters_size = integer(); expect(")");
            } else if (accept("flags")) {
                expect("("); result.flags = integer(); expect(")");
            } else if (accept("order")) {
                expect("("); result.code_order = integer(); expect(")");
            } else if (accept("record")) extra(result.preserved, sizeof(vm_executable));
            else fail("Unknown function annotation");
        }
        expect("{");
        std::map<std::string, unsigned> labels;
        std::vector<fixup_t> fixups;
        unsigned offset = 0;
        while (!accept("}")) {
            if (accept("label")) {
                auto label = token.text;
                if (token.quoted || label.empty()) fail("Expected label identifier");
                advance(); expect(":");
                if (!labels.emplace(label, offset).second) fail("Duplicate label");
                continue;
            }
            if (accept("goto")) {
                fixups.push_back({static_cast<unsigned>(result.code.size()), token.text});
                advance(); expect(";");
                result.code.push_back({OP_BRA, OP_ARG_PCR, 0, {}, offset});
            } else {
                std::optional<std::string> branch_label;
                result.code.push_back(instruction(branch_label));
                if (branch_label)
                    fixups.push_back({static_cast<unsigned>(result.code.size() - 1), std::move(*branch_label)});
                result.code.back().offset = offset;
            }
            offset += result.code.back().size();
        }
        for (const auto &fixup : fixups) {
            auto found = labels.find(fixup.label);
            if (found == labels.end()) fail("Undefined branch label " + fixup.label);
            auto &insn = result.code[fixup.index];
            auto displacement = static_cast<std::int64_t>(found->second) - insn.offset - insn.size();
            if (displacement < INT16_MIN || displacement > INT16_MAX) fail("Branch displacement exceeds retail word range");
            insn.value = static_cast<std::uint16_t>(displacement);
        }
        return result;
    }

    object_t object()
    {
        object_t result;
        result.name = string();
        result.name_hash = name_hash(result.name);
        if (accept("extends")) result.parent = string();
        while (accept("@")) {
            if (accept("hash")) { expect("("); result.name_hash = integer(); expect(")"); }
            else if (accept("flags")) { expect("("); result.flags = integer(); expect(")"); }
            else if (accept("storage")) { expect("("); result.instance_size = integer(); expect(")"); }
            else if (accept("destructor")) { expect("("); result.destructor = static_cast<std::int32_t>(integer()); expect(")"); }
            else if (accept("record")) extra(result.preserved, sizeof(script_object));
            else if (accept("evidence")) { expect("("); result.parent_evidence = string(); expect(")"); }
            else fail("Unknown object annotation");
        }
        expect("{");
        while (!accept("}")) {
            if (accept("static_data")) {
                expect("("); auto size = integer(); expect(")");
                if (size > 64u * 1024u * 1024u) fail("Static data exceeds compiler allocation limit");
                if (accept("=")) {
                    result.static_data = bytes();
                    if (result.static_data.size() != size) fail("Static initializer size mismatch");
                } else result.static_data.resize(size);
                expect(";");
            } else if (accept("function")) result.functions.push_back(function());
            else if (accept("@")) {
                expect("slots"); result.function_slots = bytes(); expect(";");
            } else fail("Expected function or static data declaration");
        }
        return result;
    }

    initializer_t initializer()
    {
        initializer_t result;
        expect("("); result.name_hash = integer(); expect(","); result.offset = integer();
        expect(","); result.kind = static_cast<std::int32_t>(integer()); expect(","); result.value = integer();
        expect(","); result.object = integer(); expect(")");
        if (accept("@")) { expect("record"); extra(result.preserved, sizeof(script_executable::info_t)); }
        if (accept("{")) {
            expect("function"); result.builder = function(); expect("}");
        } else expect(";");
        return result;
    }

public:
    explicit parser_t(std::string_view source) : parser_base_t(source) {}

    image_t parse()
    {
        expect("sxcc");
        if (integer() != 1) fail("Unsupported SXCC source version");
        expect(";"); expect("target"); expect("pc"); expect(";");
        expect("script"); image.name = string(); expect("{");
        bool has_order = false;
        while (!accept("}")) {
            if (accept("@")) { expect("image"); image_metadata(); }
            else if (accept("strings")) {
                expect("{");
                while (!accept("}")) {
                    auto text = string();
                    bytes_t bytes(text.begin(), text.end());
                    if (!accept("unterminated")) bytes.push_back(0);
                    image.strings.push_back(std::move(bytes));
                    expect(";");
                }
            } else if (accept("object")) image.objects.push_back(object());
            else if (accept("object_order")) {
                if (has_order) fail("Duplicate object order");
                has_order = true;
                expect("{");
                while (!accept("}")) { image.object_order.push_back(integer()); expect(";"); }
            } else if (accept("initialize")) image.initializers.push_back(initializer());
            else fail("Expected script declaration");
        }
        if (!token.text.empty()) fail("Unexpected text after script");
        if (!has_order) {
            image.object_order.resize(image.objects.size());
            for (unsigned i = 0; i < image.objects.size(); ++i) image.object_order[i] = i;
            std::stable_sort(image.object_order.begin(), image.object_order.end(), [&](auto a, auto b) {
                return image.objects[a].name_hash < image.objects[b].name_hash;
            });
        }
        validate_image(image);
        return std::move(image);
    }
};

void emit_extra(std::ostream &out, const bytes_t &record, std::string_view prefix, std::string_view indent)
{
    bool nonzero = std::any_of(record.begin(), record.end(), [](auto byte) { return byte != 0; });
    if (!nonzero) return;
    out << indent << prefix << " { ";
    for (unsigned i = 0; i < record.size(); ++i)
        if (record[i] != 0) out << hex(i) << ": " << hex(record[i]) << "; ";
    out << "}\n";
}

std::string format_num(std::uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    if (!std::isfinite(value)) return "bits(" + hex(bits) + ")";
    char buffer[64];
    auto count = std::snprintf(buffer, sizeof(buffer), "%.*g", std::numeric_limits<float>::max_digits10, static_cast<double>(value));
    if (count < 0 || static_cast<std::size_t>(count) >= sizeof(buffer)) throw error_t("Cannot format num literal");
    return std::string(buffer, static_cast<std::size_t>(count));
}

void emit_function(std::ostream &out, const function_t &function, std::string_view indent)
{
    out << indent << "function " << quote(function.name) << '\n';
    if (name_hash(function.name) != function.fullname_hash ||
        name_hash(function.name.substr(0, function.name.find('('))) != function.name_hash)
        out << indent << "@hash(" << hex(function.fullname_hash) << ", " << hex(function.name_hash) << ")\n";
    out << indent << "@parameters(" << function.parameters_size << ") @flags(" << function.flags << ") @order(" << function.code_order << ")\n";
    emit_extra(out, function.preserved, "@record", indent);
    out << indent << "{\n";
    std::set<unsigned> labels;
    unsigned offset = 0;
    for (const auto &instruction : function.code) {
        if (is_branch(instruction.opcode) && (instruction.argument == OP_ARG_PCR || instruction.argument == OP_ARG_WORD))
            labels.insert(static_cast<unsigned>(static_cast<std::int64_t>(offset) + instruction.size() +
                                                static_cast<std::int16_t>(instruction.value)));
        offset += instruction.size();
    }
    offset = 0;
    for (const auto &instruction : function.code) {
        if (labels.count(offset)) out << indent << "    label L" << hex(offset).substr(2) << ":\n";
        out << indent << "    " << opcode_name(instruction.opcode);
        if (instruction.argument != OP_ARG_NULL) {
            out << ' ' << argument_name(instruction.argument) << ' ';
            if (instruction.argument == OP_ARG_NUM || instruction.argument == OP_ARG_NUMR)
                out << format_num(instruction.value);
            else if (is_branch(instruction.opcode) && (instruction.argument == OP_ARG_PCR || instruction.argument == OP_ARG_WORD))
                out << 'L' << hex(static_cast<unsigned>(static_cast<std::int64_t>(offset) + instruction.size() +
                                                      static_cast<std::int16_t>(instruction.value))).substr(2);
            else if (instruction.argument == OP_ARG_WORD || instruction.argument == OP_ARG_PCR || instruction.argument == OP_ARG_SPR || instruction.argument == OP_ARG_POPO)
                out << static_cast<std::int16_t>(instruction.value);
            else
                out << hex(instruction.value);
        }
        if (instruction.size_override) out << " @size(" << *instruction.size_override << ')';
        out << ";\n";
        offset += instruction.size();
    }
    out << indent << "}\n";
}

}

void emit_source_record(std::ostream &out, const bytes_t &record, std::string_view name, std::string_view indent)
{
    emit_extra(out, record, name, indent);
}

image_t assemble_source(std::string_view source)
{
    return parser_t(source).parse();
}

void emit_source_header(std::ostream &out, const image_t &image)
{
    out << "sxcc 1;\ntarget pc;\nscript " << quote(image.name) << " {\n";
    out << "    @image {\n";
    emit_extra(out, image.mash_preserved, "mash", "        ");
    emit_extra(out, image.preserved, "record", "        ");
    auto emit_slots = [&](std::string_view name, const bytes_t &slots) {
        if (std::any_of(slots.begin(), slots.end(), [](auto byte) { return byte != 0; }))
            out << "        " << name << ' ' << quote(encode_bytes(slots)) << ";\n";
    };
    emit_slots("object_slots", image.object_slots);
    emit_slots("string_slots", image.string_slots);
    bool unusual_padding = std::any_of(image.padding.begin(), image.padding.end(), [](const auto &bytes) {
        return std::any_of(bytes.begin(), bytes.end(), [](auto byte) { return byte != 0xe3; });
    });
    if (unusual_padding) {
        out << "        padding {\n";
        for (const auto &padding : image.padding) out << "            " << quote(encode_bytes(padding)) << ";\n";
        out << "        }\n";
    }
    out << "    }\n\n    strings {\n";
    for (const auto &bytes : image.strings) {
        bool terminated = !bytes.empty() && bytes.back() == 0;
        auto size = bytes.size() - (terminated ? 1 : 0);
        out << "        " << quote(std::string_view(reinterpret_cast<const char *>(bytes.data()), size));
        if (!terminated) out << " unterminated";
        out << ";\n";
    }
    out << "    }\n\n    object_order { ";
    for (auto index : image.object_order) out << index << "; ";
    out << "}\n\n";
}

std::string emit_assembly(const image_t &image)
{
    std::ostringstream out;
    emit_source_header(out, image);
    for (const auto &object : image.objects) {
        out << "    object " << quote(object.name);
        if (object.parent) out << " extends " << quote(*object.parent);
        out << '\n';
        if (name_hash(object.name) != object.name_hash) out << "    @hash(" << hex(object.name_hash) << ")\n";
        out << "    @flags(" << object.flags << ") @storage(" << object.instance_size << ") @destructor(" << object.destructor << ")\n";
        if (!object.parent_evidence.empty()) out << "    @evidence(" << quote(object.parent_evidence) << ")\n";
        emit_extra(out, object.preserved, "@record", "    ");
        out << "    {\n        static_data(" << object.static_data.size() << ')';
        if (std::any_of(object.static_data.begin(), object.static_data.end(), [](auto byte) { return byte != 0; }))
            out << " = " << quote(encode_bytes(object.static_data));
        out << ";\n";
        if (std::any_of(object.function_slots.begin(), object.function_slots.end(), [](auto byte) { return byte != 0; }))
            out << "        @slots " << quote(encode_bytes(object.function_slots)) << ";\n";
        for (const auto &function : object.functions) emit_function(out, function, "        ");
        out << "    }\n\n";
    }
    for (const auto &initializer : image.initializers) {
        out << "    initialize(" << hex(initializer.name_hash) << ", " << initializer.offset << ", " << initializer.kind << ", "
            << hex(initializer.value) << ", " << initializer.object << ')';
        bool extra = std::any_of(initializer.preserved.begin(), initializer.preserved.end(), [](auto byte) { return byte != 0; });
        if (extra) { out << '\n'; emit_extra(out, initializer.preserved, "@record", "    "); }
        if (initializer.builder) {
            out << " {\n"; emit_function(out, *initializer.builder, "        "); out << "    }\n";
        } else out << ";\n";
    }
    out << "}\n";
    return out.str();
}

}
