#include "language.h"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <functional>
#include <limits>

namespace sxcc::language {
namespace {

struct value_type_t {
    std::string name;
    unsigned count = 1;
    bool is_array = false;
    bool array() const
    {
        return is_array || count != 1;
    }
};

struct field_t {
    value_type_t type;
    unsigned owner = 0;
    unsigned offset = 0;
    bool is_static = false;
    access_t access = access_t::private_member;
};

class compiler_t {
    program_t program;
    const symbols_t &symbols;
    std::map<std::string, unsigned, std::less<>> classes;
    std::vector<std::map<std::string, field_t, std::less<>>> fields;
    std::vector<unsigned> lookup_index;
    std::map<const native_t *, std::string> native_results;
    std::map<std::string, unsigned> string_object_indices;
    std::set<unsigned> named_string_indices;
    unsigned intern_object(const std::string &name)
    {
        auto found = string_object_indices.find(name);
        if (found != string_object_indices.end())
            return found->second;
        const auto &text = program.string_objects.at(name);
        auto &strings = program.image.strings;
        if (strings.size() == UINT16_MAX)
            throw error_t("Permanent string table exceeds VM indices");
        const auto index = static_cast<unsigned>(strings.size());
        bytes_t bytes(text.begin(), text.end());
        bytes.push_back(0);
        strings.push_back(std::move(bytes));
        string_object_indices.emplace(name, index);
        named_string_indices.insert(index);
        return index;
    }
    struct text_initializer_t {
        unsigned index, line;
        std::string text;
    };
    std::vector<text_initializer_t> text_initializers;
    unsigned next_text_initializer = 0;
    void initialize_text_before(unsigned line)
    {
        while (next_text_initializer < text_initializers.size() &&
               text_initializers[next_text_initializer].line < line) {
            const auto &source = text_initializers[next_text_initializer++];
            program.image.initializers[source.index].kind = static_cast<int>(intern(source.text));
        }
    }

    unsigned size(const value_type_t &type) const
    {
        if (type.name == "nullptr")
            return 4;
        unsigned element = 0;
        if (type.name == "void") {
            if (type.array())
                throw error_t("An array cannot have void elements");
            return 0;
        }
        auto found = symbols.type_sizes.find(type.name);
        if (found != symbols.type_sizes.end())
            element = found->second;
        else if (classes.count(type.name))
            element = sizeof(std::uint32_t);
        else if (type.name.starts_with("opaque<") && type.name.back() == '>') {
            auto parsed = std::from_chars(type.name.data() + 7, type.name.data() + type.name.size() - 1, element);
            if (parsed.ec != std::errc{} || parsed.ptr != type.name.data() + type.name.size() - 1 || !element ||
                element % 4)
                throw error_t("Invalid internal argument storage type " + type.name);
        } else
            throw error_t("Unknown type " + type.name);
        if (!element)
            throw error_t("A library namespace is not a value type: " + type.name);
        if (!type.count || element > UINT16_MAX / type.count)
            throw error_t("Value exceeds a VM transfer width");
        return element * type.count;
    }

    bool assignable(const value_type_t &from, const value_type_t &to) const
    {
        if (from.count != to.count || from.array() != to.array())
            return false;
        if (from.name == to.name)
            return true;
        if (from.array())
            return false;
        if (from.name == "nullptr")
            return to.name != "num" && to.name != "void" && size(to) == 4;
        std::set<std::string> visited;
        auto current = from.name;
        while (visited.insert(current).second) {
            auto script = classes.find(current);
            if (script != classes.end()) {
                auto &parent = program.image.objects[script->second].parent;
                if (!parent)
                    return false;
                current = *parent;
            } else {
                auto parent = symbols.parents.find(current);
                if (parent == symbols.parents.end())
                    return false;
                current = parent->second;
            }
            if (current == to.name)
                return true;
        }
        return false;
    }

    void require(const value_type_t &actual, const value_type_t &expected) const
    {
        if (!assignable(actual, expected))
            throw error_t("Cannot use " + actual.name + " as " + expected.name);
    }

    void layout_parameters(const language::function_t &function, sxcc::function_t &binary, bool instance)
    {
        unsigned parameters = instance ? sizeof(std::uint32_t) : 0;
        std::set<std::string> names;
        for (const auto &parameter : function.parameters) {
            auto width = size({parameter.type, parameter.count});
            if (!width || !names.insert(parameter.name).second)
                throw error_t("Invalid or duplicate function parameter");
            if (parameters > UINT16_MAX - width)
                throw error_t("Parameter storage exceeds VM addressing");
            parameters += width;
        }
        binary.parameters_size = parameters;
        size({function.result});
    }

    std::uint64_t next_code_order() const
    {
        std::uint64_t order = 0;
        for (const auto &object : program.image.objects)
            for (const auto &function : object.functions)
                order = std::max(order, static_cast<std::uint64_t>(function.code_order) + 1);
        for (const auto &initializer : program.image.initializers)
            if (initializer.builder)
                order = std::max(order, static_cast<std::uint64_t>(initializer.builder->code_order) + 1);
        return order;
    }

    void prepare_constructors()
    {
        auto next_order = next_code_order();
        std::vector<bool> constructed(program.classes.size());
        std::function<void(const expression_t &)> inspect_expression = [&](const expression_t &expression) {
            if (expression.kind == expression_t::kind_t::call && !expression.operands.empty()) {
                const auto &callee = *expression.operands.front();
                if (callee.kind == expression_t::kind_t::name) {
                    if (auto found = classes.find(callee.text); found != classes.end())
                        constructed[found->second] = true;
                } else if (callee.kind == expression_t::kind_t::member && callee.bits &&
                           callee.text == "create_named" && callee.operands.size() == 1) {
                    if (auto found = classes.find(callee.operands.front()->text); found != classes.end())
                        constructed[found->second] = true;
                }
            }
            for (const auto &operand : expression.operands)
                inspect_expression(*operand);
        };
        std::function<void(const statement_t &)> inspect_statement = [&](const statement_t &statement) {
            for (const auto &expression : statement.expressions)
                inspect_expression(*expression);
            for (const auto &child : statement.children)
                inspect_statement(*child);
        };
        for (const auto &owner : program.classes) {
            constructed[owner.index] = constructed[owner.index] ||
                                       (program.image.objects[owner.index].flags & 1u) != 0 || owner.functions.empty();
            for (const auto &function : owner.functions) {
                constructed[owner.index] = constructed[owner.index] || function.constructor;
                for (const auto &argument : function.base_arguments)
                    inspect_expression(*argument);
                inspect_statement(*function.body);
            }
            for (const auto &[name, initializer] : owner.field_initializers) {
                inspect_expression(*initializer);
                for (const auto &field : owner.fields)
                    if (field.name == name && !field.is_static)
                        constructed[owner.index] = true;
            }
        }
        std::function<void(unsigned)> require_base = [&](unsigned index) {
            const auto &parent = program.image.objects[index].parent;
            if (!parent)
                return;
            auto found = classes.find(*parent);
            if (found == classes.end() || constructed[found->second])
                return;
            auto ancestor = program.image.objects[found->second].parent;
            for (unsigned depth = 0; ancestor && depth < program.classes.size(); ++depth) {
                auto base = classes.find(*ancestor);
                if (base == classes.end())
                    return;
                if (constructed[base->second]) {
                    constructed[found->second] = true;
                    require_base(found->second);
                    return;
                }
                ancestor = program.image.objects[base->second].parent;
            }
        };
        for (unsigned index = 0; index < constructed.size(); ++index)
            if (constructed[index])
                require_base(index);
        auto append = [&](unsigned index, bool constructor) {
            auto &object = program.image.objects[index];
            auto &owner = program.classes[index];
            language::function_t function;
            function.index = static_cast<unsigned>(owner.functions.size());
            function.name = constructor ? object.name : '~' + object.name;
            function.constructor = constructor;
            function.access = access_t::public_member;
            function.destructor = !constructor;
            function.body = std::make_unique<statement_t>();
            sxcc::function_t record;
            record.flags = 4u | (object.flags & 1u);
            record.name = function.name + '(';
            if (constructor && (object.flags & 1u)) {
                declaration_t reason;
                reason.type = "num";
                reason.name = "startup_reason";
                function.parameters.push_back(std::move(reason));
                record.name += "num";
            }
            record.name += ')';
            record.name_hash = name_hash(function.name);
            record.fullname_hash = name_hash(record.name);
            if (next_order > UINT32_MAX)
                throw error_t("Function encoding order overflows");
            record.code_order = static_cast<unsigned>(next_order++);
            owner.functions.push_back(std::move(function));
            object.functions.push_back(std::move(record));
        };
        for (auto &owner : program.classes) {
            auto &object = program.image.objects[owner.index];
            if (!constructed[owner.index] && !owner.functions.empty())
                owner.functions.front().entry = true;
            unsigned constructor = UINT32_MAX;
            for (unsigned i = 0; i < owner.functions.size(); ++i) {
                if (!owner.functions[i].constructor && !owner.functions[i].entry)
                    continue;
                if (constructor != UINT32_MAX)
                    throw error_t("The retail VM permits one constructor per class");
                constructor = i;
            }
            if (constructor == UINT32_MAX) {
                constructor = static_cast<unsigned>(owner.functions.size());
                append(owner.index, true);
            }

            std::rotate(owner.functions.begin(),
                        owner.functions.begin() + constructor,
                        owner.functions.begin() + constructor + 1);
            std::rotate(object.functions.begin(),
                        object.functions.begin() + constructor,
                        object.functions.begin() + constructor + 1);
            object.destructor = -1;
            for (unsigned i = 0; i < owner.functions.size(); ++i) {
                owner.functions[i].index = i;
                if (owner.functions[i].destructor &&
                    (!owner.functions[i].parameters.empty() || ((object.functions[i].flags ^ object.flags) & 1u)))
                    throw error_t("A destructor cannot have explicit parameters or change its receiver convention");
                if (owner.functions[i].destructor)
                    object.destructor = static_cast<int>(i);
            }
            const auto &entry = owner.functions.front();
            if (object.flags & 1u) {
                if (entry.parameters.size() != 1 || entry.parameters[0].type != "num" ||
                    entry.parameters[0].count != 1 || entry.parameters[0].is_array || !(object.functions[0].flags & 1u))
                    throw error_t(
                        "The global constructor must accept one num startup reason and have no instance receiver");
            } else if (object.functions[0].flags & 1u) {
                throw error_t("An instance constructor cannot be static");
            }
        }
        std::vector<unsigned> state(program.classes.size());
        std::function<void(unsigned)> destructors = [&](unsigned index) {
            if (state[index] == 2)
                return;
            if (state[index] == 1)
                throw error_t("Inheritance cycle");
            state[index] = 1;
            auto &object = program.image.objects[index];
            if (object.parent) {
                auto found = classes.find(*object.parent);
                if (found == classes.end())
                    throw error_t("A script class requires a script-class base");
                if (program.image.objects[found->second].flags & 1u)
                    throw error_t("The global class cannot be inherited");
                destructors(found->second);
                if (object.destructor == -1 && program.image.objects[found->second].destructor != -1) {
                    object.destructor = static_cast<int>(object.functions.size());
                    append(index, false);
                }
            }
            state[index] = 2;
        };
        for (unsigned i = 0; i < program.classes.size(); ++i)
            destructors(i);
    }

