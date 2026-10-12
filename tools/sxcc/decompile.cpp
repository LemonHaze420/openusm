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
    if (name.empty())
        return std::string(fallback);
    std::string result;
    for (unsigned char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (!result.empty() && c >= '0' && c <= '9'))
            result += static_cast<char>(c);
        else
            result += '_';
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
    std::map<unsigned, std::string> string_objects;
    std::vector<std::pair<unsigned, unsigned>> text_definitions;
    std::vector<std::pair<unsigned, std::string>> builder_definitions;
    struct field_t {
        unsigned width;
        std::string type;
    };
    std::vector<std::map<unsigned, field_t>> fields;
    std::map<std::pair<unsigned, unsigned>, std::pair<std::string, std::string>> variables;
    std::set<std::pair<unsigned, unsigned>> variable_arrays;
    std::map<std::pair<unsigned, unsigned>, std::string> inferred_native_results;
    std::vector<std::map<unsigned, field_t>> shared_fields;
    std::vector<std::map<unsigned, std::string>> shared_names;
    struct array_t {
        unsigned width = 0, count = 1;
        std::string type;
    };
    std::vector<std::map<unsigned, array_t>> shared_arrays;
    std::vector<std::map<unsigned, array_t>> instance_arrays;
    std::optional<std::pair<std::string, std::string>>
    instance_member(const lifted_function_t &graph, const lifted_address_t &address, std::string_view index = {}) const
    {
        if (address.kind != lifted_address_t::kind_t::instance || !address.receiver)
            return {};
        auto owner = receiver_class(graph, *address.receiver);
        if (!owner)
            return {};
        for (unsigned depth = 0; depth < image.objects.size() && parents[*owner]; ++depth) {
            auto parent = std::find_if(image.objects.begin(), image.objects.end(), [&](const auto &candidate) {
                return candidate.name == *parents[*owner];
            });
            if (parent == image.objects.end() || address.offset + address.width > parent->instance_size)
                break;
            owner = static_cast<unsigned>(parent - image.objects.begin());
        }
        const auto &arrays = instance_arrays[*owner];
        auto found = arrays.upper_bound(address.offset);
        if (found == arrays.begin())
            return {};
        --found;
        const auto &array = found->second;
        const auto displacement = address.offset - found->first;
        if (displacement + address.width > array.width * array.count)
            return {};
        const bool component =
            !address.index && address.width == 4 && (array.type == "vector3d" || array.type == "position3d");
        if (!component && (address.width != array.width || displacement % array.width))
            return {};
        auto name = "field_" + std::to_string(found->first) + '[';
        if (address.index) {
            if (index.empty())
                return {};
            name += index;
            if (displacement)
                name += " + " + std::to_string(displacement / array.width);
        } else
            name += std::to_string(displacement / array.width);
        name += ']';
        if (component) {
            name += '.';
            name += "xyz"[(displacement % array.width) / 4];
        }
        return std::make_pair(std::move(name), component ? "num" : type(array.type));
    }
    void recover_static_fields();
    std::optional<std::pair<std::string, std::string>> shared_member(const lifted_address_t &address,
                                                                     std::string_view index = {}) const;
    void emit_static_fields(unsigned object);

    std::optional<unsigned> receiver_class(const lifted_function_t &graph, value_ref_t ref) const
    {
        if (ref.offset)
            return {};
        const auto &value = graph.values[ref.value];
        for (unsigned i = 0; i < image.objects.size(); ++i)
            if (value.type == image.objects[i].name)
                return i;
        return {};
    }

    unsigned field_owner(unsigned owner, unsigned offset, unsigned width) const
    {
        for (unsigned depth = 0; depth < image.objects.size() && parents[owner]; ++depth) {
            auto parent = std::find_if(image.objects.begin(), image.objects.end(), [&](const auto &object) {
                return object.name == *parents[owner];
            });
            if (parent == image.objects.end())
                break;
            auto index = static_cast<unsigned>(parent - image.objects.begin());
            auto found = fields[index].find(offset);
            if (found == fields[index].end() || found->second.width != width)
                break;
            owner = index;
        }
        return owner;
    }

    const field_t *field(const lifted_function_t &graph, const lifted_address_t &address) const
    {
        if (address.kind == lifted_address_t::kind_t::shared && !address.index) {
            auto found = shared_fields[address.owner].find(address.offset);
            return found != shared_fields[address.owner].end() && found->second.width == address.width ? &found->second
                                                                                                       : nullptr;
        }
        if (address.kind != lifted_address_t::kind_t::instance || !address.receiver || address.index)
            return nullptr;
        auto owner = receiver_class(graph, *address.receiver);
        if (!owner)
            return nullptr;
        owner = field_owner(*owner, address.offset, address.width);
        auto found = fields[*owner].find(address.offset);
        return found != fields[*owner].end() && found->second.width == address.width ? &found->second : nullptr;
    }

    static std::string number_literal(std::uint32_t bits)
    {
        float value;
        std::memcpy(&value, &bits, 4);
        if (!std::isfinite(value) || bits == 0x80000000u)
            return "num_bits(" + hex(bits) + ')';
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
            if (encoded == bits)
                return buffer;
        }
        throw error_t("Cannot render numeric constant");
    }
    std::map<std::pair<unsigned, unsigned>, unsigned> functions;
    std::ostringstream out;

    std::string type(std::string_view name) const
    {
        for (unsigned i = 0; i < image.objects.size(); ++i)
            if (name == image.objects[i].name)
                return classes[i];
        return std::string(name);
    }

    static std::string opaque(unsigned width)
    {
        return width ? "opaque<" + std::to_string(width) + '>' : "void";
    }

    std::string native_result(std::pair<unsigned, unsigned> identity) const
    {
        const auto &native = symbols.natives.at(identity);
        if (native.result)
            return *native.result;
        if (auto found = inferred_native_results.find(identity); found != inferred_native_results.end())
            return type(found->second);
        if (native.result_bytes)
            return opaque(*native.result_bytes);
        return opaque(stack.native_results.at(identity));
    }

    const function_t &binary(unsigned index) const
    {
        const auto &graph = graphs[index];
        return graph.initializer ? *image.initializers[graph.function].builder
                                 : image.objects[graph.object].functions[graph.function];
    }

    unsigned target(unsigned identity) const
    {
        return functions.at({image.object_order.at(identity >> 16), identity & 0xffff});
    }

    std::string text(unsigned index) const
    {
        if (auto named = string_objects.find(index); named != string_objects.end())
            return named->second;
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
        std::map<unsigned, std::pair<std::string, std::string>> recovered_local_names;
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
        void collapse_trivial_merges();
        std::set<unsigned> short_circuit_values;
        std::set<unsigned> disjunction_values;
        std::map<unsigned, unsigned> postfix_locals;
        std::map<unsigned, unsigned> parameter_slots;
        void recover_short_circuits();
        std::map<unsigned, unsigned> initialized_slots;
        std::set<unsigned> lexical_initializer_values;
        std::map<unsigned, unsigned> declaration_initializers;
        std::set<std::pair<unsigned, unsigned>> scoped_declaration_initializers;
        std::map<unsigned, std::vector<std::string>> recovered_local_operations;
        unsigned recovered_scope_depth = 0;
        std::string scoped_line(std::string_view line) const
        {
            return std::string(line) + std::string(recovered_scope_depth * 4, ' ');
        }
        void emit_recovered_operation(const std::string &operation, std::string_view line)
        {
            if (operation == "}") {
                if (!recovered_scope_depth)
                    throw error_t("Recovered lexical scope is unbalanced");
                --recovered_scope_depth;
            }
            source.out << scoped_line(line) << operation << '\n';
            if (operation == "{")
                ++recovered_scope_depth;
        }

        void emit_recovered_before(unsigned block, unsigned instruction, const std::string &line)
        {
            auto it = recovered_local_operations.lower_bound(graph.blocks[block].begin);
            while (it != recovered_local_operations.end() && it->first < graph.blocks[block].end &&
                   it->first <= instruction) {
                for (const auto &operation : it->second)
                    emit_recovered_operation(operation, line);
                it = recovered_local_operations.erase(it);
            }
        }
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
            if (auto parameter = parameter_slots.find(slot); parameter != parameter_slots.end())
                return "p" + std::to_string(parameter->second);
            if (auto pretty = pretty_slots.find(slot); pretty != pretty_slots.end())
                return pretty->second;
            if (auto array = array_values.find(slot_values.at(slot)); array != array_values.end())
                return local_arrays[array->second].name + "_" + std::to_string(slot);
            return "v" + std::to_string(slot);
        }

        std::string cast(std::string expression, std::string_view actual, std::string_view expected) const
        {
            if (actual == expected)
                return expression;
            if (actual == "num" && expression == "0" && expected != "num") {
                const auto native = source.symbols.type_sizes.find(std::string(expected));
                if ((native != source.symbols.type_sizes.end() && native->second == 4) ||
                    std::find(source.classes.begin(), source.classes.end(), expected) != source.classes.end())
                    return "nullptr";
            }
            auto script = std::find(source.classes.begin(), source.classes.end(), actual);
            for (unsigned depth = 0; script != source.classes.end() && depth < source.classes.size(); ++depth) {
                const auto &parent = source.parents[static_cast<unsigned>(script - source.classes.begin())];
                if (!parent)
                    break;
                const auto name = source.type(*parent);
                if (name == expected)
                    return expression;
                script = std::find(source.classes.begin(), source.classes.end(), name);
            }
            auto scope = std::string(actual);
            for (auto parent = source.symbols.parents.find(scope); parent != source.symbols.parents.end();
                 parent = source.symbols.parents.find(scope)) {
                scope = parent->second;
                if (scope == expected)
                    return expression;
            }
            return "bit_cast<" + std::string(expected) + ">(" + expression + ')';
        }

        std::string reference(value_ref_t ref, std::string_view expected = {}) const
        {
            const auto &value = graph.values.at(ref.value);
            if (!ref.offset) {
                auto local = recovered_local_names.find(ref.value);
                if (local != recovered_local_names.end())
                    return expected.empty() ? local->second.first
                                            : cast(local->second.first, local->second.second, expected);
            }
            if (auto array = array_values.find(ref.value); array != array_values.end()) {
                const auto &layout = local_arrays[array->second];
                auto expression = name(ref.value) + "[" + std::to_string(ref.offset / layout.width) + "]";
                if (layout.width == 12)
                    expression += std::string(".") + "xyz"[(ref.offset % layout.width) / 4];
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
                expression = number_literal(value.immediate);
                actual = "num";
                break;
            case lifted_value_t::kind_t::string:
                expression = source.text(value.immediate);
                actual = "str";
                break;
            case lifted_value_t::kind_t::class_value: {
                std::string native;
                for (const auto &[name, id] : source.symbols.class_ids)
                    if (id == value.immediate >> 16) {
                        native = name;
                        break;
                    }
                if (native.empty())
                    throw error_t("Class value has no evidenced native type");
                expression = native + '(' + source.text(value.immediate & 0xffff) + ')';
                actual = native;
                break;
            }
            case lifted_value_t::kind_t::parameter:
                expression =
                    !(binary.flags & 1u) && value.immediate == 0 ? "this" : "p" + std::to_string(value.immediate);
                for (const auto &parameter : types.parameters)
                    if (parameter.offset == value.immediate) {
                        actual = source.type(parameter.type);
                        break;
                    }
                break;
            default:
                expression = folded.count(ref.value) ? folded.at(ref.value) : name(ref.value);
                break;
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
            if (refs.empty())
                return {};
            if (expected.empty()) {
                const auto &value = graph.values.at(refs.front().value);
                bool whole = refs.front().offset == 0 && value.width == refs.size() * 4;
                for (unsigned i = 0; whole && i < refs.size(); ++i)
                    whole = refs[i].value == refs.front().value && refs[i].offset == i * 4;
                expected = whole ? value_type(refs.front().value) : opaque(static_cast<unsigned>(refs.size() * 4));
                if (refs.size() == 1 && (value.type == "vector3d" || value.type == "position3d"))
                    expected = "num";
            }
            if (refs.size() == 1)
                return reference(refs.front(), expected);
            const auto &first = graph.values.at(refs.front().value);
            if (auto array = array_values.find(refs.front().value); array != array_values.end()) {
                const auto &layout = local_arrays[array->second];
                bool element = refs.front().offset % layout.width == 0 && refs.size() * 4 == layout.width;
                for (unsigned i = 1; element && i < refs.size(); ++i)
                    element = refs[i].value == refs.front().value && refs[i].offset == refs.front().offset + i * 4;
                if (element)
                    return cast(name(refs.front().value) + "[" + std::to_string(refs.front().offset / layout.width) +
                                    "]",
                                source.type(layout.type),
                                expected);
            }
            if (first.kind == lifted_value_t::kind_t::parameter)
                for (const auto &parameter : types.parameters) {
                    if (parameter.offset != first.immediate + refs.front().offset || parameter.width != refs.size() * 4)
                        continue;
                    bool contiguous = true;
                    for (unsigned i = 0; i < refs.size(); ++i) {
                        const auto &part = graph.values[refs[i].value];
                        if (part.kind != lifted_value_t::kind_t::parameter ||
                            part.immediate + refs[i].offset != parameter.offset + i * 4)
                            contiguous = false;
                    }
                    if (contiguous)
                        return cast("p" + std::to_string(parameter.offset), source.type(parameter.type), expected);
                }
            bool whole = refs.front().offset == 0 && first.width == refs.size() * 4;
            for (unsigned i = 1; whole && i < refs.size(); ++i)
                whole = refs[i].value == refs.front().value && refs[i].offset == i * 4;
            if (whole) {
                auto expression =
                    folded.count(refs.front().value) ? folded.at(refs.front().value)
                    : first.kind == lifted_value_t::kind_t::parameter
                        ? (!(binary.flags & 1u) && !first.immediate ? "this" : "p" + std::to_string(first.immediate))
                        : name(refs.front().value);
                return cast(std::move(expression), value_type(refs.front().value), expected);
            }
            if ((expected == "vector3d" || expected == "position3d") && refs.size() == 3) {
                std::string vector = expected + '{';
                for (unsigned component = 0; component < 3; ++component) {
                    if (component)
                        vector += ", ";
                    vector += reference(refs[component], "num");
                }
                return vector + '}';
            }
            std::string result = opaque(static_cast<unsigned>(refs.size() * 4)) + '{';
            for (unsigned i = 0; i < refs.size(); ++i) {
                if (i)
                    result += ", ";
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
            } else
                result = source.classes[target.object] + "::";
            result += source.names[target.object][target.function] + '(';
            bool comma = false;
            for (const auto &parameter : parameters) {
                if (parameter.offset < first * 4)
                    continue;
                if (comma)
                    result += ", ";
                comma = true;
                auto begin = arguments.begin() + parameter.offset / 4;
                result += group({begin, begin + parameter.width / 4}, source.type(parameter.type));
            }
            return result + ')';
        }

        std::string store_type(const lifted_address_t &storage) const
        {
            if (auto member = source.shared_member(storage, storage.index ? "index" : ""))
                return member->second;
            if (auto member = source.instance_member(graph, storage, storage.index ? "index" : ""))
                return member->second;
            if (const auto *member = source.field(graph, storage))
                return source.type(member->type);
            if (storage.kind == lifted_address_t::kind_t::game_variable ||
                storage.kind == lifted_address_t::kind_t::shared_variable) {
                auto found =
                    source.variables.find({storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                if (found != source.variables.end() && !found->second.second.empty())
                    return source.type(found->second.second);
            }
            return opaque(storage.width);
        }

        std::string address(const lifted_address_t &storage, std::string_view expected,
                            const std::vector<value_ref_t> *memory = nullptr) const
        {
            if (auto member =
                    source.shared_member(storage, storage.index ? reference(*storage.index, "num") : std::string{}))
                return cast(member->first, member->second, expected);
            if (auto member =
                    source.instance_member(graph, storage, storage.index ? reference(*storage.index, "num") : "")) {
                const auto owner = *source.receiver_class(graph, *storage.receiver);
                const auto receiver = reference(*storage.receiver, source.classes[owner]);
                auto expression = (receiver == "this" ? "" : '(' + receiver + ").") + member->first;
                if (!storage.index && member->first.ends_with("[0]"))
                    expression = "(*" + expression.substr(0, expression.size() - 3) + ")";
                return cast(std::move(expression), member->second, expected);
            }
            if (const auto *field = source.field(graph, storage)) {
                auto owner = *source.receiver_class(graph, *storage.receiver);
                auto receiver = reference(*storage.receiver, source.classes[owner]);
                auto expression = (receiver == "this" ? "" : '(' + receiver + ").") + std::string("field_") +
                                  std::to_string(storage.offset);
                return cast(std::move(expression), source.type(field->type), expected);
            }
            std::string base;
            switch (storage.kind) {
            case lifted_address_t::kind_t::local: {
                if (!memory || memory->empty())
                    throw error_t("Local storage access lacks a value snapshot");
                const auto snapshot = memory->front().value;
                const auto found = array_values.find(snapshot);
                if (found == array_values.end())
                    throw error_t("Local array access lacks an allocated typed snapshot");
                const auto &array = local_arrays[found->second];
                auto element = storage.index ? reference(*storage.index, "num") : "0";
                const auto displacement = storage.offset - array.offset;
                if (displacement / array.width)
                    element += " + " + std::to_string(displacement / array.width);
                auto expression = name(snapshot) + "[" + element + "]";
                return cast(std::move(expression), source.type(array.type), expected);
            }
            case lifted_address_t::kind_t::shared:
                base = source.classes[storage.owner] + "::__shared_" + std::to_string(storage.owner);
                break;
            case lifted_address_t::kind_t::instance:
                return "instance_storage<" + std::string(expected) + ">(" + reference(*storage.receiver, "opaque<4>") +
                       ", " + std::to_string(storage.offset) +
                       (storage.index ? " + " + reference(*storage.index, "num") + " * " + std::to_string(storage.width)
                                      : "") +
                       ')';
            case lifted_address_t::kind_t::game_variable:
            case lifted_address_t::kind_t::shared_variable:
                if (auto found = source.variables.find(
                        {storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                    found != source.variables.end() && !found->second.second.empty())
                    return cast(found->second.first +
                                    (source.variable_arrays.count(found->first)
                                         ? "[" + (storage.index ? reference(*storage.index, "num") : "0") + "]"
                                         : ""),
                                source.type(found->second.second),
                                expected);
                return std::string(storage.kind == lifted_address_t::kind_t::game_variable ? "game_variable<"
                                                                                           : "shared_variable<") +
                       std::string(expected) + ">(" + std::to_string(storage.offset) + ')';
            }
            return "storage_part<" + std::string(expected) + ">(" + base + ", " + std::to_string(storage.offset) +
                   (storage.index ? " + " + reference(*storage.index, "num") + " * " + std::to_string(storage.width)
                                  : "") +
                   ')';
        }

        std::string expression(unsigned id) const
        {
            const auto &value = graph.values[id];
            const auto &instruction = binary.code.at(value.instruction);
            auto arg = [&](unsigned n, std::string type = "num") {
                return group(value.arguments.at(n), std::move(type));
            };
            if (short_circuit_values.count(id))
                return "(" + arg(0) + (disjunction_values.count(id) ? " || " : " && ") + arg(1) + ")";
            switch (value.kind) {
            case lifted_value_t::kind_t::operation: {
                switch (instruction.opcode) {
                case OP_PSH:
                case OP_POP:
                    return arg(0, value_type(id));
                case OP_NEG:
                    if (value.arguments[0].size() == 1) {
                        const auto ref = value.arguments[0][0];
                        if (!ref.offset && graph.values[ref.value].kind == lifted_value_t::kind_t::constant) {
                            if (graph.values[ref.value].immediate == 0)
                                return "(-0)";
                            return number_literal(graph.values[ref.value].immediate ^ 0x80000000u);
                        }
                    }
                    return "(-" + arg(0) + ')';
                case OP_LNT:
                    return "(!" + arg(0) + ')';
                case OP_NOT:
                    return "(~" + arg(0) + ')';
                case OP_INC:
                    return '(' + arg(0) + " + 1)";
                case OP_DEC:
                    return '(' + arg(0) + " - 1)";
                case OP_I2S:
                    return "to_integer_string(" + arg(0) + ')';
                case OP_F2S:
                    return "to_float_string(" + arg(0) + ')';
                default:
                    break;
                }
                static const std::map<opcode_t, std::string_view> operators{{OP_ADD, "+"},
                                                                            {OP_SUB, "-"},
                                                                            {OP_MUL, "*"},
                                                                            {OP_DIV, "/"},
                                                                            {OP_MOD, "%"},
                                                                            {OP_AND, "&"},
                                                                            {OP_OR, "|"},
                                                                            {OP_XOR, "^"},
                                                                            {OP_SHL, "<<"},
                                                                            {OP_SHR, ">>"},
                                                                            {OP_EQ, "=="},
                                                                            {OP_NE, "!="},
                                                                            {OP_LT, "<"},
                                                                            {OP_LE, "<="},
                                                                            {OP_GT, ">"},
                                                                            {OP_GE, ">="},
                                                                            {OP_STR_EQ, "=="},
                                                                            {OP_STR_NE, "!="},
                                                                            {OP_PSH_STR, "+"}};
                const auto operand = instruction.opcode == OP_STR_EQ || instruction.opcode == OP_STR_NE ||
                                             instruction.opcode == OP_PSH_STR
                                         ? "str"
                                         : "num";
                return '(' + arg(0, operand) + ' ' + std::string(operators.at(instruction.opcode)) + ' ' +
                       arg(1, operand) + ')';
            }
            case lifted_value_t::kind_t::native_call: {
                const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
                const auto method = native.signature.substr(0, native.signature.find('('));
                const bool receiver = native.class_name != "_global_slc" && method != native.class_name;
                if (receiver && method.starts_with("operator") && value.arguments.size() == 2)
                    return '(' + arg(0, native.parameters.at(0)) + ' ' + method.substr(8) + ' ' +
                           arg(1, native.parameters.at(1)) + ')';
                const bool global =
                    native.class_name == "_global_slc" &&
                    std::find(source.names[graph.object].begin(), source.names[graph.object].end(), method) ==
                        source.names[graph.object].end();
                std::string result = receiver ? '(' + arg(0, native.parameters.at(0)) + ")."
                                     : global || method == native.class_name ? ""
                                                                             : native.class_name + "::";
                result += method + '(';
                for (unsigned i = receiver ? 1 : 0; i < value.arguments.size(); ++i) {
                    if (i > (receiver ? 1u : 0u))
                        result += ", ";
                    result += arg(i, i < native.parameters.size() ? native.parameters[i] : "num");
                }
                return result + ')';
            }
            case lifted_value_t::kind_t::script_call:
                return call(value.immediate, value.arguments.at(0));
            case lifted_value_t::kind_t::load:
                return address(*value.address, value_type(id), value.arguments.empty() ? nullptr : &value.arguments[0]);
            case lifted_value_t::kind_t::signal:
                return "signal_id(" + (instruction.argument == OP_ARG_SIG ? std::string{} : arg(0, {}) + ", ") +
                       source.text(value.immediate >> 16) + ')';
            default:
                throw error_t("Value does not have a standalone source expression");
            }
        }

        void use(value_ref_t ref)
        {
            if (used[ref.value])
                return;
            used[ref.value] = true;
            const auto &value = graph.values[ref.value];
            for (unsigned a = 0; a < value.arguments.size(); ++a)
                for (auto part : value.arguments[a]) {
                    if ((value.kind == lifted_value_t::kind_t::merge ||
                         (array_values.count(ref.value) &&
                          (value.kind != lifted_value_t::kind_t::indexed_store || a == 0))) &&
                        graph.values[part.value].kind == lifted_value_t::kind_t::uninitialized)
                        continue;
                    use(part);
                }
            if (value.address) {
                if (value.address->receiver)
                    use(*value.address->receiver);
                if (value.address->index)
                    use(*value.address->index);
            }
        }

        bool stored(unsigned id) const
        {
            using kind = lifted_value_t::kind_t;
            const auto &value = graph.values[id];
            return used[id] && value.width && value.kind != kind::parameter && value.kind != kind::constant &&
                   value.kind != kind::string && value.kind != kind::class_value;
        }


        unsigned retained_result_serial = 0;
        bool retains_unused_result(unsigned id) const
        {
            const auto &v = graph.values[id];
            if (used[id] || !v.width ||
                (v.kind != lifted_value_t::kind_t::native_call && v.kind != lifted_value_t::kind_t::script_call &&
                 !(v.kind == lifted_value_t::kind_t::operation && v.instruction < binary.code.size() &&
                   binary.code[v.instruction].opcode != OP_POP)) ||
                v.instruction + 1 >= binary.code.size())
                return false;
            const auto &next = binary.code[v.instruction + 1];
            if (next.opcode == OP_SPA && static_cast<int16_t>(next.value) < 0)
                return false;
            if (next.opcode == OP_POP || next.opcode == OP_RET)
                return false;
            return true;
        }

        bool effectful(unsigned id) const
        {
            using kind = lifted_value_t::kind_t;
            const auto &value = graph.values[id];


            if (value.kind == kind::operation)
                return true;
            if (value.kind == kind::load && value.address)
                return value.address->index.has_value() || value.address->kind == lifted_address_t::kind_t::instance;
            return value.kind == kind::native_call || value.kind == kind::script_call ||
                   value.kind == kind::indexed_store || value.kind == kind::signal;
        }

        void allocate();
        bool emit_linear();
        bool emit_structured();
        bool emit_effect(unsigned block, const lifted_effect_t &effect, const std::string &line);


        bool retained_initializer(unsigned from, unsigned word, value_ref_t target, value_ref_t input) const
        {
            if (target.offset || input.offset || !stored(target.value))
                return false;
            auto initial = declaration_initializers.find(slots[target.value]);
            if ((initial == declaration_initializers.end() || initial->second != input.value) &&
                !scoped_declaration_initializers.count({slots[target.value], input.value}))
                return false;
            const auto &facts = source.stack.functions[index];
            for (unsigned pc = graph.blocks[from].begin; pc < graph.blocks[from].end; ++pc) {
                const auto &op = binary.code[pc];
                if (op.opcode != OP_POP || op.argument != OP_ARG_SPR || !facts.heights[pc])
                    continue;
                const int begin = *facts.heights[pc] + static_cast<int16_t>(op.value),
                          end = begin + static_cast<int>(op.data_size());
                if (begin < static_cast<int>(word * 4 + 4) && end > static_cast<int>(word * 4))
                    return false;
            }
            return true;
        }

        void edge(unsigned from, unsigned to, std::string_view indentation, bool jump = true)
        {
            const auto &destination = graph.blocks[to];
            const auto &origin = graph.blocks[from];
            std::vector<std::pair<value_ref_t, value_ref_t>> copies;
            for (unsigned word = 0; word < destination.incoming.size(); ++word) {
                const auto ref = destination.incoming[word];
                if (used[ref.value] && graph.values[ref.value].kind == lifted_value_t::kind_t::merge &&
                    !retained_initializer(from, word, ref, origin.outgoing.at(word)))
                    copies.emplace_back(ref, origin.outgoing.at(word));
            }
            if (destination.incoming_array_index && used[destination.incoming_array_index->value])
                copies.emplace_back(*destination.incoming_array_index, *origin.outgoing_array_index);
            parallel_copy(copies, indentation);
            if (jump)
                source.out << scoped_line(indentation) << "goto block" << to << ";\n";
        }

    public:
        body_t(source_emitter_t &source, unsigned index, std::string indent)
            : source(source), index(index), graph(source.graphs[index]), binary(source.binary(index)),
              types(source.types.functions[index]), used(graph.values.size()), reachable(graph.blocks.size()),
              indent(std::move(indent))
        {}

        void emit();
        std::string initializer_arguments(unsigned target);
        std::string lifetime_initializer(bool constructor, bool destructor);
    };

    void function(unsigned index, std::string_view indentation, bool declaration_only = false, bool qualified = false);

public:
    source_emitter_t(const image_t &image, const symbols_t &symbols)
        : image(image), symbols(symbols), stack(analyze_stack(image, symbols)),
          graphs(lift_values(image, symbols, stack)), types(recover_types(image, symbols, stack, graphs))
    {
        for (unsigned i = 0; i < image.objects.size(); ++i) {
            auto name = identifier(image.objects[i].name, "Class");
            if (symbols.type_sizes.count(name) || std::find(classes.begin(), classes.end(), name) != classes.end())
                name += "_class" + std::to_string(i);
            classes.push_back(std::move(name));
            names.emplace_back();
            for (unsigned f = 0; f < image.objects[i].functions.size(); ++f) {
                const auto &function = image.objects[i].functions[f];
                if (!f && function.name_hash == image.objects[i].name_hash)
                    names.back().push_back(classes.back());
                else if (static_cast<int>(f) == image.objects[i].destructor)
                    names.back().push_back('~' + classes.back());
                else {
                    auto name =
                        identifier(std::string_view(function.name).substr(0, function.name.find('(')), "function");
                    names.back().push_back(std::move(name));
                }
            }
        }
        parents = std::move(types.parents);
        for (unsigned i = 0; i < graphs.size(); ++i) {
            if (!graphs[i].diagnostics.empty())
                throw error_t("Cannot decompile function: " + graphs[i].diagnostics.front());
            if (!graphs[i].initializer)
                functions.emplace(std::make_pair(graphs[i].object, graphs[i].function), i);
        }
        fields.resize(image.objects.size());
        instance_arrays.resize(image.objects.size());
        shared_fields.resize(image.objects.size());
        shared_arrays.resize(image.objects.size());
        shared_names.resize(image.objects.size());
        for (const auto &[name, identity] : symbols.variables)
            variables.emplace(identity, std::make_pair(name, std::string{}));
        for (const auto &graph : graphs) {
            for (const auto &value : graph.values)
                if (value.kind == lifted_value_t::kind_t::native_call && !value.type.empty() &&
                    !value.type.starts_with("opaque<"))
                    inferred_native_results[{value.immediate >> 16, value.immediate & 0xffff}] = value.type;
            auto offer = [&](const lifted_address_t &address, std::string type) {
                auto recover = [&](unsigned owner) {
                    auto found =
                        types.storage.find({static_cast<unsigned>(address.kind), owner, address.offset, address.width});
                    if (found != types.storage.end())
                        type = found->second;
                };
                if (address.kind != lifted_address_t::kind_t::instance)
                    recover(address.owner);
                if (address.kind == lifted_address_t::kind_t::shared && address.index) {
                    auto &array = shared_arrays[address.owner][address.offset];
                    if (array.width && array.width != address.width)
                        throw error_t("Indexed static storage has conflicting element widths");
                    array.width = address.width;
                    if (array.type.empty() || array.type.starts_with("opaque<"))
                        array.type = type.empty() ? opaque(address.width) : type;
                    const auto &index_value = graph.values[address.index->value];
                    if (!address.index->offset && index_value.kind == lifted_value_t::kind_t::constant) {
                        float value;
                        std::memcpy(&value, &index_value.immediate, 4);
                        if (value >= 0 && value < 65536)
                            array.count = std::max(array.count, static_cast<unsigned>(value) + 1);
                    }
                    return;
                }
                if (address.kind == lifted_address_t::kind_t::shared && !address.index) {
                    auto [it, inserted] = shared_fields[address.owner].emplace(
                        address.offset, field_t{address.width, type.empty() ? opaque(address.width) : type});
                    if (!inserted && it->second.type.starts_with("opaque<") && !type.empty())
                        it->second.type = type;
                    return;
                }
                if (address.kind == lifted_address_t::kind_t::game_variable ||
                    address.kind == lifted_address_t::kind_t::shared_variable) {
                    auto found =
                        variables.find({address.kind == lifted_address_t::kind_t::game_variable, address.offset});
                    if (address.index)
                        variable_arrays.insert(
                            {address.kind == lifted_address_t::kind_t::game_variable, address.offset});
                    if (found != variables.end() &&
                        (found->second.second.empty() || found->second.second.starts_with("opaque<")))
                        found->second.second = type.empty() ? opaque(address.width) : type;
                    return;
                }
                if (!address.receiver || address.kind != lifted_address_t::kind_t::instance)
                    return;
                auto owner = receiver_class(graph, *address.receiver);
                if (!owner || address.offset + address.width > image.objects[*owner].instance_size)
                    return;
                for (unsigned depth = 0; depth < image.objects.size() && parents[*owner]; ++depth) {
                    auto parent = std::find_if(image.objects.begin(), image.objects.end(), [&](const auto &candidate) {
                        return candidate.name == *parents[*owner];
                    });
                    if (parent == image.objects.end() || address.offset + address.width > parent->instance_size)
                        break;
                    owner = static_cast<unsigned>(parent - image.objects.begin());
                }
                recover(*owner);
                if (type.empty())
                    type = opaque(address.width);
                if (address.index) {
                    auto &array = instance_arrays[*owner][address.offset];
                    if (array.width && array.width != address.width)
                        throw error_t("Indexed instance storage has conflicting element widths");
                    array.width = address.width;
                    if (array.type.empty() || array.type.starts_with("opaque<"))
                        array.type = type;
                    const auto &index_value = graph.values[address.index->value];
                    if (!address.index->offset && index_value.kind == lifted_value_t::kind_t::constant) {
                        float value;
                        std::memcpy(&value, &index_value.immediate, 4);
                        if (value >= 0 && value < 65536)
                            array.count = std::max(array.count, static_cast<unsigned>(value) + 1);
                    }
                    return;
                }
                auto [it, inserted] = fields[*owner].emplace(address.offset, field_t{address.width, type});
                if (!inserted && it->second.type.starts_with("opaque<") && !type.starts_with("opaque<"))
                    it->second.type = type;
            };
            for (const auto &value : graph.values)
                if (value.kind == lifted_value_t::kind_t::load && value.address)
                    offer(*value.address, value.type);
            for (const auto &block : graph.blocks)
                for (const auto &effect : block.effects)
                    if (effect.kind == lifted_effect_t::kind_t::store && effect.address &&
                        !effect.arguments[0].empty()) {
                        const auto &value = graph.values[effect.arguments[0][0].value];
                        offer(*effect.address,
                              value.width == effect.address->width ? value.type : opaque(effect.address->width));
                    }
        }
        for (unsigned object = 0; object < image.objects.size(); ++object) {
            for (auto &[offset, array] : instance_arrays[object]) {
                const auto minimum_end = offset + array.width * array.count;
                auto end = image.objects[object].instance_size;
                const auto next_field = fields[object].lower_bound(minimum_end);
                if (next_field != fields[object].end())
                    end = std::min(end, next_field->first);
                const auto next_array = instance_arrays[object].upper_bound(offset);
                if (next_array != instance_arrays[object].end())
                    end = std::min(end, next_array->first);
                if (!array.width || end < minimum_end || (end - offset) % array.width)
                    throw error_t("Indexed instance storage exceeds its recovered extent");
                array.count = (end - offset) / array.width;
                auto field = fields[object].lower_bound(offset);
                while (field != fields[object].end() && field->first < end)
                    field = fields[object].erase(field);
                fields[object][offset] = {end - offset, array.type};
            }
        }
        recover_static_fields();
        for (unsigned f = 0; f < graphs.size(); ++f) {
            auto &graph = graphs[f];
            for (auto &value : graph.values) {
                if (value.kind != lifted_value_t::kind_t::load || !value.address || !value.type.starts_with("opaque<"))
                    continue;


                const auto &address = *value.address;
                if (auto member = shared_member(address, address.index ? "index" : ""))
                    value.type = member->second;
                else if (auto member = instance_member(graph, address, address.index ? "index" : ""))
                    value.type = member->second;
                else if (const auto *member = field(graph, address))
                    value.type = member->type;
            }
            for (const auto &block : graph.blocks)
                for (const auto &effect : block.effects) {
                    if (effect.kind != lifted_effect_t::kind_t::store || !effect.address ||
                        effect.arguments[0].size() != 1)
                        continue;
                    const auto *member = field(graph, *effect.address);
                    if (!member || member->type.starts_with("opaque<"))
                        continue;
                    const auto ref = effect.arguments[0][0];
                    auto &value = graph.values[ref.value];
                    if (ref.offset || value.width != member->width || value.kind != lifted_value_t::kind_t::parameter ||
                        !value.type.starts_with("opaque<"))
                        continue;
                    value.type = member->type;
                    for (auto &parameter : types.functions[f].parameters)
                        if (parameter.offset == value.immediate)
                            parameter.type = value.type;
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
        if (slots[id] == UINT32_MAX || value.kind != kind::native_call)
            continue;
        const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
        if (native.class_name == "_global_slc" && native.signature == "create_entity(str)")
            entities.push_back(id);
    }
    if (entities.empty())
        return;

    std::set<std::string> occupied;
    for (auto id : slot_values)
        occupied.insert(name(id));
    for (const auto &parameter : types.parameters)
        occupied.insert("p" + std::to_string(parameter.offset));
    for (const auto &array : local_arrays)
        occupied.insert(array.name);
    occupied.insert(source.classes.begin(), source.classes.end());
    for (const auto &methods : source.names)
        occupied.insert(methods.begin(), methods.end());
    for (const auto &[name, size] : source.symbols.type_sizes)
        occupied.insert(name);
    for (const auto &[identity, variable] : source.variables)
        occupied.insert(variable.first);
    for (const auto &fields : source.shared_names)
        for (const auto &[offset, name] : fields)
            occupied.insert(name);
    for (const auto &fields : source.fields)
        for (const auto &[offset, field] : fields)
            occupied.insert("field_" + std::to_string(offset));
    for (const auto &[identity, native] : source.symbols.natives)
        occupied.insert(native.signature.substr(0, native.signature.find('(')));

    for (auto id : entities) {
        const auto slot = slots[id];
        if (pretty_slots.count(slot))
            continue;
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
    if ((!constructor && !destructor) || !source.parents[graph.object])
        return {};
    auto parent = std::find_if(source.image.objects.begin(), source.image.objects.end(), [&](const auto &object) {
        return object.name == *source.parents[graph.object];
    });
    if (parent == source.image.objects.end())
        throw error_t("Missing recovered base class");
    const auto owner = static_cast<unsigned>(parent - source.image.objects.begin());
    auto is_call = [&](const lifted_effect_t &effect, unsigned function) {
        if (effect.kind != lifted_effect_t::kind_t::value)
            return false;
        const auto &value = graph.values.at(effect.value);
        if (value.kind != lifted_value_t::kind_t::script_call ||
            source.image.object_order.at(value.immediate >> 16) != owner || (value.immediate & 0xffff) != function ||
            value.arguments.empty() || value.arguments.front().empty())
            return false;
        const auto receiver = value.arguments.front().front();
        const auto &self = graph.values.at(receiver.value);
        return !receiver.offset && self.kind == lifted_value_t::kind_t::parameter && !self.immediate;
    };
    if (constructor) {
        if (parent->functions.empty() || parent->functions.front().name_hash != parent->name_hash)
            return {};
        auto &effects = graph.blocks.front().effects;
        if (effects.empty() || !is_call(effects.front(), 0))
            throw error_t("Recovered base constructor is not the constructor's first effect");
        const auto &call = graph.values.at(effects.front().value);
        std::string result = " : " + source.classes[owner] + '(';
        bool comma = false;
        for (const auto &parameter : source.types.functions.at(source.functions.at({owner, 0})).parameters) {
            if (!parameter.offset)
                continue;
            if (comma)
                result += ", ";
            comma = true;
            const auto &arguments = call.arguments.front();
            result += group({arguments.begin() + parameter.offset / 4,
                             arguments.begin() + (parameter.offset + parameter.width) / 4},
                            parameter.type);
        }
        effects.erase(effects.begin());
        return result + ')';
    }
    if (parent->destructor < 0)
        return {};
    for (auto &block : graph.blocks)
        for (unsigned i = 0; i < block.effects.size(); ++i) {
            if (block.effects[i].kind != lifted_effect_t::kind_t::return_value)
                continue;
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
    auto known = [](std::string_view type) {
        return !type.empty() && !type.starts_with("opaque<");
    };
    for (unsigned id = 0; id < original.values.size(); ++id) {
        const auto &value = original.values[id];
        if (!value.address || value.address->kind != lifted_address_t::kind_t::local || !value.address->index)
            continue;
        const auto &address = *value.address;
        auto [location, inserted] = locations.emplace(address.offset, static_cast<unsigned>(local_arrays.size()));
        if (inserted)
            local_arrays.push_back({address.offset, address.width, 0, {}, {}});
        auto &array = local_arrays[location->second];
        if (array.width != address.width)
            throw error_t("Local array has conflicting indexed element widths");
        accesses[id] = location->second;
        std::string type = value.kind == value_kind::load ? value.type : std::string{};
        if (value.kind == value_kind::indexed_store) {
            const auto &refs = value.arguments.at(1);
            if (!refs.empty()) {
                const auto &stored = original.values[refs[0].value];
                bool whole = !refs[0].offset && stored.width == address.width;
                for (unsigned word = 1; whole && word < refs.size(); ++word)
                    whole = refs[word].value == refs[0].value && refs[word].offset == word * 4;
                if (whole)
                    type = stored.type;
                if (whole && stored.address) {
                    const auto &storage = *stored.address;
                    if (storage.kind == lifted_address_t::kind_t::game_variable ||
                        storage.kind == lifted_address_t::kind_t::shared_variable) {
                        const auto symbol = source.variables.find(
                            {storage.kind == lifted_address_t::kind_t::game_variable, storage.offset});
                        if (symbol != source.variables.end()) {
                            if (!known(type))
                                type = symbol->second.second;
                            auto stem = symbol->second.first;
                            if (stem.starts_with("gv_"))
                                stem.erase(0, 3);
                            else if (stem.starts_with("g_"))
                                stem.erase(0, 2);
                            const auto name = identifier(stem, "local") + "_values";
                            if (array.name.empty())
                                array.name = name;
                            else if (array.name != name)
                                array.name = "local_values_" + std::to_string(array.offset);
                        }
                    }
                }
            }
        }
        if (known(type)) {
            const auto layout = location->second;
            if (value.kind == value_kind::indexed_store) {
                if (producer_types.insert(layout).second)
                    array.type.clear();
            } else if (producer_types.count(layout))
                continue;
            if (array.type.empty())
                array.type = type;
            else if (array.type != type) {
                if (!producer_types.count(layout) && array.type == "num")
                    array.type = type;
                else if (!producer_types.count(layout) && type == "num")
                    continue;
                else if ((array.type == "vector3d" || array.type == "position3d") &&
                         (type == "vector3d" || type == "position3d"))
                    array.type = "vector3d";
                else
                    throw error_t("Local array has conflicting producer element types");
            }
        }
    }
    if (local_arrays.empty())
        return;
    for (auto &array : local_arrays) {
        for (unsigned instruction = 0; instruction < binary.code.size(); ++instruction) {
            const auto &op = binary.code[instruction];
            if (op.opcode != OP_SPA || !facts.heights[instruction] || *facts.heights[instruction] < 0)
                continue;
            const auto begin = static_cast<unsigned>(*facts.heights[instruction]);
            const auto bytes = static_cast<std::int16_t>(op.value);
            if (bytes <= 0 || array.offset < begin || array.offset >= begin + static_cast<unsigned>(bytes))
                continue;
            auto end = begin + static_cast<unsigned>(bytes);


            const auto next = locations.upper_bound(array.offset);
            if (next != locations.end())
                end = std::min(end, next->first);
            for (unsigned pc = instruction + 1; pc < binary.code.size(); ++pc) {
                const auto &access = binary.code[pc];
                if (access.argument != OP_ARG_SPR || access.data_size() <= array.width || !facts.heights[pc])
                    continue;
                if (access.opcode != OP_PSH && access.opcode != OP_POP)
                    continue;
                const int start = *facts.heights[pc] + static_cast<int16_t>(access.value);
                if (start > static_cast<int>(array.offset) && start < static_cast<int>(end) &&
                    start + static_cast<int>(access.data_size()) <= static_cast<int>(begin) + bytes)
                    end = static_cast<unsigned>(start);
            }
            if ((end - array.offset) % array.width)
                continue;
            const auto count = (end - array.offset) / array.width;
            if (array.count && array.count != count)
                throw error_t("Local array allocation extent is ambiguous");
            array.count = count;
        }
        if (!array.count)
            throw error_t("Indexed local storage has no matching fixed frame allocation");
        if (!known(array.type))
            throw error_t("Indexed local storage has no evidenced element type");
        if (array.width != 4 && !(array.width == 12 && (array.type == "vector3d" || array.type == "position3d")))
            throw error_t("Local array element layout is unresolved");
        if (array.name.empty())
            array.name = identifier(array.type, "local") + "_values_" + std::to_string(array.offset);
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
        if (original.values[id].kind != value_kind::indexed_store)
            continue;
        auto value = original.values[id];
        const auto &array = local_arrays[layout];
        const auto &memory = value.arguments[0];
        value.arguments[0] = {memory.begin() + array.offset / 4,
                              memory.begin() + (array.offset + array.width * array.count) / 4};
        stores[id] = append(std::move(value), layout);
    }
    for (unsigned b = 0; b < original.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        const auto &block = original.blocks[b];
        for (unsigned layout = 0; layout < local_arrays.size(); ++layout) {
            const auto &array = local_arrays[layout];
            const auto first = array.offset / 4, words = array.width * array.count / 4;
            if (!facts.heights[block.begin] ||
                *facts.heights[block.begin] < static_cast<int>(array.offset + array.width * array.count))
                continue;
            bool merged = true;
            for (unsigned word = 0; word < words; ++word)
                if (original.values[block.incoming.at(first + word).value].kind != value_kind::merge)
                    merged = false;
            if (!merged)
                continue;
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
            for (unsigned word = 0; word < words; ++word)
                phis.emplace(block.incoming[first + word].value, value_ref_t{id, word * 4});
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
    auto resolve_group = [&](auto &refs) {
        for (auto &ref : refs)
            ref = resolve(ref);
    };
    auto resolve_address = [&](auto &address) {
        if (!address)
            return;
        if (address->receiver)
            address->receiver = resolve(*address->receiver);
        if (address->index)
            address->index = resolve(*address->index);
    };
    for (auto &value : graph.values) {
        for (auto &argument : value.arguments)
            resolve_group(argument);
        resolve_address(value.address);
    }
    for (auto &block : graph.blocks) {
        resolve_group(block.incoming);
        resolve_group(block.outgoing);
        if (block.incoming_array_index)
            block.incoming_array_index = resolve(*block.incoming_array_index);
        if (block.outgoing_array_index)
            block.outgoing_array_index = resolve(*block.outgoing_array_index);
        for (auto &effect : block.effects) {
            for (auto &argument : effect.arguments)
                resolve_group(argument);
            resolve_address(effect.address);
            if (auto store = stores.find(effect.value); effect.kind == effect_kind::value && store != stores.end())
                effect.value = store->second;
        }
    }
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        std::vector<lifted_effect_t> effects;
        std::vector<unsigned> snapshots;
        for (auto effect : graph.blocks[b].effects) {
            if (effect.kind == effect_kind::value && accesses.count(effect.value) &&
                graph.values[effect.value].kind == value_kind::load) {
                const auto layout = accesses.at(effect.value);
                const auto &array = local_arrays[layout];
                auto memory = graph.values[effect.value].arguments[0];
                memory = {memory.begin() + array.offset / 4,
                          memory.begin() + (array.offset + array.width * array.count) / 4};
                auto snapshot = memory.front().value;
                bool whole = array_values.count(snapshot) && array_values.at(snapshot) == layout;
                for (unsigned word = 0; whole && word < memory.size(); ++word)
                    whole = memory[word] == value_ref_t{snapshot, word * 4};
                if (!whole) {
                    const auto existing = std::find_if(snapshots.begin(), snapshots.end(), [&](unsigned id) {
                        return array_values.at(id) == layout && graph.values[id].arguments[0] == memory;
                    });
                    if (existing != snapshots.end())
                        snapshot = *existing;
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
                if (!known(value.type))
                    value.type = array.type;
            }
            effects.push_back(std::move(effect));
        }
        graph.blocks[b].effects = std::move(effects);
    }
}


void source_emitter_t::body_t::collapse_trivial_merges()
{
    using kind = lifted_value_t::kind_t;
    std::set<unsigned> aliased;
    for (bool changed = true; changed;) {
        changed = false;
        std::map<unsigned, value_ref_t> aliases;
        for (unsigned id = 0; id < graph.values.size(); ++id) {
            const auto &v = graph.values[id];
            if (v.kind != kind::merge || v.width != 4 || array_values.count(id) || aliased.count(id))
                continue;
            std::optional<value_ref_t> unique;
            bool conflict = false;
            for (const auto &args : v.arguments) {
                if (args.size() != 1) {
                    conflict = true;
                    break;
                }
                const auto ref = args.front();
                if (ref == value_ref_t{id, 0})
                    continue;
                if (!unique)
                    unique = ref;
                else if (*unique != ref) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict && unique) {
                aliases.emplace(id, *unique);
                aliased.insert(id);
            }
        }
        if (aliases.empty())
            break;
        auto resolve = [&](value_ref_t &ref) {
            std::set<unsigned> seen;
            while (aliases.count(ref.value) && seen.insert(ref.value).second) {
                auto next = aliases.at(ref.value);
                ref = {next.value, next.offset + ref.offset};
            }
        };
        for (const auto &[id, unused] : aliases) {
            value_ref_t target{id, 0};
            resolve(target);
            if (target.value == id || target.offset || graph.values[target.value].width != graph.values[id].width)
                continue;
            const auto &from = graph.values[id];
            auto &to = graph.values[target.value];
            if (from.ambiguous_read_home ||
                (from.local_read_home && to.local_read_home && from.local_read_home != to.local_read_home)) {
                to.local_read_home.reset();
                to.ambiguous_read_home = true;
            } else if (!to.ambiguous_read_home && from.local_read_home)
                to.local_read_home = from.local_read_home;
        }
        auto group = [&](auto &refs) {
            for (auto &ref : refs)
                resolve(ref);
        };
        auto address = [&](auto &a) {
            if (a) {
                if (a->receiver)
                    resolve(*a->receiver);
                if (a->index)
                    resolve(*a->index);
            }
        };
        for (auto &v : graph.values) {
            for (auto &args : v.arguments)
                group(args);
            address(v.address);
        }
        for (auto &b : graph.blocks) {
            group(b.incoming);
            group(b.outgoing);
            if (b.incoming_array_index)
                resolve(*b.incoming_array_index);
            if (b.outgoing_array_index)
                resolve(*b.outgoing_array_index);
            for (auto &e : b.effects) {
                for (auto &args : e.arguments)
                    group(args);
                address(e.address);
            }
        }
        changed = true;
    }

    std::map<unsigned, value_ref_t> replacements;
    for (unsigned id = 0; id < graph.values.size(); ++id) {
        const auto &start = graph.values[id];
        if (start.kind != kind::merge || start.width != 4 || array_values.count(id))
            continue;
        std::set<unsigned> visited;
        std::optional<value_ref_t> external;
        bool conflict = false;
        std::function<void(value_ref_t)> trace = [&](value_ref_t ref) {
            if (conflict)
                return;
            const auto &v = graph.values[ref.value];
            if (!ref.offset && v.kind == kind::merge && v.width == 4 && !array_values.count(ref.value)) {
                if (!visited.insert(ref.value).second)
                    return;
                for (const auto &args : v.arguments) {
                    if (args.size() != 1) {
                        conflict = true;
                        return;
                    }
                    trace(args.front());
                }
            } else if (!external)
                external = ref;
            else if (*external != ref)
                conflict = true;
        };
        trace({id, 0});
        if (!conflict && external)
            replacements.emplace(id, *external);
    }
    auto resolve = [&](value_ref_t &ref) {
        auto it = replacements.find(ref.value);
        if (it != replacements.end())
            ref = {it->second.value, it->second.offset + ref.offset};
    };
    for (const auto &[id, unused] : replacements) {
        value_ref_t target{id, 0};
        resolve(target);
        if (target.value == id || target.offset || graph.values[target.value].width != graph.values[id].width)
            continue;
        const auto &from = graph.values[id];
        auto &to = graph.values[target.value];
        if (from.ambiguous_read_home ||
            (from.local_read_home && to.local_read_home && from.local_read_home != to.local_read_home)) {
            to.local_read_home.reset();
            to.ambiguous_read_home = true;
        } else if (!to.ambiguous_read_home && from.local_read_home)
            to.local_read_home = from.local_read_home;
    }
    auto group = [&](auto &refs) {
        for (auto &ref : refs)
            resolve(ref);
    };
    auto address = [&](auto &a) {
        if (a) {
            if (a->receiver)
                resolve(*a->receiver);
            if (a->index)
                resolve(*a->index);
        }
    };
    for (auto &v : graph.values) {
        for (auto &args : v.arguments)
            group(args);
        address(v.address);
    }
    for (auto &b : graph.blocks) {
        group(b.incoming);
        group(b.outgoing);
        if (b.incoming_array_index)
            resolve(*b.incoming_array_index);
        if (b.outgoing_array_index)
            resolve(*b.outgoing_array_index);
        for (auto &e : b.effects) {
            for (auto &args : e.arguments)
                group(args);
            address(e.address);
        }
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
                if (first.offset || phi.kind != kind::merge || phi.width != 4 || replacements.count(first.value))
                    continue;
                unsigned width = 0;
                std::string type;
                for (unsigned edge = 0; edge < phi.arguments.size(); ++edge) {
                    if (phi.arguments[edge].size() != 1)
                        continue;
                    const auto ref = phi.arguments[edge][0];
                    const auto &producer = graph.values[ref.value];
                    if (ref.offset || producer.width <= 4 || producer.type.empty() ||
                        producer.type.starts_with("opaque<") || array_values.count(ref.value))
                        continue;
                    const auto count = producer.width / 4;
                    if (start + count > block.incoming.size())
                        continue;
                    bool complete = true;
                    for (unsigned word = 0; word < count; ++word) {
                        const auto part = block.incoming[start + word];
                        const auto &value = graph.values[part.value];
                        if (part.offset || value.kind != kind::merge || value.width != 4 ||
                            value.arguments.size() != phi.arguments.size() || value.arguments[edge].size() != 1 ||
                            value.arguments[edge][0] != value_ref_t{ref.value, word * 4} ||
                            replacements.count(part.value)) {
                            complete = false;
                            break;
                        }
                    }
                    if (complete) {
                        width = producer.width;
                        type = producer.type;
                        break;
                    }
                }
                if (!width)
                    continue;
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
        if (!changed)
            break;
        auto replace = [&](value_ref_t &ref) {
            if (auto found = replacements.find(ref.value); found != replacements.end())
                ref = {found->second.value, found->second.offset + ref.offset};
        };
        auto group = [&](auto &refs) {
            for (auto &ref : refs)
                replace(ref);
        };
        auto address = [&](auto &storage) {
            if (!storage)
                return;
            if (storage->receiver)
                replace(*storage->receiver);
            if (storage->index)
                replace(*storage->index);
        };
        for (auto &value : graph.values) {
            for (auto &argument : value.arguments)
                group(argument);
            address(value.address);
        }
        for (auto &block : graph.blocks) {
            group(block.incoming);
            group(block.outgoing);
            if (block.incoming_array_index)
                replace(*block.incoming_array_index);
            if (block.outgoing_array_index)
                replace(*block.outgoing_array_index);
            for (auto &effect : block.effects) {
                for (auto &argument : effect.arguments)
                    group(argument);
                address(effect.address);
            }
        }
    }
}

void source_emitter_t::body_t::declaration(unsigned id)
{
    if (parameter_slots.count(slots[id]))
        return;
    if (!required_slots.empty() && !required_slots[slots[id]])
        return;
    source.out << scoped_line(indent) << value_type(id) << ' ' << name(id);
    if (auto array = array_values.find(id); array != array_values.end())
        source.out << '[' << local_arrays[array->second].count << ']';
    if (auto init = declaration_initializers.find(slots[id]); init != declaration_initializers.end()) {
        const auto &v = graph.values[init->second];
        source.out << " = "
                   << (v.kind == lifted_value_t::kind_t::constant    ? number_literal(v.immediate)
                       : v.kind == lifted_value_t::kind_t::string    ? source.text(v.immediate)
                       : v.kind == lifted_value_t::kind_t::operation ? expression(init->second)
                                                                     : reference({init->second, 0}));
    }
    source.out << ";\n";
}

void source_emitter_t::body_t::parallel_copy(const std::vector<std::pair<value_ref_t, value_ref_t>> &transfers,
                                             std::string_view line)
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
        if (graph.values[input.value].kind == lifted_value_t::kind_t::uninitialized)
            continue;
        const location_t destination{slots[target.value], target.offset};
        const auto dependency =
            stored(input.value) ? std::optional<location_t>{{slots[input.value], input.offset}} : std::nullopt;
        if (dependency == destination)
            continue;
        if (auto parameter = parameter_slots.find(slots[target.value]);
            parameter != parameter_slots.end() && !input.offset &&
            graph.values[input.value].kind == lifted_value_t::kind_t::parameter &&
            graph.values[input.value].immediate == parameter->second)
            continue;
        const auto width = graph.values[target.value].width;
        bool whole = !target.offset && !input.offset && width > 4 && graph.values[input.value].width == width &&
                     !array_values.count(target.value) && !array_values.count(input.value) &&
                     i + width / 4 <= transfers.size();
        for (unsigned word = 0; whole && word < width / 4; ++word)
            whole = transfers[i + word] ==
                    std::make_pair(value_ref_t{target.value, word * 4}, value_ref_t{input.value, word * 4});
        if (whole) {
            std::vector<value_ref_t> words;
            for (unsigned word = 0; word < width / 4; ++word)
                words.push_back({input.value, word * 4});
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
            return std::none_of(
                copies.begin(), copies.end(), [&](const auto &other) { return other.dependency == copy.destination; });
        });
        if (ready != copies.end()) {
            source.out << scoped_line(line) << ready->target << " = " << ready->expression << ";\n";
            copies.erase(ready);
        } else {
            auto &copy = copies.front();
            const auto saved_name = temporary + "_" + std::to_string(saved++);
            source.out << scoped_line(line) << copy.type << ' ' << saved_name << " = " << copy.expression << ";\n";
            copy.expression = saved_name;
            copy.dependency.reset();
        }
    }
}

void source_emitter_t::body_t::array_copy(unsigned id, const std::vector<value_ref_t> &refs, std::string_view line)
{
    std::vector<std::pair<value_ref_t, value_ref_t>> transfers;
    transfers.reserve(refs.size());
    for (unsigned word = 0; word < refs.size(); ++word)
        transfers.push_back({{id, word * 4}, refs[word]});
    parallel_copy(transfers, line);
}

void source_emitter_t::body_t::entry_values()
{
    std::set<unsigned> initialized;
    for (auto ref : graph.blocks[0].incoming) {
        const auto &value = graph.values[ref.value];
        if (!used[ref.value] || value.kind != lifted_value_t::kind_t::merge || !initialized.insert(ref.value).second)
            continue;
        if (parameter_slots.count(slots[ref.value]))
            continue;
        if (array_values.count(ref.value))
            array_copy(ref.value, value.arguments.at(0), indent);
        else
            source.out << scoped_line(indent) << name(ref.value) << " = "
                       << group(value.arguments.at(0), value_type(ref.value)) << ";\n";
    }
}

bool source_emitter_t::body_t::emit_linear()
{
    using effect_kind = lifted_effect_t::kind_t;
    using value_kind = lifted_value_t::kind_t;
    if (graph.blocks.size() != 1 || !graph.blocks[0].successors.empty())
        return false;
    if (!local_arrays.empty())
        return false;
    bool initialized_prefix = false;
    for (const auto &op : binary.code) {
        if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) > 0) {
            if (initialized_prefix)
                return false;
            continue;
        }
        if (op.opcode == OP_PSH &&
            (op.argument == OP_ARG_NUM || op.argument == OP_ARG_STR || op.argument == OP_ARG_SPR)) {
            initialized_prefix = true;
            continue;
        }
        if (op.opcode == OP_NEG)
            continue;
        break;
    }
    bool entry_allocation = true;
    for (const auto &op : binary.code) {
        if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) > 0) {
            if (!entry_allocation)
                return false;
            continue;
        }
        if (op.opcode == OP_PSH &&
            (op.argument == OP_ARG_NUM || op.argument == OP_ARG_STR || op.argument == OP_ARG_SPR) && entry_allocation)
            continue;
        if (op.opcode == OP_NEG && entry_allocation)
            continue;
        entry_allocation = false;
    }
    const auto &effects = graph.blocks[0].effects;
    std::vector<unsigned> uses(graph.values.size());
    std::vector<std::set<unsigned>> inputs(effects.size());
    for (unsigned i = 0; i < effects.size(); ++i) {
        const auto &effect = effects[i];
        if (effect.kind == effect_kind::value && !used[effect.value] && !effectful(effect.value))
            continue;
        if (effect.kind != effect_kind::value && effect.kind != effect_kind::store &&
            effect.kind != effect_kind::return_value)
            return false;
        auto add_group = [&](const auto &refs) {
            std::set<unsigned> unique;
            for (auto ref : refs)
                unique.insert(ref.value);
            for (auto id : unique) {
                ++uses[id];
                inputs[i].insert(id);
            }
        };
        auto add_address = [&](const std::optional<lifted_address_t> &address) {
            if (!address)
                return;
            if (address->receiver)
                add_group(std::vector<value_ref_t>{*address->receiver});
            if (address->index)
                add_group(std::vector<value_ref_t>{*address->index});
        };
        if (effect.kind == effect_kind::value) {
            const auto &value = graph.values[effect.value];
            if (value.kind == value_kind::indexed_store || value.kind == value_kind::signal)
                return false;
            for (const auto &argument : value.arguments)
                add_group(argument);
            add_address(value.address);
        } else {
            for (const auto &argument : effect.arguments)
                add_group(argument);
            add_address(effect.address);
        }
    }
    std::vector<bool> inline_value(graph.values.size()), skipped(effects.size());
    std::map<unsigned, std::string> increments;
    for (unsigned i = 0; i + 2 < effects.size(); ++i) {
        if (effects[i].kind != effect_kind::value || effects[i + 1].kind != effect_kind::value ||
            effects[i + 2].kind != effect_kind::store)
            continue;
        const auto load_id = effects[i].value, update_id = effects[i + 1].value;
        const auto &load = graph.values[load_id], &update = graph.values[update_id];
        const auto &store = effects[i + 2];
        if (load.kind != value_kind::load || !load.address || update.kind != value_kind::operation || !store.address ||
            uses[load_id] != 1 || update.arguments.empty() ||
            update.arguments[0] != std::vector<value_ref_t>{{load_id, 0}} ||
            store.arguments[0] != std::vector<value_ref_t>{{update_id, 0}})
            continue;
        if (value_type(load_id) != "num" || load.address->receiver != store.address->receiver ||
            load.address->owner != store.address->owner || load.address->index != store.address->index ||
            load.address->offset != store.address->offset || load.address->width != store.address->width ||
            load.address->kind != store.address->kind)
            continue;
        const auto op = binary.code[update.instruction].opcode;
        bool increment = op == OP_INC, decrement = op == OP_DEC;
        if ((op == OP_ADD || op == OP_SUB) && update.arguments.size() == 2 && update.arguments[1].size() == 1) {
            const auto ref = update.arguments[1][0];
            if (!ref.offset && graph.values[ref.value].kind == value_kind::constant &&
                graph.values[ref.value].immediate == 0x3f800000u) {
                increment = op == OP_ADD;
                decrement = op == OP_SUB;
            }
        }
        if (!increment && !decrement)
            continue;
        const bool returns = i + 3 < effects.size() && effects[i + 3].kind == effect_kind::return_value &&
                             effects[i + 3].arguments == std::vector<std::vector<value_ref_t>>{{{update_id, 0}}};
        if (uses[update_id] != (returns ? 2u : 1u))
            continue;
        const bool postfix = !returns && update.instruction && binary.code[update.instruction - 1].opcode == OP_DUP;
        const auto target = address(*load.address, "num");
        const std::string operation = increment ? "++" : "--";
        increments[i] = (returns ? "return " : "") + (postfix ? target + operation : operation + target) + ";";
        skipped[i + 1] = skipped[i + 2] = true;
        if (returns)
            skipped[i + 3] = true;
        inline_value[load_id] = inline_value[update_id] = true;
    }


    std::map<unsigned, unsigned> producer;
    for (unsigned i = 0; i < effects.size(); ++i)
        if (effects[i].kind == effect_kind::value && !skipped[i] && !increments.count(i))
            producer.emplace(effects[i].value, i);
    for (unsigned sink = 0; sink < effects.size(); ++sink) {
        if (skipped[sink] || increments.count(sink))
            continue;
        std::vector<unsigned> sequence;
        std::set<unsigned> visiting;
        std::function<void(const std::vector<value_ref_t> &)> collect;
        auto collect_address = [&](const auto &a) {
            if (a) {
                if (a->index)
                    collect({*a->index});
                if (a->receiver)
                    collect({*a->receiver});
            }
        };
        collect = [&](const std::vector<value_ref_t> &refs) {
            if (refs.empty())
                return;
            bool composite = false;
            for (unsigned i = 1; i < refs.size(); ++i)
                if (refs[i].value != refs[0].value || refs[i].offset != refs[0].offset + i * 4)
                    composite = true;
            if (composite) {
                for (unsigned first = 0; first < refs.size();) {
                    unsigned end = first + 1;
                    while (end < refs.size() && refs[end].value == refs[first].value &&
                           refs[end].offset == refs[first].offset + (end - first) * 4)
                        ++end;
                    collect(std::vector<value_ref_t>(refs.begin() + first, refs.begin() + end));
                    first = end;
                }
                return;
            }
            auto ref = refs.front();
            auto it = producer.find(ref.value);
            if (it == producer.end() || it->second >= sink || uses[ref.value] != 1 || ref.offset)
                return;
            const auto &v = graph.values[ref.value];
            if (v.width != refs.size() * 4)
                return;
            if (binary.code[v.instruction].opcode == OP_POP && binary.code[v.instruction].argument == OP_ARG_SPR)
                return;

            const auto &stack_facts = source.stack.functions[index];
            if (v.instruction + 1 < stack_facts.heights.size() && stack_facts.heights[v.instruction + 1]) {
                const auto begin = *stack_facts.heights[v.instruction + 1] - static_cast<int>(v.width);
                for (unsigned pc = v.instruction + 1; pc < effects[sink].instruction; ++pc) {
                    const auto &ins = binary.code[pc];
                    if (ins.opcode == OP_POP && ins.argument == OP_ARG_SPR)
                        return;
                    if (ins.opcode != OP_PSH || ins.argument != OP_ARG_SPR || !stack_facts.heights[pc])
                        continue;
                    const auto from = *stack_facts.heights[pc] + static_cast<std::int16_t>(ins.value);
                    if (from < begin + static_cast<int>(v.width) && from + static_cast<int>(ins.data_size()) > begin)
                        return;
                }
            }

            for (unsigned word = 0; word < refs.size(); ++word)
                if (refs[word] != value_ref_t{ref.value, word * 4})
                    return;
            if (!visiting.insert(ref.value).second)
                return;
            for (const auto &args : v.arguments)
                collect(args);
            collect_address(v.address);
            sequence.push_back(it->second);
        };
        const auto &effect = effects[sink];
        if (effect.kind == effect_kind::value) {
            for (const auto &args : graph.values[effect.value].arguments)
                collect(args);
            collect_address(graph.values[effect.value].address);
        } else {
            const auto count = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
            if (effect.kind == effect_kind::callback || effect.kind == effect_kind::thread) {
                for (unsigned a = count; a > 0; --a)
                    collect(effect.arguments[a - 1]);
            } else
                for (unsigned a = 0; a < count; ++a)
                    collect(effect.arguments[a]);
            collect_address(effect.address);
        }
        if (sequence.empty() || sequence.size() > sink)
            continue;
        bool adjacent = true;
        for (unsigned j = 0; j < sequence.size(); ++j)
            if (sequence[j] != sink - sequence.size() + j)
                adjacent = false;
        if (adjacent)
            for (auto position : sequence)
                inline_value[effects[position].value] = true;
    }


    std::map<unsigned, std::vector<std::string>> preserved_text_operations;
    const auto &facts = source.stack.functions[index];
    std::map<unsigned, unsigned> store_producer;
    for (unsigned id = 0; id < graph.values.size(); ++id) {
        const auto &v = graph.values[id];
        if (v.width == 4 && v.kind != value_kind::uninitialized && v.kind != value_kind::merge &&
            v.kind != value_kind::parameter)
            store_producer.emplace(v.instruction, id);
    }
    std::vector<unsigned> last_use(graph.values.size());
    for (const auto &v : graph.values) {
        for (const auto &args : v.arguments)
            for (auto ref : args)
                last_use[ref.value] = std::max(last_use[ref.value], v.instruction);
        if (v.address) {
            if (v.address->receiver)
                last_use[v.address->receiver->value] = std::max(last_use[v.address->receiver->value], v.instruction);
            if (v.address->index)
                last_use[v.address->index->value] = std::max(last_use[v.address->index->value], v.instruction);
        }
    }
    for (const auto &e : effects) {
        for (const auto &args : e.arguments)
            for (auto ref : args)
                last_use[ref.value] = std::max(last_use[ref.value], e.instruction);
        if (e.address) {
            if (e.address->receiver)
                last_use[e.address->receiver->value] = std::max(last_use[e.address->receiver->value], e.instruction);
            if (e.address->index)
                last_use[e.address->index->value] = std::max(last_use[e.address->index->value], e.instruction);
        }
    }

    std::vector<unsigned> consumed_bytes(binary.code.size());
    for (const auto &v : graph.values) {
        if (v.instruction >= binary.code.size() || v.kind == value_kind::merge || v.kind == value_kind::uninitialized ||
            v.kind == value_kind::parameter)
            continue;
        unsigned bytes = 0;
        for (const auto &args : v.arguments)
            bytes += static_cast<unsigned>(args.size() * 4);
        if (v.address) {
            if (v.address->receiver)
                bytes += 4;
            if (v.address->index)
                bytes += 4;
        }
        consumed_bytes[v.instruction] = std::max(consumed_bytes[v.instruction], bytes);
    }
    for (const auto &e : effects)
        if (e.kind == effect_kind::store) {
            unsigned bytes = 0;
            for (const auto &args : e.arguments)
                bytes += static_cast<unsigned>(args.size() * 4);
            if (e.address) {
                if (e.address->receiver)
                    bytes += 4;
                if (e.address->index)
                    bytes += 4;
            }
            consumed_bytes[e.instruction] = std::max(consumed_bytes[e.instruction], bytes);
        }
    std::set<unsigned> value_effects;
    for (const auto &e : effects)
        if (e.kind == effect_kind::value)
            value_effects.insert(e.value);
    std::set<unsigned> recovered_assignments;
    unsigned local_serial = 0;
    std::map<std::string, unsigned> recovered_local_offsets;
    for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
        const auto &ins = binary.code[pc];
        if (ins.opcode != OP_PSH || (ins.argument != OP_ARG_NUM && ins.argument != OP_ARG_STR) || ins.data_size() != 4)
            continue;
        if (!facts.heights[pc] || !store_producer.count(pc))
            continue;
        const int slot = *facts.heights[pc];
        if (slot < static_cast<int>(binary.parameters_size))
            continue;
        bool safe = true;
        unsigned release = static_cast<unsigned>(binary.code.size());
        std::vector<std::pair<unsigned, unsigned>> writes;
        unsigned previous = store_producer.at(pc);
        for (unsigned j = pc + 1; j < binary.code.size(); ++j) {
            const auto &op = binary.code[j];
            if (!facts.heights[j]) {
                safe = false;
                break;
            }
            if (*facts.heights[j] < slot + 4) {
                release = j - 1;
                break;
            }
            if (consumed_bytes[j] && *facts.heights[j] - static_cast<int>(consumed_bytes[j]) < slot + 4) {
                safe = false;
                break;
            }
            if (op.opcode == OP_POP && *facts.heights[j] - static_cast<int>(op.data_size()) < slot + 4) {
                safe = false;
                break;
            }
            if (op.argument == OP_ARG_SPR) {
                const int from = *facts.heights[j] + static_cast<int16_t>(op.value);
                if (from < slot + 4 && from + static_cast<int>(op.data_size()) > slot) {
                    if (from != slot || op.data_size() != 4) {
                        safe = false;
                        break;
                    }
                    if (op.opcode == OP_PSH)
                        continue;
                    if (op.opcode != OP_POP || !j || !store_producer.count(j - 1) || last_use[previous] > j) {
                        safe = false;
                        break;
                    }
                    const auto id = store_producer.at(j - 1);
                    if (last_use[id] < j && !effectful(id) && graph.values[id].kind != value_kind::constant &&
                        graph.values[id].kind != value_kind::string) {
                        safe = false;
                        break;
                    }
                    writes.emplace_back(j, id);
                    previous = id;
                }
            }
            if (op.opcode == OP_ASF || op.opcode == OP_PSF || op.opcode == OP_COFF) {
                safe = false;
                break;
            }
        }
        if (!safe || release >= binary.code.size() || binary.code[release].opcode != OP_SPA)
            continue;
        for (unsigned j = release + 1; j < binary.code.size(); ++j) {
            const auto &op = binary.code[j];
            if (op.opcode != OP_RET && op.opcode != OP_SPA &&
                !(op.opcode == OP_PSH && (op.argument == OP_ARG_SPR || op.argument == OP_ARG_POPO)))
                safe = false;
        }
        if (!safe)
            continue;
        std::string type = ins.argument == OP_ARG_STR ? "str" : "num";
        for (const auto &[at, id] : writes)
            if (graph.values[id].kind != value_kind::constant && graph.values[id].kind != value_kind::string) {
                type = value_type(id);
                break;
            }
        if (type.starts_with("opaque<") || type == "void")
            continue;
        if (ins.argument == OP_ARG_STR && type != "str")
            continue;
        if (ins.argument == OP_ARG_NUM && type != "num" && ins.value != 0)
            continue;
        const auto local = "local" + std::to_string(local_serial++);
        recovered_local_offsets.emplace(local, static_cast<unsigned>(slot));
        const auto initial = ins.argument == OP_ARG_STR ? source.text(ins.value >> 16)
                             : type == "num"            ? number_literal(ins.value)
                                                        : "nullptr";
        preserved_text_operations[pc].push_back(type + " " + local + " = " + initial + ";");
        recovered_local_names.emplace(store_producer.at(pc), std::make_pair(local, type));
        for (const auto &[at, id] : writes) {
            if (value_effects.count(id))
                recovered_assignments.insert(id);
            else {
                const auto rendered = reference({id, 0}, type);
                preserved_text_operations[at].push_back(local + " = " + rendered + ";");
            }
            recovered_local_names.emplace(id, std::make_pair(local, type));
        }
    }
    for (unsigned i = 0; i < effects.size(); ++i) {
        const auto &e = effects[i];
        if (e.kind != effect_kind::value)
            continue;
        const auto &v = graph.values[e.value];
        if (v.kind != value_kind::operation || binary.code[v.instruction].opcode != OP_POP || v.arguments.size() != 1 ||
            v.arguments[0].size() != 1 || v.arguments[0][0].offset)
            continue;
        const auto producer = v.arguments[0][0].value;
        auto local = recovered_local_names.find(producer);
        if (local != recovered_local_names.end() && v.address &&
            recovered_local_offsets.at(local->second.first) == v.address->offset &&
            (preserved_text_operations.count(e.instruction) || recovered_assignments.count(producer))) {
            recovered_local_names.emplace(e.value, local->second);
            skipped[i] = true;
        }
    }
    auto next_text_operation = preserved_text_operations.begin();
    auto emit_text_operations = [&](unsigned before) {
        while (next_text_operation != preserved_text_operations.end() && next_text_operation->first <= before) {
            for (const auto &line : next_text_operation->second)
                source.out << scoped_line(indent) << line << '\n';
            ++next_text_operation;
        }
    };
    std::set<unsigned> declared;
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && graph.values[id].kind == value_kind::uninitialized && declared.insert(slots[id]).second)
            source.out << scoped_line(indent) << value_type(id) << ' ' << name(id) << ";\n";

    std::map<int, unsigned> reserved_declarations;
    int prefix_end = static_cast<int>(binary.parameters_size);
    for (const auto &op : binary.code) {
        if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) <= 0)
            break;
        prefix_end += static_cast<int16_t>(op.value);
    }
    for (const auto &effect : effects) {
        if (effect.kind != effect_kind::value || !stored(effect.value) || inline_value[effect.value] ||
            recovered_local_names.count(effect.value))
            continue;
        const auto &v = graph.values[effect.value];
        const auto pc = v.instruction + 1;
        if (pc >= binary.code.size() || !facts.heights[pc])
            continue;
        const auto &op = binary.code[pc];
        if (op.opcode != OP_POP || op.argument != OP_ARG_SPR || op.data_size() != v.width)
            continue;
        const int offset = *facts.heights[pc] + static_cast<int16_t>(op.value);
        if (offset >= static_cast<int>(binary.parameters_size) && offset + static_cast<int>(v.width) <= prefix_end)
            reserved_declarations.emplace(offset, effect.value);
    }
    for (const auto &[offset, id] : reserved_declarations)
        if (declared.insert(slots[id]).second)
            source.out << scoped_line(indent) << value_type(id) << ' ' << name(id) << ";\n";
    for (unsigned i = 0; i < effects.size(); ++i) {
        emit_text_operations(effects[i].instruction);
        if (effects[i].kind == effect_kind::value && recovered_assignments.count(effects[i].value)) {
            const auto id = effects[i].value;
            source.out << scoped_line(indent) << recovered_local_names.at(id).first << " = " << expression(id) << ";\n";
            continue;
        }
        if (skipped[i])
            continue;
        if (increments.count(i)) {
            source.out << scoped_line(indent) << increments[i] << '\n';
            continue;
        }
        const auto &effect = effects[i];
        if (effect.kind == effect_kind::value) {
            if (!used[effect.value] && !effectful(effect.value))
                continue;
            const auto &value = graph.values[effect.value];
            auto rendered = expression(effect.value);
            if (inline_value[effect.value])
                folded[effect.value] = std::move(rendered);
            else {
                source.out << scoped_line(indent);
                if (value.width && used[effect.value]) {
                    if (declared.insert(slots[effect.value]).second && !parameter_slots.count(slots[effect.value]))
                        source.out << value_type(effect.value) << ' ';
                    source.out << name(effect.value) << " = ";
                }
                if (retains_unused_result(effect.value))
                    source.out << value_type(effect.value) << " unused_result" << retained_result_serial++ << " = ";
                source.out << rendered << ";\n";
            }
        } else if (effect.kind == effect_kind::store) {
            const auto type = store_type(*effect.address);
            source.out << scoped_line(indent) << address(*effect.address, type) << " = "
                       << group(effect.arguments[0], type) << ";\n";
        } else {
            const auto &ret = binary.code[effect.instruction];
            if (types.result == "void" && effect.instruction + 1 == binary.code.size() && ret.argument == OP_ARG_WORD &&
                ret.value == binary.parameters_size)
                continue;
            source.out << scoped_line(indent) << "return";
            if (!effect.arguments.empty() && !effect.arguments[0].empty())
                source.out << ' ' << group(effect.arguments[0], source.type(types.result));
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
            if (next_field != shared_fields[object].end())
                end = std::min(end, next_field->first);
            const auto next_array = shared_arrays[object].upper_bound(offset);
            if (next_array != shared_arrays[object].end())
                end = std::min(end, next_array->first);
            if (!object)
                for (const auto &initializer : image.initializers)
                    if (initializer.offset >= minimum_end &&
                        (initializer.name_hash ||
                         (initializer.kind != -4 && !(initializer.kind >= 0 && array.type == "str"))))
                        end = std::min(end, initializer.offset);
            if (!array.width || end < minimum_end)
                throw error_t("Indexed static storage exceeds its recovered extent");
            array.count = (end - offset) / array.width;
            if (array.type.empty() || array.type.starts_with("opaque<")) {
                if (array.width == 4)
                    array.type = "num";
                else if (array.width == 12)
                    array.type = "vector3d";
                else
                    throw error_t("Indexed static storage has an unresolved element type");
            }
        }
    }
    for (const auto &initializer : image.initializers) {
        auto &field = shared_fields[0][initializer.offset];
        if (initializer.kind != -4 || !field.width)
            field.width = 4;
        if (initializer.kind >= 0)
            field.type = "str";
        else if (initializer.kind != -4)
            field.type = image.objects.at(image.object_order.at(initializer.object)).name;
        else if (field.type.empty() || field.type.starts_with("opaque<"))
            field.type = "num";
        if (initializer.name_hash) {
            auto symbol = symbols.names.find(initializer.name_hash);
            shared_names[0][initializer.offset] = symbol == symbols.names.end()
                                                      ? "global_" + hex(initializer.name_hash).substr(2)
                                                      : identifier(symbol->second, "global");
        }
    }
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        for (const auto &[offset, array] : shared_arrays[object]) {
            const unsigned end = offset + array.width * array.count;
            auto next = shared_fields[object].lower_bound(offset);
            while (next != shared_fields[object].end() && next->first < end) {
                if (next->first != offset)
                    shared_names[object].erase(next->first);
                next = shared_fields[object].erase(next);
            }
            shared_fields[object][offset] = {array.width * array.count, array.type};
        }
        for (unsigned offset = 0; offset < image.objects[object].static_data.size();) {
            auto [field, inserted] = shared_fields[object].emplace(offset, field_t{4, "num"});
            if (field->second.width == 0)
                throw error_t("Static field has no recovered width");
            if (!shared_names[object].count(offset))
                shared_names[object][offset] = "global_" + std::to_string(offset);
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

std::optional<std::pair<std::string, std::string>> source_emitter_t::shared_member(const lifted_address_t &address,
                                                                                   std::string_view index) const
{
    if (address.kind != lifted_address_t::kind_t::shared)
        return {};
    const auto &members = shared_fields[address.owner];
    auto found = members.upper_bound(address.offset);
    if (found == members.begin())
        return {};
    --found;
    const auto displacement = address.offset - found->first;
    auto name = classes[address.owner] + "::" + shared_names[address.owner].at(found->first);
    if (auto array = shared_arrays[address.owner].find(found->first); array != shared_arrays[address.owner].end()) {
        const auto &element = array->second;
        if (displacement >= element.width * element.count)
            return {};
        name += "[";
        if (address.index) {
            if (index.empty())
                return {};
            name += index;
            if (displacement / element.width)
                name += " + " + std::to_string(displacement / element.width);
        } else
            name += std::to_string(displacement / element.width);
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
    if (address.index)
        return {};
    if ((found->second.type == "vector3d" || found->second.type == "position3d") && address.width == 4 &&
        displacement < 12) {
        name += ".";
        name += "xyz"[displacement / 4];
        return std::make_pair(name, "num");
    }
    if (displacement || address.width != found->second.width)
        return {};
    return std::make_pair(name, type(found->second.type));
}

std::string source_emitter_t::body_t::initializer_arguments(unsigned target)
{
    if (graph.blocks.size() != 1)
        throw error_t("Constructor argument builder requires structured expression recovery");
    const std::vector<value_ref_t> *result = nullptr;
    for (const auto &effect : graph.blocks.front().effects) {
        if (effect.kind == lifted_effect_t::kind_t::return_value) {
            if (effect.arguments.size() != 1)
                throw error_t("Constructor argument builder has no return storage");
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
    if (!result)
        throw error_t("Constructor argument builder does not return");
    const auto constructor = source.functions.at({target, 0});
    const auto &signature = source.types.functions[constructor];
    const auto &record = source.binary(constructor);
    std::string arguments;
    unsigned consumed = 0;
    for (const auto &parameter : signature.parameters) {
        if (!(record.flags & 1u) && parameter.offset == 0)
            continue;
        const unsigned words = parameter.width / 4;
        if (consumed + words > result->size())
            throw error_t("Constructor argument builder returns insufficient storage");
        if (!arguments.empty())
            arguments += ", ";
        arguments +=
            group({result->begin() + consumed, result->begin() + consumed + words}, source.type(parameter.type));
        consumed += words;
    }
    if (consumed != result->size())
        throw error_t("Constructor argument builder returns excess storage");
    return arguments;
}

void source_emitter_t::emit_static_fields(unsigned object)
{
    const auto &data = image.objects[object].static_data;
    for (const auto &[offset, field] : shared_fields[object]) {
        const auto name = shared_names[object].find(offset);
        if (name == shared_names[object].end())
            throw error_t("Overlapping static field interpretations require reconciliation");
        out << "        static " << type(field.type) << ' ' << name->second;
        if (!object) {
            auto array = shared_arrays[object].find(offset);
            const unsigned width =
                array == shared_arrays[object].end() ? field.width : array->second.width * array->second.count;
            const bool initialized =
                std::any_of(image.initializers.begin(), image.initializers.end(), [&](const auto &initial) {
                    return initial.offset >= offset && initial.offset < offset + width;
                });
            if (!initialized) {
                if (array != shared_arrays[object].end())
                    out << '[' << array->second.count << ']';
                out << ";\n";
                continue;
            }
        }
        if (auto array = shared_arrays[object].find(offset); array != shared_arrays[object].end()) {
            const auto &element = array->second;
            out << '[' << element.count << "] = {";
            for (unsigned index = 0; index < element.count; ++index) {
                if (index)
                    out << ", ";
                const bool vector = element.type == "vector3d" || element.type == "position3d";
                if (vector)
                    out << element.type << '{';
                for (unsigned component = 0; component < element.width / 4; ++component) {
                    if (component)
                        out << ", ";
                    const unsigned location = offset + index * element.width + component * 4;
                    auto bits = read_word(data, location);
                    const initializer_t *initial = nullptr;
                    if (!object)
                        for (const auto &candidate : image.initializers)
                            if (candidate.offset == location) {
                                initial = &candidate;
                                break;
                            }
                    if (!object && !initial) {
                        out << "{}";
                        continue;
                    }
                    if (initial && initial->kind == -4)
                        bits = initial->value;
                    if (initial && initial->kind >= 0)
                        out << text(static_cast<unsigned>(initial->kind));
                    else if (vector || element.type == "num")
                        out << number_literal(bits);
                    else if (!bits)
                        out << "nullptr";
                    else
                        throw error_t("Static handle array contains an unresolved non-null initializer");
                }
                if (vector)
                    out << '}';
            }
            out << "};\n";
            continue;
        }
        const initializer_t *initializer = nullptr;
        if (!object)
            for (const auto &candidate : image.initializers)
                if (candidate.offset == offset) {
                    initializer = &candidate;
                    break;
                }
        if (initializer && initializer->kind >= 0)
            text_definitions.emplace_back(offset, static_cast<unsigned>(initializer->kind));
        else if (initializer && (initializer->kind == -3 || initializer->kind == -2 || initializer->kind == -1)) {
            std::ostringstream saved;
            if (initializer->builder)
                out.swap(saved);
            out << " = " << type(field.type) << (initializer->kind == -2 ? "::from_game_data(" : "::create_named(");
            auto symbol = symbols.names.find(initializer->name_hash);
            out << (symbol == symbols.names.end() ? hex(initializer->name_hash) : quote(symbol->second));
            if (initializer->builder) {
                const auto position = static_cast<unsigned>(initializer - image.initializers.data());
                for (unsigned f = 0; f < graphs.size(); ++f) {
                    if (!graphs[f].initializer || graphs[f].function != position)
                        continue;
                    auto arguments =
                        body_t(*this, f, "").initializer_arguments(image.object_order.at(initializer->object));
                    if (!arguments.empty())
                        out << ", " << arguments;
                    break;
                }
            }
            out << ')';
            if (initializer->builder) {
                const auto expression = out.str();
                out.swap(saved);
                builder_definitions.emplace_back(initializer->builder->code_order,
                                                 "    " + type(field.type) + " " + classes[object] +
                                                     "::" + name->second + expression + ";\n\n");
            }
        } else if (!initializer || initializer->kind == -4) {
            if (field.width == 4) {
                const auto bits = initializer ? initializer->value : read_word(data, offset);
                if (field.type == "num")
                    out << " = " << number_literal(bits);
                else if (!bits)
                    out << " = nullptr";
                else
                    out << " = bit_cast<" << type(field.type) << ">(" << number_literal(bits) << ')';
            } else if (field.type == "vector3d" || field.type == "position3d") {
                out << " = " << field.type << '{';
                for (unsigned component = 0; component < 3; ++component) {
                    if (component)
                        out << ", ";
                    auto bits = read_word(data, offset + component * 4);
                    if (!object)
                        for (const auto &candidate : image.initializers)
                            if (candidate.offset == offset + component * 4 && candidate.kind == -4)
                                bits = candidate.value;
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
    emit_recovered_before(b, effect.instruction, line);
    using effect_kind = lifted_effect_t::kind_t;
    using value_kind = lifted_value_t::kind_t;
    const auto &block = graph.blocks[b];
    const auto &instruction = binary.code[effect.instruction];
    bool ended = false;
    switch (effect.kind) {
    case effect_kind::value: {
        const auto &value = graph.values[effect.value];
        if (!used[effect.value] && !effectful(effect.value))
            break;
        if (stored(effect.value) && !initialized_slots.count(slots[effect.value]) &&
            (adjacent_expressions.empty() || !adjacent_expressions[effect.value])) {
            auto init = declaration_initializers.find(slots[effect.value]);
            if (init != declaration_initializers.end() && init->second == effect.value)
                break;
        }
        if (auto update = postfix_locals.find(effect.value); update != postfix_locals.end()) {
            if (value.kind == value_kind::native_call) {
                const auto &native = source.symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
                source.out << scoped_line(line) << name(update->second)
                           << (native.signature.starts_with("operator++(") ? "++" : "--") << ";\n";
            } else
                source.out << scoped_line(line) << name(update->second)
                           << (instruction.opcode == OP_INC ? "++;\n" : "--;\n");
            break;
        }
        if (!adjacent_expressions.empty() && adjacent_expressions[effect.value]) {
            folded[effect.value] = expression(effect.value);
            break;
        }
        if (value.kind == value_kind::operation && (instruction.opcode == OP_INC || instruction.opcode == OP_DEC) &&
            stored(effect.value) && value_type(effect.value) == "num" && value.arguments.size() == 1 &&
            value.arguments[0].size() == 1 && !value.arguments[0][0].offset && effect.instruction >= 2 &&
            effect.instruction + 1 < binary.code.size() && binary.code[effect.instruction - 1].opcode == OP_DUP &&
            binary.code[effect.instruction - 2].opcode == OP_PSH &&
            binary.code[effect.instruction - 2].argument == OP_ARG_SPR &&
            binary.code[effect.instruction + 1].opcode == OP_POP &&
            binary.code[effect.instruction + 1].argument == OP_ARG_SPR &&
            group(value.arguments[0], "num") == name(effect.value)) {
            const auto &facts = source.stack.functions[index];
            if (facts.heights[effect.instruction - 2] && facts.heights[effect.instruction + 1] &&
                *facts.heights[effect.instruction - 2] +
                        static_cast<int16_t>(binary.code[effect.instruction - 2].value) ==
                    *facts.heights[effect.instruction + 1] +
                        static_cast<int16_t>(binary.code[effect.instruction + 1].value)) {
                source.out << scoped_line(line) << name(effect.value)
                           << (instruction.opcode == OP_INC ? "++;\n" : "--;\n");
                break;
            }
        }
        if (value.kind == value_kind::operation && (instruction.opcode == OP_INC || instruction.opcode == OP_DEC) &&
            stored(effect.value) && value_type(effect.value) == "num" && value.arguments.size() == 1 &&
            value.arguments[0].size() == 1 && !value.arguments[0][0].offset &&
            (effect.instruction == 0 || binary.code[effect.instruction - 1].opcode != OP_DUP) &&
            effect.instruction + 1 < binary.code.size() && binary.code[effect.instruction + 1].opcode == OP_POP &&
            binary.code[effect.instruction + 1].argument == OP_ARG_SPR &&
            group(value.arguments[0], "num") == name(effect.value)) {
            source.out << scoped_line(line) << (instruction.opcode == OP_INC ? "++" : "--") << name(effect.value)
                       << ";\n";
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
                        (!aliases(ref) || ref.offset != word * 4))
                        changes = true;
                }
                if (changes && (aliases(*storage.index) ||
                                std::any_of(value.arguments[1].begin(), value.arguments[1].end(), aliases))) {
                    const auto temporary = "array_write" + std::to_string(edge_serial++);
                    source.out << scoped_line(line) << "num " << temporary << "_index = " << element << ";\n";
                    source.out << scoped_line(line) << source.type(array.type) << ' ' << temporary
                               << "_value = " << stored_value << ";\n";
                    element = temporary + "_index";
                    stored_value = temporary + "_value";
                }
                array_copy(effect.value, value.arguments[0], line);
                source.out << scoped_line(line) << name(effect.value) << '[' << element << "] = " << stored_value
                           << ";\n";
            } else
                array_copy(effect.value, value.arguments[0], line);
        } else {
            source.out << scoped_line(line);
            if (retains_unused_result(effect.value))
                source.out << value_type(effect.value) << " unused_result" << retained_result_serial++ << " = ";
            if (value.width && used[effect.value]) {
                auto init = initialized_slots.find(slots[effect.value]);
                if (!parameter_slots.count(slots[effect.value]) &&
                    ((init != initialized_slots.end() && init->second == effect.value) ||
                     lexical_initializer_values.count(effect.value)))
                    source.out << value_type(effect.value) << ' ';
                source.out << name(effect.value) << " = ";
            }
            auto actual = value_type(effect.value);
            if (value.kind == value_kind::operation && instruction.opcode != OP_PSH && instruction.opcode != OP_POP)
                actual =
                    instruction.opcode == OP_I2S || instruction.opcode == OP_F2S || instruction.opcode == OP_PSH_STR
                        ? "str"
                        : "num";
            else if (value.kind == value_kind::signal)
                actual = "signal";
            else if (value.kind == value_kind::script_call)
                actual = source.type(source.types.functions[source.target(value.immediate)].result);
            else if (value.kind == value_kind::native_call) {
                const auto identity = std::make_pair(value.immediate >> 16, value.immediate & 0xffff);
                actual = source.native_result(identity);
            }
            auto rendered = expression(effect.value);
            source.out << (value.width && used[effect.value]
                               ? cast(std::move(rendered), actual, value_type(effect.value))
                               : rendered)
                       << ";\n";
        }
        break;
    }
    case effect_kind::store: {
        if (effect.arguments.size() == 1 && effect.arguments[0].size() == 1 && !effect.arguments[0][0].offset) {
            const auto &update = graph.values[effect.arguments[0][0].value];
            if (update.kind == value_kind::operation && update.instruction > 0 &&
                (binary.code[update.instruction].opcode == OP_INC ||
                 binary.code[update.instruction].opcode == OP_DEC) &&
                binary.code[update.instruction - 1].opcode == OP_DUP && update.arguments.size() == 1 &&
                update.arguments[0].size() == 1 && !update.arguments[0][0].offset) {
                const auto &old = graph.values[update.arguments[0][0].value];
                if (old.kind == value_kind::load && old.address && effect.address &&
                    old.address->kind == effect.address->kind && old.address->owner == effect.address->owner &&
                    old.address->offset == effect.address->offset && old.address->width == 4 &&
                    effect.address->width == 4 && old.address->receiver == effect.address->receiver &&
                    old.address->index == effect.address->index && adjacent_expressions[effect.arguments[0][0].value] &&
                    adjacent_expressions[update.arguments[0][0].value]) {
                    source.out << scoped_line(line) << address(*effect.address, "num")
                               << (binary.code[update.instruction].opcode == OP_INC ? "++;\n" : "--;\n");
                    break;
                }
            }
        }
        const auto type = store_type(*effect.address);
        source.out << scoped_line(line) << address(*effect.address, type) << " = " << group(effect.arguments[0], type)
                   << ";\n";
    } break;
    case effect_kind::branch:
        if (effect.arguments.empty() || block.successors.size() == 1)
            edge(b, block.successors.at(0), line);
        else {
            const auto destination =
                static_cast<unsigned>(static_cast<std::int64_t>(instruction.offset) + instruction.size() +
                                      static_cast<std::int16_t>(instruction.value));
            auto taken = block.successors.at(0), other = block.successors.at(1);
            if (binary.code[graph.blocks[taken].begin].offset != destination)
                std::swap(taken, other);
            source.out << scoped_line(line) << "if (!" << group(effect.arguments[0], "num") << ") {\n";
            edge(b, taken, line + "    ");
            source.out << scoped_line(line) << "} else {\n";
            edge(b, other, line + "    ");
            source.out << scoped_line(line) << "}\n";
        }
        ended = true;
        break;
    case effect_kind::return_value:
        if (types.result == "void" && line == indent && effect.instruction + 1 == binary.code.size() &&
            instruction.argument == OP_ARG_WORD && instruction.value == binary.parameters_size) {
            ended = true;
            break;
        }
        source.out << scoped_line(line) << "return";
        if (!effect.arguments.empty() && !effect.arguments[0].empty())
            source.out << ' ' << group(effect.arguments[0], source.type(types.result));
        source.out << ";\n";
        ended = true;
        break;
    case effect_kind::thread: {
        const bool owner = instruction.opcode == OP_BTH;
        source.out << scoped_line(line) << (owner ? "start_thread_on(" : "start_thread(");
        if (owner)
            source.out << group(effect.arguments[0]) << ", ";
        source.out << call(instruction.value, effect.arguments[owner ? 1 : 0]) << ");\n";
        break;
    }
    case effect_kind::callback: {
        const bool owner = instruction.opcode == OP_ECB || instruction.opcode == OP_ECO;
        const bool once = instruction.opcode == OP_ECO || instruction.opcode == OP_SCO;
        source.out << scoped_line(line) << (once ? "bind_signal_once" : "bind_signal") << (owner ? "_on(" : "(");
        if (owner)
            source.out << group(effect.arguments[0]) << ", ";
        source.out << group(effect.arguments[owner ? 2 : 1], "signal") << ", "
                   << call(instruction.value, effect.arguments[owner ? 1 : 0]) << ");\n";
        break;
    }
    case effect_kind::signal:
        source.out << scoped_line(line) << (instruction.opcode == OP_RAE ? "raise_all_signals(" : "raise_signal(");
        if (!effect.arguments.empty())
            source.out << group(effect.arguments[0]) << ", ";
        source.out << source.text(instruction.value >> 16) << ");\n";
        break;
    case effect_kind::termination: {
        const bool massacre = instruction.opcode == OP_DEL_THREADS || instruction.opcode == OP_MS2;
        if (instruction.argument == OP_ARG_NULL) {
            source.out << scoped_line(line) << (massacre ? "delete_all_threads();" : "stop_thread();") << '\n';
            ended = true;
        } else {
            auto callee = source.target(instruction.value);
            const auto &target = source.graphs[callee];
            source.out << scoped_line(line) << (massacre ? "delete_threads" : "kill_threads")
                       << (effect.arguments.empty() ? "(" : "_on(");
            if (!effect.arguments.empty())
                source.out << group(effect.arguments[0]) << ", ";
            const auto &method = source.names[target.object][target.function];
            const auto &methods = source.names[target.object];
            const bool overloaded = std::count(methods.begin(), methods.end(), method) > 1;
            if (overloaded) {
                const auto &record = source.binary(callee);
                const auto &signature = source.types.functions[callee];
                source.out << "static_cast<" << source.type(signature.result) << " (";
                if (record.flags & 1u)
                    source.out << '*';
                else
                    source.out << source.classes[target.object] << "::*";
                source.out << ")(";
                bool comma = false;
                for (const auto &parameter : signature.parameters) {
                    if (!(record.flags & 1u) && parameter.offset == 0)
                        continue;
                    if (comma)
                        source.out << ',';
                    source.out << source.type(parameter.type);
                    comma = true;
                }
                source.out << ")>(&";
            }
            source.out << source.classes[target.object] << "::" << method;
            if (overloaded)
                source.out << ')';
            source.out << ");\n";
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
    if (!exit || !reachable[0])
        return false;
    set_t blocks, nodes{exit};
    for (unsigned b = 0; b < exit; ++b)
        if (reachable[b]) {
            blocks.insert(b);
            nodes.insert(b);
        }
    std::vector<std::vector<unsigned>> predecessors(exit);
    for (unsigned b = 0; b < exit; ++b)
        if (reachable[b])
            for (auto successor : graph.blocks[b].successors)
                predecessors[successor].push_back(b);
    auto intersect = [](set_t &left, const set_t &right) {
        for (auto i = left.begin(); i != left.end();) {
            if (!right.count(*i))
                i = left.erase(i);
            else
                ++i;
        }
    };

    std::vector<set_t> dominators(exit, blocks);
    dominators[0] = {0};
    for (bool changed = true; changed;) {
        changed = false;
        for (unsigned b = 1; b < exit; ++b)
            if (reachable[b]) {
                auto next = blocks;
                for (auto predecessor : predecessors[b])
                    intersect(next, dominators[predecessor]);
                next.insert(b);
                if (next != dominators[b]) {
                    dominators[b] = std::move(next);
                    changed = true;
                }
            }
    }


    std::vector<std::vector<unsigned>> onward(exit);
    for (unsigned b = 0; b < exit; ++b)
        if (reachable[b]) {
            onward[b] = graph.blocks[b].successors;
            if (onward[b].empty())
                onward[b].push_back(exit);
        }
    for (;;) {
        std::vector<bool> escapes(exit + 1);
        escapes[exit] = true;
        for (bool changed = true; changed;) {
            changed = false;
            for (unsigned b = 0; b < exit; ++b)
                if (reachable[b] && !escapes[b])
                    for (auto successor : onward[b])
                        if (escapes[successor]) {
                            escapes[b] = true;
                            changed = true;
                        }
        }
        unsigned trapped = exit;
        for (unsigned b = 0; b < exit; ++b)
            if (reachable[b] && !escapes[b] && (trapped == exit || dominators[b].size() < dominators[trapped].size()))
                trapped = b;
        if (trapped == exit)
            break;
        onward[trapped].push_back(exit);
    }
    std::vector<set_t> postdominators(exit + 1, nodes);
    postdominators[exit] = {exit};
    for (bool changed = true; changed;) {
        changed = false;
        for (unsigned b = 0; b < exit; ++b)
            if (reachable[b]) {
                auto next = nodes;
                for (auto successor : onward[b])
                    intersect(next, postdominators[successor]);
                next.insert(b);
                if (next != postdominators[b]) {
                    postdominators[b] = std::move(next);
                    changed = true;
                }
            }
    }

    std::vector<unsigned> join(exit, exit);
    for (unsigned b = 0; b < exit; ++b)
        if (reachable[b])
            for (auto candidate : postdominators[b])
                if (candidate != b && postdominators[candidate].size() > postdominators[join[b]].size())
                    join[b] = candidate;

    std::map<unsigned, set_t> loops;
    for (unsigned tail = 0; tail < exit; ++tail)
        if (reachable[tail])
            for (auto head : graph.blocks[tail].successors)
                if (dominators[tail].count(head)) {
                    auto &members = loops[head];
                    members.insert(head);
                    std::vector<unsigned> work;
                    if (tail != head && members.insert(tail).second)
                        work.push_back(tail);
                    while (!work.empty()) {
                        auto node = work.back();
                        work.pop_back();
                        for (auto predecessor : predecessors[node])
                            if (members.insert(predecessor).second)
                                work.push_back(predecessor);
                    }
                }


    auto choose_follows = [&](bool from_header) {
        std::map<unsigned, unsigned> chosen;
        for (const auto &[head, members] : loops) {
            unsigned follow = exit;
            if (from_header) {
                unsigned outside = 0;
                for (auto successor : graph.blocks[head].successors)
                    if (!members.count(successor)) {
                        ++outside;
                        follow = successor;
                    }
                if (outside != 1)
                    follow = exit;
            }
            if (follow == exit) {
                set_t candidates, returning;
                for (auto member : members)
                    for (auto successor : graph.blocks[member].successors)
                        if (!members.count(successor))
                            (graph.blocks[successor].successors.empty() ? returning : candidates).insert(successor);
                if (candidates.empty())
                    candidates = std::move(returning);
                if (!candidates.empty()) {
                    auto common = nodes;
                    for (auto candidate : candidates)
                        intersect(common, postdominators[candidate]);
                    for (auto candidate : common)
                        if (!members.count(candidate) &&
                            postdominators[candidate].size() > postdominators[follow].size())
                            follow = candidate;
                    if (follow == exit && candidates.size() == 1)
                        follow = *candidates.begin();
                }
            }
            chosen[head] = follow;
        }
        return chosen;
    };
    std::map<unsigned, unsigned> follows;


    auto quiet_effect = [&](const lifted_effect_t &effect) {
        if (effect.kind == effect_kind::branch)
            return true;
        if (effect.kind != effect_kind::value)
            return false;
        if (!used[effect.value] && !effectful(effect.value))
            return true;
        return !adjacent_expressions.empty() && adjacent_expressions[effect.value];
    };
    int prefix_slot = static_cast<int>(binary.parameters_size);
    for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
        const auto &ins = binary.code[pc];
        if (ins.opcode == OP_SPA && static_cast<int16_t>(ins.value) > 0) {
            prefix_slot += static_cast<int16_t>(ins.value);
            continue;
        }
        if (ins.opcode != OP_PSH ||
            (ins.argument != OP_ARG_NUM && ins.argument != OP_ARG_STR && ins.argument != OP_ARG_SPR) ||
            ins.data_size() != 4)
            break;
        const bool negated =
            ins.argument == OP_ARG_NUM && pc + 1 < binary.code.size() && binary.code[pc + 1].opcode == OP_NEG;
        if (negated)
            ++pc;
        const auto word = static_cast<unsigned>(prefix_slot / 4);
        if (word >= graph.blocks[0].outgoing.size())
            break;
        auto initial = graph.blocks[0].outgoing[word];
        if (ins.argument == OP_ARG_NUM || ins.argument == OP_ARG_STR) {
            for (unsigned id = 0; id < graph.values.size(); ++id) {
                const auto &v = graph.values[id];
                if (v.instruction == pc && v.width == 4 &&
                    (v.kind == value_kind::constant || v.kind == value_kind::string ||
                     (negated && v.kind == value_kind::operation))) {
                    initial = {id, 0};
                    break;
                }
            }
        }
        {
            bool survives = true;
            const auto &facts = source.stack.functions[index];
            for (unsigned next = pc + 1; next < graph.blocks[0].end; ++next)
                if (facts.heights[next] && *facts.heights[next] < prefix_slot + 4)
                    survives = false;
            if (!graph.blocks[0].effects.empty()) {
                const auto last = graph.blocks[0].effects.back().instruction;
                if (binary.code[last].opcode == OP_BF && facts.heights[last] &&
                    *facts.heights[last] - 4 < prefix_slot + 4)
                    survives = false;
            }
            for (auto successor : graph.blocks[0].successors) {
                const auto at = graph.blocks[successor].begin;
                if (facts.heights[at] && *facts.heights[at] < prefix_slot + 4)
                    survives = false;
            }
            if (!survives)
                break;
        }
        const auto &value = graph.values[initial.value];
        if (initial.offset)
            break;
        if (ins.argument == OP_ARG_SPR) {
            if (value.kind != value_kind::parameter || value.immediate != prefix_slot + static_cast<int16_t>(ins.value))
                break;
        } else if (value.instruction != pc || (value.kind != value_kind::constant && value.kind != value_kind::string &&
                                               !(negated && value.kind == value_kind::operation)))
            break;
        std::set<unsigned> candidates;
        for (unsigned b = 0; b < graph.blocks.size(); ++b)
            if (reachable[b]) {
                auto collect = [&](const auto &refs) {
                    if (word < refs.size() && !refs[word].offset && stored(refs[word].value))
                        candidates.insert(slots[refs[word].value]);
                };
                collect(graph.blocks[b].incoming);
                collect(graph.blocks[b].outgoing);
            }
        if (candidates.size() != 1)
            break;
        const auto slot = *candidates.begin();
        if (initialized_slots.count(slot) || graph.values[slot_values[slot]].width != 4)
            break;
        if (value_type(slot_values[slot]) != value_type(initial.value))
            break;
        declaration_initializers.emplace(slot, initial.value);
        prefix_slot += 4;
    }

    auto quiet_block = [&](unsigned b) {
        auto operations = recovered_local_operations.lower_bound(graph.blocks[b].begin);
        if (operations != recovered_local_operations.end() && operations->first < graph.blocks[b].end)
            return false;
        for (const auto &effect : graph.blocks[b].effects)
            if (!quiet_effect(effect))
                return false;
        return true;
    };


    auto quiet_transfer = [&](value_ref_t target, value_ref_t input) {
        if (graph.values[input.value].kind == value_kind::uninitialized)
            return true;
        if (stored(target.value)) {
            auto parameter = parameter_slots.find(slots[target.value]);
            if (parameter != parameter_slots.end() && !input.offset &&
                graph.values[input.value].kind == value_kind::parameter &&
                graph.values[input.value].immediate == parameter->second)
                return true;
        }
        if (!stored(input.value))
            return false;
        return slots[input.value] == slots[target.value] && input.offset == target.offset;
    };
    auto quiet_edge = [&](unsigned from, unsigned to) {
        const auto &origin = graph.blocks[from];
        const auto &destination = graph.blocks[to];
        for (unsigned word = 0; word < destination.incoming.size(); ++word) {
            const auto ref = destination.incoming[word];
            if (used[ref.value] && graph.values[ref.value].kind == value_kind::merge &&
                !retained_initializer(from, word, ref, origin.outgoing.at(word)) &&
                !quiet_transfer(ref, origin.outgoing.at(word)))
                return false;
        }
        if (destination.incoming_array_index && used[destination.incoming_array_index->value] &&
            !quiet_transfer(*destination.incoming_array_index, *origin.outgoing_array_index))
            return false;
        return true;
    };
    auto conditional = [&](unsigned b) {
        const auto &block = graph.blocks[b];
        return block.successors.size() == 2 && block.successors[0] != block.successors[1] && !block.effects.empty() &&
               block.effects.back().kind == effect_kind::branch && !block.effects.back().arguments.empty() &&
               !block.effects.back().arguments[0].empty();
    };


    auto arms = [&](unsigned b, unsigned &on_false, unsigned &on_true) {
        const auto &block = graph.blocks[b];
        const auto &instruction = binary.code[block.effects.back().instruction];
        const auto destination =
            static_cast<unsigned>(static_cast<std::int64_t>(instruction.offset) + instruction.size() +
                                  static_cast<std::int16_t>(instruction.value));
        on_false = block.successors[0];
        on_true = block.successors[1];
        if (binary.code[graph.blocks[on_false].begin].offset != destination)
            std::swap(on_false, on_true);
    };


    set_t bypassed;
    auto resolve = [&](unsigned node) {
        for (unsigned guard = 0; guard <= exit; ++guard) {
            const auto &block = graph.blocks[node];
            if (block.successors.size() != 1 || loops.count(node))
                break;
            if (!quiet_block(node) || !quiet_edge(node, block.successors[0]))
                break;
            bypassed.insert(node);
            node = block.successors[0];
        }
        return node;
    };

    struct term_t {
        unsigned block;
        bool negate;
        bool conjunction;
    };
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
        auto case_test = [&](unsigned block) {
            if (graph.blocks[block].effects.empty())
                return false;
            const auto pc = graph.blocks[block].effects.back().instruction;
            if (pc < 3 || pc + 1 >= binary.code.size())
                return false;
            const auto &load = binary.code[pc - 3], &constant = binary.code[pc - 2], &compare = binary.code[pc - 1],
                       &branch = binary.code[pc], &jump = binary.code[pc + 1];
            return load.opcode == OP_PSH && load.argument == OP_ARG_SPR && static_cast<int16_t>(load.value) == -4 &&
                   constant.opcode == OP_PSH && constant.argument == OP_ARG_NUM && compare.opcode == OP_EQ &&
                   branch.opcode == OP_BF && static_cast<int16_t>(branch.value) == 4 && jump.opcode == OP_BRA;
        };
        if (!case_test(node))
            return chain;
        for (bool extended = true; extended;) {
            extended = false;
            for (unsigned side = 0; side < 2 && !extended; ++side) {
                auto &probe = side ? chain.positive : chain.negative;
                auto &probe_from = side ? chain.positive_from : chain.negative_from;
                auto &held = side ? chain.negative : chain.positive;
                auto &held_from = side ? chain.negative_from : chain.positive_from;
                const auto candidate = probe;
                if (!case_test(candidate))
                    continue;


                if (loops.count(candidate))
                    continue;
                if (std::any_of(chain.terms.begin(), chain.terms.end(), [&](const term_t &term) {
                        return term.block == candidate;
                    }))
                    continue;
                if (!conditional(candidate) || !quiet_block(candidate) || predecessors[candidate].size() != 1)
                    continue;
                if (probe_from != exit && !quiet_edge(probe_from, candidate))
                    continue;
                if (held_from != exit && !quiet_edge(held_from, held))
                    continue;
                unsigned candidate_false = 0, candidate_true = 0;
                arms(candidate, candidate_false, candidate_true);
                unsigned shared = exit, rest = exit;
                bool negate = false;
                if (resolve(candidate_true) == resolve(held)) {
                    shared = candidate_true;
                    rest = candidate_false;
                    negate = side != 0;
                } else if (resolve(candidate_false) == resolve(held)) {
                    shared = candidate_false;
                    rest = candidate_true;
                    negate = side == 0;
                } else
                    continue;
                if (side || negate)
                    continue;
                auto case_destination = [&](unsigned b) {
                    const auto &jump = binary.code[graph.blocks[b].effects.back().instruction + 1];
                    return static_cast<int64_t>(jump.offset) + jump.size() + static_cast<int16_t>(jump.value);
                };
                if (case_destination(node) != case_destination(candidate))
                    continue;
                if (!quiet_edge(candidate, shared))
                    continue;
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
        enum class kind_t { effects, transfer, branch, loop, break_loop, continue_loop, switch_statement, switch_case };
        statement_t(kind_t kind, unsigned block = 0, unsigned target = 0) : kind(kind), block(block), target(target) {}
        kind_t kind;
        unsigned block = 0, target = 0;
        bool negate = false;
        std::vector<term_t> condition;
        std::vector<value_ref_t> selector;
        std::vector<uint32_t> case_values;
        std::vector<statement_t> body, otherwise;
    };
    using kind = statement_t::kind_t;
    struct frame_t {
        unsigned header, follow;
    };
    std::vector<frame_t> stack;
    std::vector<bool> planned(exit), inside(exit);
    unsigned budget = 8 * static_cast<unsigned>(blocks.size()) + 64;

    std::function<bool(unsigned, unsigned, unsigned, std::vector<statement_t> &, bool &)> region;
    region = [&](unsigned node, unsigned stop, unsigned entering, std::vector<statement_t> &out, bool &falls) -> bool {
        for (;;) {
            const bool opening = node == entering;
            entering = exit;
            if (node == stop) {
                falls = true;
                return true;
            }
            if (!budget)
                return false;
            --budget;
            if (!opening && !stack.empty()) {
                if (node == stack.back().header) {
                    out.push_back({kind::continue_loop});
                    falls = false;
                    return true;
                }
                if (node == stack.back().follow) {
                    out.push_back({kind::break_loop});
                    falls = false;
                    return true;
                }
            }


            if (!opening)
                for (const auto &frame : stack)
                    if (node == frame.header)
                        return false;
            if (inside[node])
                return false;

            if (!opening && loops.count(node)) {
                const auto follow = follows.at(node);
                const auto &members = loops.at(node);
                statement_t loop{kind::loop};
                unsigned entry = exit, entry_from = exit;
                if (conditional(node) && quiet_block(node)) {
                    auto test = build_chain(node);
                    bool contained = true;
                    for (const auto &term : test.terms)
                        if (!members.count(term.block))
                            contained = false;
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
                    if (!region(node, exit, node, loop.body, ignored))
                        return false;
                } else {
                    if (entry_from != exit)
                        loop.body.push_back({kind::transfer, entry_from, entry});
                    if (!region(entry, exit, exit, loop.body, ignored))
                        return false;
                }
                stack.pop_back();
                out.push_back(std::move(loop));
                if (follow == exit) {
                    falls = false;
                    return true;
                }
                node = follow;
                continue;
            }

            const auto &block = graph.blocks[node];
            out.push_back({kind::effects, node});
            planned[node] = true;
            if (block.successors.empty()) {
                falls = false;
                return true;
            }
            if (block.successors.size() == 1 ||
                (block.successors.size() == 2 && block.successors[0] == block.successors[1])) {
                if (!block.effects.empty()) {
                    const auto &effect = block.effects.back();
                    if (effect.kind == effect_kind::branch && binary.code[effect.instruction].opcode == OP_BF &&
                        !effect.arguments.empty()) {
                        statement_t branch{kind::branch, node};
                        branch.condition.push_back({node, false, false});
                        out.push_back(std::move(branch));
                    }
                }
                const auto next = block.successors[0];
                out.push_back({kind::transfer, node, next});
                node = next;
                continue;
            }
            if (!conditional(node))
                return false;
            auto test = build_chain(node);
            for (unsigned term = 1; term < test.terms.size(); ++term) {
                out.push_back({kind::effects, test.terms[term].block});
                planned[test.terms[term].block] = true;
            }
            statement_t branch{kind::branch};
            branch.condition = test.terms;
            const auto follow = join[test.last];
            if (test.positive_from != exit)
                branch.body.push_back({kind::transfer, test.positive_from, test.positive});
            if (test.negative_from != exit)
                branch.otherwise.push_back({kind::transfer, test.negative_from, test.negative});
            for (const auto &term : test.terms)
                inside[term.block] = true;
            bool holds = false, fails = false;
            if (!region(test.positive, follow, exit, branch.body, holds))
                return false;
            if (!region(test.negative, follow, exit, branch.otherwise, fails))
                return false;
            for (const auto &term : test.terms)
                inside[term.block] = false;
            out.push_back(std::move(branch));
            if (!holds && !fails) {
                falls = false;
                return true;
            }
            node = follow;
        }
    };

    std::vector<statement_t> plan;
    bool complete = false;
    for (unsigned attempt = 0; attempt < 2 && !complete; ++attempt) {
        auto chosen = choose_follows(attempt == 0);
        if (attempt && chosen == follows)
            break;
        follows = std::move(chosen);
        plan.clear();
        stack.clear();
        bypassed.clear();
        planned.assign(exit, false);
        inside.assign(exit, false);
        budget = 8 * static_cast<unsigned>(blocks.size()) + 64;
        bool falls = false;
        if (!region(0, exit, exit, plan, falls))
            continue;
        complete = true;
        for (unsigned b = 0; b < exit; ++b)
            if (reachable[b] && !planned[b] && !bypassed.count(b))
                complete = false;
    }
    if (!complete)
        return false;


    std::function<bool(std::vector<statement_t> &)> reduce;
    reduce = [&](std::vector<statement_t> &statements) {
        bool silent = true;
        for (auto &statement : statements)
            switch (statement.kind) {
            case kind::effects:
                silent = silent && quiet_block(statement.block);
                break;
            case kind::transfer:
                silent = silent && quiet_edge(statement.block, statement.target);
                break;
            case kind::branch:
                if (reduce(statement.body))
                    statement.body.clear();
                if (reduce(statement.otherwise))
                    statement.otherwise.clear();
                silent = false;
                break;
            case kind::loop:
                reduce(statement.body);
                silent = false;
                break;
            default:
                silent = false;
                break;
            }
        return silent;
    };
    reduce(plan);


    auto closed = [&](const std::vector<statement_t> &statements) {
        if (statements.empty())
            return false;
        const auto &last = statements.back();
        if (last.kind == kind::break_loop || last.kind == kind::continue_loop)
            return true;
        return last.kind == kind::effects && graph.blocks[last.block].successors.empty();
    };


    std::function<void(std::vector<statement_t> &)> share_terminal;
    share_terminal = [&](auto &statements) {
        for (auto &s : statements) {
            share_terminal(s.body);
            share_terminal(s.otherwise);
        }
        for (unsigned i = 0; i < statements.size(); ++i) {
            auto &s = statements[i];
            if (s.kind != kind::branch || s.otherwise.empty())
                continue;
            const auto terminal = s.otherwise.back();
            if (terminal.kind != kind::effects || !graph.blocks[terminal.block].successors.empty())
                continue;
            bool prefix_quiet = true;
            for (unsigned j = 0; j + 1 < s.otherwise.size(); ++j) {
                const auto &e = s.otherwise[j];
                if (e.kind == kind::transfer  ? !quiet_edge(e.block, e.target)
                    : e.kind == kind::effects ? !quiet_block(e.block)
                                              : true)
                    prefix_quiet = false;
            }
            if (!prefix_quiet)
                continue;
            std::function<bool(std::vector<statement_t> &)> remove_tail;
            remove_tail = [&](auto &arm) {
                if (arm.empty())
                    return false;
                if (arm.back().kind == kind::effects && arm.back().block == terminal.block) {
                    arm.pop_back();
                    return true;
                }
                if (arm.back().kind != kind::branch)
                    return false;
                bool a = remove_tail(arm.back().body), b = remove_tail(arm.back().otherwise);
                return a || b;
            };
            if (!remove_tail(s.body))
                continue;
            s.otherwise.clear();
            statements.insert(statements.begin() + i + 1, terminal);
            ++i;
        }
    };
    share_terminal(plan);
    std::set<unsigned> preserved_return_branches;
    std::function<std::pair<unsigned, unsigned>(const std::vector<statement_t> &)> physical_bounds;
    physical_bounds = [&](const auto &statements) {
        std::pair<unsigned, unsigned> bounds{UINT32_MAX, 0};
        for (const auto &s : statements) {
            if (s.kind == kind::effects || s.kind == kind::transfer) {
                bounds.first = std::min(bounds.first, graph.blocks[s.block].begin);
                bounds.second = std::max(bounds.second, graph.blocks[s.block].end);
            }
            for (const auto &term : s.condition) {
                bounds.first = std::min(bounds.first, graph.blocks[term.block].begin);
                bounds.second = std::max(bounds.second, graph.blocks[term.block].end);
            }
            for (const auto *arm : {&s.body, &s.otherwise}) {
                auto child = physical_bounds(*arm);
                bounds.first = std::min(bounds.first, child.first);
                bounds.second = std::max(bounds.second, child.second);
            }
        }
        return bounds;
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
                    if (!arm->empty() && arm->back().kind == kind::continue_loop) {
                        arm->pop_back();
                        again = true;
                    }
                if (again)
                    continue;
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
                        for (auto &statement : rest)
                            statements.push_back(std::move(statement));
                        again = true;
                        continue;
                    }
                }
            }
            for (unsigned i = 0; i < statements.size() && !again; ++i) {
                if (statements[i].kind != kind::branch)
                    continue;
                auto &test = statements[i];
                const bool leaves = closed(test.body), otherwise_leaves = closed(test.otherwise);
                if (!test.condition.empty() && preserved_return_branches.count(test.condition.front().block))
                    continue;
                if (leaves && otherwise_leaves && types.result == "void" && binary.code.back().opcode == OP_RET &&
                    binary.code.back().value == binary.parameters_size) {
                    auto yes = physical_bounds(test.body), no = physical_bounds(test.otherwise);
                    auto *terminal = yes.second == binary.code.size()  ? &test.body
                                     : no.second == binary.code.size() ? &test.otherwise
                                                                       : nullptr;
                    if (terminal && (yes.second == binary.code.size()) != (no.second == binary.code.size())) {
                        const auto cutoff = terminal == &test.body ? no.second : yes.second;
                        unsigned split = 0;
                        for (; split < terminal->size(); ++split) {
                            std::vector<statement_t> one{(*terminal)[split]};
                            if (physical_bounds(one).first >= cutoff)
                                break;
                        }
                        if (split < terminal->size()) {
                            std::vector<statement_t> rest(std::make_move_iterator(terminal->begin() + split),
                                                          std::make_move_iterator(terminal->end()));
                            terminal->erase(terminal->begin() + split, terminal->end());
                            if (!test.condition.empty())
                                preserved_return_branches.insert(test.condition.front().block);
                            statements.insert(statements.begin() + i + 1,
                                              std::make_move_iterator(rest.begin()),
                                              std::make_move_iterator(rest.end()));
                            again = true;
                            continue;
                        }
                    }
                }
                if (leaves == otherwise_leaves)
                    continue;
                auto &staying = leaves ? test.otherwise : test.body;
                if (staying.empty())
                    continue;
                auto rest = std::move(staying);
                staying.clear();
                statements.insert(statements.begin() + i + 1,
                                  std::make_move_iterator(rest.begin()),
                                  std::make_move_iterator(rest.end()));
                again = true;
            }
        }
        for (unsigned i = 0; i < statements.size(); ++i) {
            auto &statement = statements[i];
            const bool last = i + 1 == statements.size();
            if (statement.kind == kind::branch) {
                flatten(statement.body, tail && last);
                flatten(statement.otherwise, tail && last);
            } else if (statement.kind == kind::loop)
                flatten(statement.body, true);
            else if (statement.kind == kind::switch_statement || statement.kind == kind::switch_case)
                flatten(statement.body, false);
        }
    };

    struct dispatch_t {
        std::map<unsigned, uint32_t> tests;
        unsigned default_target;
    };
    std::map<unsigned, dispatch_t> dispatches;
    for (unsigned pc = 0; pc + 4 < binary.code.size(); ++pc) {
        dispatch_t dispatch;
        unsigned cursor = pc;
        while (cursor + 4 < binary.code.size()) {
            const auto &a = binary.code[cursor], &b = binary.code[cursor + 1], &eq = binary.code[cursor + 2],
                       &bf = binary.code[cursor + 3], &br = binary.code[cursor + 4];
            if (a.opcode != OP_PSH || a.argument != OP_ARG_SPR || static_cast<int16_t>(a.value) != -4 ||
                b.opcode != OP_PSH || b.argument != OP_ARG_NUM || eq.opcode != OP_EQ || bf.opcode != OP_BF ||
                static_cast<int16_t>(bf.value) != 4 || br.opcode != OP_BRA)
                break;
            dispatch.tests.emplace(cursor + 3, b.value);
            cursor += 5;
        }
        if (dispatch.tests.empty() || cursor >= binary.code.size())
            continue;
        const auto &last = binary.code[cursor];
        dispatch.default_target =
            last.opcode == OP_BRA ? last.offset + last.size() + static_cast<int16_t>(last.value) : last.offset;
        dispatches.emplace(pc + 3, std::move(dispatch));
        pc = cursor;
    }
    std::function<bool(const std::vector<statement_t> &)> has_loop_escape = [&](const auto &statements) {
        for (const auto &s : statements) {
            if (s.kind == kind::break_loop || s.kind == kind::continue_loop)
                return true;
            if (s.kind != kind::loop && (has_loop_escape(s.body) || has_loop_escape(s.otherwise)))
                return true;
        }
        return false;
    };
    std::set<unsigned> live_effect_values;
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b])
            for (const auto &e : graph.blocks[b].effects)
                if (e.kind == effect_kind::value)
                    live_effect_values.insert(e.value);
    std::function<void(std::vector<statement_t> &)> recover_switch = [&](auto &statements) {
        for (auto &statement : statements) {
            recover_switch(statement.body);
            recover_switch(statement.otherwise);
            if (statement.kind != kind::branch || statement.condition.empty())
                continue;
            const auto first_pc = graph.blocks[statement.condition.front().block].effects.back().instruction;
            auto match = dispatches.find(first_pc);
            if (match == dispatches.end())
                continue;
            const auto &dispatch = match->second;
            std::vector<std::pair<statement_t *, std::vector<uint32_t>>> arms;
            std::set<unsigned> consumed;
            statement_t *current = &statement;
            bool valid = true;
            std::vector<value_ref_t> selector;
            for (;;) {
                std::vector<uint32_t> labels;
                for (const auto &term : current->condition) {
                    const auto &effect = graph.blocks[term.block].effects.back();
                    auto test = dispatch.tests.find(effect.instruction);
                    if (test == dispatch.tests.end() || term.negate || (labels.size() && term.conjunction) ||
                        effect.arguments.empty() || effect.arguments[0].size() != 1) {
                        valid = false;
                        break;
                    }
                    const auto &eq = graph.values[effect.arguments[0][0].value];
                    if (eq.kind != value_kind::operation || eq.arguments.size() != 2 ||
                        binary.code[eq.instruction].opcode != OP_EQ) {
                        valid = false;
                        break;
                    }
                    if (selector.empty())
                        selector = eq.arguments[0];

                    if (group(selector, "num") != group(eq.arguments[0], "num")) {
                        valid = false;
                        break;
                    }
                    labels.push_back(test->second);
                    consumed.insert(effect.instruction);
                }
                if (!valid || has_loop_escape(current->body)) {
                    valid = false;
                    break;
                }
                arms.emplace_back(current, std::move(labels));
                if (consumed.size() == dispatch.tests.size())
                    break;
                statement_t *next = nullptr;
                for (auto &s : current->otherwise) {
                    if (s.kind == kind::branch && !next)
                        next = &s;
                    else if (s.kind == kind::effects && quiet_block(s.block)) {
                    } else if (s.kind == kind::transfer && quiet_edge(s.block, s.target)) {
                    } else {
                        valid = false;
                        break;
                    }
                }
                if (!valid || !next) {
                    valid = false;
                    break;
                }
                current = next;
            }
            if (!valid || consumed.size() != dispatch.tests.size() || has_loop_escape(current->otherwise))
                continue;
            statement_t replacement{kind::switch_statement};
            replacement.selector = std::move(selector);
            for (auto &[branch, labels] : arms) {
                statement_t arm{kind::switch_case};
                arm.case_values = std::move(labels);
                arm.body = std::move(branch->body);
                replacement.body.push_back(std::move(arm));
            }
            const auto follow = join[statement.condition.front().block];
            const auto follow_offset = follow < exit ? binary.code[graph.blocks[follow].begin].offset : UINT32_MAX;
            if (!current->otherwise.empty() || dispatch.default_target != follow_offset) {
                statement_t arm{kind::switch_case};
                arm.body = std::move(current->otherwise);
                replacement.body.push_back(std::move(arm));
            }

            if (replacement.selector.size() == 1 && !replacement.selector[0].offset) {
                const auto id = replacement.selector[0].value;
                const auto &value = graph.values[id];
                bool exclusive = stored(id) && value.kind != value_kind::merge && !array_values.count(id) &&
                                 value.instruction + 4 == first_pc;
                for (unsigned user = 0; user < graph.values.size() && exclusive; ++user) {
                    const auto &v = graph.values[user];
                    if (!used[user] && !live_effect_values.count(user))
                        continue;
                    bool reads = false;
                    for (const auto &args : v.arguments)
                        for (auto ref : args)
                            if (ref.value == id)
                                reads = true;
                    if (v.address && ((v.address->receiver && v.address->receiver->value == id) ||
                                      (v.address->index && v.address->index->value == id)))
                        reads = true;
                    if (reads && (v.kind != value_kind::operation || binary.code[v.instruction].opcode != OP_EQ ||
                                  !dispatch.tests.count(v.instruction + 1)))
                        exclusive = false;
                }
                for (const auto &block : graph.blocks)
                    for (const auto &e : block.effects) {
                        for (const auto &args : e.arguments)
                            for (auto ref : args)
                                if (ref.value == id)
                                    exclusive = false;
                        if (e.address && ((e.address->receiver && e.address->receiver->value == id) ||
                                          (e.address->index && e.address->index->value == id)))
                            exclusive = false;
                    }
                if (exclusive) {
                    adjacent_expressions[id] = true;
                    if (value.kind == value_kind::operation && binary.code[value.instruction].opcode == OP_PSH)
                        for (const auto &args : value.arguments)
                            for (auto ref : args)
                                if (stored(ref.value))
                                    adjacent_expressions[ref.value] = false;
                }
            }
            statement = std::move(replacement);
        }
    };
    recover_switch(plan);
    std::fill(required_slots.begin(), required_slots.end(), false);
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && !adjacent_expressions[id])
            required_slots[slots[id]] = true;
    flatten(plan, false);
    std::function<void(std::vector<statement_t> &)> recover_late_loop_tests;
    recover_late_loop_tests = [&](auto &statements) {
        for (unsigned si = 0; si < statements.size(); ++si) {
            auto &s = statements[si];
            recover_late_loop_tests(s.body);
            recover_late_loop_tests(s.otherwise);
            if (s.kind != kind::loop || !s.condition.empty())
                continue;
            unsigned test = 0;
            auto silent = [&](const statement_t &e) {
                return e.kind == kind::effects    ? quiet_block(e.block)
                       : e.kind == kind::transfer ? quiet_edge(e.block, e.target)
                                                  : false;
            };
            while (test < s.body.size() && silent(s.body[test]))
                ++test;
            if (test == s.body.size() || s.body[test].kind != kind::branch)
                continue;
            auto &branch = s.body[test];
            if (branch.condition.empty() || !loops.count(branch.condition.front().block) || !branch.otherwise.empty())
                continue;
            unsigned after = test + 1;
            while (after < s.body.size() && silent(s.body[after]))
                ++after;
            if (after + 1 != s.body.size() || s.body[after].kind != kind::break_loop)
                continue;
            if (branch.body.empty() || branch.body.back().kind != kind::continue_loop)
                continue;
            s.condition = branch.condition;
            s.negate = branch.negate;
            auto body = std::move(branch.body);
            body.pop_back();
            std::vector<statement_t> prefix(s.body.begin(), s.body.begin() + test);
            s.body = std::move(body);
            statements.insert(statements.begin() + si, prefix.begin(), prefix.end());
            si += prefix.size();
        }
    };


    auto wrapped = [](const std::string &text) {
        if (text.size() < 2 || text.front() != '(')
            return false;
        unsigned depth = 0;
        for (unsigned i = 0; i < text.size(); ++i) {
            if (text[i] == '(')
                ++depth;
            else if (text[i] == ')' && !--depth)
                return i + 1 == text.size();
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
            if (condition[i].negate)
                term = negated(std::move(term));
            if (!i) {
                text = std::move(term);
                continue;
            }


            if (i > 1 && condition[i].conjunction != conjunction)
                text = '(' + text + ')';
            conjunction = condition[i].conjunction;
            text += conjunction ? " && " : " || ";
            text += term;
        }
        return negate ? negated(std::move(text)) : text;
    };


    auto quiet_statement = [&](const statement_t &statement) {
        if (statement.kind == kind::effects)
            return quiet_block(statement.block);
        return statement.kind == kind::transfer && quiet_edge(statement.block, statement.target);
    };
    auto chained = [&](const std::vector<statement_t> &alternative) -> const statement_t * {
        if (alternative.empty() || alternative.back().kind != kind::branch)
            return nullptr;
        for (unsigned i = 0; i + 1 < alternative.size(); ++i)
            if (!quiet_statement(alternative[i]))
                return nullptr;
        return &alternative.back();
    };


    std::set<unsigned> empty_else_headers;
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b] || graph.blocks[b].effects.empty())
            continue;
        const auto &last = graph.blocks[b].effects.back();
        const auto &ins = binary.code[last.instruction];
        if (ins.opcode != OP_BRA || static_cast<int16_t>(ins.value) != 0 || graph.blocks[b].successors.size() != 1)
            continue;
        auto target = graph.blocks[b].successors.front();
        unsigned owner = exit, depth = 0;
        for (auto candidate : dominators[b]) {
            if (!conditional(candidate))
                continue;
            unsigned no = 0, yes = 0;
            arms(candidate, no, yes);
            if (no != target || yes == target)
                continue;
            if (owner == exit || dominators[candidate].size() > depth) {
                owner = candidate;
                depth = dominators[candidate].size();
            }
        }
        if (owner != exit)
            empty_else_headers.insert(owner);
    }


    std::set<unsigned> encountered_slots;
    for (const auto &effect : graph.blocks[0].effects) {
        if (effect.kind != effect_kind::value)
            continue;
        const auto id = effect.value;
        if (!stored(id) || !used[id] || array_values.count(id) ||
            (!adjacent_expressions.empty() && adjacent_expressions[id]))
            continue;
        const auto slot = slots[id];
        if (auto init = declaration_initializers.find(slot);
            init != declaration_initializers.end() && init->second == id)
            continue;
        if (!encountered_slots.insert(slot).second)
            continue;
        const auto &v = graph.values[id];
        if (binary.code[v.instruction].opcode == OP_POP && binary.code[v.instruction].argument == OP_ARG_SPR)
            continue;
        const auto &facts = source.stack.functions[index];
        if (v.instruction + 1 >= facts.heights.size() || !facts.heights[v.instruction + 1])
            continue;
        const auto &next = binary.code[v.instruction + 1];
        if (next.opcode == OP_POP && next.argument == OP_ARG_SPR)
            continue;
        const int origin = *facts.heights[v.instruction + 1] - static_cast<int>(v.width);
        if (origin < static_cast<int>(binary.parameters_size))
            continue;
        bool reserved = false;
        for (unsigned pc = 0; pc < v.instruction; ++pc) {
            const auto &ins = binary.code[pc];
            if (!facts.heights[pc])
                continue;
            if (ins.opcode == OP_SPA && static_cast<int16_t>(ins.value) > 0) {
                const int begin = *facts.heights[pc], end = begin + static_cast<int16_t>(ins.value);
                if (begin < origin + static_cast<int>(v.width) && end > origin)
                    reserved = true;
            }
        }
        if (!reserved)
            initialized_slots.emplace(slot, id);
    }

    const bool root_entry = std::any_of(plan.begin(), plan.end(), [](const auto &statement) {
        return statement.kind == kind::effects && statement.block == 0;
    });
    if (!root_entry)
        initialized_slots.clear();


    using lexical_region = const std::vector<statement_t> *;
    std::map<lexical_region, lexical_region> lexical_parent;
    std::map<unsigned, std::set<lexical_region>> lexical_blocks;
    std::function<void(const std::vector<statement_t> &, lexical_region)> locate_regions = [&](const auto &statements,
                                                                                               lexical_region parent) {
        const auto here = &statements;
        lexical_parent[here] = parent;
        for (const auto &s : statements) {
            if (s.kind == kind::effects)
                lexical_blocks[s.block].insert(here);
            for (const auto &term : s.condition)
                lexical_blocks[term.block].insert(here);
            if (!s.body.empty())
                locate_regions(s.body, here);
            if (!s.otherwise.empty())
                locate_regions(s.otherwise, here);
        }
    };
    locate_regions(plan, nullptr);
    auto contains_region = [&](lexical_region outer, lexical_region inner) {
        for (auto p = inner; p; p = lexical_parent[p])
            if (p == outer)
                return true;
        return false;
    };
    std::map<unsigned, std::vector<std::pair<unsigned, unsigned>>> slot_events;
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b])
            for (const auto &e : graph.blocks[b].effects) {
                auto record = [&](value_ref_t ref) {
                    if (stored(ref.value) && !adjacent_expressions[ref.value])
                        slot_events[slots[ref.value]].emplace_back(b, e.instruction);
                };
                if (e.kind == effect_kind::value) {
                    if (!used[e.value] && !effectful(e.value))
                        continue;
                    record({e.value, 0});
                    const auto &v = graph.values[e.value];
                    for (const auto &args : v.arguments)
                        for (auto ref : args)
                            record(ref);
                    if (v.address) {
                        if (v.address->receiver)
                            record(*v.address->receiver);
                        if (v.address->index)
                            record(*v.address->index);
                    }
                } else {
                    const auto count = e.arguments.size() - (e.kind == effect_kind::callback ? 1 : 0);
                    for (unsigned i = 0; i < count; ++i)
                        for (auto ref : e.arguments[i])
                            record(ref);
                    if (e.address) {
                        if (e.address->receiver)
                            record(*e.address->receiver);
                        if (e.address->index)
                            record(*e.address->index);
                    }
                }
            }

    for (unsigned from = 0; from < graph.blocks.size(); ++from)
        if (reachable[from])
            for (auto to : graph.blocks[from].successors) {
                const auto &origin = graph.blocks[from], &destination = graph.blocks[to];
                for (unsigned word = 0; word < destination.incoming.size(); ++word) {
                    const auto target = destination.incoming[word], input = origin.outgoing.at(word);
                    if (!used[target.value] || graph.values[target.value].kind != value_kind::merge ||
                        quiet_transfer(target, input) || retained_initializer(from, word, target, input))
                        continue;
                    if (stored(target.value))
                        slot_events[slots[target.value]].emplace_back(from, origin.end - 1);
                    if (stored(input.value) && !adjacent_expressions[input.value])
                        slot_events[slots[input.value]].emplace_back(from, origin.end - 1);
                }
            }
    struct scoped_constant_t {
        unsigned block, id, pc;
        lexical_region region;
    };
    std::map<unsigned, std::vector<scoped_constant_t>> scoped_constants;
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b] && lexical_blocks[b].size() == 1) {
            const auto region = *lexical_blocks[b].begin();
            for (const auto &e : graph.blocks[b].effects) {
                if (e.kind != effect_kind::value || !stored(e.value) || adjacent_expressions[e.value] ||
                    array_values.count(e.value) || postfix_locals.count(e.value))
                    continue;
                const auto id = e.value, slot = slots[id];
                if (initialized_slots.count(slot) || declaration_initializers.count(slot))
                    continue;
                const auto &v = graph.values[id];
                const auto &facts = source.stack.functions[index];
                if (binary.code[v.instruction].opcode == OP_POP && binary.code[v.instruction].argument == OP_ARG_SPR)
                    continue;
                if (v.instruction + 1 >= facts.heights.size() || !facts.heights[v.instruction + 1])
                    continue;
                const auto &next = binary.code[v.instruction + 1];
                if (next.opcode == OP_POP && next.argument == OP_ARG_SPR)
                    continue;
                const int origin = *facts.heights[v.instruction + 1] - static_cast<int>(v.width);
                if (origin < static_cast<int>(binary.parameters_size))
                    continue;
                bool reserved = false;
                for (unsigned pc = 0; pc < v.instruction; ++pc) {
                    const auto &ins = binary.code[pc];
                    if (!facts.heights[pc])
                        continue;
                    if (ins.opcode == OP_SPA && static_cast<int16_t>(ins.value) > 0) {
                        bool dominates = false;
                        for (unsigned prior = 0; prior < graph.blocks.size(); ++prior)
                            if (pc >= graph.blocks[prior].begin && pc < graph.blocks[prior].end &&
                                dominators[b].count(prior))
                                dominates = true;
                        if (!dominates)
                            continue;
                        const int begin = *facts.heights[pc], end = begin + static_cast<int16_t>(ins.value);
                        bool released = false;
                        for (unsigned at = pc + 1; at < v.instruction; ++at)
                            if (facts.heights[at] && *facts.heights[at] <= std::max(begin, origin)) {
                                for (unsigned block = 0; block < graph.blocks.size(); ++block)
                                    if (at >= graph.blocks[block].begin && at < graph.blocks[block].end &&
                                        dominators[b].count(block))
                                        released = true;
                            }
                        if (!released && begin < origin + static_cast<int>(v.width) && end > origin)
                            reserved = true;
                    }
                }
                if (!reserved)
                    scoped_constants[slot].push_back({b, id, v.instruction, region});
                bool valid = true;
                for (const auto &[user, pc] : slot_events[slot]) {
                    if (!dominators[user].count(b) || (user == b && pc < v.instruction))
                        valid = false;
                    for (auto inner : lexical_blocks[user])
                        if (!contains_region(region, inner))
                            valid = false;
                }
                for (unsigned pc = 0; pc < v.instruction; ++pc) {
                    const auto &ins = binary.code[pc];
                    if (!facts.heights[pc])
                        continue;
                    if (ins.opcode == OP_SPA && static_cast<int16_t>(ins.value) > 0) {
                        bool dominates = false;
                        for (unsigned prior = 0; prior < graph.blocks.size(); ++prior)
                            if (pc >= graph.blocks[prior].begin && pc < graph.blocks[prior].end &&
                                dominators[b].count(prior))
                                dominates = true;
                        if (!dominates)
                            continue;
                        const int begin = *facts.heights[pc], end = begin + static_cast<int16_t>(ins.value);
                        bool released = false;
                        for (unsigned at = pc + 1; at < v.instruction; ++at)
                            if (facts.heights[at] && *facts.heights[at] <= std::max(begin, origin)) {
                                for (unsigned block = 0; block < graph.blocks.size(); ++block)
                                    if (at >= graph.blocks[block].begin && at < graph.blocks[block].end &&
                                        dominators[b].count(block))
                                        released = true;
                            }
                        if (!released && begin < origin + static_cast<int>(v.width) && end > origin)
                            valid = false;
                    }
                }
                if (valid)
                    initialized_slots.emplace(slot, id);
            }
        }

    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b] && lexical_blocks[b].size() == 1) {
            const auto region = *lexical_blocks[b].begin();
            const auto &block = graph.blocks[b];
            const auto &facts = source.stack.functions[index];
            int end_height;
            if (block.successors.empty()) {
                if (block.end <= block.begin || binary.code[block.end - 1].opcode != OP_RET)
                    continue;
                unsigned epilogue = block.end - 1;
                while (epilogue > block.begin && binary.code[epilogue - 1].opcode == OP_PSH &&
                       binary.code[epilogue - 1].argument == OP_ARG_SPR)
                    --epilogue;
                if (epilogue == block.begin || binary.code[epilogue - 1].opcode != OP_SPA ||
                    static_cast<int16_t>(binary.code[epilogue - 1].value) >= 0 || !facts.heights[epilogue - 1])
                    continue;
                end_height = *facts.heights[epilogue - 1];
            } else {
                const auto next = graph.blocks[block.successors.front()].begin;
                if (next >= facts.heights.size() || !facts.heights[next])
                    continue;
                end_height = *facts.heights[next];
            }
            for (unsigned id = 0; id < graph.values.size(); ++id) {
                const auto &v = graph.values[id];
                const auto pc = v.instruction;
                if ((v.kind != value_kind::constant && v.kind != value_kind::string &&
                     v.kind != value_kind::class_value) ||
                    v.width != 4 || pc < block.begin || pc >= block.end || !facts.heights[pc])
                    continue;
                if (binary.code[pc].opcode != OP_PSH ||
                    (binary.code[pc].argument != OP_ARG_NUM && binary.code[pc].argument != OP_ARG_STR &&
                     binary.code[pc].argument != OP_ARG_CLV))
                    continue;
                bool direct_use = false;
                for (unsigned user = 0; user < graph.blocks.size(); ++user)
                    if (reachable[user])
                        for (const auto &e : graph.blocks[user].effects) {
                            auto check = [&](const auto &args) {
                                for (const auto &refs : args)
                                    for (auto ref : refs)
                                        if (ref.value == id)
                                            direct_use = true;
                            };
                            auto address = [&](const auto &a) {
                                if (a &&
                                    ((a->index && a->index->value == id) || (a->receiver && a->receiver->value == id)))
                                    direct_use = true;
                            };
                            if (e.kind == effect_kind::value) {
                                const auto &consumer = graph.values[e.value];
                                if (consumer.kind != value_kind::merge) {
                                    check(consumer.arguments);
                                    address(consumer.address);
                                }
                            } else {
                                check(e.arguments);
                                address(e.address);
                            }
                        }
                if (direct_use) {
                    const int offset = *facts.heights[pc];
                    if (offset < static_cast<int>(binary.parameters_size))
                        continue;
                    bool survives = offset + 4 <= end_height &&
                                    static_cast<unsigned>(offset / 4) < block.outgoing.size() &&
                                    block.outgoing[offset / 4] == value_ref_t{id, 0};
                    if (!survives && v.local_read_home && !v.ambiguous_read_home &&
                        *v.local_read_home == static_cast<unsigned>(offset)) {
                        for (unsigned at = pc + 1; at < block.end; ++at) {
                            if (!facts.heights[at] || *facts.heights[at] < offset + 4)
                                break;
                            const auto &op = binary.code[at];
                            if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) < 0 &&
                                *facts.heights[at] + static_cast<int16_t>(op.value) <= offset) {
                                survives = true;
                                break;
                            }
                        }
                    }
                    if (!survives)
                        continue;
                    bool copied = false, alive = true;
                    for (unsigned at = pc + 1; at < binary.code.size(); ++at) {
                        if (!facts.heights[at] || *facts.heights[at] < offset + 4) {
                            alive = false;
                            break;
                        }
                        const auto &op = binary.code[at];
                        if (op.opcode == OP_PSH && op.argument == OP_ARG_SPR && op.data_size() == 4 &&
                            *facts.heights[at] + static_cast<int16_t>(op.value) == offset) {
                            copied = true;
                            break;
                        }
                        if (op.opcode == OP_POP && op.argument == OP_ARG_SPR &&
                            *facts.heights[at] + static_cast<int16_t>(op.value) == offset) {
                            alive = false;
                            break;
                        }
                    }
                    bool already = false;
                    for (const auto &[slot, initial] : declaration_initializers)
                        if (initial == id)
                            already = true;
                    if (copied && alive && !already && !recovered_local_names.count(id)) {
                        auto owner = [&]() -> std::optional<unsigned> {
                            std::set<unsigned> candidates;
                            for (unsigned user = 0; user < graph.blocks.size(); ++user)
                                if (reachable[user] && dominators[user].count(b)) {
                                    auto collect = [&](const auto &refs, int height) {
                                        if (offset + 4 <= height && static_cast<unsigned>(offset / 4) < refs.size()) {
                                            const auto ref = refs[offset / 4];
                                            if (!ref.offset && stored(ref.value))
                                                candidates.insert(slots[ref.value]);
                                        }
                                    };
                                    const auto &user_block = graph.blocks[user];
                                    if (facts.heights[user_block.begin])
                                        collect(user_block.incoming, *facts.heights[user_block.begin]);
                                    if (!user_block.successors.empty()) {
                                        const auto next = graph.blocks[user_block.successors.front()].begin;
                                        if (facts.heights[next])
                                            collect(user_block.outgoing, *facts.heights[next]);
                                    }
                                }
                            for (unsigned value = 0; value < graph.values.size(); ++value)
                                if (stored(value)) {
                                    const auto &stored_value = graph.values[value];
                                    if (stored_value.width != 4 || stored_value.instruction >= binary.code.size())
                                        continue;
                                    auto at = stored_value.instruction;
                                    if (binary.code[at].opcode != OP_POP || binary.code[at].argument != OP_ARG_SPR)
                                        ++at;
                                    if (at >= binary.code.size() || at <= pc || !facts.heights[at])
                                        continue;
                                    const auto &store = binary.code[at];
                                    if (store.opcode != OP_POP || store.argument != OP_ARG_SPR ||
                                        store.data_size() != 4 ||
                                        *facts.heights[at] + static_cast<int16_t>(store.value) != offset)
                                        continue;
                                    for (unsigned user = 0; user < graph.blocks.size(); ++user)
                                        if (at >= graph.blocks[user].begin && at < graph.blocks[user].end &&
                                            dominators[user].count(b))
                                            candidates.insert(slots[value]);
                                }
                            if (candidates.size() != 1)
                                return std::nullopt;
                            const auto slot = *candidates.begin();
                            if (!required_slots[slot] || initialized_slots.count(slot) ||
                                declaration_initializers.count(slot) || parameter_slots.count(slot) ||
                                graph.values[slot_values[slot]].width != 4 ||
                                value_type(slot_values[slot]) != value_type(id))
                                return std::nullopt;

                            bool valid = true;
                            for (const auto &[user, at] : slot_events[slot]) {
                                if (!dominators[user].count(b) || (user == b && at < pc))
                                    valid = false;
                                for (auto inner : lexical_blocks[user])
                                    if (!contains_region(region, inner))
                                        valid = false;
                            }
                            if (!valid)
                                return std::nullopt;
                            return slot;
                        }();
                        const auto local = owner ? name(slot_values[*owner]) : "retained_value" + std::to_string(id);
                        if (owner) {
                            initialized_slots.emplace(*owner, id);
                            declaration_initializers.emplace(*owner, id);
                        }
                        recovered_local_names.emplace(id, std::make_pair(local, value_type(id)));
                        recovered_local_operations[pc].push_back(
                            value_type(id) + " " + local + " = " +
                            (v.kind == value_kind::class_value
                                 ? value_type(id) + "(" + source.text(v.immediate & 0xffff) + ")"
                             : v.kind == value_kind::string ? source.text(v.immediate)
                                                            : number_literal(v.immediate)) +
                            ";");
                    }
                    continue;
                }
                const int offset = *facts.heights[pc];
                bool scoped_storage = offset + 4 <= end_height;
                for (unsigned at = block.end; !scoped_storage && at > pc + 1;) {
                    --at;
                    const auto &op = binary.code[at];
                    if (op.opcode == OP_BRA)
                        continue;
                    if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) >= 0)
                        break;
                    if (facts.heights[at] && *facts.heights[at] >= offset + 4 &&
                        *facts.heights[at] + static_cast<int16_t>(op.value) <= offset)
                        scoped_storage = true;
                }
                if (offset < static_cast<int>(binary.parameters_size) || !scoped_storage ||
                    static_cast<unsigned>(offset / 4) >= block.outgoing.size())
                    continue;
                if (block.outgoing[offset / 4] != value_ref_t{id, 0}) {
                    bool overwritten = false, retained = true;
                    for (unsigned at = pc + 1; at < block.end; ++at) {
                        if (!facts.heights[at] || *facts.heights[at] < offset + 4) {
                            retained = false;
                            break;
                        }
                        const auto &op = binary.code[at];
                        if (op.opcode == OP_POP && op.argument == OP_ARG_SPR && op.data_size() == 4 &&
                            *facts.heights[at] + static_cast<int16_t>(op.value) == offset) {
                            overwritten = true;
                            break;
                        }
                    }
                    if (!retained || !overwritten)
                        continue;
                }
                std::set<unsigned> candidates;
                for (unsigned user = 0; user < graph.blocks.size(); ++user)
                    if (reachable[user] && dominators[user].count(b)) {
                        auto collect = [&](const auto &refs, int height) {
                            if (offset + 4 <= height && static_cast<unsigned>(offset / 4) < refs.size()) {
                                const auto ref = refs[offset / 4];
                                if (!ref.offset && stored(ref.value))
                                    candidates.insert(slots[ref.value]);
                            }
                        };
                        const auto &user_block = graph.blocks[user];
                        if (facts.heights[user_block.begin])
                            collect(user_block.incoming, *facts.heights[user_block.begin]);
                        if (!user_block.successors.empty()) {
                            const auto next = graph.blocks[user_block.successors.front()].begin;
                            if (facts.heights[next])
                                collect(user_block.outgoing, *facts.heights[next]);
                        }
                    }
                for (unsigned value = 0; value < graph.values.size(); ++value)
                    if (stored(value)) {
                        const auto &stored_value = graph.values[value];
                        if (stored_value.width != 4 || stored_value.instruction >= binary.code.size())
                            continue;
                        auto at = stored_value.instruction;
                        if (binary.code[at].opcode != OP_POP || binary.code[at].argument != OP_ARG_SPR)
                            ++at;
                        if (at >= binary.code.size() || at <= pc || !facts.heights[at])
                            continue;
                        const auto &store = binary.code[at];
                        if (store.opcode != OP_POP || store.argument != OP_ARG_SPR || store.data_size() != 4 ||
                            *facts.heights[at] + static_cast<int16_t>(store.value) != offset)
                            continue;
                        for (unsigned user = 0; user < graph.blocks.size(); ++user)
                            if (at >= graph.blocks[user].begin && at < graph.blocks[user].end &&
                                dominators[user].count(b))
                                candidates.insert(slots[value]);
                    }
                if (candidates.size() != 1)
                    continue;
                const auto slot = *candidates.begin();
                if (!required_slots[slot] || initialized_slots.count(slot) || declaration_initializers.count(slot) ||
                    parameter_slots.count(slot) || graph.values[slot_values[slot]].width != 4 ||
                    value_type(slot_values[slot]) != value_type(id))
                    continue;
                scoped_constants[slot].push_back({b, id, pc, region});
                bool valid = true;
                for (const auto &[user, at] : slot_events[slot]) {
                    if (!dominators[user].count(b) || (user == b && at < pc))
                        valid = false;
                    for (auto inner : lexical_blocks[user])
                        if (!contains_region(region, inner))
                            valid = false;
                }
                if (!valid)
                    continue;
                initialized_slots.emplace(slot, id);
                declaration_initializers.emplace(slot, id);
                recovered_local_operations[pc].push_back(
                    value_type(slot_values[slot]) + " " + name(slot_values[slot]) + " = " +
                    (v.kind == value_kind::class_value ? value_type(id) + "(" + source.text(v.immediate & 0xffff) + ")"
                     : v.kind == value_kind::string    ? source.text(v.immediate)
                                                       : number_literal(v.immediate)) +
                    ";");
            }
        }
    {
        std::map<unsigned, std::map<lexical_region, std::vector<std::pair<unsigned, unsigned>>>> regions;
        for (const auto &[slot, events] : slot_events)
            if (!initialized_slots.count(slot) && !declaration_initializers.count(slot) &&
                !parameter_slots.count(slot)) {
                bool valid = true;
                for (const auto &[b, pc] : events) {
                    if (lexical_blocks[b].size() != 1) {
                        valid = false;
                        break;
                    }
                    regions[slot][*lexical_blocks[b].begin()].emplace_back(b, pc);
                }
                if (!valid)
                    regions.erase(slot);
            }
        for (const auto &[slot, groups] : regions) {
            if (groups.size() < 2 || groups.count(&plan))
                continue;
            bool valid = true;
            std::vector<unsigned> initializers;
            for (const auto &[region, events] : groups) {
                for (const auto &[other, unused] : groups)
                    if (region != other && (contains_region(region, other) || contains_region(other, region)))
                        valid = false;
                std::optional<unsigned> initializer;
                for (const auto &[b, pc] : events) {
                    for (const auto &e : graph.blocks[b].effects) {
                        if (e.kind != effect_kind::value || e.instruction != pc || !stored(e.value) ||
                            slots[e.value] != slot || adjacent_expressions[e.value] || array_values.count(e.value))
                            continue;
                        const auto &v = graph.values[e.value];
                        const auto &facts = source.stack.functions[index];
                        if (binary.code[v.instruction].opcode == OP_POP &&
                            binary.code[v.instruction].argument == OP_ARG_SPR)
                            continue;
                        if (v.instruction + 1 >= binary.code.size() || !facts.heights[v.instruction + 1])
                            continue;
                        const auto &next = binary.code[v.instruction + 1];
                        if (next.opcode == OP_POP && next.argument == OP_ARG_SPR)
                            continue;
                        bool dominates = true;
                        for (const auto &[user, at] : events)
                            if (!dominators[user].count(b) || (user == b && at < pc))
                                dominates = false;
                        if (dominates) {
                            if (initializer && *initializer != e.value)
                                valid = false;
                            initializer = e.value;
                        }
                    }
                }
                if (!initializer)
                    valid = false;
                else
                    initializers.push_back(*initializer);
            }
            if (valid) {
                initialized_slots.emplace(slot, initializers.front());
                for (auto id : initializers)
                    lexical_initializer_values.insert(id);
            }
        }
    }
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b] && lexical_blocks[b].size() == 1) {
            const auto region = *lexical_blocks[b].begin();
            const auto &block = graph.blocks[b];
            const auto &facts = source.stack.functions[index];
            for (unsigned pc = block.begin; pc < block.end; ++pc) {
                const auto &op = binary.code[pc];
                if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) <= 0 || !facts.heights[pc])
                    continue;
                bool prior_effect = false;
                for (unsigned before = 0; before < pc; ++before) {
                    const auto &prev = binary.code[before];
                    if (prev.opcode != OP_SPA && prev.opcode != OP_NEG &&
                        !(prev.opcode == OP_PSH && (prev.argument == OP_ARG_NUM || prev.argument == OP_ARG_STR)))
                        prior_effect = true;
                }
                if (!prior_effect)
                    continue;
                const int first = *facts.heights[pc], end = first + static_cast<int16_t>(op.value);
                for (int offset = first; offset < end; offset += 4) {
                    std::set<unsigned> candidates;
                    for (unsigned user = 0; user < graph.blocks.size(); ++user)
                        if (reachable[user] && dominators[user].count(b)) {
                            auto collect = [&](const auto &refs, int height) {
                                if (offset + 4 <= height && static_cast<unsigned>(offset / 4) < refs.size()) {
                                    const auto ref = refs[offset / 4];
                                    if (!ref.offset && stored(ref.value))
                                        candidates.insert(slots[ref.value]);
                                }
                            };
                            const auto &user_block = graph.blocks[user];
                            if (facts.heights[user_block.begin])
                                collect(user_block.incoming, *facts.heights[user_block.begin]);
                            if (!user_block.successors.empty()) {
                                const auto next = graph.blocks[user_block.successors.front()].begin;
                                if (facts.heights[next])
                                    collect(user_block.outgoing, *facts.heights[next]);
                            } else
                                collect(user_block.outgoing, static_cast<int>(user_block.outgoing.size() * 4));
                        }
                    for (unsigned id = 0; id < graph.values.size(); ++id)
                        if (stored(id) && !array_values.count(id)) {
                            const auto &v = graph.values[id];
                            auto at = v.instruction;
                            if (at < pc || at >= binary.code.size())
                                continue;
                            if (binary.code[at].opcode != OP_POP || binary.code[at].argument != OP_ARG_SPR)
                                ++at;
                            if (at >= binary.code.size() || !facts.heights[at])
                                continue;
                            const auto &store = binary.code[at];
                            if (store.opcode != OP_POP || store.argument != OP_ARG_SPR ||
                                store.data_size() != v.width ||
                                *facts.heights[at] + static_cast<int16_t>(store.value) != offset)
                                continue;
                            bool dominated = false;
                            for (unsigned user = 0; user < graph.blocks.size(); ++user)
                                if (at >= graph.blocks[user].begin && at < graph.blocks[user].end &&
                                    dominators[user].count(b))
                                    dominated = true;
                            if (dominated)
                                candidates.insert(slots[id]);
                        }
                    if (candidates.size() != 1)
                        continue;
                    const auto slot = *candidates.begin();
                    if (!required_slots[slot] || initialized_slots.count(slot) ||
                        declaration_initializers.count(slot) || parameter_slots.count(slot) ||
                        offset + static_cast<int>(graph.values[slot_values[slot]].width) > end ||
                        array_values.count(slot_values[slot]))
                        continue;
                    scoped_constants[slot].push_back({b, UINT32_MAX, pc, region});
                    bool valid = offset >= static_cast<int>(binary.parameters_size);
                    for (const auto &[user, at] : slot_events[slot]) {
                        if (!dominators[user].count(b) || (user == b && at < pc))
                            valid = false;
                        for (auto inner : lexical_blocks[user])
                            if (!contains_region(region, inner))
                                valid = false;
                    }
                    if (!valid)
                        continue;
                    initialized_slots.emplace(slot, UINT32_MAX);
                    recovered_local_operations[pc].push_back(value_type(slot_values[slot]) + " " +
                                                             name(slot_values[slot]) + ";");
                }
            }
        }
    for (const auto &[slot, candidates] : scoped_constants) {
        if (candidates.size() < 2 || initialized_slots.count(slot) || declaration_initializers.count(slot))
            continue;
        bool valid = true;
        for (unsigned a = 0; a < candidates.size(); ++a)
            for (unsigned b = a + 1; b < candidates.size(); ++b)
                if (contains_region(candidates[a].region, candidates[b].region) ||
                    contains_region(candidates[b].region, candidates[a].region))
                    valid = false;
        for (const auto &[user, at] : slot_events[slot]) {
            if (lexical_blocks[user].empty()) {
                valid = false;
                continue;
            }
            for (auto region : lexical_blocks[user]) {
                unsigned owners = 0;
                for (const auto &candidate : candidates)
                    if (contains_region(candidate.region, region)) {
                        ++owners;
                        if (!dominators[user].count(candidate.block) || (user == candidate.block && at < candidate.pc))
                            valid = false;
                    }
                if (owners != 1)
                    valid = false;
            }
        }
        if (!valid)
            continue;
        initialized_slots.emplace(slot, UINT32_MAX);
        for (const auto &candidate : candidates) {
            if (candidate.id == UINT32_MAX) {
                recovered_local_operations[candidate.pc].push_back(value_type(slot_values[slot]) + " " +
                                                                   name(slot_values[slot]) + ";");
                continue;
            }
            const auto &v = graph.values[candidate.id];
            if (v.kind == value_kind::constant || v.kind == value_kind::string) {
                scoped_declaration_initializers.emplace(slot, candidate.id);
                recovered_local_operations[candidate.pc].push_back(
                    value_type(slot_values[slot]) + " " + name(slot_values[slot]) + " = " +
                    (v.kind == value_kind::string ? source.text(v.immediate) : number_literal(v.immediate)) + ";");
            } else
                lexical_initializer_values.insert(candidate.id);
        }
    }
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b] && lexical_blocks[b].size() == 1) {
            const auto &block = graph.blocks[b];
            const auto &facts = source.stack.functions[index];
            for (unsigned id = 0; id < graph.values.size(); ++id) {
                const auto &v = graph.values[id];
                const auto pc = v.instruction;
                if ((v.kind != value_kind::constant && v.kind != value_kind::string) || v.width != 4 || used[id] ||
                    pc < block.begin || pc >= block.end || recovered_local_operations.count(pc) || !facts.heights[pc])
                    continue;
                const auto &op = binary.code[pc];
                if (op.opcode != OP_PSH || (op.argument != OP_ARG_NUM && op.argument != OP_ARG_STR))
                    continue;
                const int offset = *facts.heights[pc];
                if (offset < static_cast<int>(binary.parameters_size) ||
                    static_cast<unsigned>(offset / 4) >= block.outgoing.size() ||
                    block.outgoing[offset / 4] != value_ref_t{id, 0})
                    continue;
                bool valid = true;
                for (const auto &other : graph.values)
                    if (other.kind != value_kind::merge) {
                        for (const auto &args : other.arguments)
                            for (auto ref : args)
                                if (ref.value == id)
                                    valid = false;
                    }
                std::map<unsigned, unsigned> instruction_at;
                for (unsigned at = 0; at < binary.code.size(); ++at)
                    instruction_at.emplace(binary.code[at].offset, at);
                std::vector<unsigned> work{pc + 1};
                std::set<unsigned> visited;
                while (valid && !work.empty()) {
                    const auto at = work.back();
                    work.pop_back();
                    if (!visited.insert(at).second)
                        continue;
                    if (at >= binary.code.size() || !facts.heights[at] || *facts.heights[at] < offset + 4) {
                        valid = false;
                        break;
                    }
                    const auto &next = binary.code[at];
                    if (next.opcode == OP_SPA && *facts.heights[at] + static_cast<int16_t>(next.value) < offset + 4)
                        continue;
                    if (next.opcode == OP_RET) {
                        valid = false;
                        break;
                    }
                    if (next.opcode == OP_ASF || (next.opcode == OP_PSF && at + 1 < binary.code.size() &&
                                                  binary.code[at + 1].argument == OP_ARG_SPR)) {
                        valid = false;
                        break;
                    }
                    if (next.argument == OP_ARG_SPR) {
                        const int from = *facts.heights[at] + static_cast<int16_t>(next.value);
                        if (from < offset + 4 && from + static_cast<int>(next.data_size()) > offset) {
                            valid = false;
                            break;
                        }
                    }
                    if (next.opcode == OP_BF || next.opcode == OP_BRA || next.opcode == OP_FEQZB) {
                        const auto target = static_cast<unsigned>(static_cast<int64_t>(next.offset) + next.size() +
                                                                  static_cast<int16_t>(next.value));
                        auto found = instruction_at.find(target);
                        if (found == instruction_at.end()) {
                            valid = false;
                            break;
                        }
                        work.push_back(found->second);
                        if (next.opcode == OP_BRA)
                            continue;
                    }
                    work.push_back(at + 1);
                }
                if (!valid)
                    continue;
                recovered_local_operations[pc].push_back(
                    std::string(v.kind == value_kind::string ? "str " : "num ") + "unused_value" + std::to_string(id) +
                    " = " + (v.kind == value_kind::string ? source.text(v.immediate) : number_literal(v.immediate)) +
                    ";");
            }
        }
    std::vector<unsigned> array_layout_order;
    for (unsigned layout = 0; layout < local_arrays.size(); ++layout)
        array_layout_order.push_back(layout);
    std::stable_sort(array_layout_order.begin(), array_layout_order.end(), [&](unsigned a, unsigned b) {
        return local_arrays[a].offset < local_arrays[b].offset;
    });
    for (auto layout : array_layout_order) {
        const auto &array = local_arrays[layout];
        std::set<unsigned> versions;
        for (const auto &[id, which] : array_values)
            if (which == layout && stored(id) && required_slots[slots[id]])
                versions.insert(slots[id]);
        if (versions.size() != 1)
            continue;
        const auto slot = *versions.begin();
        if (initialized_slots.count(slot))
            continue;
        const auto &facts = source.stack.functions[index];
        std::optional<unsigned> allocation;
        for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
            const auto &op = binary.code[pc];
            if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) <= 0 || !facts.heights[pc])
                continue;
            const int start = *facts.heights[pc], end = start + static_cast<int16_t>(op.value);
            if (start <= static_cast<int>(array.offset) &&
                end >= static_cast<int>(array.offset + array.width * array.count)) {
                if (allocation) {
                    allocation.reset();
                    break;
                }
                allocation = pc;
            }
        }
        if (!allocation)
            continue;
        initialized_slots.emplace(slot, UINT32_MAX);
        recovered_local_operations[*allocation].push_back(
            value_type(slot_values[slot]) + " " + name(slot_values[slot]) + "[" + std::to_string(array.count) + "];");
    }
    if (!local_arrays.empty()) {
        const auto &facts = source.stack.functions[index];
        for (auto id : slot_values) {
            const auto slot = slots[id];
            if (initialized_slots.count(slot) || declaration_initializers.count(slot) || parameter_slots.count(slot) ||
                !required_slots[slot] || array_values.count(id))
                continue;
            std::set<unsigned> homes;
            for (const auto &block : graph.blocks) {
                auto collect = [&](const auto &refs, int height) {
                    for (unsigned word = 0; word < refs.size() && word * 4 < static_cast<unsigned>(height); ++word)
                        if (!refs[word].offset && stored(refs[word].value) && slots[refs[word].value] == slot)
                            homes.insert(word * 4);
                };
                if (facts.heights[block.begin])
                    collect(block.incoming, *facts.heights[block.begin]);
                if (!block.successors.empty()) {
                    const auto next = graph.blocks[block.successors.front()].begin;
                    if (facts.heights[next])
                        collect(block.outgoing, *facts.heights[next]);
                }
            }
            if (homes.size() != 1)
                continue;
            const auto home = *homes.begin();
            for (unsigned pc = 0; pc < graph.blocks[0].end; ++pc) {
                const auto &op = binary.code[pc];
                if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) <= 0 || !facts.heights[pc])
                    continue;
                const int begin = *facts.heights[pc], end = begin + static_cast<int16_t>(op.value);
                if (home < static_cast<unsigned>(begin) || home + graph.values[id].width > static_cast<unsigned>(end))
                    continue;
                initialized_slots.emplace(slot, UINT32_MAX);
                recovered_local_operations[pc].push_back(value_type(id) + " " + name(id) + ";");
                break;
            }
        }
    }
    if (!local_arrays.empty())
        for (const auto &[slot, id] : declaration_initializers) {
            if (initialized_slots.count(slot) || parameter_slots.count(slot))
                continue;
            const auto &v = graph.values[id];
            if (v.kind != value_kind::constant && v.kind != value_kind::string)
                continue;
            initialized_slots.emplace(slot, id);
            recovered_local_operations[v.instruction].push_back(
                value_type(slot_values[slot]) + " " + name(slot_values[slot]) + " = " +
                (v.kind == value_kind::string ? source.text(v.immediate) : number_literal(v.immediate)) + ";");
        }
    if (local_arrays.empty()) {
        const auto &facts = source.stack.functions[index];
        for (unsigned pc = graph.blocks[0].end; pc < binary.code.size(); ++pc) {
            const auto &allocation = binary.code[pc];
            if (allocation.opcode != OP_SPA || static_cast<int16_t>(allocation.value) <= 0 || !facts.heights[pc] ||
                recovered_local_operations.count(pc))
                continue;
            const int first = *facts.heights[pc], last = first + static_cast<int16_t>(allocation.value);
            bool unused = true, terminal_release = false;
            for (unsigned at = pc + 1; at < binary.code.size(); ++at) {
                if (!facts.heights[at])
                    continue;
                const auto &op = binary.code[at];
                if (*facts.heights[at] < last) {
                    terminal_release =
                        op.opcode == OP_RET || (op.opcode == OP_PSH && op.argument == OP_ARG_SPR &&
                                                at + 1 < binary.code.size() && binary.code[at + 1].opcode == OP_RET);
                    break;
                }
                if (op.opcode == OP_ASF || (op.opcode == OP_PSF && at + 1 < binary.code.size() &&
                                            binary.code[at + 1].argument == OP_ARG_SPR)) {
                    unused = false;
                    break;
                }
                if (op.argument != OP_ARG_SPR)
                    continue;
                const int from = *facts.heights[at] + static_cast<int16_t>(op.value);
                if (from < last && from + static_cast<int>(op.data_size()) > first) {
                    unused = false;
                    break;
                }
            }
            if (!unused || !terminal_release)
                continue;
            for (int at = first; at < last; at += 4)
                recovered_local_operations[pc].push_back("num unused_reserved" + std::to_string(pc) + "_" +
                                                         std::to_string((at - first) / 4) + ";");
        }
    }
    bool emitted_prefix_layout = false;
    if (local_arrays.empty()) {
        const auto &facts = source.stack.functions[index];
        unsigned prefix_end = binary.parameters_size;
        std::set<unsigned> reserved;
        for (unsigned pc = 0; pc < graph.blocks[0].end; ++pc) {
            const auto &op = binary.code[pc];
            if (op.opcode == OP_NEG && pc && binary.code[pc - 1].opcode == OP_PSH &&
                binary.code[pc - 1].argument == OP_ARG_NUM)
                continue;
            if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) > 0) {
                for (unsigned at = prefix_end; at < prefix_end + op.value; at += 4)
                    reserved.insert(at);
                prefix_end += op.value;
            } else if (op.opcode == OP_PSH && (op.argument == OP_ARG_NUM || op.argument == OP_ARG_STR) &&
                       op.data_size() == 4)
                prefix_end += 4;
            else {
                unsigned consumed = 0;
                for (const auto &v : graph.values)
                    if (v.instruction == pc && v.kind != value_kind::merge && v.kind != value_kind::uninitialized &&
                        v.kind != value_kind::parameter) {
                        unsigned bytes = 0;
                        for (const auto &args : v.arguments)
                            bytes += static_cast<unsigned>(args.size() * 4);
                        if (v.address) {
                            if (v.address->receiver)
                                bytes += 4;
                            if (v.address->index)
                                bytes += 4;
                        }
                        consumed = std::max(consumed, bytes);
                    }
                if (consumed && facts.heights[pc] && *facts.heights[pc] >= static_cast<int>(consumed))
                    prefix_end = std::min(prefix_end, static_cast<unsigned>(*facts.heights[pc]) - consumed);
                break;
            }
        }
        if (!graph.blocks[0].effects.empty()) {
            const auto pc = graph.blocks[0].effects.back().instruction;
            const auto &facts = source.stack.functions[index];
            if (pc < binary.code.size() && facts.heights[pc] && binary.code[pc].opcode == OP_BF)
                prefix_end = std::min(prefix_end, static_cast<unsigned>(*facts.heights[pc] - 4));
        }
        bool valid = prefix_end > binary.parameters_size;
        std::map<unsigned, unsigned> declarations;
        for (auto id : slot_values) {
            const auto slot = slots[id];
            if (initialized_slots.count(slot) || parameter_slots.count(slot) ||
                (!required_slots.empty() && !required_slots[slot]))
                continue;
            if (!graph.values[id].width || graph.values[id].width % 4) {
                valid = false;
                break;
            }
            std::set<unsigned> positions;
            for (unsigned b = 0; b < graph.blocks.size(); ++b)
                if (reachable[b]) {
                    auto inspect = [&](const auto &refs) {
                        for (unsigned at = binary.parameters_size; at < prefix_end && at / 4 < refs.size(); at += 4) {
                            const auto ref = refs[at / 4];
                            if (!ref.offset && stored(ref.value) && slots[ref.value] == slot)
                                positions.insert(at);
                        }
                    };
                    inspect(graph.blocks[b].incoming);
                    inspect(graph.blocks[b].outgoing);
                }
            if (positions.size() != 1 || !declarations.emplace(*positions.begin(), id).second) {
                valid = false;
                break;
            }
        }
        std::map<unsigned, unsigned> recovered_prefix;
        for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
            const auto &op = binary.code[pc];
            if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) > 0)
                continue;
            if (op.opcode != OP_PSH || (op.argument != OP_ARG_NUM && op.argument != OP_ARG_STR) || op.data_size() != 4)
                break;
            if (facts.heights[pc] && recovered_local_operations.count(pc)) {
                const auto at = static_cast<unsigned>(*facts.heights[pc]);
                if (at >= binary.parameters_size && at < prefix_end && !declarations.count(at))
                    recovered_prefix.emplace(at, pc);
            }
        }
        std::set<unsigned> occupied;
        for (const auto &[at, id] : declarations) {
            const auto width = graph.values[id].width;
            if (at + width > prefix_end)
                valid = false;
            for (unsigned part = at; part < at + width; part += 4)
                if (!occupied.insert(part).second)
                    valid = false;
        }
        std::set<unsigned> unused;
        for (unsigned at = binary.parameters_size; valid && at < prefix_end; at += 4)
            if (!occupied.count(at)) {
                if (recovered_prefix.count(at))
                    continue;
                if (!reserved.count(at)) {
                    valid = false;
                    break;
                }
                for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
                    const auto &op = binary.code[pc];
                    if (op.opcode == OP_ASF || (op.opcode == OP_PSF && (pc + 1 >= binary.code.size() ||
                                                                        binary.code[pc + 1].argument == OP_ARG_SPR))) {
                        valid = false;
                        break;
                    }
                    if (op.argument != OP_ARG_SPR || !facts.heights[pc])
                        continue;
                    const int from = *facts.heights[pc] + static_cast<int16_t>(op.value);
                    if (from < static_cast<int>(at + 4) &&
                        from + static_cast<int>(op.data_size()) > static_cast<int>(at)) {
                        valid = false;
                        break;
                    }
                }
                unused.insert(at);
            }
        if (valid) {
            unsigned serial = 0;
            for (unsigned at = binary.parameters_size; at < prefix_end; at += 4) {
                if (recovered_prefix.count(at)) {
                    const auto pc = recovered_prefix.at(at);
                    for (const auto &line : recovered_local_operations.at(pc))
                        emit_recovered_operation(line, indent);
                    recovered_local_operations.erase(pc);
                } else if (unused.count(at))
                    source.out << scoped_line(indent) << "num unused_local" << serial++ << ";\n";
                else if (declarations.count(at))
                    declaration(declarations.at(at));
            }
            emitted_prefix_layout = true;
        }
    }
    struct recovered_scope_t {
        unsigned first, last;
    };
    std::vector<recovered_scope_t> recovered_scopes;
    for (const auto &[slot, id] : initialized_slots) {
        if (parameter_slots.count(slot) || array_values.count(slot_values[slot]) ||
            lexical_initializer_values.count(id))
            continue;
        const auto &v = graph.values[id == UINT32_MAX ? slot_values[slot] : id];
        unsigned pc = v.instruction;
        const auto &facts = source.stack.functions[index];
        if (id == UINT32_MAX) {
            const auto candidates = scoped_constants.find(slot);
            if (candidates == scoped_constants.end() || candidates->second.size() != 1)
                continue;
            pc = candidates->second.front().pc;
            if (binary.code[pc].opcode != OP_SPA ||
                static_cast<int16_t>(binary.code[pc].value) != static_cast<int>(v.width))
                continue;
        }
        if (pc + 1 >= binary.code.size() || !facts.heights[pc + 1] || binary.code[pc].opcode == OP_POP)
            continue;
        const int offset = *facts.heights[pc + 1] - static_cast<int>(v.width);
        if (offset < static_cast<int>(binary.parameters_size))
            continue;
        unsigned owner = UINT32_MAX;
        for (unsigned b = 0; b < graph.blocks.size(); ++b)
            if (pc >= graph.blocks[b].begin && pc < graph.blocks[b].end)
                owner = b;
        if (owner == UINT32_MAX || lexical_blocks[owner].size() != 1)
            continue;
        for (unsigned at = pc + 1; at < binary.code.size(); ++at) {
            const auto &op = binary.code[at];
            if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) >= 0 || !facts.heights[at] ||
                *facts.heights[at] < offset + static_cast<int>(v.width) ||
                *facts.heights[at] + static_cast<int16_t>(op.value) > offset)
                continue;
            unsigned closer = UINT32_MAX;
            for (unsigned b = 0; b < graph.blocks.size(); ++b)
                if (at >= graph.blocks[b].begin && at < graph.blocks[b].end)
                    closer = b;
            if (closer == UINT32_MAX || lexical_blocks[closer] != lexical_blocks[owner] ||
                !dominators[closer].count(owner) || !postdominators[owner].count(closer))
                break;
            bool followed = false;
            for (const auto &e : graph.blocks[closer].effects)
                if (e.instruction > at && e.kind != effect_kind::branch && e.kind != effect_kind::return_value &&
                    !quiet_effect(e))
                    followed = true;
            bool valid = followed;
            for (const auto &[user, event] : slot_events[slot])
                if (event < pc || event >= at)
                    valid = false;
            if (valid)
                recovered_scopes.push_back({pc, at});
            break;
        }
    }
    for (const auto &scope : recovered_scopes) {
        bool crosses = false;
        for (const auto &other : recovered_scopes)
            if ((scope.first < other.first && other.first < scope.last && scope.last < other.last) ||
                (other.first < scope.first && scope.first < other.last && other.last < scope.last))
                crosses = true;
        if (crosses)
            continue;
        recovered_local_operations[scope.first].insert(recovered_local_operations[scope.first].begin(), "{");
        recovered_local_operations[scope.last].push_back("}");
    }
    for (const auto &[slot, id] : declaration_initializers) {
        const auto &v = graph.values[id];
        if ((v.kind == value_kind::constant || v.kind == value_kind::string) && v.local_read_home &&
            !v.ambiguous_read_home)
            recovered_local_names.emplace(id, std::make_pair(name(slot_values[slot]), value_type(id)));
    }
    if (!emitted_prefix_layout) {
        std::vector<unsigned> declarations;
        std::map<unsigned, unsigned> homes;
        bool ordered = true;
        for (auto id : slot_values)
            if (!initialized_slots.count(slots[id]) && !parameter_slots.count(slots[id]) &&
                (required_slots.empty() || required_slots[slots[id]])) {
                declarations.push_back(id);
                auto found = declaration_initializers.find(slots[id]);
                if (found == declaration_initializers.end()) {
                    ordered = false;
                    continue;
                }
                const auto pc = graph.values[found->second].instruction;
                const auto &facts = source.stack.functions[index];
                if (pc >= facts.heights.size() || !facts.heights[pc]) {
                    ordered = false;
                    continue;
                }
                homes.emplace(id, static_cast<unsigned>(*facts.heights[pc]));
            }
        if (ordered)
            std::stable_sort(declarations.begin(), declarations.end(), [&](unsigned a, unsigned b) {
                return homes.at(a) < homes.at(b);
            });
        for (auto id : declarations)
            declaration(id);
    }
    entry_values();
    std::function<void(const std::vector<statement_t> &, const std::string &)> emit_all;
    std::function<void(const statement_t &, const std::string &, bool)> emit_conditional;
    emit_all = [&](const std::vector<statement_t> &statements, const std::string &line) {
        for (const auto &statement : statements) {
            switch (statement.kind) {
            case kind::effects:
                for (const auto &effect : graph.blocks[statement.block].effects)
                    if (effect.kind != effect_kind::branch)
                        emit_effect(statement.block, effect, line);
                emit_recovered_before(statement.block, graph.blocks[statement.block].end - 1, line);
                break;
            case kind::transfer:
                edge(statement.block, statement.target, line, false);
                break;
            case kind::branch:
                emit_conditional(statement, line, false);
                break;
            case kind::loop:
                source.out << scoped_line(line) << "while ("
                           << (statement.condition.empty() ? "1" : render(statement.condition, statement.negate))
                           << ") {\n";
                emit_all(statement.body, line + "    ");
                source.out << scoped_line(line) << "}\n";
                break;

            case kind::switch_statement:
                source.out << scoped_line(line) << "switch (" << group(statement.selector, "num") << ") {\n";
                for (const auto &arm : statement.body) {
                    if (arm.case_values.empty())
                        source.out << scoped_line(line) << "default:\n";
                    else
                        for (auto bits : arm.case_values)
                            source.out << scoped_line(line) << "case " << number_literal(bits) << ":\n";
                    source.out << scoped_line(line) << "{\n";
                    emit_all(arm.body, line + "    ");
                    if (!closed(arm.body))
                        source.out << scoped_line(line) << "    break;\n";
                    source.out << scoped_line(line) << "}\n";
                }
                source.out << scoped_line(line) << "}\n";
                break;
            case kind::switch_case:
                break;
            case kind::break_loop:
                source.out << scoped_line(line) << "break;\n";
                break;
            case kind::continue_loop:
                source.out << scoped_line(line) << "continue;\n";
                break;
            }
        }
    };
    emit_conditional = [&](const statement_t &branch, const std::string &line, bool alternative) {
        const bool invert = false;
        const auto &primary = invert ? branch.otherwise : branch.body;
        const auto &secondary = invert ? branch.body : branch.otherwise;
        const auto opening = scoped_line(line) + (alternative ? "} else if (" : "if (") +
                             render(branch.condition, invert != branch.negate) + ')';

        if (secondary.empty() && primary.size() == 1 &&
            (primary.front().kind == kind::break_loop || primary.front().kind == kind::continue_loop)) {
            source.out << opening << (primary.front().kind == kind::break_loop ? " break;\n" : " continue;\n");
            return;
        }
        source.out << opening << " {\n";
        emit_all(primary, line + "    ");
        if (secondary.empty()) {
            bool explicit_empty_else =
                !invert && !branch.condition.empty() && empty_else_headers.count(branch.condition.back().block);
            if (explicit_empty_else)
                source.out << scoped_line(line) << "} else {\n";
            source.out << scoped_line(line) << "}\n";
            return;
        }
        if (const auto *test = chained(secondary)) {
            for (unsigned i = 0; i + 1 < secondary.size(); ++i) {
                const auto &statement = secondary[i];
                if (statement.kind == kind::transfer)
                    edge(statement.block, statement.target, line, false);
                else
                    for (const auto &effect : graph.blocks[statement.block].effects)
                        if (effect.kind != effect_kind::branch)
                            emit_effect(statement.block, effect, line);
            }
            emit_conditional(*test, line, true);
            return;
        }
        std::ostringstream saved;
        source.out.swap(saved);
        emit_all(secondary, line + "    ");
        auto secondary_source = source.out.str();
        source.out.swap(saved);
        const bool explicit_empty_else =
            !branch.condition.empty() && empty_else_headers.count(branch.condition.back().block);
        if (!secondary_source.empty() || explicit_empty_else)
            source.out << scoped_line(line) << "} else {\n" << secondary_source;
        source.out << scoped_line(line) << "}\n";
    };
    reduce(plan);
    recover_late_loop_tests(plan);
    emit_all(plan, indent);
    if (recovered_scope_depth)
        throw error_t("Recovered lexical scope remains open");
    return true;
}

