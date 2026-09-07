#include "language.h"
#include "type_recovery.h"
#include "pretty.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

namespace sxcc {
namespace {

std::string identifier(std::string_view name, std::string_view fallback)
{
    if (name.empty()) return std::string(fallback);
    std::string result;
    for (unsigned char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
            (!result.empty() && c >= '0' && c <= '9')) result += static_cast<char>(c);
        else result += '_';
    }
    return result;
}

class source_emitter_t {
    const image_t &image;
    const symbols_t &symbols;
    stack_analysis_t stack;
    std::vector<lifted_function_t> graphs;
    recovered_types_t types;
    std::vector<std::string> classes;
    std::vector<std::vector<std::string>> names;
    std::vector<std::optional<std::string>> parents;
    struct field_t { unsigned width; std::string type; };
    std::vector<std::map<unsigned, field_t>> fields;
    std::map<std::pair<unsigned, unsigned>, std::pair<std::string, std::string>> variables;
    std::map<std::pair<unsigned, unsigned>, std::string> inferred_native_results;
    std::vector<std::map<unsigned, field_t>> shared_fields;
    std::vector<std::map<unsigned, std::string>> shared_names;
    struct array_t { unsigned width = 0, count = 1; std::string type; };
    std::vector<std::map<unsigned, array_t>> shared_arrays;
    std::vector<std::map<unsigned, array_t>> instance_arrays;
    std::optional<std::pair<std::string, std::string>> instance_member(
        const lifted_function_t &graph, const lifted_address_t &address, std::string_view index = {}) const
    {
        if (address.kind != lifted_address_t::kind_t::instance || !address.receiver) return {};
        auto owner = receiver_class(graph, *address.receiver);
        if (!owner) return {};
        for (unsigned depth = 0; depth < image.objects.size() && parents[*owner]; ++depth) {
            auto parent = std::find_if(image.objects.begin(), image.objects.end(),
                [&](const auto &candidate) { return candidate.name == *parents[*owner]; });
            if (parent == image.objects.end() || address.offset + address.width > parent->instance_size) break;
            owner = static_cast<unsigned>(parent - image.objects.begin());
        }
        const auto &arrays = instance_arrays[*owner];
        auto found = arrays.upper_bound(address.offset);
        if (found == arrays.begin()) return {};
        --found;
        const auto &array = found->second;
        const auto displacement = address.offset - found->first;
        if (displacement + address.width > array.width * array.count) return {};
        const bool component = !address.index && address.width == 4 &&
            (array.type == "vector3d" || array.type == "position3d");
        if (!component && (address.width != array.width || displacement % array.width)) return {};
        auto name = "field_" + std::to_string(found->first) + '[';
        if (address.index) {
            if (index.empty()) return {};
            name += index;
            if (displacement) name += " + " + std::to_string(displacement / array.width);
        } else name += std::to_string(displacement / array.width);
        name += ']';
        if (component) {
            name += '.';
            name += "xyz"[(displacement % array.width) / 4];
        }
        return std::make_pair(std::move(name), component ? "num" : type(array.type));
    }
    void recover_static_fields();
    std::optional<std::pair<std::string, std::string>> shared_member(const lifted_address_t &address, std::string_view index = {}) const;
    void emit_static_fields(unsigned object);

    std::optional<unsigned> receiver_class(const lifted_function_t &graph, value_ref_t ref) const
    {
        if (ref.offset) return {};
        const auto &value = graph.values[ref.value];
        for (unsigned i = 0; i < image.objects.size(); ++i)
            if (value.type == image.objects[i].name) return i;
        return {};
    }

    unsigned field_owner(unsigned owner, unsigned offset, unsigned width) const
    {
        for (unsigned depth = 0; depth < image.objects.size() && parents[owner]; ++depth) {
            auto parent = std::find_if(image.objects.begin(), image.objects.end(), [&](const auto &object) { return object.name == *parents[owner]; });
            if (parent == image.objects.end()) break;
            auto index = static_cast<unsigned>(parent - image.objects.begin());
            auto found = fields[index].find(offset);
            if (found == fields[index].end() || found->second.width != width) break;
            owner = index;
        }
        return owner;
    }

    const field_t *field(const lifted_function_t &graph, const lifted_address_t &address) const
    {
        if (address.kind == lifted_address_t::kind_t::shared && !address.index) {
            auto found = shared_fields[address.owner].find(address.offset);
            return found != shared_fields[address.owner].end() && found->second.width == address.width ? &found->second : nullptr;
        }
        if (address.kind != lifted_address_t::kind_t::instance || !address.receiver || address.index) return nullptr;
        auto owner = receiver_class(graph, *address.receiver);
        if (!owner) return nullptr;
        owner = field_owner(*owner, address.offset, address.width);
        auto found = fields[*owner].find(address.offset);
        return found != fields[*owner].end() && found->second.width == address.width ? &found->second : nullptr;
    }

    static std::string number_literal(std::uint32_t bits)
    {
        float value;
        std::memcpy(&value, &bits, 4);
        if (!std::isfinite(value) || bits == 0x80000000u) return "num_bits(" + hex(bits) + ')';
        char buffer[64];
        const float integral = std::trunc(value);
        std::uint32_t integral_bits;
        std::memcpy(&integral_bits, &integral, 4);
        if (std::abs(value) <= 1000000 && integral_bits == bits) {
            std::snprintf(buffer, sizeof(buffer), "%.0f", static_cast<double>(value));
            return buffer;
        }
        for (int precision = 1; precision <= 9; ++precision) {
            std::snprintf(buffer, sizeof(buffer), "%.*g", precision, static_cast<double>(value));
            const auto parsed = std::strtof(buffer, nullptr);
            std::uint32_t encoded;
            std::memcpy(&encoded, &parsed, 4);
            if (encoded == bits) return buffer;
        }
        throw error_t("Cannot render numeric constant");
    }
    std::map<std::pair<unsigned, unsigned>, unsigned> functions;
    std::ostringstream out;

    std::string type(std::string_view name) const
    {
        for (unsigned i = 0; i < image.objects.size(); ++i)
            if (name == image.objects[i].name) return classes[i];
        return std::string(name);
    }

    static std::string opaque(unsigned width)
    {
        return width ? "opaque<" + std::to_string(width) + '>' : "void";
    }

    std::string native_result(std::pair<unsigned, unsigned> identity) const
    {
        const auto &native = symbols.natives.at(identity);
        if (native.result) return *native.result;
        if (auto found = inferred_native_results.find(identity); found != inferred_native_results.end()) return type(found->second);
        if (native.result_bytes) return opaque(*native.result_bytes);
        return opaque(stack.native_results.at(identity));
    }

    const function_t &binary(unsigned index) const
    {
        const auto &graph = graphs[index];
        return graph.initializer ? *image.initializers[graph.function].builder :
               image.objects[graph.object].functions[graph.function];
    }

    unsigned target(unsigned identity) const
    {
        return functions.at({image.object_order.at(identity >> 16), identity & 0xffff});
    }

    std::string text(unsigned index) const
    {
        const auto &bytes = image.strings.at(index);
        const auto size = bytes.size() - (!bytes.empty() && bytes.back() == 0 ? 1 : 0);
        return quote({reinterpret_cast<const char *>(bytes.data()), size});
    }

    class body_t {
        source_emitter_t &source;
        unsigned index;
        lifted_function_t graph;
        const function_t &binary;
        const recovered_function_t &types;
        std::vector<bool> used;
        std::vector<bool> reachable;
        std::vector<unsigned> slots;
        std::vector<unsigned> slot_values;
        std::map<unsigned, std::string> pretty_slots;
        void prettify();
        unsigned edge_serial = 0;
        std::string indent;
        std::map<unsigned, std::string> folded;
        std::vector<bool> adjacent_expressions;
        std::vector<bool> required_slots;
        struct local_array_t {
            unsigned offset = 0, width = 0, count = 0;
            std::string type, name;
        };
        std::vector<local_array_t> local_arrays;
        std::map<unsigned, unsigned> array_values;

        void recover_local_arrays();
        void recover_native_values();
        void declaration(unsigned id);
        void entry_values();
        void parallel_copy(const std::vector<std::pair<value_ref_t, value_ref_t>> &transfers, std::string_view line);
        void array_copy(unsigned id, const std::vector<value_ref_t> &refs, std::string_view line);

        std::string reference_type(value_ref_t ref) const
        {
            if (auto array = array_values.find(ref.value); array != array_values.end()) {
                const auto &layout = local_arrays[array->second];
                return layout.width == 4 ? source.type(layout.type) : "num";
            }
            const auto type = value_type(ref.value);
            return type == "vector3d" || type == "position3d" ? "num" : type;
        }

        std::string value_type(unsigned id) const
        {
            const auto &value = graph.values[id];
            return source.type(value.type.empty() ? opaque(value.width) : value.type);
        }

        std::string name(unsigned id) const
        {
            const auto slot = slots.at(id);
            if (auto pretty = pretty_slots.find(slot); pretty != pretty_slots.end()) return pretty->second;
            if (auto array = array_values.find(slot_values.at(slot)); array != array_values.end())
                return local_arrays[array->second].name + "_" + std::to_string(slot);
            return "v" + std::to_string(slot);
        }

        std::string cast(std::string expression, std::string_view actual, std::string_view expected) const
        {
            if (actual == expected) return expression;
            if (actual == "num" && expression == "0" && expected != "num") {
                const auto native = source.symbols.type_sizes.find(std::string(expected));
                if ((native != source.symbols.type_sizes.end() && native->second == 4) ||
                    std::find(source.classes.begin(), source.classes.end(), expected) != source.classes.end())
                    return "nullptr";
            }
            auto script = std::find(source.classes.begin(), source.classes.end(), actual);
            for (unsigned depth = 0; script != source.classes.end() && depth < source.classes.size(); ++depth) {
                const auto &parent = source.parents[static_cast<unsigned>(script - source.classes.begin())];
                if (!parent) break;
                const auto name = source.type(*parent);
                if (name == expected) return expression;
                script = std::find(source.classes.begin(), source.classes.end(), name);
            }
            auto scope = std::string(actual);
            for (auto parent = source.symbols.parents.find(scope); parent != source.symbols.parents.end(); parent = source.symbols.parents.find(scope)) {
                scope = parent->second;
                if (scope == expected) return expression;
            }
            return "bit_cast<" + std::string(expected) + ">(" + expression + ')';
        }

        std::string reference(value_ref_t ref, std::string_view expected = {}) const
        {
            const auto &value = graph.values.at(ref.value);
            if (auto array = array_values.find(ref.value); array != array_values.end()) {
                const auto &layout = local_arrays[array->second];
                auto expression = name(ref.value) + "[" + std::to_string(ref.offset / layout.width) + "]";
                if (layout.width == 12) expression += std::string(".") + "xyz"[(ref.offset % layout.width) / 4];
                const auto actual = reference_type(ref);
                return expected.empty() ? expression : cast(std::move(expression), actual, expected);
            }
            std::string expression;
            auto actual = value_type(ref.value);
            if (value.kind == lifted_value_t::kind_t::parameter)
                for (const auto &parameter : types.parameters)
                    if ((parameter.type == "vector3d" || parameter.type == "position3d") &&
                        value.immediate + ref.offset >= parameter.offset &&
                        value.immediate + ref.offset < parameter.offset + parameter.width) {
                        const auto component = (value.immediate + ref.offset - parameter.offset) / 4;
                        auto member = "p" + std::to_string(parameter.offset) + "." + "xyz"[component];
                        return expected.empty() ? member : cast(std::move(member), "num", expected);
                    }
            switch (value.kind) {
            case lifted_value_t::kind_t::constant:
                expression = number_literal(value.immediate); actual = "num"; break;
            case lifted_value_t::kind_t::string:
                expression = source.text(value.immediate); actual = "str"; break;
            case lifted_value_t::kind_t::class_value: {
                std::string native;
                for (const auto &[name, id] : source.symbols.class_ids)
                    if (id == value.immediate >> 16) { native = name; break; }
                if (native.empty()) throw error_t("Class value has no evidenced native type");
                expression = native + '(' + source.text(value.immediate & 0xffff) + ')'; actual = native;
                break;
            }
            case lifted_value_t::kind_t::parameter:
                expression = !(binary.flags & 1u) && value.immediate == 0 ? "this" : "p" + std::to_string(value.immediate);
                for (const auto &parameter : types.parameters)
                    if (parameter.offset == value.immediate) { actual = source.type(parameter.type); break; }
                break;
            default: expression = folded.count(ref.value) ? folded.at(ref.value) : name(ref.value); break;
            }
            if ((actual == "vector3d" || actual == "position3d") && ref.offset < 12) {
                expression = '(' + expression + ")." + "xyz"[ref.offset / 4];
                return expected.empty() ? expression : cast(std::move(expression), "num", expected);
            }
            if (value.width != 4 || ref.offset) {
                expression = "storage_part<opaque<4>>(" + expression + ", " + std::to_string(ref.offset) + ')';
                actual = "opaque<4>";
            }
            return expected.empty() ? expression : cast(std::move(expression), actual, expected);
        }