    void layout()
    {
        auto &image = program.image;
        for (unsigned i = 0; i < program.classes.size(); ++i) {
            const auto &name = image.objects[i].name;
            if (symbols.type_sizes.count(name) || !classes.emplace(name, i).second)
                throw error_t("Duplicate or reserved class name " + name);
        }
        prepare_constructors();
        fields.resize(program.classes.size());
        std::vector<unsigned> state(program.classes.size());
        std::function<void(unsigned)> place = [&](unsigned index) {
            if (state[index] == 2)
                return;
            if (state[index] == 1)
                throw error_t("Inheritance cycle involving " + image.objects[index].name);
            state[index] = 1;
            auto &object = image.objects[index];
            unsigned instance_size = 0, static_size = 0;
            if (object.parent) {
                auto parent = classes.find(*object.parent);
                if (parent == classes.end())
                    throw error_t("Script instances cannot inherit a native handle layout: " + *object.parent);
                place(parent->second);
                fields[index] = fields[parent->second];
                instance_size = image.objects[parent->second].instance_size;
            }
            for (const auto &declaration : program.classes[index].fields) {
                field_t field{{declaration.type, declaration.count, declaration.is_array},
                              index,
                              0,
                              declaration.is_static,
                              declaration.access};
                auto width = size(field.type);
                if (!width)
                    throw error_t("A field cannot have void type");
                auto &cursor = field.is_static ? static_size : instance_size;
                field.offset = declaration.offset.value_or(cursor);
                if (field.offset % 4 || field.offset > UINT16_MAX || width > UINT16_MAX - field.offset)
                    throw error_t("Field exceeds the VM storage addressing range");
                if (!fields[index].emplace(declaration.name, field).second)
                    throw error_t("Duplicate or hidden field " + declaration.name);
                cursor = std::max(cursor, field.offset + width);
            }
            object.instance_size = std::max(object.instance_size, instance_size);
            if (object.flags & 1u) {
                if (object.instance_size)
                    throw error_t("The global class cannot have instance storage");
            }
            if (object.static_data.size() < static_size)
                object.static_data.resize(static_size);
            state[index] = 2;
        };
        for (unsigned i = 0; i < program.classes.size(); ++i)
            place(i);
        for (auto &owner : program.classes) {
            auto &object = image.objects[owner.index];
            std::set<std::uint32_t> signatures;
            for (auto &function : owner.functions) {
                auto &binary = object.functions[function.index];
                if (!signatures.insert(binary.fullname_hash).second)
                    throw error_t("Duplicate function signature in " + object.name);
                layout_parameters(function, binary, !(binary.flags & 1u));
            }
        }
        image.object_order.resize(image.objects.size());
        for (unsigned i = 0; i < image.objects.size(); ++i)
            image.object_order[i] = i;
        std::stable_sort(image.object_order.begin(), image.object_order.end(), [&](auto a, auto b) {
            return image.objects[a].name_hash < image.objects[b].name_hash;
        });
        lookup_index.resize(image.objects.size());
        for (unsigned i = 0; i < image.object_order.size(); ++i)
            lookup_index[image.object_order[i]] = i;
        for (const auto &declaration : program.natives) {
            const native_t *found = nullptr;
            for (const auto &[identity, native] : symbols.natives)
                if (native.class_name == declaration.class_name && native.signature == declaration.signature) {
                    found = &native;
                    break;
                }
            if (!found)
                throw error_t("Unknown native declaration " + declaration.class_name + "::" + declaration.signature);
            auto width = size({*declaration.result});
            if (found->result_bytes && width != *found->result_bytes)
                throw error_t("Native return type disagrees with the retail ABI: " + declaration.signature);
            if (!native_results.emplace(found, *declaration.result).second)
                throw error_t("Duplicate native declaration " + declaration.signature);
        }
    }

    unsigned intern(std::string_view text)
    {
        auto &strings = program.image.strings;
        for (unsigned i = 0; i < strings.size(); ++i)
            if (!named_string_indices.count(i) && strings[i].size() == text.size() + 1 && strings[i].back() == 0 &&
                (text.empty() || std::memcmp(text.data(), strings[i].data(), text.size()) == 0))
                return i;
        if (strings.size() == UINT16_MAX)
            throw error_t("Permanent string table exceeds VM indices");
        bytes_t bytes(text.begin(), text.end());
        bytes.push_back(0);
        strings.push_back(std::move(bytes));
        return static_cast<unsigned>(strings.size() - 1);
    }

    class function_compiler_t;
    void initialize_fields();

public:
    compiler_t(program_t source, const symbols_t &library) : program(std::move(source)), symbols(library) {}
    image_t run();
};

class compiler_t::function_compiler_t {
    compiler_t &module;
    unsigned owner;
    const language::function_t &source;
    sxcc::function_t &binary;
    int height;
    unsigned value_depth = 0;
    unsigned string_temporary = 0;
    int capacity;
    int pending_discard = 0;
    bool initializer;
    unsigned offset = 0;
    bool reachable = true;
    struct local_t {
        std::string name;
        value_type_t type;
        unsigned offset;
    };
    std::vector<local_t> locals;
    struct label_t {
        std::optional<unsigned> offset;
        std::optional<int> height;
        bool reachable = false;
    };
    struct fixup_t {
        unsigned instruction;
        unsigned label;
    };
    std::vector<label_t> labels;
    std::vector<fixup_t> fixups;
    std::map<std::string, unsigned, std::less<>> named_labels;
    struct loop_t {
        unsigned end;
        unsigned next;
        int height;
        bool is_switch = false;
    };
    std::vector<loop_t> loops;
    enum class intrinsic_t {
        none,
        number_bits,
        integer_string,
        float_string,
        class_value,
        signal,
        raise_event,
        deferred_call,
        terminate
    };
    struct call_t {
        const native_t *native = nullptr;
        unsigned owner = 0, index = 0;
        bool receiver = false;
        value_type_t result;
        intrinsic_t intrinsic = intrinsic_t::none;
        opcode_t operation = OP_NOP;
        bool dynamic_owner = false;
    };
    std::map<const expression_t *, call_t> calls;

    [[noreturn]] void fail(const std::string &message) const
    {
        throw error_t(module.program.image.objects[owner].name + "::" + source.name + ": " + message);
    }

    void change(int amount)
    {
        height += amount;
        if (height < 0 || height > capacity)
            fail("Expression exceeds VM stack storage");
    }

    void emit(opcode_t opcode, opcode_arg_t argument = OP_ARG_NULL, std::uint32_t value = 0, unsigned width = 4)
    {
        if (pending_discard) {
            const int pending = pending_discard;
            pending_discard = 0;
            if (opcode == OP_SPA && argument == OP_ARG_WORD && static_cast<int16_t>(value) < 0)
                value = static_cast<uint16_t>(static_cast<int16_t>(value) + pending);
            else
                emit(OP_SPA, OP_ARG_WORD, static_cast<uint16_t>(pending));
        }
        if (opcode == OP_RET && initializer) {
            argument = OP_ARG_NULL;
            value = 0;
        }
        instruction_t instruction{opcode, argument, value, {}, offset};
        if (width != 4)
            instruction.size_override = static_cast<std::uint16_t>(width);
        validate_instruction(instruction);
        offset += instruction.size();
        binary.code.push_back(instruction);
    }

    void adjust(int amount)
    {
        if (amount) {
            if (amount < INT16_MIN || amount > INT16_MAX)
                fail("Stack adjustment exceeds VM word range");
            const bool boundary = std::any_of(labels.begin(), labels.end(), [&](const auto &label) {
                return label.offset && *label.offset == offset;
            });
            if (amount > 0 && !boundary && !binary.code.empty()) {
                auto &previous = binary.code.back();
                if (previous.opcode == OP_SPA && previous.argument == OP_ARG_WORD &&
                    static_cast<int16_t>(previous.value) > 0 &&
                    static_cast<int16_t>(previous.value) + amount <= INT16_MAX) {
                    previous.value += amount;
                    change(amount);
                    return;
                }
            }
            emit(OP_SPA, OP_ARG_WORD, static_cast<std::uint16_t>(amount));
            change(amount);
        }
    }

    void push_slot(unsigned slot, unsigned width)
    {
        auto displacement = static_cast<int>(slot) - height;
        if (displacement < INT16_MIN || displacement > INT16_MAX)
            fail("Stack reference exceeds VM word range");
        emit(OP_PSH, OP_ARG_SPR, static_cast<std::uint16_t>(displacement), width);
        change(width);
    }

    void store_slot(unsigned slot, unsigned width)
    {
        auto displacement = static_cast<int>(slot) - height;
        if (displacement < INT16_MIN || displacement > INT16_MAX)
            fail("Stack reference exceeds VM word range");
        emit(OP_POP, OP_ARG_SPR, static_cast<std::uint16_t>(displacement), width);
        change(-static_cast<int>(width));
    }

    void retain(unsigned base, unsigned width)
    {
        if (height != static_cast<int>(base + width)) {
            if (width)
                store_slot(base, width);
            adjust(static_cast<int>(base + width) - height);
        }
    }

    unsigned label()
    {
        labels.push_back({});
        return static_cast<unsigned>(labels.size() - 1);
    }

    void edge(opcode_t opcode, unsigned target)
    {
        if (opcode == OP_BF)
            change(-4);
        if (!reachable)
            return;
        auto &destination = labels.at(target);
        if (destination.height && *destination.height != height)
            fail("Branch crosses incompatible local lifetimes");
        destination.height = height;
        destination.reachable = destination.reachable || reachable;
        fixups.push_back({static_cast<unsigned>(binary.code.size()), target});
        emit(opcode, OP_ARG_PCR);
        if (opcode == OP_BRA)
            reachable = false;
    }

    void mark(unsigned target, int expected)
    {
        auto &destination = labels.at(target);
        if (destination.offset)
            fail("Duplicate label");
        if (destination.height && *destination.height != expected)
            fail("Label crosses incompatible local lifetimes");
        destination.offset = offset;
        destination.height = expected;
        height = expected;
        reachable = reachable || destination.reachable;
    }

    unsigned named(std::string_view name)
    {
        auto found = named_labels.find(name);
        if (found != named_labels.end())
            return found->second;
        auto index = label();
        named_labels.emplace(name, index);
        return index;
    }

    const local_t *local(std::string_view name) const
    {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it)
            if (it->name == name)
                return &*it;
        return nullptr;
    }

    bool accessible(access_t access, unsigned declaring, std::string_view receiver = {}) const
    {
        if (access == access_t::public_member || owner == declaring)
            return true;
        if (access != access_t::protected_member)
            return false;
        const auto &current = module.program.image.objects[owner].name;
        if (!module.assignable({current}, {module.program.image.objects[declaring].name}))
            return false;
        return receiver.empty() || module.assignable({std::string(receiver)}, {current});
    }

    const field_t &field(unsigned scope, std::string_view name, std::string_view receiver = {}) const
    {
        auto found = module.fields[scope].find(name);
        if (found == module.fields[scope].end())
            fail("Unknown field " + std::string(name));
        const auto &result = found->second;
        if (!accessible(result.access, result.owner, result.is_static ? std::string_view{} : receiver))
            fail("Field is not accessible: " + std::string(name));
        return result;
    }

    value_type_t native_result(const native_t &native) const
    {
        auto override = module.native_results.find(&native);
        if (override != module.native_results.end())
            return {override->second};
        if (native.result)
            return {*native.result};
        if (native.result_bytes && *native.result_bytes == 0)
            return {"void"};
        fail("Declare the return type of native " + native.signature + " with extern");
    }

    std::uint32_t literal_bits(const expression_t &expression) const
    {
        if (expression.kind != expression_t::kind_t::number)
            fail("Expected an unsigned integer literal");
        auto text = std::string_view(expression.text);
        int radix = 10;
        if (text.starts_with("0x") || text.starts_with("0X")) {
            text.remove_prefix(2);
            radix = 16;
        }
        std::uint32_t result = 0;
        auto parsed = std::from_chars(text.data(), text.data() + text.size(), result, radix);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
            fail("Expected an exact 32-bit integer encoding");
        return result;
    }