void source_emitter_t::body_t::allocate()
{
    std::map<unsigned, unsigned> retained_frame_homes;
    for (unsigned id = 0; id < graph.values.size(); ++id) {
        const auto &v = graph.values[id];
        const bool local_store = v.kind == lifted_value_t::kind_t::operation && v.address &&
                                 v.address->kind == lifted_address_t::kind_t::local && !v.address->index &&
                                 v.instruction < binary.code.size() && binary.code[v.instruction].opcode == OP_POP;
        const bool retained = retains_unused_result(id);
        if (retained) {
            const auto &facts = source.stack.functions[index];
            if (v.instruction + 1 < facts.heights.size() && facts.heights[v.instruction + 1] &&
                *facts.heights[v.instruction + 1] >= static_cast<int>(v.width))
                retained_frame_homes.emplace(id, static_cast<unsigned>(*facts.heights[v.instruction + 1]) - v.width);
        }
        if (retained || local_store)
            use({id, 0});
    }
    using set_t = std::set<unsigned>;
    using effect_kind = lifted_effect_t::kind_t;
    const auto count = graph.blocks.size();
    std::vector<set_t> inputs(count), definitions(count), phis(count), live_in(count), live_out(count);
    auto references = [&](const lifted_effect_t &effect) {
        set_t result;
        auto add = [&](value_ref_t ref) {
            if (stored(ref.value))
                result.insert(ref.value);
        };
        auto address = [&](const std::optional<lifted_address_t> &storage) {
            if (!storage)
                return;
            if (storage->receiver)
                add(*storage->receiver);
            if (storage->index)
                add(*storage->index);
        };
        if (effect.kind == effect_kind::value) {
            if (!used[effect.value] && !effectful(effect.value))
                return result;
            const auto &value = graph.values[effect.value];
            for (const auto &argument : value.arguments)
                for (auto ref : argument)
                    add(ref);
            address(value.address);
        } else {
            const auto n = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
            for (unsigned a = 0; a < n; ++a)
                for (auto ref : effect.arguments[a])
                    add(ref);
            address(effect.address);
        }
        return result;
    };
    for (unsigned b = 0; b < count; ++b) {
        if (!reachable[b])
            continue;
        const auto &block = graph.blocks[b];
        for (unsigned word = 0; word < block.incoming.size(); ++word) {
            const auto ref = block.incoming[word];
            if (!stored(ref.value) || graph.values[ref.value].kind != lifted_value_t::kind_t::merge)
                continue;
            bool defines = b == 0;
            for (const auto &predecessor : graph.blocks)
                if (std::find(predecessor.successors.begin(), predecessor.successors.end(), b) !=
                        predecessor.successors.end() &&
                    word < predecessor.outgoing.size() && predecessor.outgoing[word] != ref)
                    defines = true;
            if (defines)
                phis[b].insert(ref.value);
        }
        if (block.incoming_array_index && stored(block.incoming_array_index->value))
            phis[b].insert(block.incoming_array_index->value);
        definitions[b] = phis[b];
        for (const auto &effect : block.effects) {
            for (auto ref : references(effect))
                if (!definitions[b].count(ref))
                    inputs[b].insert(ref);
            if (effect.kind == effect_kind::value && stored(effect.value))
                definitions[b].insert(effect.value);
        }
    }
    bool changed;
    do {
        changed = false;
        for (unsigned b = static_cast<unsigned>(count); b-- > 0;) {
            if (!reachable[b])
                continue;
            set_t outgoing;
            const auto &block = graph.blocks[b];
            for (auto next : block.successors) {
                outgoing.insert(live_in[next].begin(), live_in[next].end());
                const auto &destination = graph.blocks[next];
                for (unsigned word = 0; word < destination.incoming.size(); ++word) {
                    if (!phis[next].count(destination.incoming[word].value))
                        continue;
                    const auto ref = block.outgoing.at(word).value;
                    if (stored(ref))
                        outgoing.insert(ref);
                }
                if (destination.incoming_array_index && phis[next].count(destination.incoming_array_index->value) &&
                    stored(block.outgoing_array_index->value))
                    outgoing.insert(block.outgoing_array_index->value);
            }
            auto incoming = inputs[b];
            for (auto value : outgoing)
                if (!definitions[b].count(value))
                    incoming.insert(value);
            if (incoming != live_in[b] || outgoing != live_out[b]) {
                live_in[b] = std::move(incoming);
                live_out[b] = std::move(outgoing);
                changed = true;
            }
        }
    } while (changed);
    std::vector<set_t> interference(graph.values.size());
    auto connect = [&](unsigned a, unsigned b) {
        if (a == b)
            return;
        interference[a].insert(b);
        interference[b].insert(a);
    };
    for (unsigned b = 0; b < count; ++b) {
        if (!reachable[b])
            continue;
        auto live = live_out[b];
        const auto &effects = graph.blocks[b].effects;
        for (auto effect = effects.rbegin(); effect != effects.rend(); ++effect) {
            if (effect->kind == effect_kind::value && stored(effect->value)) {
                for (auto other : live)
                    connect(effect->value, other);
                live.erase(effect->value);
            }
            auto refs = references(*effect);
            live.insert(refs.begin(), refs.end());
        }
        for (auto phi : phis[b]) {
            for (auto other : live)
                connect(phi, other);
            live.erase(phi);
        }
    }

    for (unsigned b = 0; b < count; ++b) {
        if (!reachable[b])
            continue;
        const auto &block = graph.blocks[b];
        const auto &facts = source.stack.functions[index];
        auto resident = [&](const auto &refs, int height) {
            std::set<unsigned> values;
            for (unsigned word = 0; word < refs.size() && word * 4 < static_cast<unsigned>(height); ++word)
                if (stored(refs[word].value))
                    values.insert(refs[word].value);
            for (auto a : values)
                for (auto c : values)
                    connect(a, c);
        };
        if (facts.heights[block.begin])
            resident(block.incoming, *facts.heights[block.begin]);
        if (!block.successors.empty()) {
            const auto next = graph.blocks[block.successors.front()].begin;
            if (facts.heights[next]) {
                int retained_height = *facts.heights[next];
                for (unsigned pc = block.end; pc > block.begin;) {
                    --pc;
                    const auto &op = binary.code[pc];
                    if (op.opcode == OP_BRA)
                        continue;
                    if (op.opcode != OP_SPA || static_cast<int16_t>(op.value) >= 0)
                        break;
                    if (facts.heights[pc])
                        retained_height = std::max(retained_height, *facts.heights[pc]);
                }
                resident(block.outgoing, retained_height);
            }
        } else
            resident(block.outgoing, static_cast<int>(block.outgoing.size() * 4));
    }
    {
        unsigned prefix_end = binary.parameters_size;
        for (unsigned pc = 0; pc < graph.blocks[0].end; ++pc) {
            const auto &op = binary.code[pc];
            if (op.opcode == OP_NEG && pc && binary.code[pc - 1].opcode == OP_PSH &&
                binary.code[pc - 1].argument == OP_ARG_NUM)
                continue;
            if (op.opcode == OP_SPA && static_cast<int16_t>(op.value) > 0)
                prefix_end += op.value;
            else if (op.opcode == OP_PSH && (op.argument == OP_ARG_NUM || op.argument == OP_ARG_STR) &&
                     op.data_size() == 4)
                prefix_end += 4;
            else
                break;
        }
        if (!graph.blocks[0].effects.empty()) {
            const auto pc = graph.blocks[0].effects.back().instruction;
            const auto &facts = source.stack.functions[index];
            if (pc < binary.code.size() && facts.heights[pc] && binary.code[pc].opcode == OP_BF)
                prefix_end = std::min(prefix_end, static_cast<unsigned>(*facts.heights[pc] - 4));
        }
        std::map<unsigned, std::set<unsigned>> positions;
        for (unsigned b = 0; b < count; ++b)
            if (reachable[b]) {
                auto inspect = [&](const auto &refs) {
                    for (unsigned at = binary.parameters_size; at < prefix_end && at / 4 < refs.size(); at += 4) {
                        auto ref = refs[at / 4];
                        if (!ref.offset && stored(ref.value))
                            positions[ref.value].insert(at);
                    }
                };
                inspect(graph.blocks[b].incoming);
                inspect(graph.blocks[b].outgoing);
            }
        for (const auto &[a, ap] : positions)
            if (ap.size() == 1)
                for (const auto &[b, bp] : positions)
                    if (bp.size() == 1 && *ap.begin() != *bp.begin())
                        connect(a, b);
    }
    for (auto a : live_in[0])
        for (auto b : live_in[0])
            connect(a, b);
    std::map<unsigned, std::set<unsigned>> frame_positions;
    for (unsigned b = 0; b < count; ++b)
        if (reachable[b]) {
            const auto &block = graph.blocks[b];
            const auto &facts = source.stack.functions[index];
            auto inspect = [&](const auto &refs, int height) {
                for (unsigned at = 0; at < static_cast<unsigned>(height) && at / 4 < refs.size(); at += 4) {
                    const auto ref = refs[at / 4];
                    if (!ref.offset && stored(ref.value))
                        frame_positions[ref.value].insert(at);
                }
            };
            if (facts.heights[block.begin])
                inspect(block.incoming, *facts.heights[block.begin]);
            if (!block.successors.empty()) {
                const auto next = graph.blocks[block.successors.front()].begin;
                if (facts.heights[next])
                    inspect(block.outgoing, *facts.heights[next]);
            }
        }
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && !array_values.count(id)) {
            const auto &v = graph.values[id];
            if (v.instruction >= binary.code.size())
                continue;
            auto pc = v.instruction;
            if (binary.code[pc].opcode != OP_POP || binary.code[pc].argument != OP_ARG_SPR)
                ++pc;
            if (pc >= binary.code.size())
                continue;
            const auto &op = binary.code[pc];
            const auto &facts = source.stack.functions[index];
            if (op.opcode != OP_POP || op.argument != OP_ARG_SPR || op.data_size() != v.width || !facts.heights[pc])
                continue;
            const auto position = *facts.heights[pc] + static_cast<int16_t>(op.value);
            if (position >= 0)
                frame_positions[id].insert(static_cast<unsigned>(position));
        }
    for (const auto &[id, home] : retained_frame_homes)
        if (frame_positions[id].empty())
            frame_positions[id].insert(home);
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && frame_positions[id].empty() && graph.values[id].local_read_home &&
            !graph.values[id].ambiguous_read_home)
            frame_positions[id].insert(*graph.values[id].local_read_home);
    for (auto &[id, positions] : frame_positions)
        if (positions.size() > 1 && std::count_if(positions.begin(), positions.end(), [&](unsigned at) {
                                        return at < binary.parameters_size;
                                    }) == 1) {
            const auto first = *positions.begin();
            positions.clear();
            positions.insert(first);
        }
    for (const auto &[a, ap] : frame_positions)
        if (ap.size() == 1)
            for (const auto &[b, bp] : frame_positions)
                if (bp.size() == 1 && *ap.begin() != *bp.begin())
                    connect(a, b);
    {
        const auto &facts = source.stack.functions[index];
        std::vector<unsigned> owners(binary.code.size(), UINT32_MAX);
        for (unsigned b = 0; b < graph.blocks.size(); ++b)
            for (unsigned pc = graph.blocks[b].begin; pc < graph.blocks[b].end; ++pc)
                owners[pc] = b;
        std::vector<unsigned> births(graph.values.size(), UINT32_MAX);
        for (unsigned id = 0; id < graph.values.size(); ++id) {
            if (graph.values[id].kind != lifted_value_t::kind_t::merge) {
                births[id] = graph.values[id].instruction;
                continue;
            }
            std::set<unsigned> definitions;
            for (unsigned b = 0; b < graph.blocks.size(); ++b)
                if (reachable[b])
                    for (unsigned word = 0; word < graph.blocks[b].incoming.size(); ++word) {
                        if (graph.blocks[b].incoming[word].value != id || graph.blocks[b].incoming[word].offset)
                            continue;
                        bool defines = b == 0;
                        for (const auto &pred : graph.blocks)
                            if (std::find(pred.successors.begin(), pred.successors.end(), b) != pred.successors.end() &&
                                word < pred.outgoing.size() && pred.outgoing[word] != graph.blocks[b].incoming[word])
                                defines = true;
                        if (defines)
                            definitions.insert(graph.blocks[b].begin);
                    }
            if (definitions.size() == 1)
                births[id] = *definitions.begin();
        }
        for (const auto &[a, ap] : frame_positions)
            if (ap.size() == 1)
                for (const auto &[b, bp] : frame_positions) {
                    if (bp != ap || a == b || !stored(a) || !stored(b))
                        continue;
                    const auto first = births[a], last = births[b], home = *ap.begin();
                    if (first >= last || last >= binary.code.size() || home < binary.parameters_size)
                        continue;
                    bool possible = false;
                    for (unsigned pc = first + 1; pc < last; ++pc)
                        if (binary.code[pc].opcode == OP_SPA && static_cast<int16_t>(binary.code[pc].value) < 0 &&
                            facts.heights[pc] &&
                            *facts.heights[pc] + static_cast<int16_t>(binary.code[pc].value) <=
                                static_cast<int>(home)) {
                            possible = true;
                            break;
                        }
                    if (!possible)
                        continue;
                    std::vector<std::pair<unsigned, bool>> pending{{first + 1, false}};
                    std::set<std::pair<unsigned, bool>> visited;
                    bool after_release = false, without_release = false;
                    while (!pending.empty()) {
                        const auto [pc, was_released] = pending.back();
                        pending.pop_back();
                        if (pc >= binary.code.size() || pc == first || !visited.emplace(pc, was_released).second)
                            continue;
                        if (pc == last) {
                            if (was_released)
                                after_release = true;
                            else
                                without_release = true;
                            continue;
                        }
                        bool released = was_released;
                        if (binary.code[pc].opcode == OP_SPA && static_cast<int16_t>(binary.code[pc].value) < 0 &&
                            facts.heights[pc] &&
                            *facts.heights[pc] + static_cast<int16_t>(binary.code[pc].value) <= static_cast<int>(home))
                            released = true;
                        const auto block = owners[pc];
                        if (block == UINT32_MAX)
                            continue;
                        if (pc + 1 < graph.blocks[block].end)
                            pending.emplace_back(pc + 1, released);
                        else
                            for (auto next : graph.blocks[block].successors)
                                pending.emplace_back(graph.blocks[next].begin, released);
                    }
                    if (after_release && !without_release)
                        connect(a, b);
                }
    }
    std::vector<unsigned> order;
    slots.assign(graph.values.size(), UINT32_MAX);
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id))
            order.push_back(id);
    std::stable_sort(order.begin(), order.end(), [&](unsigned a, unsigned b) {
        const bool ah = frame_positions[a].size() == 1, bh = frame_positions[b].size() == 1;
        if (ah != bh)
            return ah;
        return interference[a].size() > interference[b].size();
    });
    for (auto id : order) {
        set_t unavailable;
        for (auto other : interference[id])
            if (slots[other] != UINT32_MAX)
                unavailable.insert(slots[other]);
        auto compatible = [&](unsigned slot) {
            const auto &left = frame_positions[id], &right = frame_positions[slot_values[slot]];
            const auto parameter = [&](const auto &positions) {
                return positions.size() == 1 && *positions.begin() < binary.parameters_size;
            };
            if ((parameter(left) || parameter(right)) && left != right)
                return false;
            return !unavailable.count(slot) && value_type(id) == value_type(slot_values[slot]) &&
                   graph.values[id].width == graph.values[slot_values[slot]].width &&
                   array_values.count(id) == array_values.count(slot_values[slot]) &&
                   (!array_values.count(id) || array_values.at(id) == array_values.at(slot_values[slot]));
        };
        unsigned slot = slot_values.size();
        if (frame_positions[id].size() == 1)
            for (unsigned candidate = 0; candidate < slot_values.size(); ++candidate)
                if (compatible(candidate) && frame_positions[slot_values[candidate]] == frame_positions[id]) {
                    slot = candidate;
                    break;
                }
        if (slot == slot_values.size())
            for (slot = 0; slot < slot_values.size(); ++slot)
                if (compatible(slot))
                    break;
        if (slot == slot_values.size())
            slot_values.push_back(id);
        slots[id] = slot;
    }

    std::map<unsigned, unsigned> local_postfix;
    const auto &facts = source.stack.functions[index];
    for (unsigned id = 0; id < graph.values.size(); ++id) {
        const auto &v = graph.values[id];
        const auto pc = v.instruction;
        if (!stored(id) || !pc || pc + 2 >= binary.code.size() || v.arguments.size() != 1 || v.arguments[0].empty() ||
            v.arguments[0][0].offset)
            continue;
        const auto old = v.arguments[0][0].value;
        if (!stored(old) || v.width != graph.values[old].width || value_type(old) != value_type(id))
            continue;
        int from = -1, to = -2;
        if (v.kind == lifted_value_t::kind_t::operation) {
            if (pc < 2 || v.arguments[0].size() != 1 || value_type(id) != "num" || v.width != 4)
                continue;
            if ((binary.code[pc].opcode != OP_INC && binary.code[pc].opcode != OP_DEC) ||
                binary.code[pc - 1].opcode != OP_DUP || binary.code[pc - 2].opcode != OP_PSH ||
                binary.code[pc - 2].argument != OP_ARG_SPR || binary.code[pc + 1].opcode != OP_POP ||
                binary.code[pc + 1].argument != OP_ARG_SPR || !facts.heights[pc - 2] || !facts.heights[pc + 1])
                continue;
            from = *facts.heights[pc - 2] + static_cast<int16_t>(binary.code[pc - 2].value);
            to = *facts.heights[pc + 1] + static_cast<int16_t>(binary.code[pc + 1].value);
        } else if (v.kind == lifted_value_t::kind_t::native_call) {
            const auto &native = source.symbols.natives.at({v.immediate >> 16, v.immediate & 0xffff});
            if ((!native.signature.starts_with("operator++(") && !native.signature.starts_with("operator--(")) ||
                native.parameters.size() != 1 || native.class_name != value_type(id))
                continue;
            bool whole = v.arguments[0].size() * 4 == v.width;
            for (unsigned word = 0; whole && word < v.arguments[0].size(); ++word)
                whole = v.arguments[0][word] == value_ref_t{old, word * 4};
            if (!whole || pc < 2 || binary.code[pc - 2].opcode != OP_PSH ||
                binary.code[pc - 2].argument != OP_ARG_SPR || binary.code[pc - 2].data_size() != v.width ||
                binary.code[pc - 1].opcode != OP_DUP || binary.code[pc - 1].data_size() != v.width ||
                binary.code[pc + 1].opcode != OP_POP || binary.code[pc + 1].argument != OP_ARG_SPR ||
                binary.code[pc + 1].data_size() != v.width || !facts.heights[pc - 2] || !facts.heights[pc + 1])
                continue;
            from = *facts.heights[pc - 2] + static_cast<int16_t>(binary.code[pc - 2].value);
            to = *facts.heights[pc + 1] + static_cast<int16_t>(binary.code[pc + 1].value);
        } else
            continue;
        if (from != to)
            continue;
        unsigned origin = UINT32_MAX;
        for (unsigned b = 0; b < graph.blocks.size(); ++b)
            if (pc >= graph.blocks[b].begin && pc < graph.blocks[b].end)
                origin = b;
        if (origin == UINT32_MAX)
            continue;
        std::set<unsigned> after_blocks;
        std::vector<unsigned> pending(graph.blocks[origin].successors);
        while (!pending.empty()) {
            const auto b = pending.back();
            pending.pop_back();
            if (after_blocks.count(b))
                continue;
            bool redefines = false;
            if (graph.values[old].kind == lifted_value_t::kind_t::merge) {
                for (unsigned word = 0; word < graph.blocks[b].incoming.size(); ++word)
                    if (graph.blocks[b].incoming[word].value == old) {
                        for (const auto &predecessor : graph.blocks)
                            if (std::find(predecessor.successors.begin(), predecessor.successors.end(), b) !=
                                    predecessor.successors.end() &&
                                word < predecessor.outgoing.size() &&
                                predecessor.outgoing[word] != graph.blocks[b].incoming[word])
                                redefines = true;
                    }
            } else if (graph.values[old].instruction >= graph.blocks[b].begin &&
                       graph.values[old].instruction < graph.blocks[b].end)
                redefines = true;
            if (redefines)
                continue;
            after_blocks.insert(b);
            for (auto next : graph.blocks[b].successors)
                pending.push_back(next);
        }
        auto after = [&](unsigned at) {
            if (at > pc && at < graph.blocks[origin].end)
                return true;
            for (auto b : after_blocks)
                if (at >= graph.blocks[b].begin && at < graph.blocks[b].end)
                    return true;
            return false;
        };
        bool safe = true;
        for (unsigned user = 0; user < graph.values.size(); ++user) {
            const auto &u = graph.values[user];
            if (u.instruction >= binary.code.size() || !after(u.instruction) || u.kind == lifted_value_t::kind_t::merge)
                continue;
            for (const auto &args : u.arguments)
                for (auto ref : args)
                    if (ref.value == old)
                        safe = false;
            if (u.address && ((u.address->receiver && u.address->receiver->value == old) ||
                              (u.address->index && u.address->index->value == old)))
                safe = false;
        }
        for (const auto &block : graph.blocks)
            for (const auto &e : block.effects)
                if (after(e.instruction) && e.kind != effect_kind::value) {
                    for (const auto &args : e.arguments)
                        for (auto ref : args)
                            if (ref.value == old)
                                safe = false;
                    if (e.address && ((e.address->receiver && e.address->receiver->value == old) ||
                                      (e.address->index && e.address->index->value == old)))
                        safe = false;
                }
        if (safe)
            local_postfix.emplace(id, old);
    }

    if (!local_postfix.empty()) {
        std::vector<unsigned> parent(graph.values.size());
        for (unsigned i = 0; i < parent.size(); ++i)
            parent[i] = i;
        std::function<unsigned(unsigned)> root = [&](unsigned n) {
            return parent[n] == n ? n : parent[n] = root(parent[n]);
        };
        for (const auto &[id, old] : local_postfix)
            parent[root(id)] = root(old);
        std::map<unsigned, set_t> conflicts;
        for (unsigned id = 0; id < graph.values.size(); ++id)
            if (stored(id)) {
                auto a = root(id);
                conflicts[a];
                for (auto other : interference[id])
                    if (stored(other) && root(other) != a)
                        conflicts[a].insert(root(other));
            }
        std::map<unsigned, set_t> group_positions;
        for (unsigned id = 0; id < graph.values.size(); ++id)
            if (stored(id))
                group_positions[root(id)].insert(frame_positions[id].begin(), frame_positions[id].end());
        auto compatible_position = [&](unsigned a, unsigned b) {
            const auto &left = group_positions[a], &right = group_positions[b];
            const auto parameter = [&](const auto &positions) {
                return positions.size() == 1 && *positions.begin() < binary.parameters_size;
            };
            return !(parameter(left) || parameter(right)) || left == right;
        };
        std::vector<unsigned> groups;
        for (const auto &[id, edges] : conflicts)
            groups.push_back(id);
        std::stable_sort(groups.begin(), groups.end(), [&](unsigned a, unsigned b) {
            const bool ah = group_positions[a].size() == 1, bh = group_positions[b].size() == 1;
            if (ah != bh)
                return ah;
            return conflicts[a].size() > conflicts[b].size();
        });
        slots.assign(graph.values.size(), UINT32_MAX);
        slot_values.clear();
        for (auto id : groups) {
            set_t unavailable;
            for (auto other : conflicts[id])
                if (slots[other] != UINT32_MAX)
                    unavailable.insert(slots[other]);
            unsigned slot = 0;
            for (; slot < slot_values.size(); ++slot)
                if (compatible_position(id, slot_values[slot]) && !unavailable.count(slot) &&
                    value_type(id) == value_type(slot_values[slot]) &&
                    graph.values[id].width == graph.values[slot_values[slot]].width &&
                    array_values.count(id) == array_values.count(slot_values[slot]) &&
                    (!array_values.count(id) || array_values.at(id) == array_values.at(slot_values[slot])))
                    break;
            if (slot == slot_values.size())
                slot_values.push_back(id);
            slots[id] = slot;
        }
        for (unsigned id = 0; id < graph.values.size(); ++id)
            if (stored(id))
                slots[id] = slots[root(id)];
        postfix_locals = std::move(local_postfix);
    }

    std::set<unsigned> mutable_parameters;
    for (unsigned word = 0; word < graph.blocks[0].incoming.size() && word * 4 < binary.parameters_size; ++word) {
        const auto ref = graph.blocks[0].incoming[word];
        if (!ref.offset && stored(ref.value) && graph.values[ref.value].kind == lifted_value_t::kind_t::merge)
            mutable_parameters.insert(word * 4);
    }
    for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
        const auto &op = binary.code[pc];
        if (op.opcode != OP_POP || op.argument != OP_ARG_SPR || !facts.heights[pc])
            continue;
        const int offset = *facts.heights[pc] + static_cast<int16_t>(op.value);
        if (offset >= 0 && offset + static_cast<int>(op.data_size()) <= static_cast<int>(binary.parameters_size) &&
            ((binary.flags & 1u) || offset != 0))
            mutable_parameters.insert(static_cast<unsigned>(offset));
    }
    std::map<unsigned, unsigned> parameter_versions;
    std::set<unsigned> conflicted_versions;
    for (unsigned b = 0; b < graph.blocks.size(); ++b)
        if (reachable[b]) {
            auto collect = [&](const auto &refs) {
                for (unsigned word = 0; word < refs.size() && word * 4 < binary.parameters_size; ++word) {
                    auto ref = refs[word];
                    if (!mutable_parameters.count(word * 4) || ref.offset || !stored(ref.value))
                        continue;
                    const auto width = graph.values[ref.value].width;
                    bool whole = width && !(width % 4) && word + width / 4 <= refs.size();
                    for (unsigned component = 0; whole && component < width / 4; ++component)
                        whole = refs[word + component] == value_ref_t{ref.value, component * 4};
                    if (!whole)
                        continue;
                    const auto parameter =
                        std::find_if(types.parameters.begin(), types.parameters.end(), [&](const auto &p) {
                            return p.offset == word * 4 && p.width == width;
                        });
                    if (parameter == types.parameters.end() || value_type(ref.value) != source.type(parameter->type) ||
                        (!(binary.flags & 1u) && word == 0))
                        continue;
                    auto [it, inserted] = parameter_versions.emplace(ref.value, word * 4);
                    if (!inserted && it->second != word * 4)
                        conflicted_versions.insert(ref.value);
                }
            };
            collect(graph.blocks[b].incoming);
            collect(graph.blocks[b].outgoing);
        }
    for (unsigned slot = 0; slot < slot_values.size(); ++slot) {
        std::optional<unsigned> offset;
        bool valid = true;
        for (unsigned id = 0; id < slots.size(); ++id)
            if (slots[id] == slot) {
                auto found = parameter_versions.find(id);
                if (found == parameter_versions.end() || conflicted_versions.count(id)) {
                    valid = false;
                    break;
                }
                if (offset && *offset != found->second) {
                    valid = false;
                    break;
                }
                offset = found->second;
            }
        if (valid && offset)
            parameter_slots.emplace(slot, *offset);
    }
}