        std::string group(const std::vector<value_ref_t> &refs, std::string expected = {}) const
        {
            if (refs.empty()) return {};
            if (expected.empty()) {
                const auto &value = graph.values.at(refs.front().value);
                bool whole = refs.front().offset == 0 && value.width == refs.size() * 4;
                for (unsigned i = 0; whole && i < refs.size(); ++i)
                    whole = refs[i].value == refs.front().value && refs[i].offset == i * 4;
                expected = whole ? value_type(refs.front().value) : opaque(static_cast<unsigned>(refs.size() * 4));
                if (refs.size() == 1 && (value.type == "vector3d" || value.type == "position3d")) expected = "num";
            }
            if (refs.size() == 1) return reference(refs.front(), expected);
            const auto &first = graph.values.at(refs.front().value);
            if (auto array = array_values.find(refs.front().value); array != array_values.end()) {
                const auto &layout = local_arrays[array->second];
                bool element = refs.front().offset % layout.width == 0 && refs.size() * 4 == layout.width;
                for (unsigned i = 1; element && i < refs.size(); ++i)
                    element = refs[i].value == refs.front().value && refs[i].offset == refs.front().offset + i * 4;
                if (element)
                    return cast(name(refs.front().value) + "[" + std::to_string(refs.front().offset / layout.width) + "]",
                                source.type(layout.type), expected);
            }
            if (first.kind == lifted_value_t::kind_t::parameter)
                for (const auto &parameter : types.parameters) {
                    if (parameter.offset != first.immediate + refs.front().offset || parameter.width != refs.size() * 4) continue;
                    bool contiguous = true;
                    for (unsigned i = 0; i < refs.size(); ++i) {
                        const auto &part = graph.values[refs[i].value];
                        if (part.kind != lifted_value_t::kind_t::parameter || part.immediate + refs[i].offset != parameter.offset + i * 4)
                            contiguous = false;
                    }
                    if (contiguous) return cast("p" + std::to_string(parameter.offset), source.type(parameter.type), expected);
                }
            bool whole = refs.front().offset == 0 && first.width == refs.size() * 4;
            for (unsigned i = 1; whole && i < refs.size(); ++i)
                whole = refs[i].value == refs.front().value && refs[i].offset == i * 4;
            if (whole) {
                auto expression = folded.count(refs.front().value) ? folded.at(refs.front().value) :
                    first.kind == lifted_value_t::kind_t::parameter ?
                    (!(binary.flags & 1u) && !first.immediate ? "this" : "p" + std::to_string(first.immediate)) : name(refs.front().value);
                return cast(std::move(expression), value_type(refs.front().value), expected);
            }
            if ((expected == "vector3d" || expected == "position3d") && refs.size() == 3) {
                std::string vector = expected + '{';
                for (unsigned component = 0; component < 3; ++component) {
                    if (component) vector += ", ";
                    vector += reference(refs[component], "num");
                }
                return vector + '}';
            }
            std::string result = opaque(static_cast<unsigned>(refs.size() * 4)) + '{';
            for (unsigned i = 0; i < refs.size(); ++i) {
                if (i) result += ", ";
                result += reference(refs[i], "opaque<4>");
            }
            result += '}';
            return cast(std::move(result), opaque(static_cast<unsigned>(refs.size() * 4)), expected);
        }

        std::string call(unsigned identity, const std::vector<value_ref_t> &arguments) const
        {
            auto callee = source.target(identity);
            const auto &target = source.graphs[callee];
            const auto &record = source.binary(callee);
            const auto &parameters = source.types.functions[callee].parameters;
            std::string result;
            unsigned first = 0;
            if (!(record.flags & 1u)) {
                result = '(' + reference(arguments.at(0), source.classes[target.object]) + ").";
                first = 1;
            } else result = source.classes[target.object] + "::";
            result += source.names[target.object][target.function] + '(';
            bool comma = false;
            for (const auto &parameter : parameters) {
                if (parameter.offset < first * 4) continue;
                if (comma) result += ", ";
                comma = true;
                auto begin = arguments.begin() + parameter.offset / 4;
                result += group({begin, begin + parameter.width / 4}, source.type(parameter.type));
            }
            return result + ')';
        }