    std::optional<call_t> resolve_service(const expression_t &expression, std::string_view name)
    {
        const auto count = expression.operands.size() - 1;
        auto word = [&](unsigned index) {
            if (module.size(infer(*expression.operands.at(index))) != 4)
                fail("VM service requires a word-sized handle");
        };
        auto arity = [&](unsigned expected) {
            if (count != expected)
                fail(std::string(name) + " has an incorrect argument count");
        };
        auto signal = [&](unsigned index) {
            const auto &argument = *expression.operands.at(index);
            if (argument.kind != expression_t::kind_t::string)
                fail("A signal requires its symbolic string name");
            return module.intern(argument.text) << 16;
        };
        call_t target;
        target.result = {"void"};
        if (name == "signal_id") {
            if (count != 1 && count != 2)
                fail("Signal lookup requires a name and optional owner");
            target.dynamic_owner = count == 2;
            if (target.dynamic_owner)
                word(1);
            target.index = signal(static_cast<unsigned>(count));
            target.result = {"signal"};
            target.intrinsic = intrinsic_t::signal;
        } else if (name == "raise_signal" || name == "raise_all_signals") {
            if (count != 1 && count != 2)
                fail("Signal raising requires an identifier and optional owner");
            target.dynamic_owner = count == 2;
            if (target.dynamic_owner)
                word(1);
            target.index = signal(static_cast<unsigned>(count));
            target.operation = name == "raise_signal" ? OP_RE : OP_RAE;
            if (target.operation == OP_RAE && target.dynamic_owner)
                fail("Broadcast signals do not take an instance owner");
            target.intrinsic = intrinsic_t::raise_event;
        } else if (name == "start_thread" || name == "start_thread_on" || name == "bind_signal" ||
                   name == "bind_signal_on" || name == "bind_signal_once" || name == "bind_signal_once_on") {
            const bool thread = name.starts_with("start_thread");
            const bool owner = name.ends_with("_on");
            arity(1 + (owner ? 1 : 0) + (thread ? 0 : 1));
            if (owner)
                word(1);
            if (!thread)
                module.require(infer(*expression.operands[owner ? 2 : 1]), {"signal"});
            const auto &call = *expression.operands.back();
            if (call.kind != expression_t::kind_t::call)
                fail("Deferred execution requires a script function invocation");
            target = resolve(call);
            if (target.native || target.intrinsic != intrinsic_t::none)
                fail("Only script functions can run as threads or callbacks");
            target.result = {"void"};
            target.dynamic_owner = owner;
            target.intrinsic = intrinsic_t::deferred_call;
            target.operation = thread                                 ? (owner ? OP_BTH : OP_BST)
                               : name.starts_with("bind_signal_once") ? (owner ? OP_ECO : OP_SCO)
                                                                      : (owner ? OP_ECB : OP_ESB);
        } else if (name == "stop_thread" || name == "delete_all_threads") {
            arity(0);
            target.intrinsic = intrinsic_t::terminate;
            target.operation = name == "stop_thread" ? OP_KIL : OP_DEL_THREADS;
            target.index = UINT32_MAX;
        } else if (name == "kill_threads" || name == "kill_threads_on" || name == "delete_threads" ||
                   name == "delete_threads_on") {
            const bool owner = name.ends_with("_on");
            arity(owner ? 2 : 1);
            if (owner)
                word(1);
            const auto &argument = *expression.operands.back();
            const bool typed = argument.kind == expression_t::kind_t::bit_cast;
            const auto &reference = typed ? *argument.operands.at(0) : argument;
            if (reference.kind != expression_t::kind_t::member || !reference.bits ||
                reference.operands[0]->kind != expression_t::kind_t::name)
                fail("Thread cancellation requires a class-qualified script function");
            auto found = module.classes.find(reference.operands[0]->text);
            if (found == module.classes.end())
                fail("Unknown thread function class");
            target.owner = found->second;
            target.index = UINT32_MAX;
            for (const auto &function : module.program.classes[target.owner].functions) {
                if (function.name != reference.text)
                    continue;
                if (typed) {
                    const auto &record = module.program.image.objects[target.owner].functions[function.index];
                    std::string signature = function.result + " (";
                    if (record.flags & 1u)
                        signature += "*";
                    else
                        signature += module.program.image.objects[target.owner].name + "::*";
                    signature += ")(";
                    for (unsigned p = 0; p < function.parameters.size(); ++p) {
                        if (p)
                            signature += ",";
                        signature += function.parameters[p].type;
                    }
                    signature += ")";
                    if (signature != argument.text)
                        continue;
                }
                if (target.index != UINT32_MAX)
                    fail("Thread function reference requires an unambiguous type");
                target.index = function.index;
            }
            if (target.index == UINT32_MAX)
                fail("Unknown thread function");
            target.dynamic_owner = owner;
            const bool massacre = name.starts_with("delete_threads");
            target.operation = massacre ? (owner ? OP_MS2 : OP_DEL_THREADS) : (owner ? OP_KILL_THREAD : OP_KIL);
            target.intrinsic = intrinsic_t::terminate;
        } else
            return std::nullopt;
        return target;
    }

    unsigned call_arguments(const expression_t &expression, const call_t &target)
    {
        const auto &callee = *expression.operands[0];
        if (target.receiver) {
            if (callee.kind == expression_t::kind_t::member && !callee.bits)
                value(*callee.operands[0]);
            else
                push_slot(0, 4);
        }
        if (!target.receiver && callee.kind == expression_t::kind_t::member && !callee.bits) {
            auto start = height;
            value(*callee.operands[0]);
            adjust(start - height);
        }
        unsigned bytes = target.receiver ? (target.native ? module.size({target.native->class_name}) : 4u) : 0u;
        for (unsigned i = 1; i < expression.operands.size(); ++i)
            bytes += module.size(value(*expression.operands[i]));
        return bytes;
    }

    void emit_service(const expression_t &expression, const call_t &target)
    {
        switch (target.intrinsic) {
        case intrinsic_t::signal:
            if (target.dynamic_owner)
                value(*expression.operands[1]);
            emit(OP_PSH, target.dynamic_owner ? OP_ARG_PSIG : OP_ARG_SIG, target.index);
            if (!target.dynamic_owner)
                change(4);
            break;
        case intrinsic_t::raise_event:
            if (target.dynamic_owner)
                value(*expression.operands[1]);
            emit(target.operation, target.dynamic_owner ? OP_ARG_PSIG : OP_ARG_SIG, target.index);
            if (target.dynamic_owner)
                change(-4);
            break;
        case intrinsic_t::deferred_call: {
            const bool callback = target.operation != OP_BST && target.operation != OP_BTH;
            unsigned consumed = 0;
            if (callback) {
                value(*expression.operands[target.dynamic_owner ? 2 : 1]);
                consumed += 4;
            }
            consumed += call_arguments(*expression.operands.back(), target);
            if (target.dynamic_owner) {
                value(*expression.operands[1]);
                consumed += 4;
            }
            emit(target.operation, OP_ARG_SFR, (module.lookup_index[target.owner] << 16) | target.index);
            change(-static_cast<int>(consumed));
            break;
        }
        case intrinsic_t::terminate:
            if (target.index == UINT32_MAX) {
                emit(target.operation);
                reachable = false;
            } else {
                if (target.dynamic_owner)
                    value(*expression.operands[1]);
                emit(target.operation, OP_ARG_SFR, (module.lookup_index[target.owner] << 16) | target.index);
                if (target.dynamic_owner)
                    change(-4);
            }
            break;
        default:
            fail("Internal invalid VM service");
        }
    }
    bool pure_storage(const expression_t &expression, unsigned &slot)
    {
        using kind = expression_t::kind_t;
        if (expression.kind == kind::name) {
            if (auto value = local(expression.text)) {
                slot = value->offset;
                return true;
            }
        } else if (expression.kind == kind::bit_cast)
            return pure_storage(*expression.operands[0], slot);
        return false;
    }

    bool pure_word(const expression_t &expression, std::optional<unsigned> &slot)
    {
        using kind = expression_t::kind_t;
        if (module.size(infer(expression)) != 4)
            return false;
        unsigned position = 0;
        if (pure_storage(expression, position)) {
            slot = position;
            return true;
        }
        if (expression.kind == kind::number || expression.kind == kind::string)
            return true;
        if (expression.kind == kind::bit_cast)
            return pure_word(*expression.operands[0], slot);
        if (expression.kind == kind::call) {
            auto target = resolve(expression);
            return target.intrinsic == intrinsic_t::number_bits || target.intrinsic == intrinsic_t::class_value;
        }
        return false;
    }

    bool assign_aggregate(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        if (expression.kind != kind::assignment || expression.text != "=" ||
            expression.operands[0]->kind != kind::name || expression.operands[1]->kind != kind::aggregate)
            return false;
        const auto *destination = local(expression.operands[0]->text);
        if (!destination)
            return false;
        infer(expression);
        struct copy_t {
            const expression_t *expression;
            unsigned destination;
            std::optional<unsigned> source;
            bool saved = false;
        };
        std::vector<copy_t> copies;
        unsigned offset = destination->offset;
        for (const auto &operand : expression.operands[1]->operands) {
            std::optional<unsigned> source;
            if (!pure_word(*operand, source))
                return false;
            if (source != offset)
                copies.push_back({operand.get(), offset, source});
            offset += 4;
        }
        const auto initial = height;
        std::optional<unsigned> temporary;
        while (!copies.empty()) {
            auto ready = std::find_if(copies.begin(), copies.end(), [&](const auto &copy) {
                return std::none_of(copies.begin(), copies.end(), [&](const auto &other) {
                    return other.source && *other.source < copy.destination + 4 && copy.destination < *other.source + 4;
                });
            });
            if (ready != copies.end()) {
                if (ready->saved)
                    push_slot(*temporary, 4);
                else
                    value(*ready->expression);
                store_slot(ready->destination, 4);
                copies.erase(ready);
            } else {
                if (!temporary) {
                    temporary = height;
                    adjust(4);
                }
                auto &copy = copies.front();
                value(*copy.expression);
                store_slot(*temporary, 4);
                copy.source.reset();
                copy.saved = true;
            }
        }
        adjust(initial - height);
        return true;
    }


    call_t resolve(const expression_t &expression)
    {
        if (auto found = calls.find(&expression); found != calls.end())
            return found->second;
        const auto &callee = *expression.operands.at(0);
        std::string name;
        std::optional<value_type_t> receiver;
        std::string class_name;
        if (callee.kind == expression_t::kind_t::name)
            name = callee.text;
        else if (callee.kind == expression_t::kind_t::member) {
            name = callee.text;
            if (callee.bits) {
                const auto &scope = *callee.operands.at(0);
                if (scope.kind != expression_t::kind_t::name)
                    fail("A qualified call requires a class name");
                class_name = scope.text;
            } else {
                receiver = infer(*callee.operands.at(0));
                class_name = receiver->name;
            }
        } else
            fail("The VM requires a statically resolved call target");
        if (callee.kind == expression_t::kind_t::name) {
            if (auto service = resolve_service(expression, name)) {
                calls.emplace(&expression, *service);
                return *service;
            }
        }
        if (callee.kind == expression_t::kind_t::name &&
            (name == "num_bits" || name == "to_integer_string" || name == "to_float_string")) {
            if (expression.operands.size() != 2)
                fail(name + " requires one argument");
            call_t target;
            if (name == "num_bits") {
                literal_bits(*expression.operands[1]);
                target.result = {"num"};
                target.intrinsic = intrinsic_t::number_bits;
            } else {
                module.require(infer(*expression.operands[1]), {"num"});
                target.result = {"str"};
                target.intrinsic =
                    name == "to_integer_string" ? intrinsic_t::integer_string : intrinsic_t::float_string;
            }
            calls.emplace(&expression, target);
            return target;
        }
        std::vector<value_type_t> arguments;
        for (unsigned i = 1; i < expression.operands.size(); ++i)
            arguments.push_back(infer(*expression.operands[i]));
        std::optional<call_t> best;
        unsigned best_cost = UINT32_MAX;
        auto consider = [&](call_t candidate, const std::vector<value_type_t> &parameters, bool variadic) {
            if (arguments.size() < parameters.size() || (!variadic && arguments.size() != parameters.size()))
                return;
            unsigned cost = 0;
            for (unsigned i = 0; i < parameters.size(); ++i) {
                if (!module.assignable(arguments[i], parameters[i]))
                    return;
                cost += arguments[i].name != parameters[i].name;
            }
            for (unsigned i = static_cast<unsigned>(parameters.size()); i < arguments.size(); ++i)
                if (module.size(arguments[i]) != 4)
                    return;
            if (cost < best_cost) {
                best = std::move(candidate);
                best_cost = cost;
            } else if (cost == best_cost)
                fail("Ambiguous overload for " + name);
        };
        auto script_candidates = [&](unsigned index, bool implicit) {
            std::set<unsigned> visited;
            while (visited.insert(index).second) {
                const auto &object = module.program.image.objects[index];
                for (const auto &function : module.program.classes[index].functions) {
                    if (function.name != name)
                        continue;
                    if ((function.constructor || function.destructor) && callee.kind != expression_t::kind_t::member)
                        continue;
                    const auto &record = object.functions[function.index];
                    const bool needs_receiver = !(record.flags & 1u);
                    if (needs_receiver) {
                        auto actual = receiver;
                        if (!actual && implicit && !(binary.flags & 1u))
                            actual = value_type_t{module.program.image.objects[owner].name};
                        if (!actual || !module.assignable(*actual, {object.name}))
                            continue;
                        if (!accessible(function.access, index, actual->name))
                            continue;
                    }
                    if (!needs_receiver && !accessible(function.access, index))
                        continue;
                    std::vector<value_type_t> parameters;
                    for (const auto &parameter : function.parameters)
                        parameters.push_back({parameter.type, parameter.count, parameter.is_array});
                    consider({nullptr, index, function.index, needs_receiver, {function.result}}, parameters, false);
                }
                if (best || !object.parent)
                    break;
                index = module.classes.at(*object.parent);
            }
        };
        if (class_name.empty()) {
            script_candidates(owner, true);
            if (!best && owner != 0)
                script_candidates(0, false);
        } else if (auto found = module.classes.find(class_name); found != module.classes.end()) {
            script_candidates(found->second, false);
        }
        if (!best) {
            if (class_name.empty()) {
                if (module.symbols.class_ids.count(name))
                    class_name = name;
                else
                    class_name = "_global_slc";
            }
            std::set<std::string> visited;
            while (visited.insert(class_name).second) {
                for (const auto &[identity, native] : module.symbols.natives) {
                    if (native.class_name != class_name ||
                        std::string_view(native.signature).substr(0, native.signature.find('(')) != name)
                        continue;
                    const bool needs_receiver = class_name != "_global_slc" && name != class_name;
                    if (needs_receiver && (!receiver || !module.assignable(*receiver, {class_name})))
                        continue;
                    std::vector<value_type_t> parameters;
                    for (unsigned i = needs_receiver ? 1 : 0; i < native.parameters.size(); ++i)
                        parameters.push_back({native.parameters[i]});
                    consider({&native, 0, 0, needs_receiver, native_result(native)}, parameters, native.variadic);
                }
                if (best)
                    break;
                auto parent = module.symbols.parents.find(class_name);
                if (parent == module.symbols.parents.end())
                    break;
                class_name = parent->second;
            }
        }
        if (!best && !receiver && module.symbols.class_ids.count(name) && name != "num" && name != "str" &&
            module.size({name}) == 4 && expression.operands.size() == 2 &&
            expression.operands[1]->kind == expression_t::kind_t::string &&
            (callee.kind == expression_t::kind_t::name ||
             (callee.kind == expression_t::kind_t::member && callee.bits && callee.operands[0]->text == name))) {
            call_t constant;
            constant.result = {name};
            constant.intrinsic = intrinsic_t::class_value;
            best = std::move(constant);
        }
        if (!best)
            fail("No matching function for " + name);
        calls.emplace(&expression, *best);
        return *best;
    }