void source_emitter_t::body_t::recover_short_circuits()
{
    const auto old_live = std::count(reachable.begin(), reachable.end(), true);
    using vk = lifted_value_t::kind_t;
    using ek = lifted_effect_t::kind_t;
    for (bool changed = true; changed;) {
        changed = false;
        std::vector<unsigned> predecessors(graph.blocks.size());
        for (unsigned b = 0; b < graph.blocks.size(); ++b)
            if (reachable[b])
                for (auto next : graph.blocks[b].successors)
                    ++predecessors[next];
        for (unsigned b = 0; b < graph.blocks.size(); ++b) {
            if (!reachable[b])
                continue;
            auto &left = graph.blocks[b];
            if (left.effects.empty() || left.successors.size() != 2)
                continue;
            const auto branch = left.effects.back();
            const auto &ins = binary.code[branch.instruction];
            if (branch.kind != ek::branch || ins.opcode != OP_FEQZB || branch.arguments.size() != 1 ||
                branch.arguments[0].size() != 1)
                continue;
            unsigned join = left.successors[0], rhs = left.successors[1];
            const auto destination = ins.offset + ins.size() + static_cast<int16_t>(ins.value);
            if (binary.code[graph.blocks[join].begin].offset != destination)
                std::swap(join, rhs);
            bool disjunction = false;
            unsigned bypass = UINT32_MAX;
            const auto &candidate = graph.blocks[rhs];
            if (predecessors[rhs] == 1 && candidate.effects.size() == 1 && candidate.effects[0].kind == ek::branch &&
                binary.code[candidate.effects[0].instruction].opcode == OP_BRA && candidate.successors.size() == 1) {
                bypass = rhs;
                rhs = join;
                join = candidate.successors[0];
                disjunction = true;
            }
            auto &right = graph.blocks[rhs];
            auto &follow = graph.blocks[join];
            if (rhs == b || join == b || predecessors[rhs] != 1 || right.successors != std::vector<unsigned>{join} ||
                follow.effects.empty())
                continue;
            const auto &facts = source.stack.functions[index];
            if (!facts.heights[branch.instruction])
                continue;
            const unsigned word = (*facts.heights[branch.instruction] - 4) / 4;
            if (word >= follow.incoming.size() || word >= right.outgoing.size())
                continue;
            const auto result = follow.incoming[word], lhs = branch.arguments[0][0], rvalue = right.outgoing[word];
            if (result.offset || graph.values[result.value].kind != vk::merge ||
                graph.values[result.value].width != 4 || lhs == rvalue)
                continue;
            bool valid = true;
            unsigned consumers = 0;
            for (const auto &block : graph.blocks)
                for (const auto &e : block.effects)
                    if (e.kind != ek::value) {
                        for (const auto &args : e.arguments)
                            for (auto ref : args)
                                if (ref.value == result.value) {
                                    if (e.kind != ek::branch || (binary.code[e.instruction].opcode != OP_BF &&
                                                                 binary.code[e.instruction].opcode != OP_FEQZB))
                                        valid = false;
                                    ++consumers;
                                }
                    }
            for (const auto &e : follow.effects)
                if (e.kind == ek::value) {
                    const auto &v = graph.values[e.value];
                    if (v.kind == vk::operation && binary.code[v.instruction].opcode == OP_LNT &&
                        v.arguments == std::vector<std::vector<value_ref_t>>{{result}})
                        ++consumers;
                }
            if (!valid || consumers != 1)
                continue;
            std::set<unsigned> dependencies;
            std::function<void(unsigned)> depend = [&](unsigned id) {
                if (!dependencies.insert(id).second)
                    return;
                const auto &v = graph.values[id];
                for (const auto &args : v.arguments)
                    for (auto ref : args)
                        depend(ref.value);
                if (v.address) {
                    if (v.address->receiver)
                        depend(v.address->receiver->value);
                    if (v.address->index)
                        depend(v.address->index->value);
                }
            };
            depend(rvalue.value);
            for (const auto &e : right.effects) {
                if (e.kind == ek::branch) {
                    if (binary.code[e.instruction].opcode != OP_BRA)
                        valid = false;
                } else if (e.kind != ek::value || (effectful(e.value) && !dependencies.count(e.value)))
                    valid = false;
            }
            for (unsigned pc = right.begin; pc < right.end; ++pc)
                if (binary.code[pc].opcode == OP_POP && binary.code[pc].argument != OP_ARG_NULL)
                    valid = false;
            for (unsigned i = 0; i < word && i < follow.incoming.size(); ++i)
                if (i < left.outgoing.size() && i < right.outgoing.size() && left.outgoing[i] != right.outgoing[i])
                    valid = false;
            if (!valid)
                continue;
            auto combined_result = result;
            if (predecessors[join] > 2) {
                combined_result = {static_cast<unsigned>(graph.values.size()), 0};
                graph.values.push_back(graph.values[result.value]);
            }
            auto &v = graph.values[combined_result.value];
            v.kind = vk::operation;
            v.instruction = right.end - 1;
            v.arguments = {{lhs}, {rvalue}};
            v.address.reset();
            v.type = "num";
            short_circuit_values.insert(combined_result.value);
            if (disjunction)
                disjunction_values.insert(combined_result.value);
            left.effects.pop_back();
            for (const auto &e : right.effects)
                if (e.kind != ek::branch)
                    left.effects.push_back(e);
            lifted_effect_t combined;
            combined.kind = ek::value;
            combined.instruction = v.instruction;
            combined.value = combined_result.value;
            left.effects.push_back(combined);
            left.outgoing = right.outgoing;
            left.outgoing[word] = combined_result;
            left.successors = {join};
            left.end = right.end;
            reachable[rhs] = false;
            right.successors.clear();
            right.effects.clear();
            if (bypass != UINT32_MAX) {
                reachable[bypass] = false;
                graph.blocks[bypass].successors.clear();
                graph.blocks[bypass].effects.clear();
            }
            if (combined_result != result) {
                for (unsigned slot = 0; slot < follow.incoming.size(); ++slot) {
                    const auto ref = follow.incoming[slot];
                    auto &phi = graph.values[ref.value];
                    if (ref.offset || phi.kind != vk::merge || phi.width != 4)
                        continue;
                    std::vector<std::vector<value_ref_t>> inputs;
                    bool changed_input = false;
                    for (unsigned pred = 0; pred < graph.blocks.size(); ++pred)
                        if (reachable[pred]) {
                            const auto &incoming = graph.blocks[pred];
                            if (std::find(incoming.successors.begin(), incoming.successors.end(), join) !=
                                    incoming.successors.end() &&
                                slot < incoming.outgoing.size()) {
                                inputs.push_back({incoming.outgoing[slot]});
                                changed_input |= incoming.outgoing[slot] != ref;
                            }
                        }
                    if (changed_input)
                        phi.arguments = std::move(inputs);
                }
            }
            changed = true;
        }
    }

    for (bool fused = true; fused;) {
        fused = false;
        for (unsigned b = 0; b < graph.blocks.size(); ++b) {
            if (!reachable[b])
                continue;
            auto &left = graph.blocks[b];
            if (left.successors.size() != 1 || left.effects.empty() || left.effects.back().kind != ek::value ||
                !short_circuit_values.count(left.effects.back().value))
                continue;
            const auto join = left.successors[0];
            auto &follow = graph.blocks[join];

            unsigned live_predecessors = 0;
            for (unsigned source = 0; source < graph.blocks.size(); ++source)
                if (reachable[source])
                    for (auto successor : graph.blocks[source].successors)
                        if (successor == join)
                            ++live_predecessors;
            if (join != b && join != 0 && live_predecessors == 1) {
                std::map<unsigned, value_ref_t> replacements;
                for (unsigned i = 0; i < follow.incoming.size() && i < left.outgoing.size(); ++i) {
                    const auto ref = follow.incoming[i];
                    if (!ref.offset && graph.values[ref.value].kind == vk::merge &&
                        graph.values[ref.value].width == 4 && ref != left.outgoing[i])
                        replacements.emplace(ref.value, left.outgoing[i]);
                }
                auto rewrite = [&](value_ref_t &ref) {
                    auto found = replacements.find(ref.value);
                    if (found != replacements.end())
                        ref = {found->second.value, found->second.offset + ref.offset};
                };
                auto refs = [&](auto &values) {
                    for (auto &ref : values)
                        rewrite(ref);
                };
                auto address = [&](auto &a) {
                    if (a) {
                        if (a->receiver)
                            rewrite(*a->receiver);
                        if (a->index)
                            rewrite(*a->index);
                    }
                };
                for (auto &value : graph.values) {
                    for (auto &args : value.arguments)
                        refs(args);
                    address(value.address);
                }
                for (auto &block : graph.blocks) {
                    refs(block.incoming);
                    refs(block.outgoing);
                    if (block.incoming_array_index)
                        rewrite(*block.incoming_array_index);
                    if (block.outgoing_array_index)
                        rewrite(*block.outgoing_array_index);
                    for (auto &e : block.effects) {
                        for (auto &args : e.arguments)
                            refs(args);
                        address(e.address);
                    }
                }
                left.effects.insert(left.effects.end(), follow.effects.begin(), follow.effects.end());
                left.end = follow.end;
                left.successors = follow.successors;
                left.outgoing = follow.outgoing;
                reachable[join] = false;
                follow.effects.clear();
                follow.successors.clear();
                fused = true;
            }
        }
    }
    if (std::count(reachable.begin(), reachable.end(), true) < old_live)
        recover_short_circuits();
}