        std::string store_type(const lifted_address_t &storage) const
        {
            if (auto member = source.shared_member(storage, storage.index ? "index" : "")) return member->second;
            if (auto member = source.instance_member(graph, storage, storage.index ? "index" : "")) return member->second;
            if (const auto *member = source.field(graph, storage)) return source.type(member->type);
            if (storage.kind == lifted_address_t::kind_t::game_variable || storage.kind == lifted_address_t::kind_t::shared_variable) {
                auto found = source.variables.find({storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                if (found != source.variables.end() && !found->second.second.empty()) return source.type(found->second.second);
            }
            return opaque(storage.width);
        }

        std::string address(const lifted_address_t &storage, std::string_view expected,
                            const std::vector<value_ref_t> *memory = nullptr) const
        {
            if (auto member = source.shared_member(storage, storage.index ? reference(*storage.index, "num") : std::string{}))
                return cast(member->first, member->second, expected);
            if (auto member = source.instance_member(graph, storage, storage.index ? reference(*storage.index, "num") : "")) {
                const auto owner = *source.receiver_class(graph, *storage.receiver);
                const auto receiver = reference(*storage.receiver, source.classes[owner]);
                auto expression = (receiver == "this" ? "" : '(' + receiver + ").") + member->first;
                return cast(std::move(expression), member->second, expected);
            }
            if (const auto *field = source.field(graph, storage)) {
                auto owner = *source.receiver_class(graph, *storage.receiver);
                auto receiver = reference(*storage.receiver, source.classes[owner]);
                auto expression = (receiver == "this" ? "" : '(' + receiver + ").") + std::string("field_") + std::to_string(storage.offset);
                return cast(std::move(expression), source.type(field->type), expected);
            }
            std::string base;
            switch (storage.kind) {
            case lifted_address_t::kind_t::local: {
                if (!memory || memory->empty()) throw error_t("Local storage access lacks a value snapshot");
                const auto snapshot = memory->front().value;
                const auto found = array_values.find(snapshot);
                if (found == array_values.end()) throw error_t("Local array access lacks an allocated typed snapshot");
                const auto &array = local_arrays[found->second];
                auto element = storage.index ? reference(*storage.index, "num") : "0";
                const auto displacement = storage.offset - array.offset;
                if (displacement / array.width) element += " + " + std::to_string(displacement / array.width);
                auto expression = name(snapshot) + "[" + element + "]";
                return cast(std::move(expression), source.type(array.type), expected);
            }
            case lifted_address_t::kind_t::shared:
                base = source.classes[storage.owner] + "::__shared_" + std::to_string(storage.owner); break;
            case lifted_address_t::kind_t::instance:
                return "instance_storage<" + std::string(expected) + ">(" + reference(*storage.receiver, "opaque<4>") + ", " +
                       std::to_string(storage.offset) + (storage.index ? " + " + reference(*storage.index, "num") + " * " + std::to_string(storage.width) : "") + ')';
            case lifted_address_t::kind_t::game_variable:
            case lifted_address_t::kind_t::shared_variable:
                if (auto found = source.variables.find({storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                    found != source.variables.end() && !found->second.second.empty())
                    return cast(found->second.first, source.type(found->second.second), expected);
                return std::string(storage.kind == lifted_address_t::kind_t::game_variable ? "game_variable<" : "shared_variable<") +
                       std::string(expected) + ">(" + std::to_string(storage.offset) + ')';
            }
            return "storage_part<" + std::string(expected) + ">(" + base + ", " + std::to_string(storage.offset) +
                   (storage.index ? " + " + reference(*storage.index, "num") + " * " + std::to_string(storage.width) : "") + ')';
        }

        std::string expression(unsigned id) const
        {
            const auto &value = graph.values[id];
            const auto &instruction = binary.code.at(value.instruction);
            auto arg = [&](unsigned n, std::string type = "num") { return group(value.arguments.at(n), std::move(type)); };
            switch (value.kind) {
            case lifted_value_t::kind_t::operation: {
                switch (instruction.opcode) {
                case OP_NEG:
                    if (value.arguments[0].size() == 1) {
                        const auto ref = value.arguments[0][0];
                        if (!ref.offset && graph.values[ref.value].kind == lifted_value_t::kind_t::constant)
                            return number_literal(graph.values[ref.value].immediate ^ 0x80000000u);
                    }
                    return "(-" + arg(0) + ')';
                case OP_LNT: return "(!" + arg(0) + ')';
                case OP_NOT: return "(~" + arg(0) + ')';
                case OP_INC: return '(' + arg(0) + " + 1)";
                case OP_DEC: return '(' + arg(0) + " - 1)";
                case OP_I2S: return "to_integer_string(" + arg(0) + ')';
                case OP_F2S: return "to_float_string(" + arg(0) + ')';
                default: break;
                }
                static const std::map<opcode_t, std::string_view> operators{
                    {OP_ADD,"+"}, {OP_SUB,"-"}, {OP_MUL,"*"}, {OP_DIV,"/"}, {OP_MOD,"%"},
                    {OP_AND,"&"}, {OP_OR,"|"}, {OP_XOR,"^"}, {OP_SHL,"<<"}, {OP_SHR,">>"},
                    {OP_EQ,"=="}, {OP_NE,"!="}, {OP_LT,"<"}, {OP_LE,"<="}, {OP_GT,">"}, {OP_GE,">="},
                    {OP_STR_EQ,"=="}, {OP_STR_NE,"!="}, {OP_PSH_STR,"+"}
                };
                const auto operand = instruction.opcode == OP_STR_EQ || instruction.opcode == OP_STR_NE || instruction.opcode == OP_PSH_STR ? "str" : "num";
                return '(' + arg(0, operand) + ' ' + std::string(operators.at(instruction.opcode)) + ' ' + arg(1, operand) + ')';
            }
            case lifted_value_t::kind_t::native_call: {
                const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
                const auto method = native.signature.substr(0, native.signature.find('('));
                const bool receiver = native.class_name != "_global_slc" && method != native.class_name;
                if (receiver && method.starts_with("operator") && value.arguments.size() == 2)
                    return '(' + arg(0, native.parameters.at(0)) + ' ' + method.substr(8) + ' ' + arg(1, native.parameters.at(1)) + ')';
                const bool global = native.class_name == "_global_slc" &&
                    std::find(source.names[graph.object].begin(), source.names[graph.object].end(), method) == source.names[graph.object].end();
                std::string result = receiver ? '(' + arg(0, native.parameters.at(0)) + ")." :
                                     global || method == native.class_name ? "" : native.class_name + "::";
                result += method + '(';
                for (unsigned i = receiver ? 1 : 0; i < value.arguments.size(); ++i) {
                    if (i > (receiver ? 1u : 0u)) result += ", ";
                    result += arg(i, i < native.parameters.size() ? native.parameters[i] : "num");
                }
                return result + ')';
            }
            case lifted_value_t::kind_t::script_call: return call(value.immediate, value.arguments.at(0));
            case lifted_value_t::kind_t::load:
                return address(*value.address, value_type(id), value.arguments.empty() ? nullptr : &value.arguments[0]);
            case lifted_value_t::kind_t::signal:
                return "signal_id(" + (instruction.argument == OP_ARG_SIG ? std::string{} : arg(0, {}) + ", ") +
                       source.text(value.immediate >> 16) + ')';
            default: throw error_t("Value does not have a standalone source expression");
            }
        }

        void use(value_ref_t ref)
        {
            if (used[ref.value]) return;
            used[ref.value] = true;
            const auto &value = graph.values[ref.value];
            for (unsigned a = 0; a < value.arguments.size(); ++a) for (auto part : value.arguments[a]) {
                if (array_values.count(ref.value) && (value.kind != lifted_value_t::kind_t::indexed_store || a == 0) &&
                    graph.values[part.value].kind == lifted_value_t::kind_t::uninitialized) continue;
                use(part);
            }
            if (value.address) {
                if (value.address->receiver) use(*value.address->receiver);
                if (value.address->index) use(*value.address->index);
            }
        }

        bool stored(unsigned id) const
        {
            using kind = lifted_value_t::kind_t;
            const auto &value = graph.values[id];
            return used[id] && value.width && value.kind != kind::parameter && value.kind != kind::constant &&
                   value.kind != kind::string && value.kind != kind::class_value;
        }

        bool effectful(unsigned id) const
        {
            using kind = lifted_value_t::kind_t;
            const auto &value = graph.values[id];



            if (value.kind == kind::operation) return true;
            if (value.kind == kind::load && value.address)
                return value.address->index.has_value() || value.address->kind == lifted_address_t::kind_t::instance;
            return value.kind == kind::native_call || value.kind == kind::script_call ||
                   value.kind == kind::indexed_store || value.kind == kind::signal;
        }

        void allocate();
        bool emit_linear();
        bool emit_structured();
        bool emit_effect(unsigned block, const lifted_effect_t &effect, const std::string &line);

        void edge(unsigned from, unsigned to, std::string_view indentation, bool jump = true)
        {
            const auto &destination = graph.blocks[to];
            const auto &origin = graph.blocks[from];
            std::vector<std::pair<value_ref_t, value_ref_t>> copies;
            for (unsigned word = 0; word < destination.incoming.size(); ++word) {
                const auto ref = destination.incoming[word];
                if (used[ref.value] && graph.values[ref.value].kind == lifted_value_t::kind_t::merge)
                    copies.emplace_back(ref, origin.outgoing.at(word));
            }
            if (destination.incoming_array_index && used[destination.incoming_array_index->value])
                copies.emplace_back(*destination.incoming_array_index, *origin.outgoing_array_index);
            parallel_copy(copies, indentation);
            if (jump) source.out << indentation << "goto block" << to << ";\n";
        }

    public:
        body_t(source_emitter_t &source, unsigned index, std::string indent)
            : source(source), index(index), graph(source.graphs[index]), binary(source.binary(index)),
              types(source.types.functions[index]), used(graph.values.size()), reachable(graph.blocks.size()), indent(std::move(indent)) {}

        void emit();
        std::string initializer_arguments(unsigned target);
        std::string lifetime_initializer(bool constructor, bool destructor);
    };

    void function(unsigned index, std::string_view indentation);

public:
    source_emitter_t(const image_t &image, const symbols_t &symbols)
        : image(image), symbols(symbols), stack(analyze_stack(image, symbols)), graphs(lift_values(image, symbols, stack)),
          types(recover_types(image, symbols, stack, graphs))
    {
        for (unsigned i = 0; i < image.objects.size(); ++i) {
            auto name = identifier(image.objects[i].name, "Class");
            if (symbols.type_sizes.count(name) || std::find(classes.begin(), classes.end(), name) != classes.end()) name += "_class" + std::to_string(i);
            classes.push_back(std::move(name));
            names.emplace_back();
            for (unsigned f = 0; f < image.objects[i].functions.size(); ++f) {
                const auto &function = image.objects[i].functions[f];
                if (!f && function.name_hash == image.objects[i].name_hash) names.back().push_back(classes.back());
                else if (static_cast<int>(f) == image.objects[i].destructor) names.back().push_back('~' + classes.back());
                else {
                    auto name = identifier(std::string_view(function.name).substr(0, function.name.find('(')), "function");
                    const auto previous = std::find_if(image.objects[i].functions.begin(), image.objects[i].functions.begin() + f,
                        [&](const auto &other) { return identifier(std::string_view(other.name).substr(0, other.name.find('(')), "function") == name; });
                    if (previous != image.objects[i].functions.begin() + f) name += "_f" + std::to_string(f);
                    names.back().push_back(std::move(name));
                }
            }
        }
        parents = std::move(types.parents);
        for (unsigned i = 0; i < graphs.size(); ++i) {
            if (!graphs[i].diagnostics.empty()) throw error_t("Cannot decompile function: " + graphs[i].diagnostics.front());
            if (!graphs[i].initializer) functions.emplace(std::make_pair(graphs[i].object, graphs[i].function), i);
        }
        fields.resize(image.objects.size());
        instance_arrays.resize(image.objects.size());
        shared_fields.resize(image.objects.size());
        shared_arrays.resize(image.objects.size());
        shared_names.resize(image.objects.size());
        for (const auto &[name, identity] : symbols.variables) variables.emplace(identity, std::make_pair(name, std::string{}));
        for (const auto &graph : graphs) {
            for (const auto &value : graph.values)
                if (value.kind == lifted_value_t::kind_t::native_call && !value.type.empty() && !value.type.starts_with("opaque<"))
                    inferred_native_results[{value.immediate >> 16, value.immediate & 0xffff}] = value.type;
            auto offer = [&](const lifted_address_t &address, std::string type) {
                auto recover = [&](unsigned owner) {
                    auto found = types.storage.find({static_cast<unsigned>(address.kind), owner, address.offset, address.width});
                    if (found != types.storage.end()) type = found->second;
                };
                if (address.kind != lifted_address_t::kind_t::instance) recover(address.owner);
                if (address.kind == lifted_address_t::kind_t::shared && address.index) {
                    auto &array = shared_arrays[address.owner][address.offset];
                    if (array.width && array.width != address.width) throw error_t("Indexed static storage has conflicting element widths");
                    array.width = address.width;
                    if (array.type.empty() || array.type.starts_with("opaque<")) array.type = type.empty() ? opaque(address.width) : type;
                    const auto &index_value = graph.values[address.index->value];
                    if (!address.index->offset && index_value.kind == lifted_value_t::kind_t::constant) {
                        float value;
                        std::memcpy(&value, &index_value.immediate, 4);
                        if (value >= 0 && value < 65536) array.count = std::max(array.count, static_cast<unsigned>(value) + 1);
                    }
                    return;
                }
                if (address.kind == lifted_address_t::kind_t::shared && !address.index) {
                    auto [it, inserted] = shared_fields[address.owner].emplace(address.offset, field_t{address.width, type.empty() ? opaque(address.width) : type});
                    if (!inserted && it->second.type.starts_with("opaque<") && !type.empty()) it->second.type = type;
                    return;
                }
                if (address.kind == lifted_address_t::kind_t::game_variable || address.kind == lifted_address_t::kind_t::shared_variable) {
                    auto found = variables.find({address.kind == lifted_address_t::kind_t::game_variable, address.offset});
                    if (found != variables.end() && (found->second.second.empty() || found->second.second.starts_with("opaque<")))
                        found->second.second = type.empty() ? opaque(address.width) : type;
                    return;
                }
                if (!address.receiver || address.kind != lifted_address_t::kind_t::instance) return;
                auto owner = receiver_class(graph, *address.receiver);
                if (!owner || address.offset + address.width > image.objects[*owner].instance_size) return;
                for (unsigned depth = 0; depth < image.objects.size() && parents[*owner]; ++depth) {
                    auto parent = std::find_if(image.objects.begin(), image.objects.end(),
                        [&](const auto &candidate) { return candidate.name == *parents[*owner]; });
                    if (parent == image.objects.end() || address.offset + address.width > parent->instance_size) break;
                    owner = static_cast<unsigned>(parent - image.objects.begin());
                }
                recover(*owner);
                if (type.empty()) type = opaque(address.width);
                if (address.index) {
                    auto &array = instance_arrays[*owner][address.offset];
                    if (array.width && array.width != address.width) throw error_t("Indexed instance storage has conflicting element widths");
                    array.width = address.width;
                    if (array.type.empty() || array.type.starts_with("opaque<")) array.type = type;
                    const auto &index_value = graph.values[address.index->value];
                    if (!address.index->offset && index_value.kind == lifted_value_t::kind_t::constant) {
                        float value;
                        std::memcpy(&value, &index_value.immediate, 4);
                        if (value >= 0 && value < 65536) array.count = std::max(array.count, static_cast<unsigned>(value) + 1);
                    }
                    return;
                }
                auto [it, inserted] = fields[*owner].emplace(address.offset, field_t{address.width, type});
                if (!inserted && it->second.type.starts_with("opaque<") && !type.starts_with("opaque<")) it->second.type = type;
            };
            for (const auto &value : graph.values)
                if (value.kind == lifted_value_t::kind_t::load && value.address) offer(*value.address, value.type);
            for (const auto &block : graph.blocks) for (const auto &effect : block.effects)
                if (effect.kind == lifted_effect_t::kind_t::store && effect.address && !effect.arguments[0].empty()) {
                    const auto &value = graph.values[effect.arguments[0][0].value];
                    offer(*effect.address, value.width == effect.address->width ? value.type : opaque(effect.address->width));
                }
        }
        for (unsigned object = 0; object < image.objects.size(); ++object) {
            for (auto &[offset, array] : instance_arrays[object]) {
                const auto minimum_end = offset + array.width * array.count;
                auto end = image.objects[object].instance_size;
                const auto next_field = fields[object].lower_bound(minimum_end);
                if (next_field != fields[object].end()) end = std::min(end, next_field->first);
                const auto next_array = instance_arrays[object].upper_bound(offset);
                if (next_array != instance_arrays[object].end()) end = std::min(end, next_array->first);
                if (!array.width || end < minimum_end || (end - offset) % array.width)
                    throw error_t("Indexed instance storage exceeds its recovered extent");
                array.count = (end - offset) / array.width;
                auto field = fields[object].lower_bound(offset);
                while (field != fields[object].end() && field->first < end) field = fields[object].erase(field);
                fields[object][offset] = {end - offset, array.type};
            }
        }
        recover_static_fields();
        for (unsigned f = 0; f < graphs.size(); ++f) {
            auto &graph = graphs[f];
            for (auto &value : graph.values) {
                if (value.kind != lifted_value_t::kind_t::load || !value.address || !value.type.starts_with("opaque<")) continue;


                const auto &address = *value.address;
                if (auto member = shared_member(address, address.index ? "index" : "")) value.type = member->second;
                else if (auto member = instance_member(graph, address, address.index ? "index" : "")) value.type = member->second;
                else if (const auto *member = field(graph, address)) value.type = member->type;
            }
            for (const auto &block : graph.blocks) for (const auto &effect : block.effects) {
                if (effect.kind != lifted_effect_t::kind_t::store || !effect.address || effect.arguments[0].size() != 1) continue;
                const auto *member = field(graph, *effect.address);
                if (!member || member->type.starts_with("opaque<")) continue;
                const auto ref = effect.arguments[0][0];
                auto &value = graph.values[ref.value];
                if (ref.offset || value.width != member->width || value.kind != lifted_value_t::kind_t::parameter ||
                    !value.type.starts_with("opaque<")) continue;
                value.type = member->type;
                for (auto &parameter : types.functions[f].parameters)
                    if (parameter.offset == value.immediate) parameter.type = value.type;
            }
        }
    }

    std::string emit();
};

void source_emitter_t::body_t::prettify()
{
    using kind = lifted_value_t::kind_t;
    std::vector<unsigned> entities;
    for (unsigned id = 0; id < graph.values.size(); ++id) {
        const auto &value = graph.values[id];
        if (slots[id] == UINT32_MAX || value.kind != kind::native_call) continue;
        const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
        if (native.class_name == "_global_slc" && native.signature == "create_entity(str)") entities.push_back(id);
    }
    if (entities.empty()) return;

    std::set<std::string> occupied;
    for (auto id : slot_values) occupied.insert(name(id));
    for (const auto &parameter : types.parameters) occupied.insert("p" + std::to_string(parameter.offset));
    for (const auto &array : local_arrays) occupied.insert(array.name);
    occupied.insert(source.classes.begin(), source.classes.end());
    for (const auto &methods : source.names) occupied.insert(methods.begin(), methods.end());
    for (const auto &[name, size] : source.symbols.type_sizes) occupied.insert(name);
    for (const auto &[identity, variable] : source.variables) occupied.insert(variable.first);
    for (const auto &fields : source.shared_names)
        for (const auto &[offset, name] : fields) occupied.insert(name);
    for (const auto &fields : source.fields)
        for (const auto &[offset, field] : fields) occupied.insert("field_" + std::to_string(offset));
    for (const auto &[identity, native] : source.symbols.natives)
        occupied.insert(native.signature.substr(0, native.signature.find('(')));

    for (auto id : entities) {
        const auto slot = slots[id];
        if (pretty_slots.count(slot)) continue;
        const auto &value = graph.values[id];
        std::string_view path;
        if (!value.arguments.empty() && value.arguments[0].size() == 1) {
            const auto ref = value.arguments[0][0];
            const auto &argument = graph.values[ref.value];
            if (!ref.offset && argument.kind == kind::string) {
                const auto &bytes = source.image.strings.at(argument.immediate);
                const auto end = std::find(bytes.begin(), bytes.end(), 0);
                path = {reinterpret_cast<const char *>(bytes.data()), static_cast<std::size_t>(end - bytes.begin())};
            }
        }
        pretty_slots.emplace(slot, pretty::entity_local_name(path, occupied));
    }
}

std::string source_emitter_t::body_t::lifetime_initializer(bool constructor, bool destructor)
{
    if ((!constructor && !destructor) || !source.parents[graph.object]) return {};
    auto parent = std::find_if(source.image.objects.begin(), source.image.objects.end(),
        [&](const auto &object) { return object.name == *source.parents[graph.object]; });
    if (parent == source.image.objects.end()) throw error_t("Missing recovered base class");
    const auto owner = static_cast<unsigned>(parent - source.image.objects.begin());
    auto is_call = [&](const lifted_effect_t &effect, unsigned function) {
        if (effect.kind != lifted_effect_t::kind_t::value) return false;
        const auto &value = graph.values.at(effect.value);
        if (value.kind != lifted_value_t::kind_t::script_call ||
            source.image.object_order.at(value.immediate >> 16) != owner || (value.immediate & 0xffff) != function ||
            value.arguments.empty() || value.arguments.front().empty()) return false;
        const auto receiver = value.arguments.front().front();
        const auto &self = graph.values.at(receiver.value);
        return !receiver.offset && self.kind == lifted_value_t::kind_t::parameter && !self.immediate;
    };
    if (constructor) {
        if (parent->functions.empty() || parent->functions.front().name_hash != parent->name_hash) return {};
        auto &effects = graph.blocks.front().effects;
        if (effects.empty() || !is_call(effects.front(), 0))
            throw error_t("Recovered base constructor is not the constructor's first effect");
        const auto &call = graph.values.at(effects.front().value);
        std::string result = " : " + source.classes[owner] + '(';
        bool comma = false;
        for (const auto &parameter : source.types.functions.at(source.functions.at({owner, 0})).parameters) {
            if (!parameter.offset) continue;
            if (comma) result += ", ";
            comma = true;
            const auto &arguments = call.arguments.front();
            result += group({arguments.begin() + parameter.offset / 4,
                arguments.begin() + (parameter.offset + parameter.width) / 4}, parameter.type);
        }
        effects.erase(effects.begin());
        return result + ')';
    }
    if (parent->destructor < 0) return {};
    for (auto &block : graph.blocks) for (unsigned i = 0; i < block.effects.size(); ++i) {
        if (block.effects[i].kind != lifted_effect_t::kind_t::return_value) continue;
        if (!i || !is_call(block.effects[i - 1], static_cast<unsigned>(parent->destructor)))
            throw error_t("Recovered destructor does not end with its base destructor");
        block.effects.erase(block.effects.begin() + --i);
    }
    return {};
}

void source_emitter_t::body_t::recover_local_arrays()
{
    using value_kind = lifted_value_t::kind_t;
    using effect_kind = lifted_effect_t::kind_t;
    const auto &original = source.graphs[index];
    const auto &facts = source.stack.functions[index];
    std::map<unsigned, unsigned> locations, accesses;
    std::set<unsigned> producer_types;
    auto known = [](std::string_view type) { return !type.empty() && !type.starts_with("opaque<"); };
    for (unsigned id = 0; id < original.values.size(); ++id) {
        const auto &value = original.values[id];
        if (!value.address || value.address->kind != lifted_address_t::kind_t::local || !value.address->index) continue;
        const auto &address = *value.address;
        auto [location, inserted] = locations.emplace(address.offset, static_cast<unsigned>(local_arrays.size()));
        if (inserted) local_arrays.push_back({address.offset, address.width, 0, {}, {}});
        auto &array = local_arrays[location->second];
        if (array.width != address.width) throw error_t("Local array has conflicting indexed element widths");
        accesses[id] = location->second;
        std::string type = value.kind == value_kind::load ? value.type : std::string{};
        if (value.kind == value_kind::indexed_store) {
            const auto &refs = value.arguments.at(1);
            if (!refs.empty()) {
                const auto &stored = original.values[refs[0].value];
                bool whole = !refs[0].offset && stored.width == address.width;
                for (unsigned word = 1; whole && word < refs.size(); ++word)
                    whole = refs[word].value == refs[0].value && refs[word].offset == word * 4;
                if (whole) type = stored.type;
                if (whole && stored.address) {
                    const auto &storage = *stored.address;
                    if (storage.kind == lifted_address_t::kind_t::game_variable || storage.kind == lifted_address_t::kind_t::shared_variable) {
                        const auto symbol = source.variables.find({storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                        if (symbol != source.variables.end()) {
                            if (!known(type)) type = symbol->second.second;
                            auto stem = symbol->second.first;
                            if (stem.starts_with("gv_")) stem.erase(0, 3);
                            else if (stem.starts_with("g_")) stem.erase(0, 2);
                            const auto name = identifier(stem, "local") + "_values";
                            if (array.name.empty()) array.name = name;
                            else if (array.name != name) array.name = "local_values_" + std::to_string(array.offset);
                        }
                    }
                }
            }
        }
        if (known(type)) {
            const auto layout = location->second;
            if (value.kind == value_kind::indexed_store) {
                if (producer_types.insert(layout).second) array.type.clear();
            } else if (producer_types.count(layout)) continue;
            if (array.type.empty()) array.type = type;
            else if (array.type != type) {



                if (!producer_types.count(layout) && array.type == "num") array.type = type;
                else if (!producer_types.count(layout) && type == "num") continue;
                else if ((array.type == "vector3d" || array.type == "position3d") &&
                         (type == "vector3d" || type == "position3d")) array.type = "vector3d";
                else throw error_t("Local array has conflicting producer element types");
            }
        }
    }
    if (local_arrays.empty()) return;
    for (auto &array : local_arrays) {
        for (unsigned instruction = 0; instruction < binary.code.size(); ++instruction) {
            const auto &op = binary.code[instruction];
            if (op.opcode != OP_SPA || !facts.heights[instruction] || *facts.heights[instruction] < 0) continue;
            const auto begin = static_cast<unsigned>(*facts.heights[instruction]);
            const auto bytes = static_cast<std::int16_t>(op.value);
            if (bytes <= 0 || array.offset < begin || array.offset >= begin + static_cast<unsigned>(bytes)) continue;
            auto end = begin + static_cast<unsigned>(bytes);


            const auto next = locations.upper_bound(array.offset);
            if (next != locations.end()) end = std::min(end, next->first);
            if ((end - array.offset) % array.width) continue;
            const auto count = (end - array.offset) / array.width;
            if (array.count && array.count != count) throw error_t("Local array allocation extent is ambiguous");
            array.count = count;
        }
        if (!array.count) throw error_t("Indexed local storage has no matching fixed frame allocation");
        if (!known(array.type)) throw error_t("Indexed local storage has no evidenced element type");
        if (array.width != 4 && !(array.width == 12 && (array.type == "vector3d" || array.type == "position3d")))
            throw error_t("Local array element layout is unresolved");
        if (array.name.empty()) array.name = identifier(array.type, "local") + "_values_" + std::to_string(array.offset);
    }
    for (const auto &[id, layout] : accesses) {
        const auto &array = local_arrays[layout];
        const auto ref = *original.values[id].address->index;
        const auto &value = original.values[ref.value];
        if (!ref.offset && value.kind == value_kind::constant) {
            float number;
            std::memcpy(&number, &value.immediate, sizeof(number));
            if (!std::isfinite(number) || number < 0 || number >= array.count)
                throw error_t("Local array index exceeds its evidenced allocation");
        }
    }




    std::map<unsigned, unsigned> stores;
    std::map<unsigned, value_ref_t> phis;
    auto append = [&](lifted_value_t value, unsigned layout) {
        const auto id = static_cast<unsigned>(graph.values.size());
        const auto &array = local_arrays[layout];
        value.width = array.width * array.count;
        value.type = array.type;
        graph.values.push_back(std::move(value));
        array_values.emplace(id, layout);
        return id;
    };
    for (const auto &[id, layout] : accesses) {
        if (original.values[id].kind != value_kind::indexed_store) continue;
        auto value = original.values[id];
        const auto &array = local_arrays[layout];
        const auto &memory = value.arguments[0];
        value.arguments[0] = {memory.begin() + array.offset / 4, memory.begin() + (array.offset + array.width * array.count) / 4};
        stores[id] = append(std::move(value), layout);
    }
    for (unsigned b = 0; b < original.blocks.size(); ++b) {
        if (!reachable[b]) continue;
        const auto &block = original.blocks[b];
        for (unsigned layout = 0; layout < local_arrays.size(); ++layout) {
            const auto &array = local_arrays[layout];
            const auto first = array.offset / 4, words = array.width * array.count / 4;
            bool merged = true;
            for (unsigned word = 0; word < words; ++word)
                if (original.values[block.incoming.at(first + word).value].kind != value_kind::merge) merged = false;
            if (!merged) continue;
            lifted_value_t value;
            value.kind = value_kind::merge;
            value.instruction = block.begin;
            const auto &first_value = original.values[block.incoming[first].value];
            value.arguments.resize(first_value.arguments.size());
            for (unsigned word = 0; word < words; ++word) {
                const auto old = block.incoming[first + word];
                const auto &part = original.values[old.value];
                for (unsigned incoming = 0; incoming < value.arguments.size(); ++incoming)
                    value.arguments[incoming].push_back(part.arguments.at(incoming).at(0));
            }
            const auto id = append(std::move(value), layout);
            for (unsigned word = 0; word < words; ++word) phis.emplace(block.incoming[first + word].value, value_ref_t{id, word * 4});
        }
    }
    std::function<value_ref_t(value_ref_t)> resolve = [&](value_ref_t ref) -> value_ref_t {
        if (auto phi = phis.find(ref.value); phi != phis.end())
            return {phi->second.value, phi->second.offset + ref.offset};
        if (auto store = stores.find(ref.value); store != stores.end()) {
            const auto &array = local_arrays[array_values.at(store->second)];
            if (ref.offset >= array.offset && ref.offset < array.offset + array.width * array.count)
                return {store->second, ref.offset - array.offset};
            return resolve(original.values[ref.value].arguments[0].at(ref.offset / 4));
        }
        return ref;
    };
    auto resolve_group = [&](auto &refs) { for (auto &ref : refs) ref = resolve(ref); };
    auto resolve_address = [&](auto &address) {
        if (!address) return;
        if (address->receiver) address->receiver = resolve(*address->receiver);
        if (address->index) address->index = resolve(*address->index);
    };
    for (auto &value : graph.values) {
        for (auto &argument : value.arguments) resolve_group(argument);
        resolve_address(value.address);
    }
    for (auto &block : graph.blocks) {
        resolve_group(block.incoming);
        resolve_group(block.outgoing);
        if (block.incoming_array_index) block.incoming_array_index = resolve(*block.incoming_array_index);
        if (block.outgoing_array_index) block.outgoing_array_index = resolve(*block.outgoing_array_index);
        for (auto &effect : block.effects) {
            for (auto &argument : effect.arguments) resolve_group(argument);
            resolve_address(effect.address);
            if (auto store = stores.find(effect.value); effect.kind == effect_kind::value && store != stores.end()) effect.value = store->second;
        }
    }
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b]) continue;
        std::vector<lifted_effect_t> effects;
        std::vector<unsigned> snapshots;
        for (auto effect : graph.blocks[b].effects) {
            if (effect.kind == effect_kind::value && accesses.count(effect.value) &&
                graph.values[effect.value].kind == value_kind::load) {
                const auto layout = accesses.at(effect.value);
                const auto &array = local_arrays[layout];
                auto memory = graph.values[effect.value].arguments[0];
                memory = {memory.begin() + array.offset / 4, memory.begin() + (array.offset + array.width * array.count) / 4};
                auto snapshot = memory.front().value;
                bool whole = array_values.count(snapshot) && array_values.at(snapshot) == layout;
                for (unsigned word = 0; whole && word < memory.size(); ++word)
                    whole = memory[word] == value_ref_t{snapshot, word * 4};
                if (!whole) {
                    const auto existing = std::find_if(snapshots.begin(), snapshots.end(), [&](unsigned id) {
                        return array_values.at(id) == layout && graph.values[id].arguments[0] == memory;
                    });
                    if (existing != snapshots.end()) snapshot = *existing;
                    else {
                        lifted_value_t value;
                        value.kind = value_kind::merge;
                        value.instruction = effect.instruction;
                        value.arguments.push_back(std::move(memory));
                        snapshot = append(std::move(value), layout);
                        snapshots.push_back(snapshot);
                        lifted_effect_t packed;
                        packed.instruction = effect.instruction;
                        packed.value = snapshot;
                        effects.push_back(std::move(packed));
                    }
                }
                auto &value = graph.values[effect.value];
                value.arguments[0].clear();
                for (unsigned offset = 0; offset < array.width * array.count; offset += 4)
                    value.arguments[0].push_back({snapshot, offset});
                if (!known(value.type)) value.type = array.type;
            }
            effects.push_back(std::move(effect));
        }
        graph.blocks[b].effects = std::move(effects);
    }
}

void source_emitter_t::body_t::recover_native_values()
{
    using kind = lifted_value_t::kind_t;
    for (bool changed = true; changed;) {
        changed = false;
        std::map<unsigned, value_ref_t> replacements;
        for (const auto &block : graph.blocks) {
            for (unsigned start = 0; start < block.incoming.size(); ++start) {
                const auto first = block.incoming[start];
                const auto &phi = graph.values[first.value];
                if (first.offset || phi.kind != kind::merge || phi.width != 4 || replacements.count(first.value)) continue;
                unsigned width = 0;
                std::string type;
                for (unsigned edge = 0; edge < phi.arguments.size(); ++edge) {
                    if (phi.arguments[edge].size() != 1) continue;
                    const auto ref = phi.arguments[edge][0];
                    const auto &producer = graph.values[ref.value];
                    if (ref.offset || producer.width <= 4 || producer.type.empty() ||
                        producer.type.starts_with("opaque<") || array_values.count(ref.value)) continue;
                    const auto count = producer.width / 4;
                    if (start + count > block.incoming.size()) continue;
                    bool complete = true;
                    for (unsigned word = 0; word < count; ++word) {
                        const auto part = block.incoming[start + word];
                        const auto &value = graph.values[part.value];
                        if (part.offset || value.kind != kind::merge || value.width != 4 ||
                            value.arguments.size() != phi.arguments.size() || value.arguments[edge].size() != 1 ||
                            value.arguments[edge][0] != value_ref_t{ref.value, word * 4} || replacements.count(part.value)) {
                            complete = false;
                            break;
                        }
                    }
                    if (complete) { width = producer.width; type = producer.type; break; }
                }
                if (!width) continue;
                lifted_value_t aggregate;
                aggregate.kind = kind::merge;
                aggregate.width = width;
                aggregate.type = std::move(type);
                aggregate.instruction = block.begin;
                aggregate.arguments.resize(phi.arguments.size());
                for (unsigned word = 0; word < width / 4; ++word) {
                    const auto &value = graph.values[block.incoming[start + word].value];
                    for (unsigned edge = 0; edge < aggregate.arguments.size(); ++edge)
                        aggregate.arguments[edge].push_back(value.arguments[edge][0]);
                }
                const auto id = static_cast<unsigned>(graph.values.size());
                graph.values.push_back(std::move(aggregate));
                for (unsigned word = 0; word < width / 4; ++word)
                    replacements.emplace(block.incoming[start + word].value, value_ref_t{id, word * 4});
                start += width / 4 - 1;
                changed = true;
            }
        }
        if (!changed) break;
        auto replace = [&](value_ref_t &ref) {
            if (auto found = replacements.find(ref.value); found != replacements.end())
                ref = {found->second.value, found->second.offset + ref.offset};
        };
        auto group = [&](auto &refs) { for (auto &ref : refs) replace(ref); };
        auto address = [&](auto &storage) {
            if (!storage) return;
            if (storage->receiver) replace(*storage->receiver);
            if (storage->index) replace(*storage->index);
        };
        for (auto &value : graph.values) {
            for (auto &argument : value.arguments) group(argument);
            address(value.address);
        }
        for (auto &block : graph.blocks) {
            group(block.incoming);
            group(block.outgoing);
            if (block.incoming_array_index) replace(*block.incoming_array_index);
            if (block.outgoing_array_index) replace(*block.outgoing_array_index);
            for (auto &effect : block.effects) {
                for (auto &argument : effect.arguments) group(argument);
                address(effect.address);
            }
        }
    }
}

void source_emitter_t::body_t::declaration(unsigned id)
{
    if (!required_slots.empty() && !required_slots[slots[id]]) return;
    source.out << indent << value_type(id) << ' ' << name(id);
    if (auto array = array_values.find(id); array != array_values.end())
        source.out << '[' << local_arrays[array->second].count << ']';
    source.out << ";\n";
}

void source_emitter_t::body_t::parallel_copy(
    const std::vector<std::pair<value_ref_t, value_ref_t>> &transfers, std::string_view line)
{
    using location_t = std::pair<unsigned, unsigned>;
    struct copy_t {
        location_t destination;
        std::string target, type, expression;
        std::optional<location_t> dependency;
    };
    std::vector<copy_t> copies;
    for (unsigned i = 0; i < transfers.size(); ++i) {
        const auto [target, input] = transfers[i];
        if (graph.values[input.value].kind == lifted_value_t::kind_t::uninitialized) continue;
        const location_t destination{slots[target.value], target.offset};
        const auto dependency = stored(input.value) ? std::optional<location_t>{{slots[input.value], input.offset}} : std::nullopt;
        if (dependency == destination) continue;
        const auto width = graph.values[target.value].width;
        bool whole = !target.offset && !input.offset && width > 4 && graph.values[input.value].width == width &&
            !array_values.count(target.value) && !array_values.count(input.value) && i + width / 4 <= transfers.size();
        for (unsigned word = 0; whole && word < width / 4; ++word)
            whole = transfers[i + word] == std::make_pair(value_ref_t{target.value, word * 4}, value_ref_t{input.value, word * 4});
        if (whole) {
            std::vector<value_ref_t> words;
            for (unsigned word = 0; word < width / 4; ++word) words.push_back({input.value, word * 4});
            const auto type = value_type(target.value);
            copies.push_back({destination, name(target.value), type, group(words, type), dependency});
            i += width / 4 - 1;
            continue;
        }
        const auto type = reference_type(target);
        copies.push_back({destination, reference(target), type, reference(input, type), dependency});
    }
    unsigned saved = 0;
    const auto temporary = "edge" + std::to_string(edge_serial++);
    while (!copies.empty()) {
        auto ready = std::find_if(copies.begin(), copies.end(), [&](const auto &copy) {
            return std::none_of(copies.begin(), copies.end(), [&](const auto &other) { return other.dependency == copy.destination; });
        });
        if (ready != copies.end()) {
            source.out << line << ready->target << " = " << ready->expression << ";\n";
            copies.erase(ready);
        } else {
            auto &copy = copies.front();
            const auto saved_name = temporary + "_" + std::to_string(saved++);
            source.out << line << copy.type << ' ' << saved_name << " = " << copy.expression << ";\n";
            copy.expression = saved_name;
            copy.dependency.reset();
        }
    }
}

void source_emitter_t::body_t::array_copy(unsigned id, const std::vector<value_ref_t> &refs, std::string_view line)
{
    std::vector<std::pair<value_ref_t, value_ref_t>> transfers;
    transfers.reserve(refs.size());
    for (unsigned word = 0; word < refs.size(); ++word) transfers.push_back({{id, word * 4}, refs[word]});
    parallel_copy(transfers, line);
}

void source_emitter_t::body_t::entry_values()
{
    std::set<unsigned> initialized;
    for (auto ref : graph.blocks[0].incoming) {
        const auto &value = graph.values[ref.value];
        if (!used[ref.value] || value.kind != lifted_value_t::kind_t::merge || !initialized.insert(ref.value).second) continue;
        if (array_values.count(ref.value)) array_copy(ref.value, value.arguments.at(0), indent);
        else source.out << indent << name(ref.value) << " = " << group(value.arguments.at(0), value_type(ref.value)) << ";\n";
    }
}

bool source_emitter_t::body_t::emit_linear()
{
    using effect_kind = lifted_effect_t::kind_t;
    using value_kind = lifted_value_t::kind_t;
    if (graph.blocks.size() != 1 || !graph.blocks[0].successors.empty()) return false;
    if (!local_arrays.empty()) return false;
    const auto &effects = graph.blocks[0].effects;
    std::vector<unsigned> uses(graph.values.size());
    std::vector<std::set<unsigned>> inputs(effects.size());
    for (unsigned i = 0; i < effects.size(); ++i) {
        const auto &effect = effects[i];
        if (effect.kind == effect_kind::value && !used[effect.value] && !effectful(effect.value)) continue;
        if (effect.kind != effect_kind::value && effect.kind != effect_kind::store && effect.kind != effect_kind::return_value) return false;
        auto add_group = [&](const auto &refs) {
            std::set<unsigned> unique;
            for (auto ref : refs) unique.insert(ref.value);
            for (auto id : unique) { ++uses[id]; inputs[i].insert(id); }
        };
        auto add_address = [&](const std::optional<lifted_address_t> &address) {
            if (!address) return;
            if (address->receiver) add_group(std::vector<value_ref_t>{*address->receiver});
            if (address->index) add_group(std::vector<value_ref_t>{*address->index});
        };
        if (effect.kind == effect_kind::value) {
            const auto &value = graph.values[effect.value];
            if (value.kind == value_kind::indexed_store || value.kind == value_kind::signal) return false;
            for (const auto &argument : value.arguments) add_group(argument);
            add_address(value.address);
        } else {
            for (const auto &argument : effect.arguments) add_group(argument);
            add_address(effect.address);
        }
    }
    std::vector<bool> inline_value(graph.values.size()), skipped(effects.size());
    std::map<unsigned, std::string> increments;
    for (unsigned i = 0; i + 2 < effects.size(); ++i) {
        if (effects[i].kind != effect_kind::value || effects[i + 1].kind != effect_kind::value || effects[i + 2].kind != effect_kind::store) continue;
        const auto load_id = effects[i].value, update_id = effects[i + 1].value;
        const auto &load = graph.values[load_id], &update = graph.values[update_id];
        const auto &store = effects[i + 2];
        if (load.kind != value_kind::load || !load.address || update.kind != value_kind::operation ||
            !store.address || uses[load_id] != 1 || update.arguments.empty() || update.arguments[0] != std::vector<value_ref_t>{{load_id, 0}} ||
            store.arguments[0] != std::vector<value_ref_t>{{update_id, 0}}) continue;
        const auto *member = source.field(graph, *load.address);
        if (!member || member->type != "num" || load.address->receiver != store.address->receiver ||
            load.address->offset != store.address->offset || load.address->kind != store.address->kind) continue;
        const auto op = binary.code[update.instruction].opcode;
        bool increment = op == OP_INC, decrement = op == OP_DEC;
        if ((op == OP_ADD || op == OP_SUB) && update.arguments.size() == 2 && update.arguments[1].size() == 1) {
            const auto ref = update.arguments[1][0];
            if (!ref.offset && graph.values[ref.value].kind == value_kind::constant && graph.values[ref.value].immediate == 0x3f800000u) {
                increment = op == OP_ADD; decrement = op == OP_SUB;
            }
        }
        if (!increment && !decrement) continue;
        const bool returns = i + 3 < effects.size() && effects[i + 3].kind == effect_kind::return_value &&
                             effects[i + 3].arguments == std::vector<std::vector<value_ref_t>>{{{update_id, 0}}};
        if (uses[update_id] != (returns ? 2u : 1u)) continue;
        increments[i] = (returns ? "return " : "") + std::string(increment ? "++" : "--") + address(*load.address, "num") + ";";
        skipped[i + 1] = skipped[i + 2] = true;
        if (returns) skipped[i + 3] = true;
        inline_value[load_id] = inline_value[update_id] = true;
    }
    for (unsigned i = 0; i + 1 < effects.size(); ++i)
        if (!skipped[i] && !increments.count(i) && effects[i].kind == effect_kind::value && uses[effects[i].value] == 1 && inputs[i + 1].count(effects[i].value))
            inline_value[effects[i].value] = true;
    std::set<unsigned> declared;
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && !inline_value[id] && declared.insert(slots[id]).second)
            source.out << indent << value_type(id) << ' ' << name(id) << ";\n";
    for (unsigned i = 0; i < effects.size(); ++i) {
        if (skipped[i]) continue;
        if (increments.count(i)) { source.out << indent << increments[i] << '\n'; continue; }
        const auto &effect = effects[i];
        if (effect.kind == effect_kind::value) {
            if (!used[effect.value] && !effectful(effect.value)) continue;
            const auto &value = graph.values[effect.value];
            auto rendered = expression(effect.value);
            if (inline_value[effect.value]) folded[effect.value] = std::move(rendered);
            else {
                source.out << indent;
                if (value.width && used[effect.value]) source.out << name(effect.value) << " = ";
                source.out << rendered << ";\n";
            }
        } else if (effect.kind == effect_kind::store) {
            const auto type = store_type(*effect.address);
            source.out << indent << address(*effect.address, type) << " = " << group(effect.arguments[0], type) << ";\n";
        } else {
            source.out << indent << "return";
            if (!effect.arguments.empty() && !effect.arguments[0].empty()) source.out << ' ' << group(effect.arguments[0], source.type(types.result));
            source.out << ";\n";
        }
    }
    return true;
}
void source_emitter_t::recover_static_fields()
{
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        for (auto &[offset, array] : shared_arrays[object]) {
            unsigned end = static_cast<unsigned>(image.objects[object].static_data.size());
            const unsigned minimum_end = offset + array.width * array.count;
            const auto next_field = shared_fields[object].lower_bound(minimum_end);
            if (next_field != shared_fields[object].end()) end = std::min(end, next_field->first);
            const auto next_array = shared_arrays[object].upper_bound(offset);
            if (next_array != shared_arrays[object].end()) end = std::min(end, next_array->first);
            if (!object) for (const auto &initializer : image.initializers)
                if (initializer.offset >= minimum_end &&
                    (initializer.name_hash || (initializer.kind != -4 &&
                     !(initializer.kind >= 0 && array.type == "str"))))
                    end = std::min(end, initializer.offset);
            if (!array.width || end < minimum_end) throw error_t("Indexed static storage exceeds its recovered extent");
            array.count = (end - offset) / array.width;
            if (array.type.empty() || array.type.starts_with("opaque<")) {
                if (array.width == 4) array.type = "num";
                else if (array.width == 12) array.type = "vector3d";
                else throw error_t("Indexed static storage has an unresolved element type");
            }
        }
    }
    for (const auto &initializer : image.initializers) {
        auto &field = shared_fields[0][initializer.offset];
        if (initializer.kind != -4 || !field.width) field.width = 4;
        if (initializer.kind >= 0) field.type = "str";
        else if (initializer.kind != -4) field.type = image.objects.at(image.object_order.at(initializer.object)).name;
        else if (field.type.empty() || field.type.starts_with("opaque<")) field.type = "num";
        if (initializer.name_hash) {
            auto symbol = symbols.names.find(initializer.name_hash);
            shared_names[0][initializer.offset] = symbol == symbols.names.end() ?
                "global_" + hex(initializer.name_hash).substr(2) : identifier(symbol->second, "global");
        }
    }
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        for (const auto &[offset, array] : shared_arrays[object]) {
            const unsigned end = offset + array.width * array.count;
            auto next = shared_fields[object].lower_bound(offset);
            while (next != shared_fields[object].end() && next->first < end) {
                if (next->first != offset) shared_names[object].erase(next->first);
                next = shared_fields[object].erase(next);
            }
            shared_fields[object][offset] = {array.width * array.count, array.type};
        }
        for (unsigned offset = 0; offset < image.objects[object].static_data.size();) {
            auto [field, inserted] = shared_fields[object].emplace(offset, field_t{4, "num"});
            if (field->second.width == 0) throw error_t("Static field has no recovered width");
            if (!shared_names[object].count(offset)) shared_names[object][offset] = "global_" + std::to_string(offset);
            auto next = shared_fields[object].upper_bound(offset);
            while (next != shared_fields[object].end() && next->first < offset + field->second.width) {
                if ((field->second.type != "vector3d" && field->second.type != "position3d") ||
                    next->second.width != 4 || next->second.type != "num")
                    throw error_t("Conflicting overlapping static field types");
                shared_names[object].erase(next->first);
                next = shared_fields[object].erase(next);
            }
            offset += field->second.width;
        }
    }
}

std::optional<std::pair<std::string, std::string>> source_emitter_t::shared_member(const lifted_address_t &address, std::string_view index) const
{
    if (address.kind != lifted_address_t::kind_t::shared) return {};
    const auto &members = shared_fields[address.owner];
    auto found = members.upper_bound(address.offset);
    if (found == members.begin()) return {};
    --found;
    const auto displacement = address.offset - found->first;
    auto name = classes[address.owner] + "::" + shared_names[address.owner].at(found->first);
    if (auto array = shared_arrays[address.owner].find(found->first); array != shared_arrays[address.owner].end()) {
        const auto &element = array->second;
        if (displacement >= element.width * element.count) return {};
        name += "[";
        if (address.index) {
            if (index.empty()) return {};
            name += index;
            if (displacement / element.width) name += " + " + std::to_string(displacement / element.width);
        } else name += std::to_string(displacement / element.width);
        name += "]";
        if (address.width == element.width && displacement % element.width == 0)
            return std::make_pair(name, type(element.type));
        if (address.width == 4 && (element.type == "vector3d" || element.type == "position3d")) {
            name += ".";
            name += "xyz"[(displacement % element.width) / 4];
            return std::make_pair(name, "num");
        }
        return {};
    }
    if (address.index) return {};
    if ((found->second.type == "vector3d" || found->second.type == "position3d") && address.width == 4 && displacement < 12) {
        name += ".";
        name += "xyz"[displacement / 4];
        return std::make_pair(name, "num");
    }
    if (displacement || address.width != found->second.width) return {};
    return std::make_pair(name, type(found->second.type));
}

std::string source_emitter_t::body_t::initializer_arguments(unsigned target)
{
    if (graph.blocks.size() != 1) throw error_t("Constructor argument builder requires structured expression recovery");
    const std::vector<value_ref_t> *result = nullptr;
    for (const auto &effect : graph.blocks.front().effects) {
        if (effect.kind == lifted_effect_t::kind_t::return_value) {
            if (effect.arguments.size() != 1) throw error_t("Constructor argument builder has no return storage");
            result = &effect.arguments.front();
            continue;
        }
        if (effect.kind != lifted_effect_t::kind_t::value)
            throw error_t("Constructor argument builder has non-expression effects");
        const auto &value = graph.values[effect.value];
        if (value.kind == lifted_value_t::kind_t::native_call) {
            const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
            if (native.class_name != "vector3d" || native.signature != "vector3d(num,num,num)")
                throw error_t("Constructor builder native call requires ordered statement recovery");
        } else if (value.kind != lifted_value_t::kind_t::operation && value.kind != lifted_value_t::kind_t::load)
            throw error_t("Constructor argument builder has an unresolved value producer");
        folded[effect.value] = expression(effect.value);
    }
    if (!result) throw error_t("Constructor argument builder does not return");
    const auto constructor = source.functions.at({target, 0});
    const auto &signature = source.types.functions[constructor];
    const auto &record = source.binary(constructor);
    std::string arguments;
    unsigned consumed = 0;
    for (const auto &parameter : signature.parameters) {
        if (!(record.flags & 1u) && parameter.offset == 0) continue;
        const unsigned words = parameter.width / 4;
        if (consumed + words > result->size()) throw error_t("Constructor argument builder returns insufficient storage");
        if (!arguments.empty()) arguments += ", ";
        arguments += group({result->begin() + consumed, result->begin() + consumed + words}, source.type(parameter.type));
        consumed += words;
    }
    if (consumed != result->size()) throw error_t("Constructor argument builder returns excess storage");
    return arguments;
}

void source_emitter_t::emit_static_fields(unsigned object)
{
    const auto &data = image.objects[object].static_data;
    for (const auto &[offset, field] : shared_fields[object]) {
        const auto name = shared_names[object].find(offset);
        if (name == shared_names[object].end()) throw error_t("Overlapping static field interpretations require reconciliation");
        out << "        static " << type(field.type) << ' ' << name->second;
        if (auto array = shared_arrays[object].find(offset); array != shared_arrays[object].end()) {
            const auto &element = array->second;
            out << '[' << element.count << "] = {";
            for (unsigned index = 0; index < element.count; ++index) {
                if (index) out << ", ";
                const bool vector = element.type == "vector3d" || element.type == "position3d";
                if (vector) out << element.type << '{';
                for (unsigned component = 0; component < element.width / 4; ++component) {
                    if (component) out << ", ";
                    const unsigned location = offset + index * element.width + component * 4;
                    auto bits = read_word(data, location);
                    const initializer_t *initial = nullptr;
                    if (!object) for (const auto &candidate : image.initializers)
                        if (candidate.offset == location) { initial = &candidate; break; }
                    if (initial && initial->kind == -4) bits = initial->value;
                    if (initial && initial->kind >= 0) out << text(static_cast<unsigned>(initial->kind));
                    else if (vector || element.type == "num") out << number_literal(bits);
                    else if (!bits) out << "nullptr";
                    else throw error_t("Static handle array contains an unresolved non-null initializer");
                }
                if (vector) out << '}';
            }
            out << "};\n";
            continue;
        }
        const initializer_t *initializer = nullptr;
        if (!object) for (const auto &candidate : image.initializers)
            if (candidate.offset == offset) { initializer = &candidate; break; }
        if (initializer && initializer->kind >= 0) out << " = " << text(static_cast<unsigned>(initializer->kind));
        else if (initializer && (initializer->kind == -3 || initializer->kind == -2 || initializer->kind == -1)) {
            out << " = " << type(field.type) << (initializer->kind == -2 ? "::from_game_data(" : "::create_named(");
            auto symbol = symbols.names.find(initializer->name_hash);
            out << (symbol == symbols.names.end() ? hex(initializer->name_hash) : quote(symbol->second));
            if (initializer->builder) {
                const auto position = static_cast<unsigned>(initializer - image.initializers.data());
                for (unsigned f = 0; f < graphs.size(); ++f) {
                    if (!graphs[f].initializer || graphs[f].function != position) continue;
                    auto arguments = body_t(*this, f, "").initializer_arguments(image.object_order.at(initializer->object));
                    if (!arguments.empty()) out << ", " << arguments;
                    break;
                }
            }
            out << ')';
        }
        else if (!initializer || initializer->kind == -4) {
            if (field.width == 4) {
                const auto bits = initializer ? initializer->value : read_word(data, offset);
                if (field.type == "num") out << " = " << number_literal(bits);
                else if (!bits) out << " = nullptr";
                else out << " = bit_cast<" << type(field.type) << ">(" << number_literal(bits) << ')';
            } else if (field.type == "vector3d" || field.type == "position3d") {
                out << " = " << field.type << '{';
                for (unsigned component = 0; component < 3; ++component) {
                    if (component) out << ", ";
                    auto bits = read_word(data, offset + component * 4);
                    if (!object) for (const auto &candidate : image.initializers)
                        if (candidate.offset == offset + component * 4 && candidate.kind == -4) bits = candidate.value;
                    out << number_literal(bits);
                }
                out << '}';
            }
        }
        out << ";\n";
    }
}

bool source_emitter_t::body_t::emit_effect(unsigned b, const lifted_effect_t &effect, const std::string &line)
{
    using effect_kind = lifted_effect_t::kind_t;
    using value_kind = lifted_value_t::kind_t;
    const auto &block = graph.blocks[b];
    const auto &instruction = binary.code[effect.instruction];
    bool ended = false;
            switch (effect.kind) {
            case effect_kind::value: {
                const auto &value = graph.values[effect.value];
                if (!used[effect.value] && !effectful(effect.value)) break;
                if (!adjacent_expressions.empty() && adjacent_expressions[effect.value]) {
                    folded[effect.value] = expression(effect.value);
                    break;
                }
                if (array_values.count(effect.value)) {
                    if (value.kind == value_kind::indexed_store) {
                        const auto &storage = *value.address;
                        const auto &array = local_arrays[array_values.at(effect.value)];
                        auto element = reference(*storage.index, "num");
                        auto stored_value = group(value.arguments[1], source.type(array.type));
                        const auto destination = slots[effect.value];
                        const auto aliases = [&](value_ref_t ref) {
                            return stored(ref.value) && slots[ref.value] == destination;
                        };
                        bool changes = false;
                        for (unsigned word = 0; word < value.arguments[0].size(); ++word) {
                            const auto ref = value.arguments[0][word];
                            if (graph.values[ref.value].kind != value_kind::uninitialized &&
                                (!aliases(ref) || ref.offset != word * 4)) changes = true;
                        }
                        if (changes && (aliases(*storage.index) || std::any_of(value.arguments[1].begin(), value.arguments[1].end(), aliases))) {
                            const auto temporary = "array_write" + std::to_string(edge_serial++);
                            source.out << line << "num " << temporary << "_index = " << element << ";\n";
                            source.out << line << source.type(array.type) << ' ' << temporary << "_value = " << stored_value << ";\n";
                            element = temporary + "_index";
                            stored_value = temporary + "_value";
                        }
                        array_copy(effect.value, value.arguments[0], line);
                        source.out << line << name(effect.value) << '[' << element << "] = " << stored_value << ";\n";
                    } else array_copy(effect.value, value.arguments[0], line);
                } else {
                    source.out << line;
                    if (value.width && used[effect.value]) source.out << name(effect.value) << " = ";
                    auto actual = value_type(effect.value);
                    if (value.kind == value_kind::operation)
                        actual = instruction.opcode == OP_I2S || instruction.opcode == OP_F2S || instruction.opcode == OP_PSH_STR ? "str" : "num";
                    else if (value.kind == value_kind::signal) actual = "signal";
                    else if (value.kind == value_kind::script_call) actual = source.type(source.types.functions[source.target(value.immediate)].result);
                    else if (value.kind == value_kind::native_call) {
                        const auto identity = std::make_pair(value.immediate >> 16, value.immediate & 0xffff);
                        actual = source.native_result(identity);
                    }
                    auto rendered = expression(effect.value);
                    source.out << (value.width && used[effect.value] ? cast(std::move(rendered), actual, value_type(effect.value)) : rendered) << ";\n";
                }
                break;
            }
            case effect_kind::store:
                {
                    const auto type = store_type(*effect.address);
                    source.out << line << address(*effect.address, type) << " = " << group(effect.arguments[0], type) << ";\n";
                }
                break;
            case effect_kind::branch:
                if (effect.arguments.empty() || block.successors.size() == 1) edge(b, block.successors.at(0), line);
                else {
                    const auto destination = static_cast<unsigned>(static_cast<std::int64_t>(instruction.offset) + instruction.size() +
                                                                   static_cast<std::int16_t>(instruction.value));
                    auto taken = block.successors.at(0), other = block.successors.at(1);
                    if (binary.code[graph.blocks[taken].begin].offset != destination) std::swap(taken, other);
                    source.out << line << "if (!" << group(effect.arguments[0], "num") << ") {\n";
                    edge(b, taken, line + "    ");
                    source.out << line << "} else {\n";
                    edge(b, other, line + "    ");
                    source.out << line << "}\n";
                }
                ended = true;
                break;
            case effect_kind::return_value:
                source.out << line << "return";
                if (!effect.arguments.empty() && !effect.arguments[0].empty())
                    source.out << ' ' << group(effect.arguments[0], source.type(types.result));
                source.out << ";\n";
                ended = true;
                break;
            case effect_kind::thread: {
                const bool owner = instruction.opcode == OP_BTH;
                source.out << line << (owner ? "start_thread_on(" : "start_thread(");
                if (owner) source.out << group(effect.arguments[0]) << ", ";
                source.out << call(instruction.value, effect.arguments[owner ? 1 : 0]) << ");\n";
                break;
            }
            case effect_kind::callback: {
                const bool owner = instruction.opcode == OP_ECB || instruction.opcode == OP_ECO;
                const bool once = instruction.opcode == OP_ECO || instruction.opcode == OP_SCO;
                source.out << line << (once ? "bind_signal_once" : "bind_signal") << (owner ? "_on(" : "(");
                if (owner) source.out << group(effect.arguments[0]) << ", ";
                source.out << group(effect.arguments[owner ? 2 : 1], "signal") << ", "
                           << call(instruction.value, effect.arguments[owner ? 1 : 0]) << ");\n";
                break;
            }
            case effect_kind::signal:
                source.out << line << (instruction.opcode == OP_RAE ? "raise_all_signals(" : "raise_signal(");
                if (!effect.arguments.empty()) source.out << group(effect.arguments[0]) << ", ";
                source.out << source.text(instruction.value >> 16) << ");\n";
                break;
            case effect_kind::termination: {
                const bool massacre = instruction.opcode == OP_DEL_THREADS || instruction.opcode == OP_MS2;
                if (instruction.argument == OP_ARG_NULL) {
                    source.out << line << (massacre ? "delete_all_threads();" : "stop_thread();") << '\n';
                    ended = true;
                } else {
                    auto callee = source.target(instruction.value);
                    const auto &target = source.graphs[callee];
                    source.out << line << (massacre ? "delete_threads" : "kill_threads")
                               << (effect.arguments.empty() ? "(" : "_on(");
                    if (!effect.arguments.empty()) source.out << group(effect.arguments[0]) << ", ";
                    source.out << source.classes[target.object] << "::" << source.names[target.object][target.function] << ");\n";
                }
                break;
            }
            }
    return ended;
}





























bool source_emitter_t::body_t::emit_structured()
{
    using set_t = std::set<unsigned>;
    using value_kind = lifted_value_t::kind_t;
    using effect_kind = lifted_effect_t::kind_t;
    const unsigned exit = static_cast<unsigned>(graph.blocks.size());
    if (!exit || !reachable[0]) return false;
    set_t blocks, nodes{exit};
    for (unsigned b = 0; b < exit; ++b) if (reachable[b]) { blocks.insert(b); nodes.insert(b); }
    std::vector<std::vector<unsigned>> predecessors(exit);
    for (unsigned b = 0; b < exit; ++b) if (reachable[b])
        for (auto successor : graph.blocks[b].successors) predecessors[successor].push_back(b);
    auto intersect = [](set_t &left, const set_t &right) {
        for (auto i = left.begin(); i != left.end();) {
            if (!right.count(*i)) i = left.erase(i); else ++i;
        }
    };

    std::vector<set_t> dominators(exit, blocks);
    dominators[0] = {0};
    for (bool changed = true; changed;) {
        changed = false;
        for (unsigned b = 1; b < exit; ++b) if (reachable[b]) {
            auto next = blocks;
            for (auto predecessor : predecessors[b]) intersect(next, dominators[predecessor]);
            next.insert(b);
            if (next != dominators[b]) { dominators[b] = std::move(next); changed = true; }
        }
    }







    std::vector<std::vector<unsigned>> onward(exit);
    for (unsigned b = 0; b < exit; ++b) if (reachable[b]) {
        onward[b] = graph.blocks[b].successors;
        if (onward[b].empty()) onward[b].push_back(exit);
    }
    for (;;) {
        std::vector<bool> escapes(exit + 1);
        escapes[exit] = true;
        for (bool changed = true; changed;) {
            changed = false;
            for (unsigned b = 0; b < exit; ++b) if (reachable[b] && !escapes[b])
                for (auto successor : onward[b]) if (escapes[successor]) { escapes[b] = true; changed = true; }
        }
        unsigned trapped = exit;
        for (unsigned b = 0; b < exit; ++b)
            if (reachable[b] && !escapes[b] && (trapped == exit || dominators[b].size() < dominators[trapped].size()))
                trapped = b;
        if (trapped == exit) break;
        onward[trapped].push_back(exit);
    }
    std::vector<set_t> postdominators(exit + 1, nodes);
    postdominators[exit] = {exit};
    for (bool changed = true; changed;) {
        changed = false;
        for (unsigned b = 0; b < exit; ++b) if (reachable[b]) {
            auto next = nodes;
            for (auto successor : onward[b]) intersect(next, postdominators[successor]);
            next.insert(b);
            if (next != postdominators[b]) { postdominators[b] = std::move(next); changed = true; }
        }
    }

    std::vector<unsigned> join(exit, exit);
    for (unsigned b = 0; b < exit; ++b) if (reachable[b])
        for (auto candidate : postdominators[b])
            if (candidate != b && postdominators[candidate].size() > postdominators[join[b]].size()) join[b] = candidate;

    std::map<unsigned, set_t> loops;
    for (unsigned tail = 0; tail < exit; ++tail) if (reachable[tail])
        for (auto head : graph.blocks[tail].successors) if (dominators[tail].count(head)) {
            auto &members = loops[head];
            members.insert(head);
            std::vector<unsigned> work;
            if (tail != head && members.insert(tail).second) work.push_back(tail);
            while (!work.empty()) {
                auto node = work.back(); work.pop_back();
                for (auto predecessor : predecessors[node])
                    if (members.insert(predecessor).second) work.push_back(predecessor);
            }
        }










    auto choose_follows = [&](bool from_header) {
        std::map<unsigned, unsigned> chosen;
        for (const auto &[head, members] : loops) {
            unsigned follow = exit;
            if (from_header) {
                unsigned outside = 0;
                for (auto successor : graph.blocks[head].successors)
                    if (!members.count(successor)) { ++outside; follow = successor; }
                if (outside != 1) follow = exit;
            }
            if (follow == exit) {
                set_t candidates, returning;
                for (auto member : members) for (auto successor : graph.blocks[member].successors)
                    if (!members.count(successor))
                        (graph.blocks[successor].successors.empty() ? returning : candidates).insert(successor);
                if (candidates.empty()) candidates = std::move(returning);
                if (!candidates.empty()) {
                    auto common = nodes;
                    for (auto candidate : candidates) intersect(common, postdominators[candidate]);
                    for (auto candidate : common)
                        if (!members.count(candidate) && postdominators[candidate].size() > postdominators[follow].size())
                            follow = candidate;
                    if (follow == exit && candidates.size() == 1) follow = *candidates.begin();
                }
            }
            chosen[head] = follow;
        }
        return chosen;
    };
    std::map<unsigned, unsigned> follows;






    auto quiet_effect = [&](const lifted_effect_t &effect) {
        if (effect.kind == effect_kind::branch) return true;
        if (effect.kind != effect_kind::value) return false;
        if (!used[effect.value] && !effectful(effect.value)) return true;
        return !adjacent_expressions.empty() && adjacent_expressions[effect.value];
    };
    auto quiet_block = [&](unsigned b) {
        for (const auto &effect : graph.blocks[b].effects) if (!quiet_effect(effect)) return false;
        return true;
    };


    auto quiet_transfer = [&](value_ref_t target, value_ref_t input) {
        if (graph.values[input.value].kind == value_kind::uninitialized) return true;
        if (!stored(input.value)) return false;
        return slots[input.value] == slots[target.value] && input.offset == target.offset;
    };
    auto quiet_edge = [&](unsigned from, unsigned to) {
        const auto &origin = graph.blocks[from];
        const auto &destination = graph.blocks[to];
        for (unsigned word = 0; word < destination.incoming.size(); ++word) {
            const auto ref = destination.incoming[word];
            if (used[ref.value] && graph.values[ref.value].kind == value_kind::merge &&
                !quiet_transfer(ref, origin.outgoing.at(word))) return false;
        }
        if (destination.incoming_array_index && used[destination.incoming_array_index->value] &&
            !quiet_transfer(*destination.incoming_array_index, *origin.outgoing_array_index)) return false;
        return true;
    };
    auto conditional = [&](unsigned b) {
        const auto &block = graph.blocks[b];
        return block.successors.size() == 2 && block.successors[0] != block.successors[1] &&
               !block.effects.empty() && block.effects.back().kind == effect_kind::branch &&
               !block.effects.back().arguments.empty() && !block.effects.back().arguments[0].empty();
    };


    auto arms = [&](unsigned b, unsigned &on_false, unsigned &on_true) {
        const auto &block = graph.blocks[b];
        const auto &instruction = binary.code[block.effects.back().instruction];
        const auto destination = static_cast<unsigned>(static_cast<std::int64_t>(instruction.offset) +
                                                       instruction.size() + static_cast<std::int16_t>(instruction.value));
        on_false = block.successors[0];
        on_true = block.successors[1];
        if (binary.code[graph.blocks[on_false].begin].offset != destination) std::swap(on_false, on_true);
    };



    set_t bypassed;
    auto resolve = [&](unsigned node) {
        for (unsigned guard = 0; guard <= exit; ++guard) {
            const auto &block = graph.blocks[node];
            if (block.successors.size() != 1 || loops.count(node)) break;
            if (!quiet_block(node) || !quiet_edge(node, block.successors[0])) break;
            bypassed.insert(node);
            node = block.successors[0];
        }
        return node;
    };

    struct term_t { unsigned block; bool negate; bool conjunction; };
    struct chain_t {
        std::vector<term_t> terms;
        unsigned last = 0;
        unsigned positive = 0, negative = 0;
        unsigned positive_from = 0, negative_from = 0;
    };




    auto build_chain = [&](unsigned node) {
        chain_t chain;
        unsigned on_false = 0, on_true = 0;
        arms(node, on_false, on_true);
        chain.terms.push_back({node, false, false});
        chain.last = node;
        chain.positive = on_true;
        chain.negative = on_false;
        chain.positive_from = chain.negative_from = node;
        for (bool extended = true; extended;) {
            extended = false;
            for (unsigned side = 0; side < 2 && !extended; ++side) {
                auto &probe = side ? chain.positive : chain.negative;
                auto &probe_from = side ? chain.positive_from : chain.negative_from;
                auto &held = side ? chain.negative : chain.positive;
                auto &held_from = side ? chain.negative_from : chain.positive_from;
                const auto candidate = probe;


                if (loops.count(candidate)) continue;
                if (std::any_of(chain.terms.begin(), chain.terms.end(),
                                [&](const term_t &term) { return term.block == candidate; })) continue;
                if (!conditional(candidate) || !quiet_block(candidate) || predecessors[candidate].size() != 1) continue;
                if (probe_from != exit && !quiet_edge(probe_from, candidate)) continue;
                if (held_from != exit && !quiet_edge(held_from, held)) continue;
                unsigned candidate_false = 0, candidate_true = 0;
                arms(candidate, candidate_false, candidate_true);
                unsigned shared = exit, rest = exit;
                bool negate = false;
                if (resolve(candidate_true) == resolve(held)) { shared = candidate_true; rest = candidate_false; negate = side != 0; }
                else if (resolve(candidate_false) == resolve(held)) { shared = candidate_false; rest = candidate_true; negate = side == 0; }
                else continue;
                if (!quiet_edge(candidate, shared)) continue;
                chain.terms.push_back({candidate, negate, side != 0});
                chain.last = candidate;
                held = resolve(held);
                held_from = exit;
                probe = rest;
                probe_from = candidate;
                extended = true;
            }
        }
        return chain;
    };

    struct statement_t {
        enum class kind_t { effects, transfer, branch, loop, break_loop, continue_loop };
        statement_t(kind_t kind, unsigned block = 0, unsigned target = 0)
            : kind(kind), block(block), target(target) {}
        kind_t kind;
        unsigned block = 0, target = 0;
        bool negate = false;
        std::vector<term_t> condition;
        std::vector<statement_t> body, otherwise;
    };
    using kind = statement_t::kind_t;
    struct frame_t { unsigned header, follow; };
    std::vector<frame_t> stack;
    std::vector<bool> planned(exit), inside(exit);
    unsigned budget = 8 * static_cast<unsigned>(blocks.size()) + 64;

    std::function<bool(unsigned, unsigned, unsigned, std::vector<statement_t> &, bool &)> region;
    region = [&](unsigned node, unsigned stop, unsigned entering, std::vector<statement_t> &out, bool &falls) -> bool {
        for (;;) {
            const bool opening = node == entering;
            entering = exit;
            if (node == stop) { falls = true; return true; }
            if (!budget) return false;
            --budget;
            if (!opening && !stack.empty()) {
                if (node == stack.back().header) { out.push_back({kind::continue_loop}); falls = false; return true; }
                if (node == stack.back().follow) { out.push_back({kind::break_loop}); falls = false; return true; }
            }


            if (!opening) for (const auto &frame : stack) if (node == frame.header) return false;
            if (inside[node]) return false;

            if (!opening && loops.count(node)) {
                const auto follow = follows.at(node);
                const auto &members = loops.at(node);
                statement_t loop{kind::loop};
                unsigned entry = exit, entry_from = exit;
                if (conditional(node) && quiet_block(node)) {
                    auto test = build_chain(node);
                    bool contained = true;
                    for (const auto &term : test.terms) if (!members.count(term.block)) contained = false;
                    if (contained && test.negative == follow &&
                        (test.negative_from == exit || quiet_edge(test.negative_from, follow))) {
                        loop.condition = test.terms;
                        entry = test.positive;
                        entry_from = test.positive_from;
                    } else if (contained && test.positive == follow &&
                               (test.positive_from == exit || quiet_edge(test.positive_from, follow))) {
                        loop.condition = test.terms;
                        loop.negate = true;
                        entry = test.negative;
                        entry_from = test.negative_from;
                    }
                    if (!loop.condition.empty())
                        for (const auto &term : loop.condition) {
                            out.push_back({kind::effects, term.block});
                            planned[term.block] = true;
                        }
                }
                stack.push_back({node, follow});
                bool ignored = false;
                if (loop.condition.empty()) {
                    if (!region(node, exit, node, loop.body, ignored)) return false;
                } else {
                    if (entry_from != exit) loop.body.push_back({kind::transfer, entry_from, entry});
                    if (!region(entry, exit, exit, loop.body, ignored)) return false;
                }
                stack.pop_back();
                out.push_back(std::move(loop));
                if (follow == exit) { falls = false; return true; }
                node = follow;
                continue;
            }

            const auto &block = graph.blocks[node];
            out.push_back({kind::effects, node});
            planned[node] = true;
            if (block.successors.empty()) { falls = false; return true; }
            if (block.successors.size() == 1 || (block.successors.size() == 2 && block.successors[0] == block.successors[1])) {
                const auto next = block.successors[0];
                out.push_back({kind::transfer, node, next});
                node = next;
                continue;
            }
            if (!conditional(node)) return false;
            auto test = build_chain(node);
            for (unsigned term = 1; term < test.terms.size(); ++term) {
                out.push_back({kind::effects, test.terms[term].block});
                planned[test.terms[term].block] = true;
            }
            statement_t branch{kind::branch};
            branch.condition = test.terms;
            const auto follow = join[test.last];
            if (test.positive_from != exit) branch.body.push_back({kind::transfer, test.positive_from, test.positive});
            if (test.negative_from != exit) branch.otherwise.push_back({kind::transfer, test.negative_from, test.negative});
            for (const auto &term : test.terms) inside[term.block] = true;
            bool holds = false, fails = false;
            if (!region(test.positive, follow, exit, branch.body, holds)) return false;
            if (!region(test.negative, follow, exit, branch.otherwise, fails)) return false;
            for (const auto &term : test.terms) inside[term.block] = false;
            out.push_back(std::move(branch));
            if (!holds && !fails) { falls = false; return true; }
            node = follow;
        }
    };

    std::vector<statement_t> plan;
    bool complete = false;
    for (unsigned attempt = 0; attempt < 2 && !complete; ++attempt) {
        auto chosen = choose_follows(attempt == 0);
        if (attempt && chosen == follows) break;
        follows = std::move(chosen);
        plan.clear();
        stack.clear();
        bypassed.clear();
        planned.assign(exit, false);
        inside.assign(exit, false);
        budget = 8 * static_cast<unsigned>(blocks.size()) + 64;
        bool falls = false;
        if (!region(0, exit, exit, plan, falls)) continue;
        complete = true;
        for (unsigned b = 0; b < exit; ++b) if (reachable[b] && !planned[b] && !bypassed.count(b)) complete = false;
    }
    if (!complete) return false;






    std::function<bool(std::vector<statement_t> &)> reduce;
    reduce = [&](std::vector<statement_t> &statements) {
        bool silent = true;
        for (auto &statement : statements) switch (statement.kind) {
        case kind::effects: silent = silent && quiet_block(statement.block); break;
        case kind::transfer: silent = silent && quiet_edge(statement.block, statement.target); break;
        case kind::branch:
            if (reduce(statement.body)) statement.body.clear();
            if (reduce(statement.otherwise)) statement.otherwise.clear();
            silent = false;
            break;
        case kind::loop: reduce(statement.body); silent = false; break;
        default: silent = false; break;
        }
        return silent;
    };
    reduce(plan);



    auto closed = [&](const std::vector<statement_t> &statements) {
        if (statements.empty()) return false;
        const auto &last = statements.back();
        if (last.kind == kind::break_loop || last.kind == kind::continue_loop) return true;
        return last.kind == kind::effects && graph.blocks[last.block].successors.empty();
    };















    std::function<void(std::vector<statement_t> &, bool)> flatten;
    flatten = [&](std::vector<statement_t> &statements, bool tail) {
        for (bool again = true; again;) {
            again = false;
            if (tail && !statements.empty() && statements.back().kind == kind::continue_loop) {
                statements.pop_back();
                again = true;
                continue;
            }
            if (tail && !statements.empty() && statements.back().kind == kind::branch) {
                auto &test = statements.back();
                for (auto *arm : {&test.body, &test.otherwise})
                    if (!arm->empty() && arm->back().kind == kind::continue_loop) { arm->pop_back(); again = true; }
                if (again) continue;
            }
            if (statements.size() >= 2) {
                auto &leave = statements.back();
                auto &test = statements[statements.size() - 2];
                if ((leave.kind == kind::break_loop || leave.kind == kind::continue_loop) &&
                    test.kind == kind::branch && test.body.empty() != test.otherwise.empty()) {
                    auto &vacant = test.body.empty() ? test.body : test.otherwise;
                    auto &taken = test.body.empty() ? test.otherwise : test.body;
                    if (taken.size() > 1 && closed(taken)) {
                        auto rest = std::move(taken);
                        taken.clear();
                        vacant.push_back(std::move(leave));
                        statements.pop_back();
                        for (auto &statement : rest) statements.push_back(std::move(statement));
                        again = true;
                        continue;
                    }
                }
            }
            for (unsigned i = 0; i < statements.size() && !again; ++i) {
                if (statements[i].kind != kind::branch) continue;
                auto &test = statements[i];
                const bool leaves = closed(test.body), otherwise_leaves = closed(test.otherwise);
                if (leaves == otherwise_leaves) continue;
                auto &staying = leaves ? test.otherwise : test.body;
                if (staying.empty()) continue;
                auto rest = std::move(staying);
                staying.clear();
                statements.insert(statements.begin() + i + 1,
                                  std::make_move_iterator(rest.begin()), std::make_move_iterator(rest.end()));
                again = true;
            }
        }
        for (unsigned i = 0; i < statements.size(); ++i) {
            auto &statement = statements[i];
            const bool last = i + 1 == statements.size();
            if (statement.kind == kind::branch) {
                flatten(statement.body, tail && last);
                flatten(statement.otherwise, tail && last);
            } else if (statement.kind == kind::loop) flatten(statement.body, true);
        }
    };
    flatten(plan, false);

    auto wrapped = [](const std::string &text) {
        if (text.size() < 2 || text.front() != '(') return false;
        unsigned depth = 0;
        for (unsigned i = 0; i < text.size(); ++i) {
            if (text[i] == '(') ++depth;
            else if (text[i] == ')' && !--depth) return i + 1 == text.size();
        }
        return false;
    };
    auto negated = [&](std::string text) {
        return wrapped(text) ? '!' + text : "!(" + text + ')';
    };
    auto render = [&](const std::vector<term_t> &condition, bool negate) {
        std::string text;
        bool conjunction = false;
        for (unsigned i = 0; i < condition.size(); ++i) {
            auto term = group(graph.blocks[condition[i].block].effects.back().arguments[0], "num");
            if (condition[i].negate) term = negated(std::move(term));
            if (!i) { text = std::move(term); continue; }


            if (i > 1 && condition[i].conjunction != conjunction) text = '(' + text + ')';
            conjunction = condition[i].conjunction;
            text += conjunction ? " && " : " || ";
            text += term;
        }
        return negate ? negated(std::move(text)) : text;
    };



    auto quiet_statement = [&](const statement_t &statement) {
        if (statement.kind == kind::effects) return quiet_block(statement.block);
        return statement.kind == kind::transfer && quiet_edge(statement.block, statement.target);
    };
    auto chained = [&](const std::vector<statement_t> &alternative) -> const statement_t * {
        if (alternative.empty() || alternative.back().kind != kind::branch) return nullptr;
        for (unsigned i = 0; i + 1 < alternative.size(); ++i)
            if (!quiet_statement(alternative[i])) return nullptr;
        return &alternative.back();
    };

    for (auto id : slot_values) declaration(id);
    entry_values();
    std::function<void(const std::vector<statement_t> &, const std::string &)> emit_all;
    std::function<void(const statement_t &, const std::string &, bool)> emit_conditional;
    emit_all = [&](const std::vector<statement_t> &statements, const std::string &line) {
        for (const auto &statement : statements) {
            switch (statement.kind) {
            case kind::effects:
                for (const auto &effect : graph.blocks[statement.block].effects)
                    if (effect.kind != effect_kind::branch) emit_effect(statement.block, effect, line);
                break;
            case kind::transfer:
                edge(statement.block, statement.target, line, false);
                break;
            case kind::branch:
                emit_conditional(statement, line, false);
                break;
            case kind::loop:
                source.out << line << "while ("
                           << (statement.condition.empty() ? "1" : render(statement.condition, statement.negate)) << ") {\n";
                emit_all(statement.body, line + "    ");
                source.out << line << "}\n";
                break;
            case kind::break_loop: source.out << line << "break;\n"; break;
            case kind::continue_loop: source.out << line << "continue;\n"; break;
            }
        }
    };
    emit_conditional = [&](const statement_t &branch, const std::string &line, bool alternative) {
        const bool invert = branch.body.empty() && !branch.otherwise.empty();
        const auto &primary = invert ? branch.otherwise : branch.body;
        const auto &secondary = invert ? branch.body : branch.otherwise;
        const auto opening = line + (alternative ? "} else if (" : "if (") +
                             render(branch.condition, invert != branch.negate) + ')';

        if (secondary.empty() && primary.size() == 1 &&
            (primary.front().kind == kind::break_loop || primary.front().kind == kind::continue_loop)) {
            source.out << opening << (primary.front().kind == kind::break_loop ? " break;\n" : " continue;\n");
            return;
        }
        source.out << opening << " {\n";
        emit_all(primary, line + "    ");
        if (secondary.empty()) { source.out << line << "}\n"; return; }
        if (const auto *test = chained(secondary)) {


            for (unsigned i = 0; i + 1 < secondary.size(); ++i) {
                const auto &statement = secondary[i];
                if (statement.kind == kind::transfer) edge(statement.block, statement.target, line, false);
                else for (const auto &effect : graph.blocks[statement.block].effects)
                    if (effect.kind != effect_kind::branch) emit_effect(statement.block, effect, line);
            }
            emit_conditional(*test, line, true);
            return;
        }
        source.out << line << "} else {\n";
        emit_all(secondary, line + "    ");
        source.out << line << "}\n";
    };
    emit_all(plan, indent);
    return true;
}

void source_emitter_t::body_t::allocate()
{
    using set_t = std::set<unsigned>;
    using effect_kind = lifted_effect_t::kind_t;
    const auto count = graph.blocks.size();
    std::vector<set_t> inputs(count), definitions(count), phis(count), live_in(count), live_out(count);
    auto references = [&](const lifted_effect_t &effect) {
        set_t result;
        auto add = [&](value_ref_t ref) { if (stored(ref.value)) result.insert(ref.value); };
        auto address = [&](const std::optional<lifted_address_t> &storage) {
            if (!storage) return;
            if (storage->receiver) add(*storage->receiver);
            if (storage->index) add(*storage->index);
        };
        if (effect.kind == effect_kind::value) {
            if (!used[effect.value] && !effectful(effect.value)) return result;
            const auto &value = graph.values[effect.value];
            for (const auto &argument : value.arguments) for (auto ref : argument) add(ref);
            address(value.address);
        } else {
            const auto n = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
            for (unsigned a = 0; a < n; ++a) for (auto ref : effect.arguments[a]) add(ref);
            address(effect.address);
        }
        return result;
    };
    for (unsigned b = 0; b < count; ++b) {
        if (!reachable[b]) continue;
        const auto &block = graph.blocks[b];
        for (auto ref : block.incoming)
            if (stored(ref.value) && graph.values[ref.value].kind == lifted_value_t::kind_t::merge) phis[b].insert(ref.value);
        if (block.incoming_array_index && stored(block.incoming_array_index->value)) phis[b].insert(block.incoming_array_index->value);
        definitions[b] = phis[b];
        for (const auto &effect : block.effects) {
            for (auto ref : references(effect)) if (!definitions[b].count(ref)) inputs[b].insert(ref);
            if (effect.kind == effect_kind::value && stored(effect.value)) definitions[b].insert(effect.value);
        }
    }
    bool changed;
    do {
        changed = false;
        for (unsigned b = static_cast<unsigned>(count); b-- > 0;) {
            if (!reachable[b]) continue;
            set_t outgoing;
            const auto &block = graph.blocks[b];
            for (auto next : block.successors) {
                outgoing.insert(live_in[next].begin(), live_in[next].end());
                const auto &destination = graph.blocks[next];
                for (unsigned word = 0; word < destination.incoming.size(); ++word) {
                    if (!phis[next].count(destination.incoming[word].value)) continue;
                    const auto ref = block.outgoing.at(word).value;
                    if (stored(ref)) outgoing.insert(ref);
                }
                if (destination.incoming_array_index && phis[next].count(destination.incoming_array_index->value) &&
                    stored(block.outgoing_array_index->value)) outgoing.insert(block.outgoing_array_index->value);
            }
            auto incoming = inputs[b];
            for (auto value : outgoing) if (!definitions[b].count(value)) incoming.insert(value);
            if (incoming != live_in[b] || outgoing != live_out[b]) {
                live_in[b] = std::move(incoming);
                live_out[b] = std::move(outgoing);
                changed = true;
            }
        }
    } while (changed);
    std::vector<set_t> interference(graph.values.size());
    auto connect = [&](unsigned a, unsigned b) {
        if (a == b) return;
        interference[a].insert(b);
        interference[b].insert(a);
    };
    for (unsigned b = 0; b < count; ++b) {
        if (!reachable[b]) continue;
        auto live = live_out[b];
        const auto &effects = graph.blocks[b].effects;
        for (auto effect = effects.rbegin(); effect != effects.rend(); ++effect) {
            if (effect->kind == effect_kind::value && stored(effect->value)) {
                for (auto other : live) connect(effect->value, other);
                live.erase(effect->value);
            }
            auto refs = references(*effect);
            live.insert(refs.begin(), refs.end());
        }
        for (auto phi : phis[b]) {
            for (auto other : live) connect(phi, other);
            live.erase(phi);
        }
    }
    for (auto a : live_in[0]) for (auto b : live_in[0]) connect(a, b);
    std::vector<unsigned> order;
    slots.assign(graph.values.size(), UINT32_MAX);
    for (unsigned id = 0; id < graph.values.size(); ++id) if (stored(id)) order.push_back(id);
    std::stable_sort(order.begin(), order.end(), [&](unsigned a, unsigned b) { return interference[a].size() > interference[b].size(); });
    for (auto id : order) {
        set_t unavailable;
        for (auto other : interference[id]) if (slots[other] != UINT32_MAX) unavailable.insert(slots[other]);
        unsigned slot = 0;
        for (; slot < slot_values.size(); ++slot)
            if (!unavailable.count(slot) && value_type(id) == value_type(slot_values[slot]) &&
                graph.values[id].width == graph.values[slot_values[slot]].width &&
                array_values.count(id) == array_values.count(slot_values[slot]) &&
                (!array_values.count(id) || array_values.at(id) == array_values.at(slot_values[slot]))) break;
        if (slot == slot_values.size()) slot_values.push_back(id);
        slots[id] = slot;
    }
}

void source_emitter_t::body_t::emit()
{
    using value_kind = lifted_value_t::kind_t;
    using effect_kind = lifted_effect_t::kind_t;
    std::function<void(unsigned)> visit = [&](unsigned block) {
        if (reachable[block]) return;
        reachable[block] = true;
        for (auto next : graph.blocks[block].successors) visit(next);
    };
    visit(0);
    recover_local_arrays();
    recover_native_values();
    used.resize(graph.values.size());
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b]) continue;
        for (const auto &effect : graph.blocks[b].effects) {
            if (effect.kind == effect_kind::value) {
                const auto &value = graph.values[effect.value];
                if (value.kind == value_kind::indexed_store || value.kind == value_kind::signal) use({effect.value, 0});
                else if (effectful(effect.value)) {
                    for (const auto &argument : value.arguments) for (auto ref : argument) use(ref);
                    if (value.address) {
                        if (value.address->receiver) use(*value.address->receiver);
                        if (value.address->index) use(*value.address->index);
                    }
                }
            }
            else {
                const auto count = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
                for (unsigned a = 0; a < count; ++a) for (auto ref : effect.arguments[a]) use(ref);
                if (effect.address) {
                    if (effect.address->receiver) use(*effect.address->receiver);
                    if (effect.address->index) use(*effect.address->index);
                }
            }
        }
    }
    allocate();
    prettify();
    if (emit_linear()) return;
    std::vector<unsigned> references(graph.values.size());
    auto count_address = [&](const std::optional<lifted_address_t> &address) {
        if (!address) return;
        if (address->receiver) ++references[address->receiver->value];
        if (address->index) ++references[address->index->value];
    };
    for (const auto &value : graph.values) {
        for (const auto &argument : value.arguments) for (auto ref : argument) ++references[ref.value];
        count_address(value.address);
    }
    for (const auto &block : graph.blocks) for (const auto &effect : block.effects) {
        for (const auto &argument : effect.arguments) for (auto ref : argument) ++references[ref.value];
        count_address(effect.address);
    }
    adjacent_expressions.resize(graph.values.size());
    for (const auto &block : graph.blocks) for (unsigned i = 0; i + 1 < block.effects.size(); ++i) {
        const auto &producer = block.effects[i];
        if (producer.kind != effect_kind::value || references[producer.value] != 1) continue;
        const auto &value = graph.values[producer.value];
        if (value.kind != value_kind::operation || value.width != 4) continue;
        const auto &consumer = block.effects[i + 1];
        const auto &arguments = consumer.kind == effect_kind::value ?
            graph.values[consumer.value].arguments : consumer.arguments;
        for (const auto &argument : arguments)
            if (std::any_of(argument.begin(), argument.end(), [&](auto ref) { return ref.value == producer.value; }))
                adjacent_expressions[producer.value] = true;
    }
    required_slots.resize(slot_values.size());
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && !adjacent_expressions[id]) required_slots[slots[id]] = true;
    if (emit_structured()) return;
    for (auto id : slot_values) declaration(id);
    entry_values();
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b]) continue;
        const auto &block = graph.blocks[b];
        source.out << indent << "block" << b << ":\n" << indent << "{\n";
        const auto line = indent + "    ";
        bool ended = false;
        for (const auto &effect : block.effects) {
            ended = emit_effect(b, effect, line) || ended;
        }
        if (!ended && !block.successors.empty()) edge(b, block.successors[0], line);
        source.out << indent << "}\n";
    }
}