    call_t native_update(const expression_t &expression, const value_type_t &operand)
    {
        if (auto found = calls.find(&expression); found != calls.end())
            return found->second;
        const auto signature = "operator" + expression.text + "(";
        const native_t *match = nullptr;
        for (const auto &[identity, native] : module.symbols.natives) {
            if (native.class_name != operand.name || !native.signature.starts_with(signature) || native.variadic ||
                native.parameters.size() != 1 || native.parameters[0] != operand.name ||
                native_result(native).name != operand.name)
                continue;
            if (match)
                fail("Ambiguous native update operator");
            match = &native;
        }
        if (!match)
            fail("Missing native update operator for " + operand.name);
        call_t result{match, 0, 0, true, native_result(*match)};
        calls.emplace(&expression, result);
        return result;
    }

    call_t native_binary(const expression_t &expression, const value_type_t &left, const value_type_t &right)
    {
        if (auto found = calls.find(&expression); found != calls.end())
            return found->second;
        const auto prefix = "operator" + expression.text + '(';
        const native_t *best = nullptr;
        unsigned best_cost = UINT32_MAX;
        auto scope = left.name;
        for (;;) {
            auto class_id = module.symbols.class_ids.find(scope);
            if (class_id == module.symbols.class_ids.end())
                break;
            for (auto it = module.symbols.natives.lower_bound({class_id->second, 0});
                 it != module.symbols.natives.end() && it->first.first == class_id->second;
                 ++it) {
                const auto &native = it->second;
                if (!native.signature.starts_with(prefix) || native.variadic || native.parameters.size() != 2 ||
                    !module.assignable(right, {native.parameters[1]}))
                    continue;
                unsigned cost = (left.name != scope) + (right.name != native.parameters[1]);
                if (cost < best_cost) {
                    best = &native;
                    best_cost = cost;
                } else if (cost == best_cost)
                    fail("Ambiguous native operator " + expression.text);
            }
            auto parent = module.symbols.parents.find(scope);
            if (parent == module.symbols.parents.end())
                break;
            scope = parent->second;
        }
        if (!best)
            fail("No native operator " + expression.text + " for " + left.name + " and " + right.name);
        call_t target;
        target.native = best;
        target.receiver = true;
        target.result = native_result(*best);
        calls.emplace(&expression, target);
        return target;
    }

    value_type_t infer(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        switch (expression.kind) {
        case kind::number:
            return {"num"};
        case kind::string:
            return {"str"};
        case kind::bit_cast: {
            value_type_t target{expression.text};
            auto width = module.size(target);
            if (!width || width != module.size(infer(*expression.operands[0])))
                fail("bit_cast requires non-void values with equal storage widths");
            return target;
        }
        case kind::aggregate: {
            value_type_t target{expression.text};
            bool vector = target.name == "vector3d" || target.name == "position3d";
            if (!vector && !target.name.starts_with("opaque<"))
                fail("Type does not support aggregate initialization");
            auto width = module.size(target);
            unsigned initialized = 0;
            for (const auto &operand : expression.operands) {
                auto type = infer(*operand);
                if (vector)
                    module.require(type, {"num"});
                auto part = module.size(type);
                if (!part || part > width - initialized)
                    fail("Aggregate initializer exceeds its storage width");
                initialized += part;
            }
            if (initialized != width)
                fail("Aggregate initializer must cover its storage width");
            return target;
        }
        case kind::name:
            if (expression.text == "true" || expression.text == "false")
                return {"num"};
            if (expression.text == "nullptr")
                return {"nullptr"};
            if (auto found = local(expression.text))
                return found->type;
            if (module.fields[owner].count(expression.text))
                return field(owner, expression.text, module.program.image.objects[owner].name).type;
            if (auto found = module.program.native_variables.find(expression.text);
                found != module.program.native_variables.end())
                return {found->second.type, found->second.count, found->second.is_array};
            if (module.program.string_objects.count(expression.text))
                return {"str"};
            fail("Unknown variable " + expression.text);
        case kind::call:
            return resolve(expression).result;
        case kind::member: {
            if (expression.bits) {
                const auto &scope = *expression.operands[0];
                if (scope.kind != kind::name || !module.classes.count(scope.text))
                    fail("Unknown class scope");
                const auto &member = field(module.classes.at(scope.text), expression.text);
                if (!member.is_static)
                    fail("An instance field requires a receiver");
                return member.type;
            }
            auto receiver = infer(*expression.operands[0]);
            if (receiver.array())
                fail("An array must be indexed before accessing its members");
            if ((receiver.name == "vector3d" || receiver.name == "position3d") &&
                (expression.text == "x" || expression.text == "y" || expression.text == "z"))
                return {"num"};
            auto found = module.classes.find(receiver.name);
            if (found == module.classes.end())
                fail("Unknown member " + receiver.name + '.' + expression.text);
            return field(found->second, expression.text, receiver.name).type;
        }
        case kind::index: {
            auto base = infer(*expression.operands[0]);
            if (!base.array())
                fail("Indexing requires a fixed array");
            module.require(infer(*expression.operands[1]), {"num"});
            base.count = 1;
            base.is_array = false;
            return base;
        }
        case kind::unary:
            if ((expression.text == "++" || expression.text == "--") && infer(*expression.operands[0]).name != "num")
                return native_update(expression, infer(*expression.operands[0])).result;
            if (expression.text == "*") {
                auto type = infer(*expression.operands[0]);
                if (!type.array())
                    fail("Dereference requires a fixed array");
                type.count = 1;
                type.is_array = false;
                return type;
            }
            [[fallthrough]];
        case kind::postfix:
            if (infer(*expression.operands[0]).name != "num")
                return native_update(expression, infer(*expression.operands[0])).result;
            module.require(infer(*expression.operands[0]), {"num"});
            return {"num"};
        case kind::assignment: {
            auto left = infer(*expression.operands[0]);
            module.require(infer(*expression.operands[1]), left);
            return left;
        }
        case kind::conditional: {
            module.require(infer(*expression.operands[0]), {"num"});
            auto left = infer(*expression.operands[1]);
            module.require(infer(*expression.operands[2]), left);
            return left;
        }
        case kind::binary: {
            auto left = infer(*expression.operands[0]);
            auto right = infer(*expression.operands[1]);
            if (left.array() || right.array())
                fail("An array is not an arithmetic operand");
            if (left.name != "num" && left.name != "str")
                return native_binary(expression, left, right).result;
            module.require(right, left);
            if (left.name == "str" && expression.text == "+")
                return left;
            if (left.name != "num" && !(left.name == "str" && (expression.text == "==" || expression.text == "!=")))
                fail("Operator requires an explicit native overload for " + left.name);
            return {"num"};
        }
        }
        fail("Invalid expression");
    }

    struct address_t {
        enum class kind_t { local, shared, instance, variable } kind;
        value_type_t type;
        unsigned offset = 0, owner = 0, receiver = 0;
        std::optional<unsigned> index = std::nullopt;
        unsigned stride = 0;
        unsigned variable_index = 0, variable_width = 0;
    };

    std::pair<const field_t *, const expression_t *> instance_array(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        if (expression.kind != kind::index)
            return {};
        const auto &base = *expression.operands[0];
        if (base.kind == kind::name && !local(base.text) && module.fields[owner].count(base.text)) {
            const auto &member = field(owner, base.text, module.program.image.objects[owner].name);
            if (!member.is_static && member.type.array()) {
                if (binary.flags & 1u)
                    fail("Instance field used from a static function");
                return {&member, nullptr};
            }
        }
        if (base.kind == kind::member && !base.bits) {
            const auto &receiver = *base.operands[0];
            const auto type = infer(receiver);
            auto scope = module.classes.find(type.name);
            if (scope == module.classes.end())
                return {};
            const auto &member = field(scope->second, base.text, type.name);
            if (!member.is_static && member.type.array())
                return {&member, &receiver};
        }
        return {};
    }

    address_t address(const expression_t &expression, bool writable = true)
    {
        using kind = expression_t::kind_t;
        if (expression.kind == kind::unary && expression.text == "*") {
            auto storage = address(*expression.operands[0], writable);
            if (!storage.type.array() || storage.index)
                fail("Dereference requires a fixed array");
            storage.type.count = 1;
            storage.type.is_array = false;
            return storage;
        }
        if (expression.kind == kind::name) {
            if (writable && expression.text == "this")
                fail("The instance receiver is not assignable");
            if (auto found = local(expression.text))
                return {address_t::kind_t::local, found->type, found->offset};
            if (!module.fields[owner].count(expression.text) &&
                module.program.native_variables.count(expression.text)) {
                const auto identity = module.symbols.variables.find(expression.text);
                if (identity == module.symbols.variables.end())
                    fail("Unknown runtime variable " + expression.text);
                address_t result{address_t::kind_t::variable, infer(expression), 0, identity->second.first};
                result.variable_index = identity->second.second;
                auto element = result.type;
                if (element.array()) {
                    element.count = 1;
                    element.is_array = false;
                }
                result.variable_width = module.size(element);
                return result;
            }
            const auto &field = this->field(owner, expression.text, module.program.image.objects[owner].name);
            if (field.is_static)
                return {address_t::kind_t::shared, field.type, field.offset, field.owner};
            if (binary.flags & 1u)
                fail("Instance field used from a static function");
            unsigned receiver = height;
            push_slot(0, 4);
            return {address_t::kind_t::instance, field.type, field.offset, field.owner, receiver};
        }
        if (expression.kind == kind::index) {
            auto result = address(*expression.operands[0], writable);
            if (!result.type.array() || result.index)
                fail("Invalid array index");
            result.type.count = 1;
            result.type.is_array = false;
            result.stride = module.size(result.type);
            result.index = height;
            module.require(value(*expression.operands[1]), {"num"});
            return result;
        }
        if (expression.kind == kind::member) {
            const auto &base = *expression.operands[0];
            if (expression.bits) {
                if (base.kind != kind::name || !module.classes.count(base.text))
                    fail("Unknown class scope");
                const auto &field = this->field(module.classes.at(base.text), expression.text);
                if (!field.is_static)
                    fail("An instance field requires a receiver");
                return {address_t::kind_t::shared, field.type, field.offset, field.owner};
            }
            auto type = infer(base);
            if (type.name == "vector3d" || type.name == "position3d") {
                auto result = address(base, writable);
                if (expression.text == "x")
                    result.offset += offsetof(vm_vector3d_t, x);
                else if (expression.text == "y")
                    result.offset += offsetof(vm_vector3d_t, y);
                else if (expression.text == "z")
                    result.offset += offsetof(vm_vector3d_t, z);
                else
                    fail("Unknown vector component " + expression.text);
                result.type = {"num"};
                return result;
            }
            auto class_index = module.classes.find(type.name);
            if (class_index == module.classes.end())
                fail("Native fields require a registered accessor");
            const auto &field = this->field(class_index->second, expression.text, type.name);
            if (field.is_static) {
                auto base_height = height;
                value(base);
                adjust(base_height - height);
                return {address_t::kind_t::shared, field.type, field.offset, field.owner};
            }
            unsigned receiver = height;
            value(base);
            return {address_t::kind_t::instance, field.type, field.offset, field.owner, receiver};
        }
        if (!writable) {
            auto slot = static_cast<unsigned>(height);
            auto type = value(expression);
            return {address_t::kind_t::local, std::move(type), slot};
        }
        fail("Expression is not writable storage");
    }