void source_emitter_t::body_t::emit()
{
    using value_kind = lifted_value_t::kind_t;
    using effect_kind = lifted_effect_t::kind_t;
    std::function<void(unsigned)> visit = [&](unsigned block) {
        if (reachable[block])
            return;
        reachable[block] = true;
        for (auto next : graph.blocks[block].successors)
            visit(next);
    };
    visit(0);
    recover_local_arrays();
    collapse_trivial_merges();
    recover_native_values();
    recover_short_circuits();
    used.resize(graph.values.size());
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        for (const auto &effect : graph.blocks[b].effects) {
            if (effect.kind == effect_kind::value) {
                const auto &value = graph.values[effect.value];
                if (value.kind == value_kind::indexed_store || value.kind == value_kind::signal ||
                    (value.kind == value_kind::operation && binary.code[value.instruction].opcode == OP_POP))
                    use({effect.value, 0});
                else if (effectful(effect.value)) {
                    for (const auto &argument : value.arguments)
                        for (auto ref : argument)
                            use(ref);
                    if (value.address) {
                        if (value.address->receiver)
                            use(*value.address->receiver);
                        if (value.address->index)
                            use(*value.address->index);
                    }
                }
            } else {
                const auto count = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
                for (unsigned a = 0; a < count; ++a)
                    for (auto ref : effect.arguments[a])
                        use(ref);
                if (effect.address) {
                    if (effect.address->receiver)
                        use(*effect.address->receiver);
                    if (effect.address->index)
                        use(*effect.address->index);
                }
            }
        }
    }

    if (graph.blocks.size() > 1 && local_arrays.empty()) {
        std::map<unsigned, unsigned> constants;
        for (unsigned id = 0; id < graph.values.size(); ++id)
            if ((graph.values[id].kind == value_kind::string || graph.values[id].kind == value_kind::constant) &&
                graph.values[id].width == 4)
                constants.emplace(graph.values[id].instruction, id);
        const auto &facts = source.stack.functions[index];
        unsigned serial = 0;
        for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
            const auto &ins = binary.code[pc];
            if (ins.opcode == OP_SPA && static_cast<int16_t>(ins.value) > 0)
                continue;
            if (ins.opcode != OP_PSH || (ins.argument != OP_ARG_NUM && ins.argument != OP_ARG_STR) ||
                ins.data_size() != 4)
                break;
            if (!constants.count(pc) || used[constants.at(pc)] || !facts.heights[pc])
                continue;
            const int slot = *facts.heights[pc];
            if (slot < static_cast<int>(binary.parameters_size))
                continue;
            bool safe = true;
            std::vector<std::pair<unsigned, unsigned>> writes;
            for (unsigned j = pc + 1; j < binary.code.size(); ++j) {
                if (!facts.heights[j])
                    continue;
                const auto &op = binary.code[j];
                if (*facts.heights[j] < slot + 4) {
                    if (j && binary.code[j - 1].opcode == OP_SPA && static_cast<int16_t>(binary.code[j - 1].value) < 0)
                        break;
                    safe = false;
                    break;
                }
                if (op.opcode == OP_ASF || op.opcode == OP_PSF || op.opcode == OP_COFF) {
                    safe = false;
                    break;
                }
                if (op.argument != OP_ARG_SPR)
                    continue;
                const int from = *facts.heights[j] + static_cast<int16_t>(op.value);
                if (from >= slot + 4 || from + static_cast<int>(op.data_size()) <= slot)
                    continue;
                if (op.opcode != OP_POP || from != slot || op.data_size() != 4 || !j || !constants.count(j - 1) ||
                    used[constants.at(j - 1)]) {
                    safe = false;
                    break;
                }
                writes.emplace_back(j, binary.code[j - 1].value >> 16);
            }
            if (!safe || (ins.argument == OP_ARG_NUM && !writes.empty()))
                continue;
            const auto name = "unused_local" + std::to_string(serial++);
            recovered_local_operations[pc].push_back(
                (ins.argument == OP_ARG_STR ? "str " : "num ") + name + " = " +
                (ins.argument == OP_ARG_STR ? source.text(ins.value >> 16) : number_literal(ins.value)) + ";");
            for (const auto &[at, id] : writes)
                recovered_local_operations[at].push_back(name + " = " + source.text(id) + ";");
        }
    }

    if (graph.blocks.size() > 1 && local_arrays.empty()) {
        const auto &facts = source.stack.functions[index];
        std::map<unsigned, unsigned> constants;
        for (unsigned id = 0; id < graph.values.size(); ++id)
            if (graph.values[id].kind == value_kind::string && graph.values[id].width == 4)
                constants.emplace(graph.values[id].instruction, id);
        std::vector<unsigned> consumed(binary.code.size());
        for (const auto &v : graph.values) {
            if (v.instruction >= binary.code.size() || v.kind == value_kind::merge ||
                v.kind == value_kind::uninitialized || v.kind == value_kind::parameter)
                continue;
            unsigned bytes = 0;
            for (const auto &args : v.arguments)
                bytes += static_cast<unsigned>(args.size() * 4);
            if (v.address) {
                if (v.address->receiver)
                    bytes += 4;
                if (v.address->index)
                    bytes += 4;
            }
            consumed[v.instruction] = std::max(consumed[v.instruction], bytes);
        }
        unsigned serial = 0;
        for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
            const auto &ins = binary.code[pc];
            if (ins.opcode != OP_PSH || (ins.argument != OP_ARG_NUM && ins.argument != OP_ARG_STR) ||
                ins.data_size() != 4)
                continue;
            if (ins.argument != OP_ARG_STR || !facts.heights[pc] || !constants.count(pc) || !used[constants.at(pc)])
                continue;
            const int slot = *facts.heights[pc];
            if (slot < static_cast<int>(binary.parameters_size))
                continue;
            bool safe = true, read = false;
            for (unsigned j = pc + 1; j < binary.code.size(); ++j) {
                if (!facts.heights[j])
                    continue;
                const auto &op = binary.code[j];
                if (op.opcode == OP_ASF || op.opcode == OP_PSF || op.opcode == OP_COFF) {
                    safe = false;
                    break;
                }

                if (*facts.heights[j] >= slot + 4 && consumed[j] &&
                    *facts.heights[j] - static_cast<int>(consumed[j]) < slot + 4) {
                    safe = false;
                    break;
                }
                if (op.argument != OP_ARG_SPR)
                    continue;
                const int from = *facts.heights[j] + static_cast<int16_t>(op.value);
                if (from >= slot + 4 || from + static_cast<int>(op.data_size()) <= slot)
                    continue;
                if (op.opcode != OP_PSH || from != slot || op.data_size() != 4) {
                    safe = false;
                    break;
                }
                read = true;
            }
            if (!safe || !read)
                continue;
            const auto name = "retained_text" + std::to_string(serial++);
            recovered_local_operations[pc].push_back("str " + name + " = " + source.text(ins.value >> 16) + ";");
            recovered_local_names.emplace(constants.at(pc), std::make_pair(name, "str"));
        }
    }
    for (auto &block : graph.blocks) {
        block.effects.erase(std::remove_if(block.effects.begin(),
                                           block.effects.end(),
                                           [&](const auto &e) {
                                               if (e.kind != effect_kind::value || used[e.value] || effectful(e.value))
                                                   return false;
                                               const auto &v = graph.values[e.value];
                                               return v.kind == value_kind::load &&
                                                      v.instruction + 1 < binary.code.size() &&
                                                      binary.code[v.instruction].opcode == OP_PSH &&
                                                      binary.code[v.instruction + 1].opcode == OP_COFF;
                                           }),
                            block.effects.end());
    }
    allocate();
    prettify();
    if (emit_linear())
        return;

    std::vector<unsigned> uses(graph.values.size());
    auto count_group = [&](const auto &refs) {
        std::set<unsigned> ids;
        for (auto ref : refs)
            ids.insert(ref.value);
        for (auto id : ids)
            ++uses[id];
    };
    auto count_address = [&](const auto &a) {
        if (a) {
            if (a->receiver)
                ++uses[a->receiver->value];
            if (a->index)
                ++uses[a->index->value];
        }
    };
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (used[id] && graph.values[id].kind == value_kind::merge)
            for (const auto &args : graph.values[id].arguments)
                count_group(args);
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        for (const auto &e : graph.blocks[b].effects) {
            if (e.kind == effect_kind::value) {
                if (!used[e.value] && !effectful(e.value))
                    continue;
                const auto &v = graph.values[e.value];
                for (const auto &args : v.arguments)
                    count_group(args);
                count_address(v.address);
            } else {
                const auto count = e.arguments.size() - (e.kind == effect_kind::callback ? 1 : 0);
                for (unsigned a = 0; a < count; ++a)
                    count_group(e.arguments[a]);
                count_address(e.address);
            }
        }
    }
    adjacent_expressions.resize(graph.values.size());
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        const auto &effects = graph.blocks[b].effects;
        std::map<unsigned, unsigned> producer;
        for (unsigned i = 0; i < effects.size(); ++i)
            if (effects[i].kind == effect_kind::value &&
                graph.values[effects[i].value].kind != value_kind::indexed_store &&
                !array_values.count(effects[i].value))
                producer.emplace(effects[i].value, i);
        for (unsigned sink = 0; sink < effects.size(); ++sink) {
            const auto &effect = effects[sink];

            std::vector<unsigned> sequence;
            std::set<unsigned> visiting;
            std::function<void(const std::vector<value_ref_t> &)> collect;
            auto collect_address = [&](const auto &a) {
                if (a) {
                    if (a->index)
                        collect({*a->index});
                    if (a->receiver)
                        collect({*a->receiver});
                }
            };
            collect = [&](const std::vector<value_ref_t> &refs) {
                if (refs.empty())
                    return;
                bool composite = false;
                for (unsigned i = 1; i < refs.size(); ++i)
                    if (refs[i].value != refs[0].value || refs[i].offset != refs[0].offset + i * 4)
                        composite = true;
                if (composite) {
                    for (unsigned first = 0; first < refs.size();) {
                        unsigned end = first + 1;
                        while (end < refs.size() && refs[end].value == refs[first].value &&
                               refs[end].offset == refs[first].offset + (end - first) * 4)
                            ++end;
                        collect(std::vector<value_ref_t>(refs.begin() + first, refs.begin() + end));
                        first = end;
                    }
                    return;
                }
                auto ref = refs.front();
                auto it = producer.find(ref.value);
                if (it == producer.end() || it->second >= sink || uses[ref.value] != 1 || ref.offset)
                    return;
                const auto &v = graph.values[ref.value];
                if (v.width != refs.size() * 4)
                    return;
                if (binary.code[v.instruction].opcode == OP_POP && binary.code[v.instruction].argument == OP_ARG_SPR)
                    return;

                const auto &stack_facts = source.stack.functions[index];
                if (v.instruction + 1 < stack_facts.heights.size() && stack_facts.heights[v.instruction + 1]) {
                    const auto begin = *stack_facts.heights[v.instruction + 1] - static_cast<int>(v.width);
                    for (unsigned pc = v.instruction + 1; pc < effects[sink].instruction; ++pc) {
                        const auto &ins = binary.code[pc];
                        if (ins.opcode == OP_POP && ins.argument == OP_ARG_SPR)
                            return;
                        if (ins.opcode != OP_PSH || ins.argument != OP_ARG_SPR || !stack_facts.heights[pc])
                            continue;
                        const auto from = *stack_facts.heights[pc] + static_cast<std::int16_t>(ins.value);
                        if (from < begin + static_cast<int>(v.width) &&
                            from + static_cast<int>(ins.data_size()) > begin)
                            return;
                    }
                }

                for (unsigned word = 0; word < refs.size(); ++word)
                    if (refs[word] != value_ref_t{ref.value, word * 4})
                        return;
                if (!visiting.insert(ref.value).second)
                    return;
                for (const auto &args : v.arguments)
                    collect(args);
                collect_address(v.address);
                sequence.push_back(it->second);
            };
            if (effect.kind == effect_kind::value) {
                for (const auto &args : graph.values[effect.value].arguments)
                    collect(args);
                collect_address(graph.values[effect.value].address);
            } else {
                const auto count = effect.arguments.size() - (effect.kind == effect_kind::callback ? 1 : 0);
                if (effect.kind == effect_kind::callback || effect.kind == effect_kind::thread) {
                    for (unsigned a = count; a > 0; --a)
                        collect(effect.arguments[a - 1]);
                } else
                    for (unsigned a = 0; a < count; ++a)
                        collect(effect.arguments[a]);
                collect_address(effect.address);
            }
            if (sequence.empty() || sequence.size() > sink)
                continue;
            bool adjacent = true;
            for (unsigned j = 0; j < sequence.size(); ++j)
                if (sequence[j] != sink - sequence.size() + j)
                    adjacent = false;
            if (adjacent)
                for (auto position : sequence)
                    adjacent_expressions[effects[position].value] = true;
        }
    }
    for (auto id : short_circuit_values) {
        const auto pc = graph.values[id].instruction + 1;
        adjacent_expressions[id] =
            !(pc < binary.code.size() && binary.code[pc].opcode == OP_POP && binary.code[pc].argument == OP_ARG_SPR);
    }
    for (const auto &[id, old] : postfix_locals)
        adjacent_expressions[id] = false;
    required_slots.resize(slot_values.size());
    for (unsigned id = 0; id < graph.values.size(); ++id)
        if (stored(id) && !adjacent_expressions[id])
            required_slots[slots[id]] = true;
    if (emit_structured())
        return;
    for (auto id : slot_values)
        declaration(id);
    entry_values();
    for (unsigned b = 0; b < graph.blocks.size(); ++b) {
        if (!reachable[b])
            continue;
        const auto &block = graph.blocks[b];
        source.out << scoped_line(indent) << "block" << b << ":\n" << indent << "{\n";
        const auto line = indent + "    ";
        bool ended = false;
        for (const auto &effect : block.effects) {
            ended = emit_effect(b, effect, line) || ended;
        }
        if (!ended && !block.successors.empty())
            edge(b, block.successors[0], line);
        source.out << scoped_line(indent) << "}\n";
    }
}