void source_emitter_t::function(unsigned index, std::string_view indentation)
{
    const auto &graph = graphs[index];
    const auto &record = binary(index);
    const auto &signature = types.functions[index];
    const bool constructor = !graph.initializer && graph.function == 0 && record.name_hash == image.objects[graph.object].name_hash;
    const bool destructor = !graph.initializer && static_cast<int>(graph.function) == image.objects[graph.object].destructor;
    body_t body(*this, index, std::string(indentation) + "    ");
    const auto base_initializer = body.lifetime_initializer(constructor, destructor);
    out << indentation;
    if (graph.initializer) out << "[](";
    else {
        if ((record.flags & 1u) && !(image.objects[graph.object].flags & 1u)) out << "static ";
        if (!constructor && !destructor) out << type(signature.result) << ' ';
        out << names[graph.object][graph.function] << '(';
    }
    bool comma = false;
    if (!graph.initializer) for (const auto &parameter : signature.parameters) {
        if (!(record.flags & 1u) && !parameter.offset) continue;
        if (comma) out << ", ";
        comma = true;
        out << (!graph.initializer && graph.function == 0 && graph.object == 0 ? "num" : type(parameter.type)) << " p" << parameter.offset;
    }
    out << ')';
    if (graph.initializer || (constructor && signature.result != "void")) out << " -> " << type(signature.result);
    out << base_initializer;
    out << '\n';
    out << indentation << "{\n";
    body.emit();
    out << indentation << '}';
    if (graph.initializer) out << ';';
    out << "\n\n";
}