    void number(float value)
    {
        std::uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        emit(OP_PSH, OP_ARG_NUM, bits);
        change(4);
    }

    void index(const address_t &storage, unsigned width, unsigned component = 0)
    {
        push_slot(*storage.index, 4);
        if (storage.stride != width) {
            number(static_cast<float>(storage.stride) / static_cast<float>(width));
            emit(OP_MUL);
            change(-4);
        }
        if (component) {
            number(static_cast<float>(component));
            emit(OP_ADD);
            change(-4);
        }
    }

    void load(const address_t &storage)
    {
        auto width = module.size(storage.type);
        if (storage.kind == address_t::kind_t::variable) {
            if (storage.index && !storage.offset && storage.stride == width) {
                index(storage, width);
                emit(OP_PSH, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, width);
                change(width);
                emit(OP_COFF, OP_ARG_WORD, width);
                change(-4);
                return;
            }
            if (!storage.offset && !storage.index) {
                emit(OP_PSH, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, width);
                change(width);
            } else {
                const auto slot = static_cast<unsigned>(height);
                emit(OP_PSH, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, storage.variable_width);
                change(storage.variable_width);
                address_t part{address_t::kind_t::local, storage.type, slot + storage.offset};
                part.index = storage.index;
                part.stride = storage.stride;
                load(part);
            }
            return;
        }
        if (storage.kind == address_t::kind_t::local) {
            if (!storage.index)
                push_slot(storage.offset, width);
            else
                for (unsigned component = 0; component < width / 4; ++component) {
                    index(storage, 4, component);
                    push_slot(storage.offset, 4);
                    emit(OP_ASF, OP_ARG_WORD, 4);
                    change(-4);
                }
        } else if (storage.kind == address_t::kind_t::shared) {
            if (storage.index)
                index(storage, width);
            emit(OP_PSH, OP_ARG_SDR, (module.lookup_index[storage.owner] << 16) | storage.offset, width);
            change(width);
            if (storage.index) {
                emit(OP_COFF, OP_ARG_WORD, width);
                change(-4);
            }
        } else if (storage.index) {
            index(storage, width);
            number(static_cast<float>(storage.offset));
            push_slot(storage.receiver, 4);
            emit(OP_CPY, OP_ARG_WORD, width);
            change(static_cast<int>(width) - 12);
        } else {
            push_slot(storage.receiver, 4);
            emit(OP_PSH, OP_ARG_POPO, storage.offset, width);
            change(static_cast<int>(width) - 4);
        }
    }

    void store(const address_t &storage, unsigned source_slot)
    {
        auto width = module.size(storage.type);
        if (storage.kind == address_t::kind_t::local && !storage.index &&
            source_slot + width == static_cast<unsigned>(height)) {
            store_slot(storage.offset, width);
            push_slot(storage.offset, width);
            return;
        }
        if (storage.kind == address_t::kind_t::variable) {
            if (storage.index && !storage.offset && storage.stride == width) {
                push_slot(source_slot, width);
                index(storage, width);
                emit(OP_PSF, OP_ARG_WORD, width);
                change(-4);
                emit(OP_POP, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, width);
                change(-static_cast<int>(width));
                return;
            }
            if (!storage.offset && !storage.index && width == storage.variable_width) {
                push_slot(source_slot, width);
                emit(OP_POP, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, width);
                change(-static_cast<int>(width));
            } else {
                const auto slot = static_cast<unsigned>(height);
                emit(OP_PSH, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, storage.variable_width);
                change(storage.variable_width);
                address_t part{address_t::kind_t::local, storage.type, slot + storage.offset};
                part.index = storage.index;
                part.stride = storage.stride;
                store(part, source_slot);
                push_slot(slot, storage.variable_width);
                emit(OP_POP, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, storage.variable_width);
                change(-static_cast<int>(storage.variable_width));
                adjust(static_cast<int>(slot) - height);
            }
            return;
        }
        if (storage.kind == address_t::kind_t::local && storage.index) {
            for (unsigned component = 0; component < width / 4; ++component) {
                push_slot(source_slot + 4 * component, 4);
                index(storage, 4, component);
                emit(OP_PSF, OP_ARG_WORD, 4);
                change(-4);
                store_slot(storage.offset, 4);
            }
            return;
        }
        push_slot(source_slot, width);
        if (storage.index) {
            index(storage, width);
            emit(OP_PSF, OP_ARG_WORD, width);
            change(-4);
        }
        if (storage.kind == address_t::kind_t::local)
            store_slot(storage.offset, width);
        else if (storage.kind == address_t::kind_t::shared) {
            emit(OP_POP, OP_ARG_SDR, (module.lookup_index[storage.owner] << 16) | storage.offset, width);
            change(-static_cast<int>(width));
        } else {
            push_slot(storage.receiver, 4);
            emit(OP_POP, OP_ARG_POPO, storage.offset, width);
            change(-static_cast<int>(width) - 4);
        }
    }

    void operation(std::string_view operation, const value_type_t &type)
    {
        if (type.name == "str") {
            if (operation == "+")
                emit(OP_PSH_STR, OP_ARG_WORD, string_temporary++);
            else if (operation == "==")
                emit(OP_STR_EQ);
            else if (operation == "!=")
                emit(OP_STR_NE);
            else
                fail("Invalid string operator");
        } else {
            static const std::map<std::string_view, opcode_t> operations{{"+", OP_ADD},
                                                                         {"-", OP_SUB},
                                                                         {"*", OP_MUL},
                                                                         {"/", OP_DIV},
                                                                         {"%", OP_MOD},
                                                                         {"==", OP_EQ},
                                                                         {"!=", OP_NE},
                                                                         {"<", OP_LT},
                                                                         {">", OP_GT},
                                                                         {"<=", OP_LE},
                                                                         {">=", OP_GE},
                                                                         {"&", OP_AND},
                                                                         {"|", OP_OR},
                                                                         {"^", OP_XOR},
                                                                         {"<<", OP_SHL},
                                                                         {">>", OP_SHR}};
            auto found = operations.find(operation);
            if (found == operations.end())
                fail("Unsupported operator " + std::string(operation));
            emit(found->second);
        }
        change(-4);
    }


    bool direct_update(const expression_t &expression, bool postfix)
    {
        using kind = expression_t::kind_t;
        const auto &lhs = *expression.operands[0];
        std::function<bool(const expression_t &)> simple = [&](const expression_t &e) {
            if (e.kind != kind::name && e.kind != kind::member)
                return false;
            for (const auto &child : e.operands)
                if (!simple(*child))
                    return false;
            return true;
        };
        if (!simple(lhs) || infer(lhs).name != "num")
            return false;
        const auto saved = height;
        const auto count = binary.code.size();
        const auto old_offset = offset;
        value(lhs);
        if (postfix) {
            emit(OP_DUP);
            change(4);
        }
        emit(expression.text == "++" ? OP_INC : OP_DEC);
        auto target = address(lhs, false);
        if (target.index ||
            (target.kind == address_t::kind_t::variable && (target.offset || target.variable_width != 4))) {
            height = saved;
            binary.code.resize(count);
            offset = old_offset;
            return false;
        }
        switch (target.kind) {
        case address_t::kind_t::local:
            store_slot(target.offset, 4);
            break;
        case address_t::kind_t::shared:
            emit(OP_POP, OP_ARG_SDR, (module.lookup_index[target.owner] << 16) | target.offset);
            change(-4);
            break;
        case address_t::kind_t::variable:
            emit(OP_POP, OP_ARG_VAR, (target.variable_index << 16) | target.owner);
            change(-4);
            break;
        case address_t::kind_t::instance:
            emit(OP_POP, OP_ARG_POPO, target.offset);
            change(-8);
            break;
        }
        if (height != saved + (postfix ? 4 : 0))
            fail("Direct update stack imbalance");
        return true;
    }


    bool boolean_expression(const expression_t &expression) const
    {
        using kind = expression_t::kind_t;
        if (expression.kind == kind::unary && expression.text == "!")
            return true;
        if (expression.kind == kind::binary) {
            const auto &op = expression.text;
            return op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "&&" ||
                   op == "||";
        }
        return false;
    }