void source_emitter_t::function(unsigned index, std::string_view indentation, bool declaration_only, bool qualified)
{
    const auto &graph = graphs[index];
    const auto &record = binary(index);
    const auto &signature = types.functions[index];
    const bool constructor =
        !graph.initializer && graph.function == 0 && record.name_hash == image.objects[graph.object].name_hash;
    const bool destructor =
        !graph.initializer && static_cast<int>(graph.function) == image.objects[graph.object].destructor;
    body_t body(*this, index, std::string(indentation) + "    ");
    const auto base_initializer = declaration_only ? std::string{} : body.lifetime_initializer(constructor, destructor);
    out << indentation;
    if (graph.initializer)
        out << "[](";
    else {
        if (!qualified && (record.flags & 1u) && !(image.objects[graph.object].flags & 1u))
            out << "static ";
        if (!constructor && !destructor)
            out << type(signature.result) << ' ';
        if (qualified)
            out << classes[graph.object] << "::";
        out << names[graph.object][graph.function] << '(';
    }
    bool comma = false;
    if (!graph.initializer)
        for (const auto &parameter : signature.parameters) {
            if (!(record.flags & 1u) && !parameter.offset)
                continue;
            if (comma)
                out << ", ";
            comma = true;
            out << (!graph.initializer && graph.function == 0 && graph.object == 0 ? "num" : type(parameter.type))
                << " p" << parameter.offset;
        }
    out << ')';
    if (graph.initializer || (constructor && signature.result != "void"))
        out << " -> " << type(signature.result);
    if (declaration_only) {
        out << ";\n";
        return;
    }
    out << base_initializer;
    out << '\n';
    out << indentation << "{\n";
    body.emit();
    out << indentation << '}';
    if (graph.initializer)
        out << ';';
    out << "\n\n";
}