std::string source_emitter_t::emit()
{
    out << "namespace " << identifier(image.name, "script") << " {\n";
    for (const auto &[identity, variable] : variables)
        if (!variable.second.empty()) out << "    extern " << type(variable.second) << ' ' << variable.first << ";\n";
    std::set<std::pair<unsigned, unsigned>> natives;
    for (unsigned i = 0; i < graphs.size(); ++i) for (const auto &instruction : binary(i).code)
        if (instruction.opcode == OP_BSL) natives.emplace(instruction.value >> 16, instruction.value & 0xffff);
    for (const auto &identity : natives) {
        const auto &native = symbols.natives.at(identity);
        out << "    extern " << native_result(identity) << ' '
            << native.class_name << "::" << native.signature << ";\n";
    }
    out << '\n';
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        const auto &record = image.objects[object];
        out << "    class " << classes[object];
        if (parents[object]) out << " : public " << type(*parents[object]);
        out << "\n    {\n    public:\n";
        emit_static_fields(object);
        unsigned cursor = 0;
        if (parents[object])
            for (const auto &parent : image.objects)
                if (parent.name == *parents[object]) cursor = parent.instance_size;
        for (const auto &[offset, field] : fields[object]) {
            if (field_owner(object, offset, field.width) != object) continue;
            if (offset > cursor) out << "        num unused_" << cursor << '[' << (offset - cursor) / 4 << "];\n";
            out << "        " << type(field.type) << " field_" << offset;
            if (auto array = instance_arrays[object].find(offset); array != instance_arrays[object].end())
                out << '[' << array->second.count << ']';
            out << ";\n";
            cursor = offset + field.width;
        }
        if (record.instance_size > cursor)
            out << "        num unused_" << cursor << '[' << (record.instance_size - cursor) / 4 << "];\n";
        for (unsigned f = 0; f < record.functions.size(); ++f) function(functions.at({object, f}), "        ");
        out << "    };\n\n";
    }
    out << "}\n";
    return out.str();
}

}

std::string emit_decompiled_source(const image_t &image, const symbols_t &symbols)
{
    return source_emitter_t(image, symbols).emit();
}

}