    value_type_t value(const expression_t &expression)
    {
        ++value_depth;
        struct depth_scope_t {
            unsigned &depth;
            ~depth_scope_t()
            {
                --depth;
            }
        } depth_scope{value_depth};
        using kind = expression_t::kind_t;
        auto type = infer(expression);
        auto base = static_cast<unsigned>(height);
        auto width = module.size(type);
        switch (expression.kind) {
        case kind::number:
            emit(OP_PSH, OP_ARG_NUM, expression.bits);
            change(4);
            break;
        case kind::string:
            emit(OP_PSH, OP_ARG_STR, module.intern(expression.text) << 16);
            change(4);
            break;
        case kind::bit_cast:
            value(*expression.operands[0]);
            break;
        case kind::aggregate:
            for (const auto &operand : expression.operands)
                value(*operand);
            break;
        case kind::name:
            if (expression.text == "true" || expression.text == "false") {
                number(expression.text == "true" ? 1.0f : 0.0f);
                break;
            }
            if (expression.text == "nullptr") {
                number(0);
                break;
            }
            if (expression.text == "this") {
                push_slot(0, 4);
                break;
            }
            if (!local(expression.text) && !module.fields[owner].count(expression.text) &&
                module.program.string_objects.count(expression.text)) {
                emit(OP_PSH, OP_ARG_STR, module.intern_object(expression.text) << 16);
                change(4);
                break;
            }
            [[fallthrough]];
        case kind::member:
        case kind::index: {
            if (auto [member, receiver] = instance_array(expression); member) {
                module.require(value(*expression.operands[1]), {"num"});
                number(static_cast<float>(member->offset));
                if (receiver)
                    value(*receiver);
                else
                    push_slot(0, 4);
                emit(OP_CPY, OP_ARG_WORD, width);
                change(static_cast<int>(width) - 12);
                break;
            }
            auto storage = address(expression, false);
            if ((storage.kind == address_t::kind_t::local || storage.kind == address_t::kind_t::shared ||
                 (storage.kind == address_t::kind_t::variable && !storage.offset)) &&
                storage.index == base && storage.stride == width && height == static_cast<int>(base + 4)) {
                if (storage.kind == address_t::kind_t::local)
                    emit(OP_PSH, OP_ARG_SPR, static_cast<uint16_t>(static_cast<int>(storage.offset) - height), width);
                else if (storage.kind == address_t::kind_t::shared)
                    emit(OP_PSH, OP_ARG_SDR, (module.lookup_index[storage.owner] << 16) | storage.offset, width);
                else
                    emit(OP_PSH, OP_ARG_VAR, (storage.variable_index << 16) | storage.owner, width);
                change(width);
                emit(storage.kind == address_t::kind_t::local ? OP_ASF : OP_COFF, OP_ARG_WORD, width);
                change(-4);
            } else if (storage.kind == address_t::kind_t::instance && !storage.index && storage.receiver == base &&
                       height == static_cast<int>(base + 4)) {
                emit(OP_PSH, OP_ARG_POPO, storage.offset, width);
                change(static_cast<int>(width) - 4);
            } else {
                load(storage);
                retain(base, width);
            }
            break;
        }
        case kind::call: {
            auto target = resolve(expression);
            if (target.intrinsic != intrinsic_t::none) {
                if (target.intrinsic == intrinsic_t::number_bits) {
                    emit(OP_PSH, OP_ARG_NUM, literal_bits(*expression.operands[1]));
                    change(4);
                } else if (target.intrinsic == intrinsic_t::class_value) {
                    emit(OP_PSH,
                         OP_ARG_CLV,
                         (module.symbols.class_ids.at(target.result.name) << 16) |
                             module.intern(expression.operands[1]->text));
                    change(4);
                } else if (target.intrinsic == intrinsic_t::integer_string ||
                           target.intrinsic == intrinsic_t::float_string) {
                    value(*expression.operands[1]);
                    emit(target.intrinsic == intrinsic_t::integer_string ? OP_I2S : OP_F2S,
                         OP_ARG_WORD,
                         string_temporary);
                } else
                    emit_service(expression, target);
                break;
            }
            auto argument_bytes = call_arguments(expression, target);
            if (target.native) {
                if (target.native->variadic) {
                    number(static_cast<float>(argument_bytes / 4));
                    argument_bytes += 4;
                }
                emit(OP_BSL, OP_ARG_LFR, (target.native->class_id << 16) | target.native->function_id);
            } else
                emit(OP_BSR, OP_ARG_SFR, (module.lookup_index[target.owner] << 16) | target.index);
            change(static_cast<int>(width) - static_cast<int>(argument_bytes));
            break;
        }
        case kind::unary:
            if (expression.text == "*") {
                auto storage = address(expression, false);
                if (storage.kind == address_t::kind_t::instance && !storage.index && storage.receiver == base &&
                    height == static_cast<int>(base + 4)) {
                    emit(OP_PSH, OP_ARG_POPO, storage.offset, width);
                    change(static_cast<int>(width) - 4);
                } else {
                    load(storage);
                    retain(base, width);
                }
            } else if ((expression.text == "++" || expression.text == "--") && type.name != "num") {
                const auto target = native_update(expression, type);
                auto storage = address(*expression.operands[0]);
                load(storage);
                emit(OP_BSL, OP_ARG_LFR, (target.native->class_id << 16) | target.native->function_id);
                change(static_cast<int>(width) - static_cast<int>(target.native->parameter_bytes));
                emit(OP_DUP, OP_ARG_NULL, 0, width);
                change(width);
                if (storage.kind == address_t::kind_t::local && !storage.index)
                    store_slot(storage.offset, width);
                else
                    fail("Native update target requires direct local storage");
                retain(base, width);
            } else if (expression.text == "++" || expression.text == "--") {
                auto storage = address(*expression.operands[0]);
                auto slot = static_cast<unsigned>(height);
                load(storage);
                emit(expression.text == "++" ? OP_INC : OP_DEC);
                store(storage, slot);
                retain(base, width);
            } else {
                value(*expression.operands[0]);
                if (expression.text == "-")
                    emit(OP_NEG);
                else if (expression.text == "!")
                    emit(OP_LNT);
                else if (expression.text == "~")
                    emit(OP_NOT);
            }
            break;
        case kind::postfix: {
            if (type.name != "num") {
                const auto target = native_update(expression, type);
                auto storage = address(*expression.operands[0]);
                load(storage);
                emit(OP_DUP, OP_ARG_NULL, 0, width);
                change(width);
                emit(OP_BSL, OP_ARG_LFR, (target.native->class_id << 16) | target.native->function_id);
                change(static_cast<int>(width) - static_cast<int>(target.native->parameter_bytes));
                if (storage.kind == address_t::kind_t::local && !storage.index)
                    store_slot(storage.offset, width);
                else
                    fail("Native update target requires direct local storage");
                retain(base, width);
                break;
            }
            if (direct_update(expression, true))
                break;
            auto storage = address(*expression.operands[0]);
            auto original = static_cast<unsigned>(height);
            load(storage);
            auto updated = static_cast<unsigned>(height);
            push_slot(original, width);
            emit(expression.text == "++" ? OP_INC : OP_DEC);
            store(storage, updated);
            adjust(-static_cast<int>(width));
            retain(base, width);
            break;
        }
        case kind::assignment: {
            value(*expression.operands[1]);
            auto storage = address(*expression.operands[0]);
            unsigned result = base;
            if (expression.text != "=") {
                result = height;
                load(storage);
                push_slot(base, width);
                operation(std::string_view(expression.text).substr(0, expression.text.size() - 1), type);
            }
            store(storage, result);
            if (result != base)
                store_slot(base, width);
            adjust(static_cast<int>(base + width) - height);
            break;
        }
        case kind::conditional: {
            auto otherwise = label(), end = label();
            value(*expression.operands[0]);
            edge(OP_BF, otherwise);
            value(*expression.operands[1]);
            edge(OP_BRA, end);
            mark(otherwise, base);
            value(*expression.operands[2]);
            mark(end, base + width);
            break;
        }
        case kind::binary:
            if (expression.text == "&&" || expression.text == "||") {
                auto rhs = label(), end = label();
                value(*expression.operands[0]);
                if (expression.text == "&&")
                    edge(OP_FEQZB, end);
                else {
                    edge(OP_FEQZB, rhs);
                    edge(OP_BRA, end);
                    mark(rhs, base + 4);
                }
                emit(OP_POP);
                change(-4);
                value(*expression.operands[1]);
                mark(end, base + 4);
            } else {
                auto operand = value(*expression.operands[0]);
                value(*expression.operands[1]);
                if (operand.name != "num" && operand.name != "str") {
                    const auto &target = calls.at(&expression);
                    emit(OP_BSL, OP_ARG_LFR, (target.native->class_id << 16) | target.native->function_id);
                    change(static_cast<int>(width) - static_cast<int>(target.native->parameter_bytes));
                } else
                    operation(expression.text, operand);
            }
            break;
        }
        if (height != static_cast<int>(base + width))
            fail("Internal expression stack imbalance");
        return type;
    }


    value_type_t condition_value(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        if (expression.kind != kind::binary || (expression.text != "&&" && expression.text != "||"))
            return value(expression);
        const auto base = height;
        auto rhs = label(), end = label();
        module.require(condition_value(*expression.operands[0]), {"num"});
        if (expression.text == "&&")
            edge(OP_FEQZB, end);
        else {
            edge(OP_FEQZB, rhs);
            edge(OP_BRA, end);
            mark(rhs, base + 4);
        }
        emit(OP_POP);
        change(-4);
        module.require(condition_value(*expression.operands[1]), {"num"});
        mark(end, base + 4);
        return {"num"};
    }

    void scope(const statement_t &statement)
    {
        auto start = height;
        auto count = locals.size();
        statements(statement);
        if (reachable)
            adjust(start - height);
        if (pending_discard) {
            const int amount = pending_discard;
            pending_discard = 0;
            emit(OP_SPA, OP_ARG_WORD, static_cast<uint16_t>(amount));
        }
        height = start;
        locals.resize(count);
    }


    bool discarded_assignment(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        if (expression.kind != kind::assignment || expression.text != "=")
            return false;
        const auto &lhs = *expression.operands[0];
        if (auto [member, receiver] = instance_array(lhs); member) {
            auto type = infer(lhs);
            const auto width = module.size(type);
            const auto start = height;
            module.require(value(*expression.operands[1]), type);
            module.require(value(*lhs.operands[1]), {"num"});
            emit(OP_PSF, OP_ARG_WORD, width);
            change(-4);
            if (receiver)
                value(*receiver);
            else
                push_slot(0, 4);
            emit(OP_POP, OP_ARG_POPO, member->offset, width);
            change(-static_cast<int>(width) - 4);
            if (height != start)
                fail("Indexed instance assignment stack imbalance");
            return true;
        }
        std::function<bool(const expression_t &)> supported = [&](const expression_t &e) {
            if (e.kind == kind::index) {
                const auto &base = *e.operands[0];
                if (base.kind == kind::name && local(base.text))
                    return local(base.text)->type.array();
                if (base.kind == kind::name && !local(base.text)) {
                    if (module.fields[owner].count(base.text))
                        return field(owner, base.text).is_static;
                    return module.program.native_variables.count(base.text) != 0;
                }
                if (base.kind == kind::member && base.bits && base.operands[0]->kind == kind::name &&
                    module.classes.count(base.operands[0]->text))
                    return field(module.classes.at(base.operands[0]->text), base.text).is_static;
                return false;
            }
            if (e.kind == kind::member && e.operands[0]->kind == kind::name &&
                module.program.native_variables.count(e.operands[0]->text))
                return false;
            for (const auto &child : e.operands)
                if (!supported(*child))
                    return false;
            return true;
        };
        if (!supported(lhs))
            return false;
        auto type = infer(lhs);
        if (type.array())
            return false;
        const auto start = height;
        module.require(value(*expression.operands[1]), type);
        const auto width = module.size(type);
        auto target = address(lhs);
        if (target.index) {
            if ((target.kind != address_t::kind_t::local && target.kind != address_t::kind_t::shared &&
                 (target.kind != address_t::kind_t::variable || target.offset)) ||
                target.stride != width || height != start + static_cast<int>(width) + 4)
                fail("Unexpected indexed direct assignment");
            emit(OP_PSF, OP_ARG_WORD, width);
            change(-4);
            if (target.kind == address_t::kind_t::local)
                emit(OP_POP, OP_ARG_SPR, static_cast<uint16_t>(static_cast<int>(target.offset) - height), width);
            else if (target.kind == address_t::kind_t::shared)
                emit(OP_POP, OP_ARG_SDR, (module.lookup_index[target.owner] << 16) | target.offset, width);
            else
                emit(OP_POP, OP_ARG_VAR, (target.variable_index << 16) | target.owner, width);
            change(-static_cast<int>(width));
            return true;
        }
        switch (target.kind) {
        case address_t::kind_t::local:
            store_slot(target.offset, width);
            break;
        case address_t::kind_t::shared:
            emit(OP_POP, OP_ARG_SDR, (module.lookup_index[target.owner] << 16) | target.offset, width);
            change(-static_cast<int>(width));
            break;
        case address_t::kind_t::variable:
            if (target.offset || width != target.variable_width)
                fail("Unexpected partial direct assignment");
            emit(OP_POP, OP_ARG_VAR, (target.variable_index << 16) | target.owner, width);
            change(-static_cast<int>(width));
            break;
        case address_t::kind_t::instance:
            emit(OP_POP, OP_ARG_POPO, target.offset, width);
            change(-static_cast<int>(width) - 4);
            break;
        }
        if (height != start)
            fail("Discarded assignment stack imbalance");
        return true;
    }


    bool direct_return(const expression_t &expression)
    {
        using kind = expression_t::kind_t;
        const auto saved = height;
        unsigned return_frame = static_cast<unsigned>(saved);
        const auto width = module.size({source.result});
        module.require(infer(expression), {source.result});
        if (expression.kind == kind::number || expression.kind == kind::string ||
            (expression.kind == kind::name &&
             (expression.text == "nullptr" || expression.text == "true" || expression.text == "false" ||
              module.program.string_objects.count(expression.text)))) {
            value(expression);
            return_frame = static_cast<unsigned>(height);
            adjust(-height);
            push_slot(static_cast<unsigned>(saved), width);
        } else {
            if (expression.kind != kind::name && expression.kind != kind::member)
                return false;
            std::function<bool(const expression_t &)> simple = [&](const expression_t &e) {
                if (e.kind != kind::name && e.kind != kind::member)
                    return false;
                for (const auto &child : e.operands)
                    if (!simple(*child))
                        return false;
                return true;
            };
            if (!simple(expression))
                return false;
            const auto old_count = binary.code.size();
            const auto old_offset = offset;
            const int deferred = pending_discard;
            pending_discard = 0;
            auto storage = address(expression, false);
            pending_discard = deferred;
            const bool self = storage.kind == address_t::kind_t::instance && binary.code.size() == old_count + 1 &&
                              binary.code.back().opcode == OP_PSH && binary.code.back().argument == OP_ARG_SPR &&
                              static_cast<int16_t>(binary.code.back().value) == -saved;
            if (storage.index || (storage.kind == address_t::kind_t::instance && !self) ||
                (storage.kind == address_t::kind_t::variable && (storage.offset || width != storage.variable_width))) {
                binary.code.resize(old_count);
                offset = old_offset;
                height = saved;
                return false;
            }
            binary.code.resize(old_count);
            offset = old_offset;
            height = saved;
            if (storage.kind == address_t::kind_t::local && storage.offset == 0) {
                adjust(static_cast<int>(width) - height);
            } else if (self) {
                adjust(4 - height);
                emit(OP_PSH, OP_ARG_POPO, storage.offset, width);
                change(static_cast<int>(width) - 4);
            } else {
                adjust(-height);
                load(storage);
            }
        }
        emit(OP_RET, OP_ARG_WORD, return_frame);
        height = saved;
        reachable = false;
        return true;
    }