std::string source_emitter_t::emit()
{
    out << "namespace " << identifier(image.name, "script") << " {\n";
    for (const auto &[identity, variable] : variables)
        if (!variable.second.empty())
            out << "    extern " << type(variable.second) << ' ' << variable.first
                << (variable_arrays.count(identity) ? "[]" : "") << ";\n";
    std::set<std::pair<unsigned, unsigned>> natives;
    for (unsigned i = 0; i < graphs.size(); ++i)
        for (const auto &instruction : binary(i).code)
            if (instruction.opcode == OP_BSL)
                natives.emplace(instruction.value >> 16, instruction.value & 0xffff);
    for (const auto &identity : natives) {
        const auto &native = symbols.natives.at(identity);
        out << "    extern " << native_result(identity) << ' ' << native.class_name << "::" << native.signature
            << ";\n";
    }

    std::map<bytes_t, std::vector<unsigned>> identities;
    for (unsigned i = 0; i < image.strings.size(); ++i)
        identities[image.strings[i]].push_back(i);
    std::set<std::string> string_names;
    for (const auto &[bytes, ids] : identities)
        if (ids.size() > 1) {
            for (auto id : ids) {
                std::string base = "constant_text";
                for (unsigned g = 0; g < graphs.size(); ++g) {
                    bool used = false;
                    for (const auto &ins : binary(g).code) {
                        if ((ins.argument == OP_ARG_STR || ins.argument == OP_ARG_SIG || ins.argument == OP_ARG_PSIG) &&
                            ins.value >> 16 == id)
                            used = true;
                        if (ins.argument == OP_ARG_CLV && (ins.value & 0xffff) == id)
                            used = true;
                    }
                    if (used) {
                        base = identifier(std::string_view(binary(g).name).substr(0, binary(g).name.find('(')),
                                          "constant") +
                               "_text";
                        break;
                    }
                }
                auto name = base;
                for (unsigned suffix = 2; !string_names.insert(name).second; ++suffix)
                    name = base + "_" + std::to_string(suffix);
                out << "    static const char " << name << "[] = " << text(id) << ";\n";
                string_objects.emplace(id, std::move(name));
            }
        }
    out << '\n';
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        const auto &record = image.objects[object];
        out << "    class " << classes[object];
        if (parents[object])
            out << " : public " << type(*parents[object]);
        out << "\n    {\n    public:\n";
        emit_static_fields(object);
        unsigned cursor = 0;
        if (parents[object])
            for (const auto &parent : image.objects)
                if (parent.name == *parents[object])
                    cursor = parent.instance_size;
        for (const auto &[offset, field] : fields[object]) {
            if (field_owner(object, offset, field.width) != object)
                continue;
            if (offset > cursor)
                out << "        num unused_" << cursor << '[' << (offset - cursor) / 4 << "];\n";
            out << "        " << type(field.type) << " field_" << offset;
            if (auto array = instance_arrays[object].find(offset); array != instance_arrays[object].end())
                out << '[' << array->second.count << ']';
            out << ";\n";
            cursor = offset + field.width;
        }
        if (record.instance_size > cursor)
            out << "        num unused_" << cursor << '[' << (record.instance_size - cursor) / 4 << "];\n";
        for (unsigned f = 0; f < record.functions.size(); ++f)
            function(functions.at({object, f}), "        ", true);
        out << "    };\n\n";
    }
    std::vector<unsigned> definitions;
    for (unsigned i = 0; i < graphs.size(); ++i)
        if (!graphs[i].initializer)
            definitions.push_back(i);
    std::stable_sort(definitions.begin(), definitions.end(), [&](unsigned a, unsigned b) {
        return binary(a).code_order < binary(b).code_order;
    });

    std::stable_sort(text_definitions.begin(), text_definitions.end(), [](const auto &a, const auto &b) {
        return a.second < b.second;
    });
    unsigned next_text = 0;
    std::set<unsigned> encountered_text;
    auto emit_text_before = [&](unsigned boundary) {
        while (next_text < text_definitions.size() && text_definitions[next_text].second <= boundary) {
            const auto [offset, string_index] = text_definitions[next_text++];
            out << "    str " << classes[0] << "::" << shared_names[0].at(offset) << " = " << text(string_index)
                << ";\n\n";
            encountered_text.insert(string_index);
        }
    };
    std::stable_sort(builder_definitions.begin(), builder_definitions.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });
    unsigned next_builder = 0;
    for (auto index : definitions) {
        while (next_builder < builder_definitions.size() &&
               builder_definitions[next_builder].first < binary(index).code_order)
            out << builder_definitions[next_builder++].second;
        unsigned first = UINT32_MAX;
        std::vector<unsigned> referenced;
        for (const auto &ins : binary(index).code) {
            std::optional<unsigned> id;
            if (ins.argument == OP_ARG_STR || ins.argument == OP_ARG_SIG || ins.argument == OP_ARG_PSIG)
                id = ins.value >> 16;
            else if (ins.argument == OP_ARG_CLV)
                id = ins.value & 0xffff;
            if (id) {
                referenced.push_back(*id);
                if (!encountered_text.count(*id))
                    first = std::min(first, *id);
            }
        }
        if (first != UINT32_MAX)
            emit_text_before(first);
        function(index, "    ", false, true);
        encountered_text.insert(referenced.begin(), referenced.end());
    }
    while (next_builder < builder_definitions.size())
        out << builder_definitions[next_builder++].second;
    emit_text_before(UINT32_MAX);

    out << "}\n";
    return out.str();
}

}

std::string emit_decompiled_source(const image_t &image, const symbols_t &symbols)
{
    return source_emitter_t(image, symbols).emit();
}

}