    bool stable_target(const expression_t &expression) const
    {
        using kind = expression_t::kind_t;
        return expression.kind == kind::name || (expression.kind == kind::member && expression.operands.size() == 1 &&
                                                 stable_target(*expression.operands[0]));
    }
    bool same_target(const expression_t &a, const expression_t &b) const
    {
        if (a.kind != b.kind || a.text != b.text || a.bits != b.bits || a.operands.size() != b.operands.size())
            return false;
        for (unsigned i = 0; i < a.operands.size(); ++i)
            if (!same_target(*a.operands[i], *b.operands[i]))
                return false;
        return true;
    }
    const expression_t *single_assignment(const statement_t &statement) const
    {
        using kind = statement_t::kind_t;
        if (statement.kind == kind::block) {
            if (statement.children.size() != 1)
                return nullptr;
            return single_assignment(*statement.children.front());
        }
        if (statement.kind != kind::expression || statement.expressions.size() != 1)
            return nullptr;
        const auto &e = *statement.expressions.front();
        return e.kind == expression_t::kind_t::assignment && e.text == "=" ? &e : nullptr;
    }
    void statements(const statement_t &statement)
    {
        using kind = statement_t::kind_t;
        if (pending_discard && statement.kind != kind::return_value && statement.kind != kind::block) {
            const int amount = pending_discard;
            pending_discard = 0;
            emit(OP_SPA, OP_ARG_WORD, static_cast<uint16_t>(amount));
        }
        switch (statement.kind) {
        case kind::block:
            for (const auto &child : statement.children) {
                if (child->kind == kind::block)
                    scope(*child);
                else
                    statements(*child);
            }
            break;
        case kind::declaration: {
            const auto &declaration = statement.declaration;
            value_type_t type{declaration.type, declaration.count, declaration.is_array};
            auto slot = static_cast<unsigned>(height);
            if (!statement.expressions.empty()) {
                auto actual = value(*statement.expressions[0]);
                if (type.name == "auto")
                    type = actual;
                else
                    module.require(actual, type);
            } else {
                if (!module.size(type))
                    fail("A local cannot have void type");
                adjust(module.size(type));
            }
            if (declaration.offset && *declaration.offset != slot)
                fail("Local declaration disagrees with @offset");
            locals.push_back({declaration.name, type, slot});
            break;
        }
        case kind::expression: {
            const auto &expression = *statement.expressions[0];
            if (expression.kind == expression_t::kind_t::unary &&
                (expression.text == "++" || expression.text == "--") && direct_update(expression, false))
                break;
            if (assign_aggregate(*statement.expressions[0]))
                break;
            if (discarded_assignment(*statement.expressions[0]))
                break;
            auto start = height;
            const auto prior_strings = string_temporary;
            value(*statement.expressions[0]);
            const unsigned discarded = static_cast<unsigned>(height - start) / 4;
            string_temporary = statement.expressions[0]->kind == expression_t::kind_t::postfix ? prior_strings
                               : discarded >= prior_strings                                    ? 0
                                                            : prior_strings - discarded;
            pending_discard += start - height;
            change(start - height);
            break;
        }

        case kind::switch_statement: {
            module.require(value(*statement.expressions[0]), {"num"});
            const int start = height;
            const auto end = label();
            std::vector<unsigned> arms(statement.children.size());
            for (unsigned i = static_cast<unsigned>(statement.children.size()); i-- > 0;)
                arms[i] = statement.children[i]->children.empty() ? (i + 1 < arms.size() ? arms[i + 1] : end) : label();
            unsigned fallback = end;
            for (unsigned i = 0; i < statement.children.size(); ++i) {
                const auto &arm = *statement.children[i];
                if (arm.expressions.empty()) {
                    fallback = arms[i];
                    continue;
                }
                const auto next = label();
                push_slot(start - 4, 4);
                module.require(value(*arm.expressions[0]), {"num"});
                emit(OP_EQ);
                change(-4);
                edge(OP_BF, next);
                edge(OP_BRA, arms[i]);
                mark(next, start);
            }
            std::vector<unsigned> body_order;
            for (unsigned i = 0; i < statement.children.size(); ++i)
                if (statement.children[i]->expressions.empty() && !statement.children[i]->children.empty())
                    body_order.push_back(i);
            for (unsigned i = static_cast<unsigned>(statement.children.size()); i-- > 0;)
                if (!statement.children[i]->expressions.empty() && !statement.children[i]->children.empty())
                    body_order.push_back(i);
            if (body_order.empty() || arms[body_order.front()] != fallback)
                edge(OP_BRA, fallback);
            loops.push_back({end, end, start, true});
            const unsigned switch_strings = string_temporary;
            for (unsigned i : body_order) {
                string_temporary = switch_strings;
                mark(arms[i], start);
                const auto locals_before = locals.size();
                for (const auto &child : statement.children[i]->children)
                    statements(*child);
                if (reachable) {
                    adjust(start - height);
                    edge(OP_BRA, i + 1 < arms.size() ? arms[i + 1] : end);
                }
                height = start;
                locals.resize(locals_before);
            }
            string_temporary = switch_strings;
            loops.pop_back();
            mark(end, start);
            break;
        }
        case kind::switch_case:
            fail("Case outside switch");
        case kind::conditional: {
            bool shared_expression = false;
            if (statement.children.size() == 2) {
                const auto a = single_assignment(*statement.children[0]), b = single_assignment(*statement.children[1]);
                shared_expression =
                    a && b && stable_target(*a->operands[0]) && same_target(*a->operands[0], *b->operands[0]);
            }
            const auto &predicate = *statement.expressions[0];
            const bool literal_predicate = predicate.kind == expression_t::kind_t::number;
            if (literal_predicate)
                ++string_temporary;
            const unsigned conditional_strings = string_temporary;
            if (shared_expression)
                ++value_depth;
            struct conditional_scope_t {
                unsigned &depth;
                unsigned &strings;
                unsigned prior;
                bool active;
                ~conditional_scope_t()
                {
                    if (active)
                        --depth;
                    else
                        strings = prior;
                }
            } conditional_scope{value_depth, string_temporary, conditional_strings, shared_expression};
            int start = height;
            auto otherwise = label(), end = label();
            module.require(condition_value(*statement.expressions[0]), {"num"});
            edge(OP_BF, otherwise);
            scope(*statement.children[0]);
            bool then_live = reachable;
            if (then_live && statement.children.size() == 2)
                edge(OP_BRA, end);
            mark(otherwise, start);
            if (!shared_expression)
                string_temporary = conditional_strings;
            if (statement.children.size() == 2)
                scope(*statement.children[1]);
            bool else_live = reachable;
            if (then_live || else_live)
                mark(end, start);
            else
                reachable = false;
            break;
        }
        case kind::while_loop:
        case kind::do_loop:
        case kind::for_loop: {
            int outer = height;
            auto local_count = locals.size();
            if (statement.kind == kind::for_loop)
                statements(*statement.children[0]);
            int start = height;
            auto begin = label(), step = label(), end = label();
            mark(begin, start);
            if (statement.kind != kind::do_loop && statement.expressions[0]) {
                const auto &condition = *statement.expressions[0];
                module.require(condition_value(condition), {"num"});
                edge(OP_BF, end);
            }
            loops.push_back({end, step, start});
            scope(*statement.children[statement.kind == kind::for_loop ? 1 : 0]);
            loops.pop_back();
            mark(step, start);
            if (statement.kind == kind::do_loop) {
                const auto &condition = *statement.expressions[0];
                module.require(condition_value(condition), {"num"});
                edge(OP_BF, end);
            } else if (statement.kind == kind::for_loop && statement.expressions[1]) {
                value(*statement.expressions[1]);
                adjust(start - height);
            }
            edge(OP_BRA, begin);
            mark(end, start);
            if (string_temporary)
                --string_temporary;
            adjust(outer - height);
            locals.resize(local_count);
            break;
        }
        case kind::return_value: {
            if (!source.destructor && !statement.expressions.empty() && direct_return(*statement.expressions[0]))
                break;
            auto saved = height;
            if (!statement.expressions.empty())
                module.require(value(*statement.expressions[0]), {source.result});
            else if (source.result != "void")
                fail("A value-returning function requires a return expression");
            finalize_base();
            retain(0, module.size({source.result}));
            emit(OP_RET, OP_ARG_WORD, static_cast<unsigned>(saved));
            height = saved;
            reachable = false;
            break;
        }
        case kind::break_loop:
        case kind::continue_loop: {
            if (loops.empty())
                fail("Loop control used outside a loop");
            auto target = loops.rbegin();
            if (statement.kind == kind::continue_loop)
                while (target != loops.rend() && target->is_switch)
                    ++target;
            if (target == loops.rend())
                fail("Continue requires an enclosing loop");
            auto saved = height;
            adjust(target->height - height);
            edge(OP_BRA, statement.kind == kind::break_loop ? target->end : target->next);
            height = saved;
            reachable = false;
            break;
        }
        case kind::jump:
            edge(OP_BRA, named(statement.name));
            reachable = false;
            break;
        case kind::label:
            mark(named(statement.name), height);
            break;
        }
    }

    void initialize_base()
    {
        const auto &parent = module.program.image.objects[owner].parent;
        if (!source.constructor || !parent)
            return;
        auto index = module.classes.at(*parent);
        const auto &constructor = module.program.classes[index].functions.front();
        if (constructor.entry) {
            if (!source.base_arguments.empty())
                fail("An implicit empty base constructor takes no arguments");
            return;
        }
        if (!accessible(constructor.access, index, module.program.image.objects[owner].name))
            fail("Base constructor is not accessible");
        if (constructor.parameters.size() != source.base_arguments.size())
            fail("Base constructor requires an explicit matching initializer");
        push_slot(0, 4);
        for (unsigned i = 0; i < source.base_arguments.size(); ++i)
            module.require(
                value(*source.base_arguments[i]),
                {constructor.parameters[i].type, constructor.parameters[i].count, constructor.parameters[i].is_array});
        const auto &record = module.program.image.objects[index].functions.front();
        emit(OP_BSR, OP_ARG_SFR, module.lookup_index[index] << 16);
        const auto result_size = module.size({constructor.result});
        change(static_cast<int>(result_size) - static_cast<int>(record.parameters_size));
        adjust(-static_cast<int>(result_size));
    }

    void finalize_base()
    {
        const auto &parent = module.program.image.objects[owner].parent;
        if (!source.destructor || !parent)
            return;
        auto index = module.classes.at(*parent);
        const auto &object = module.program.image.objects[index];
        if (object.destructor == -1)
            return;
        if (!accessible(module.program.classes[index].functions[object.destructor].access,
                        index,
                        module.program.image.objects[owner].name))
            fail("Base destructor is not accessible");
        push_slot(0, 4);
        emit(OP_BSR, OP_ARG_SFR, (module.lookup_index[index] << 16) | static_cast<unsigned>(object.destructor));
        change(-4);
    }

public:
    function_compiler_t(compiler_t &module, unsigned owner, const language::function_t &source,
                        sxcc::function_t &binary, bool initializer = false)
        : module(module), owner(owner), source(source), binary(binary), height(binary.parameters_size),
          capacity(pc_vm_stack_capacity), initializer(initializer)
    {
        unsigned slot = 0;
        if (!(binary.flags & 1u)) {
            locals.push_back({"this", {module.program.image.objects[owner].name}, 0});
            slot = 4;
        }
        for (const auto &parameter : source.parameters) {
            locals.push_back({parameter.name, {parameter.type, parameter.count, parameter.is_array}, slot});
            slot += module.size({parameter.type, parameter.count});
        }
        if (height > capacity)
            fail("Parameters exceed VM stack capacity");
    }

    void run()
    try {
        initialize_base();
        statements(*source.body);
        if (reachable) {
            if (source.result != "void")
                fail("Function can reach its end without returning a value");
            finalize_base();
            if (source.constructor && !binary.code.empty() && binary.code.back().opcode == OP_SPA &&
                static_cast<int16_t>(binary.code.back().value) < 0) {
                auto &cleanup = binary.code.back();
                const int amount = static_cast<int16_t>(cleanup.value) - height;
                if (amount < INT16_MIN)
                    fail("Constructor cleanup exceeds VM displacement range");
                cleanup.value = static_cast<uint16_t>(amount);
                height = 0;
            } else
                adjust(-height);
            emit(OP_RET, OP_ARG_WORD, binary.parameters_size);
        }
        for (const auto &fixup : fixups) {
            const auto &target = labels[fixup.label];
            if (!target.offset)
                fail("Undefined label");
            auto &instruction = binary.code[fixup.instruction];
            auto displacement = static_cast<std::int64_t>(*target.offset) - instruction.offset - instruction.size();
            if (displacement < INT16_MIN || displacement > INT16_MAX)
                fail("Branch exceeds VM displacement range");
            instruction.value = static_cast<std::uint16_t>(displacement);
        }
    } catch (const error_t &error) {
        const auto context = module.program.image.objects[owner].name + "::" + source.name + ": ";
        if (std::string_view(error.what()).starts_with(context))
            throw;
        fail(error.what());
    }
};

image_t compiler_t::run()
{
    auto old_order = std::move(program.image.object_order);
    layout();
    initialize_fields();
    std::stable_sort(text_initializers.begin(), text_initializers.end(), [](const auto &a, const auto &b) {
        return a.line < b.line;
    });
    std::vector<std::pair<unsigned, const language::function_t *>> definitions;
    for (const auto &owner : program.classes)
        for (const auto &function : owner.functions)
            definitions.emplace_back(owner.index, &function);
    std::stable_sort(definitions.begin(), definitions.end(), [&](const auto &a, const auto &b) {
        return program.image.objects[a.first].functions[a.second->index].code_order <
               program.image.objects[b.first].functions[b.second->index].code_order;
    });
    struct definition_t {
        unsigned line, owner;
        const language::function_t *source;
        sxcc::function_t *binary;
        bool initializer;
    };
    std::vector<definition_t> ordered;
    for (const auto &[owner, function] : definitions)
        ordered.push_back(
            {function->body->line, owner, function, &program.image.objects[owner].functions[function->index], false});
    if (program.initializations.size() != program.image.initializers.size())
        throw error_t("Initializer declarations disagree with image metadata");
    for (unsigned i = 0; i < program.initializations.size(); ++i) {
        const auto &source = program.initializations[i];
        auto &initializer = program.image.initializers[i];
        if (source.object_name) {
            auto found = classes.find(*source.object_name);
            if (found == classes.end())
                throw error_t("Unknown initializer class " + *source.object_name);
            initializer.object = lookup_index[found->second];
        } else if (!old_order.empty()) {
            if (initializer.object >= old_order.size() || old_order[initializer.object] >= lookup_index.size())
                throw error_t("Initializer class order is invalid");
            initializer.object = lookup_index[old_order[initializer.object]];
        }
        if (initializer.object >= program.image.object_order.size())
            throw error_t("Initializer class index is invalid");
        if (source.builder.has_value() != initializer.builder.has_value())
            throw error_t("Initializer builder declaration is missing");
        if (!source.builder)
            continue;
        auto owner = program.image.object_order[initializer.object];
        const auto &object = program.image.objects[owner];
        if (object.flags & 1u)
            throw error_t("An initializer cannot construct the global class");
        auto &binary = *initializer.builder;
        layout_parameters(*source.builder, binary, false);
        if (binary.parameters_size)
            throw error_t("An initializer builder cannot require external arguments");
        const auto &constructor = object.functions.front();
        auto width = constructor.parameters_size - ((constructor.flags & 1u) ? 0u : 4u);
        if (size({source.builder->result}) != width)
            throw error_t("Initializer result does not match constructor argument storage");
        ordered.push_back({source.builder->body->line, owner, &*source.builder, &binary, true});
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) { return a.line < b.line; });
    unsigned code_order = 0;
    for (const auto &definition : ordered) {
        initialize_text_before(definition.line);
        auto &binary = *definition.binary;
        const auto flags = binary.flags;
        if (definition.initializer)
            binary.flags |= 1u;
        binary.code_order = code_order++;
        function_compiler_t(*this, definition.owner, *definition.source, binary, definition.initializer).run();
        binary.flags = flags;
    }
    initialize_text_before(UINT32_MAX);
    validate_image(program.image);
    return std::move(program.image);
}
void compiler_t::initialize_fields()
{
    using expression_kind = expression_t::kind_t;
    auto append_assignment = [&](const std::string &name, std::unique_ptr<expression_t> value) {
        auto assignment = std::make_unique<expression_t>();
        assignment->kind = expression_kind::assignment;
        assignment->text = "=";
        auto destination = std::make_unique<expression_t>();
        destination->kind = expression_kind::name;
        destination->text = name;
        assignment->operands.push_back(std::move(destination));
        assignment->operands.push_back(std::move(value));
        auto statement = std::make_unique<statement_t>();
        statement->kind = statement_t::kind_t::expression;
        statement->expressions.push_back(std::move(assignment));
        return statement;
    };
    for (auto &owner : program.classes) {
        std::vector<std::unique_ptr<statement_t>> assignments;
        for (auto &[name, expression] : owner.field_initializers) {
            const auto &field = fields[owner.index].at(name);
            if (!field.is_static) {
                assignments.push_back(append_assignment(name, std::move(expression)));
                continue;
            }
            auto &data = program.image.objects[owner.index].static_data;
            std::function<bool(const expression_t &, unsigned, value_type_t)> constant;
            constant = [&](const expression_t &value, unsigned offset, value_type_t expected) {
                std::uint32_t bits;
                if (value.kind == expression_kind::number)
                    bits = value.bits;
                else if (value.kind == expression_kind::name && value.text == "nullptr")
                    bits = 0;
                else if (value.kind == expression_kind::unary && value.text == "-" &&
                         value.operands[0]->kind == expression_kind::number)
                    bits = value.operands[0]->bits ^ 0x80000000u;
                else if (value.kind == expression_kind::aggregate) {
                    if (value.operands.empty())
                        return true;
                    value_type_t element = expected;
                    unsigned count = expected.count;
                    if (expected.array()) {
                        element.count = 1;
                        element.is_array = false;
                    } else if (value.operands.size() == 1 &&
                               (size(expected) == 4 || value.operands[0]->kind == expression_kind::aggregate))
                        return constant(*value.operands[0], offset, expected);
                    else if (expected.name == "vector3d" || expected.name == "position3d") {
                        element = {"num"};
                        count = 3;
                    } else
                        return false;
                    if (value.operands.size() > count)
                        throw error_t("Too many aggregate initializer elements");
                    for (unsigned i = 0; i < value.operands.size(); ++i)
                        if (!constant(*value.operands[i], offset + i * size(element), element))
                            return false;
                    return true;
                } else if (value.kind == expression_kind::string) {
                    require({"str"}, expected);
                    if (owner.index != 0)
                        return false;
                    initializer_t initializer;
                    initializer.offset = offset;
                    initializer.kind = 0;
                    text_initializers.push_back(
                        {static_cast<unsigned>(program.image.initializers.size()), value.line, value.text});
                    initialization_t source;
                    source.object_name = program.image.objects[owner.index].name;
                    program.image.initializers.push_back(std::move(initializer));
                    program.initializations.push_back(std::move(source));
                    return true;
                } else
                    return false;
                if (value.kind != expression_kind::name)
                    require({"num"}, expected);
                else if (size(expected) != 4 || expected.name == "num")
                    throw error_t("A null initializer requires a handle type");
                put_word(data, offset, bits);
                if (owner.index == 0) {
                    initializer_t initializer;
                    initializer.offset = offset;
                    initializer.kind = -4;
                    initializer.value = bits;
                    initialization_t source;
                    source.object_name = program.image.objects[0].name;
                    program.image.initializers.push_back(std::move(initializer));
                    program.initializations.push_back(std::move(source));
                }
                return true;
            };
            if (constant(*expression, field.offset, field.type))
                continue;
            auto instance_name = name_hash(name);
            bool game_initialized = false;
            if (expression->kind == expression_kind::call && !expression->operands.empty()) {
                auto &callee = *expression->operands.front();
                if (callee.kind == expression_kind::member && callee.bits &&
                    (callee.text == "create_named" || callee.text == "from_game_data")) {
                    if (callee.operands.size() != 1 || callee.operands[0]->kind != expression_kind::name ||
                        callee.operands[0]->text != field.type.name || !classes.count(field.type.name))
                        throw error_t("A persistent instance factory requires its declared script class");
                    game_initialized = callee.text == "from_game_data";
                    if (expression->operands.size() < 2 || (game_initialized && expression->operands.size() != 2))
                        throw error_t("A persistent instance factory requires its instance name");
                    const auto &identity = *expression->operands[1];
                    if (identity.kind == expression_kind::string)
                        instance_name = name_hash(identity.text);
                    else if (identity.kind == expression_kind::number) {
                        std::string_view text = identity.text;
                        int base = 10;
                        if (text.starts_with("0x") || text.starts_with("0X")) {
                            text.remove_prefix(2);
                            base = 16;
                        }
                        const auto parsed =
                            std::from_chars(text.data(), text.data() + text.size(), instance_name, base);
                        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
                            throw error_t("Instance identity must be a string or unsigned resource identifier");
                    } else
                        throw error_t("Instance identity must be a string or unsigned resource identifier");
                    callee.kind = expression_kind::name;
                    callee.text = field.type.name;
                    callee.operands.clear();
                    expression->operands.erase(expression->operands.begin() + 1);
                }
            }
            if (expression->kind == expression_kind::call && !expression->operands.empty() &&
                expression->operands[0]->kind == expression_kind::name &&
                classes.count(expression->operands[0]->text)) {
                if (owner.index != 0 || field.type.array() || field.type.name != expression->operands[0]->text)
                    throw error_t("A persistent script instance requires a matching global field");
                auto target = classes.at(field.type.name);
                const auto &constructor = program.classes[target].functions.front();
                if (!game_initialized && constructor.parameters.size() + 1 != expression->operands.size())
                    throw error_t("Persistent instance constructor argument count mismatch");
                initializer_t initializer;
                initializer.name_hash = instance_name;
                initializer.offset = field.offset;
                initializer.kind = game_initialized ? -2 : constructor.parameters.empty() ? -3 : -1;
                initialization_t source;
                source.object_name = field.type.name;
                if (!game_initialized && !constructor.parameters.empty()) {
                    unsigned width = 0;
                    for (const auto &parameter : constructor.parameters)
                        width += size({parameter.type, parameter.count});
                    language::function_t builder;
                    builder.name = "initialize_" + name;
                    builder.result = constructor.parameters.size() == 1 ? constructor.parameters[0].type
                                                                        : "opaque<" + std::to_string(width) + ">";
                    builder.body = std::make_unique<statement_t>();
                    builder.body->line = expression->line;
                    auto result = std::make_unique<statement_t>();
                    result->kind = statement_t::kind_t::return_value;
                    if (constructor.parameters.size() == 1)
                        result->expressions.push_back(std::move(expression->operands[1]));
                    else {
                        auto arguments = std::make_unique<expression_t>();
                        arguments->kind = expression_kind::aggregate;
                        arguments->text = builder.result;
                        for (unsigned i = 1; i < expression->operands.size(); ++i)
                            arguments->operands.push_back(std::move(expression->operands[i]));
                        result->expressions.push_back(std::move(arguments));
                    }
                    builder.body->children.push_back(std::move(result));
                    sxcc::function_t binary;
                    binary.name = builder.name + "()";
                    binary.name_hash = name_hash(builder.name);
                    binary.fullname_hash = name_hash(binary.name);
                    binary.flags = 5;
                    binary.code_order = static_cast<unsigned>(next_code_order());
                    initializer.builder = std::move(binary);
                    source.builder = std::move(builder);
                }
                program.image.initializers.push_back(std::move(initializer));
                program.initializations.push_back(std::move(source));
            } else {
                assignments.push_back(append_assignment(name, std::move(expression)));
            }
        }
        auto &body = owner.functions.front().body->children;
        body.insert(
            body.begin(), std::make_move_iterator(assignments.begin()), std::make_move_iterator(assignments.end()));
    }
}

}

image_t lower(program_t program, const symbols_t &symbols)
{
    return compiler_t(std::move(program), symbols).run();
}

}
