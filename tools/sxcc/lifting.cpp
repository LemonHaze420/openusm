#include "lifting.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>

namespace sxcc {
namespace {

using references_t = std::vector<value_ref_t>;
using value_kind = lifted_value_t::kind_t;
using effect_kind = lifted_effect_t::kind_t;
using address_kind = lifted_address_t::kind_t;

std::string opaque_type(unsigned width)
{
    return width ? "opaque<" + std::to_string(width) + '>' : "void";
}

class value_lifter_t {
    const image_t &image;
    const symbols_t &symbols;
    const stack_analysis_t &analysis;
    const stack_function_t &facts;
    const function_t &binary;
    const std::map<std::pair<unsigned, unsigned>, unsigned> &function_ids;
    lifted_function_t result;
    const function_analysis_t &cfg;
    std::vector<bool> reachable;
    references_t memory;
    unsigned footprint = 0;
    unsigned block_index = 0;
    unsigned instruction_index = 0;
    int height = 0;
    std::optional<value_ref_t> array_index;
    value_ref_t signal_parent;

    [[noreturn]] void fail(const std::string &message) const
    {
        throw error_t("At " + hex(binary.code[instruction_index].offset) + ": " + message);
    }

    unsigned node(value_kind kind, unsigned width, std::string type = {})
    {
        auto index = static_cast<unsigned>(result.values.size());
        lifted_value_t value;
        value.kind = kind;
        value.width = width;
        value.type = type.empty() ? opaque_type(width) : std::move(type);
        value.instruction = instruction_index;
        result.values.push_back(std::move(value));
        return index;
    }

    references_t components(unsigned value) const
    {
        references_t refs;
        refs.reserve(result.values[value].width / 4);
        for (unsigned offset = 0; offset < result.values[value].width; offset += 4)
            refs.push_back({value, offset});
        return refs;
    }

    references_t read(int offset, unsigned width) const
    {
        if (offset < 0 || offset % 4 || width % 4 || static_cast<unsigned>(offset) > footprint ||
            width > footprint - static_cast<unsigned>(offset))
            fail("Value access exceeds word-aligned frame storage");
        return {memory.begin() + offset / 4, memory.begin() + (offset + width) / 4};
    }

    void write(int offset, const references_t &refs)
    {
        if (offset < 0 || offset % 4 || static_cast<unsigned>(offset) > footprint ||
            refs.size() > (footprint - offset) / 4)
            fail("Value write exceeds frame storage");
        std::copy(refs.begin(), refs.end(), memory.begin() + offset / 4);
    }

    references_t pop(unsigned width)
    {
        height -= static_cast<int>(width);
        return read(height, width);
    }

    value_ref_t pop_word()
    {
        return pop(4).front();
    }

    void push(const references_t &refs)
    {
        write(height, refs);
        height += static_cast<int>(refs.size() * 4);
    }

    void value_effect(unsigned value)
    {
        lifted_effect_t effect;
        effect.instruction = instruction_index;
        effect.value = value;
        result.blocks[block_index].effects.push_back(std::move(effect));
    }

    void effect(effect_kind kind, std::vector<references_t> arguments = {})
    {
        lifted_effect_t effect;
        effect.kind = kind;
        effect.instruction = instruction_index;
        effect.arguments = std::move(arguments);
        result.blocks[block_index].effects.push_back(std::move(effect));
    }

    references_t constant(std::uint32_t bits)
    {
        auto id = node(value_kind::constant, 4, "num");
        result.values[id].immediate = bits;
        return components(id);
    }

    references_t string(std::uint32_t operand)
    {
        auto id = node(value_kind::string, 4, "str");
        result.values[id].immediate = operand >> 16;
        return components(id);
    }

    unsigned target(std::uint32_t operand) const
    {
        return function_ids.at({image.object_order.at(operand >> 16), operand & 0xffff});
    }

    const function_t &function(unsigned id) const
    {
        const auto &record = analysis.functions[id];
        if (record.initializer)
            return *image.initializers[record.function].builder;
        return image.objects[record.object].functions[record.function];
    }

    unsigned type_size(std::string_view name) const
    {
        auto native = symbols.type_sizes.find(std::string(name));
        if (native != symbols.type_sizes.end())
            return native->second;
        auto hash = name_hash(name);
        for (const auto &object : image.objects)
            if (object.name_hash == hash)
                return 4;
        return 0;
    }

    references_t parameters()
    {
        if (facts.initializer)
            return {};
        std::vector<std::pair<std::string, unsigned>> types;
        unsigned size = 0;
        if (!(binary.flags & VM_EXECUTABLE_FLAG_STATIC) && binary.parameters_size >= 4) {
            types.emplace_back(image.objects[facts.object].name, 4);
            size = 4;
        }
        auto begin = binary.name.find('(');
        bool known = begin != std::string::npos && binary.name.ends_with(")");
        if (known) {
            auto parameters = std::string_view(binary.name).substr(begin + 1);
            parameters.remove_suffix(1);
            while (!parameters.empty()) {
                auto comma = parameters.find(',');
                auto type = parameters.substr(0, comma);
                auto width = type_size(type);
                if (!width) {
                    known = false;
                    break;
                }
                types.emplace_back(type, width);
                size += width;
                if (comma == std::string_view::npos)
                    break;
                parameters.remove_prefix(comma + 1);
            }
        }
        if (!known || size != binary.parameters_size) {
            types.clear();
            for (unsigned offset = 0; offset < binary.parameters_size; offset += 4) {
                auto type = !(binary.flags & VM_EXECUTABLE_FLAG_STATIC) && offset == 0
                                ? image.objects[facts.object].name
                                : opaque_type(4);
                types.emplace_back(std::move(type), 4);
            }
        }
        references_t refs;
        for (const auto &[type, width] : types) {
            auto id = node(value_kind::parameter, width, type);
            result.values[id].instruction = UINT32_MAX;
            result.values[id].immediate = static_cast<unsigned>(refs.size() * 4);
            auto parts = components(id);
            refs.insert(refs.end(), parts.begin(), parts.end());
        }
        return refs;
    }

    lifted_address_t address(const instruction_t &instruction, unsigned width)
    {
        lifted_address_t address;
        address.width = width;
        auto operand = instruction.value;
        switch (instruction.argument) {
        case OP_ARG_SPR:
            address.offset = static_cast<unsigned>(height + static_cast<std::int16_t>(operand));
            break;
        case OP_ARG_SDR:
            address.kind = address_kind::shared;
            address.owner = image.object_order.at(operand >> 16);
            address.offset = operand & 0xffff;
            break;
        case OP_ARG_POPO:
            address.kind = address_kind::instance;
            address.offset = operand;
            address.receiver = pop_word();
            break;
        case OP_ARG_VAR:
            address.kind = (operand & 0xffff) == 1 ? address_kind::game_variable : address_kind::shared_variable;
            address.offset = operand >> 16;
            break;
        default:
            fail("Operand is not a storage location");
        }
        return address;
    }

    references_t load(lifted_address_t address)
    {
        if (address.kind == address_kind::local && !address.index) {
            auto refs = read(static_cast<int>(address.offset), address.width);
            if (!refs.empty() && !refs.front().offset && result.values[refs.front().value].width == address.width) {
                bool contiguous = true;
                for (unsigned word = 0; word < refs.size(); ++word)
                    if (refs[word] != value_ref_t{refs.front().value, word * 4})
                        contiguous = false;
                if (contiguous) {
                    auto &v = result.values[refs.front().value];
                    if (!v.ambiguous_read_home) {
                        if (v.local_read_home && *v.local_read_home != address.offset) {
                            v.local_read_home.reset();
                            v.ambiguous_read_home = true;
                        } else
                            v.local_read_home = address.offset;
                    }
                }
            }
            const bool prefix = address.offset + address.width <= static_cast<unsigned>(height);
            if (prefix) {
                auto id = node(value_kind::operation, address.width);
                result.values[id].arguments.push_back(refs);
                value_effect(id);
                return components(id);
            }
            return refs;
        }
        auto id = node(value_kind::load, address.width);
        if (address.kind == address_kind::local)
            result.values[id].arguments.push_back(memory);
        result.values[id].address = std::move(address);
        value_effect(id);
        return components(id);
    }

    void store(lifted_address_t address, references_t refs)
    {
        if (address.kind == address_kind::local && !address.index) {
            bool alias = false;
            if (refs.size() == 1)
                for (int at = 0; at < height; at += 4)
                    if (static_cast<unsigned>(at) != address.offset && memory[at / 4] == refs.front())
                        alias = true;
            bool unread = true;
            for (unsigned pc = 0; pc < binary.code.size(); ++pc) {
                const auto &op = binary.code[pc];
                if (op.opcode == OP_ASF) {
                    unread = false;
                    break;
                }
                if (op.opcode != OP_PSH || op.argument != OP_ARG_SPR || !facts.heights[pc])
                    continue;
                const int from = *facts.heights[pc] + static_cast<int16_t>(op.value);
                if (from < static_cast<int>(address.offset + address.width) &&
                    from + static_cast<int>(op.data_size()) > static_cast<int>(address.offset)) {
                    unread = false;
                    break;
                }
            }
            for (unsigned pc = instruction_index + 1; pc < result.blocks[block_index].end; ++pc) {
                const auto &op = binary.code[pc];
                if (op.opcode == OP_ASF || op.opcode == OP_PSF || op.opcode == OP_COFF || op.opcode == OP_CPY)
                    break;
                if (op.argument != OP_ARG_SPR || !facts.heights[pc])
                    continue;
                const int from = *facts.heights[pc] + static_cast<int16_t>(op.value);
                if (from >= static_cast<int>(address.offset + address.width) ||
                    from + static_cast<int>(op.data_size()) <= static_cast<int>(address.offset))
                    continue;
                if (op.opcode == OP_POP && from == static_cast<int>(address.offset) && op.data_size() == address.width)
                    unread = true;
                break;
            }
            bool literal = refs.size() == 1 && !refs.front().offset &&
                           (result.values[refs.front().value].kind == value_kind::constant ||
                            result.values[refs.front().value].kind == value_kind::string);
            const bool self_copy =
                refs.size() == 1 && address.offset / 4 < memory.size() && memory[address.offset / 4] == refs.front();
            if (!alias && !unread && !literal && !self_copy) {
                write(static_cast<int>(address.offset), refs);
                return;
            }
            auto id = node(value_kind::operation, address.width);
            result.values[id].arguments.push_back(refs);
            result.values[id].address = address;
            value_effect(id);
            write(static_cast<int>(address.offset), components(id));
            return;
        }
        if (address.kind == address_kind::local) {
            auto id = node(value_kind::indexed_store, footprint);
            auto &value = result.values[id];
            value.address = std::move(address);
            value.arguments.push_back(std::move(memory));
            value.arguments.push_back(std::move(refs));
            value_effect(id);
            memory = components(id);
            return;
        }
        lifted_effect_t effect;
        effect.kind = effect_kind::store;
        effect.instruction = instruction_index;
        effect.address = std::move(address);
        effect.arguments.push_back(std::move(refs));
        result.blocks[block_index].effects.push_back(std::move(effect));
    }

    void native_call(const instruction_t &instruction)
    {
        auto identity = std::make_pair(instruction.value >> 16, instruction.value & 0xffff);
        const auto &native = symbols.natives.at(identity);
        unsigned count = 0;
        if (native.variadic) {
            auto refs = pop(4);
            const auto &count_value = result.values[refs.front().value];
            if (refs.front().offset || count_value.kind != value_kind::constant)
                fail("Variadic argument count is not a known immediate");
            float number;
            std::memcpy(&number, &count_value.immediate, sizeof(number));
            if (!std::isfinite(number) || number < 0 || number > 65535 || std::floor(number) < number)
                fail("Invalid variadic argument count");
            count = static_cast<unsigned>(number);
            if (count < native.parameters.size())
                fail("Variadic call lacks its fixed parameters");
        }
        std::vector<references_t> arguments;
        if (native.variadic)
            for (unsigned i = static_cast<unsigned>(native.parameters.size()); i < count; ++i)
                arguments.push_back(pop(4));
        for (auto it = native.parameters.rbegin(); it != native.parameters.rend(); ++it)
            arguments.push_back(pop(symbols.type_sizes.at(*it)));
        std::reverse(arguments.begin(), arguments.end());
        auto width = native.result_bytes;
        if (!width) {
            auto found = analysis.native_results.find(identity);
            if (found != analysis.native_results.end())
                width = found->second;
        }
        if (!width)
            fail("Native result width remains unresolved");
        auto id = node(value_kind::native_call, *width, native.result.value_or(opaque_type(*width)));
        result.values[id].arguments = std::move(arguments);
        result.values[id].immediate = instruction.value;
        value_effect(id);
        push(components(id));
    }

    void operation(const instruction_t &instruction)
    {
        auto op = instruction.opcode;
        unsigned arity =
            op == OP_NEG || op == OP_LNT || op == OP_NOT || op == OP_INC || op == OP_DEC || op == OP_I2S || op == OP_F2S
                ? 1
                : 2;
        std::vector<references_t> arguments;
        if (arity == 1)
            arguments.push_back(pop(4));
        else {
            references_t left, right;
            if (instruction.argument == OP_ARG_NUM || instruction.argument == OP_ARG_NUMR) {
                auto immediate = constant(instruction.value);
                auto top = pop(4);
                if (instruction.argument == OP_ARG_NUMR) {
                    left = std::move(immediate);
                    right = std::move(top);
                } else {
                    left = std::move(top);
                    right = std::move(immediate);
                }
            } else if (instruction.argument == OP_ARG_STR) {
                right = string(instruction.value);
                left = pop(4);
            } else {
                right = pop(4);
                left = pop(4);
            }
            arguments.push_back(std::move(left));
            arguments.push_back(std::move(right));
        }
        auto type = op == OP_I2S || op == OP_F2S || op == OP_PSH_STR ? "str" : "num";
        auto id = node(value_kind::operation, 4, type);
        result.values[id].arguments = std::move(arguments);
        value_effect(id);
        push(components(id));
    }

    void instruction()
    {
        const auto &instruction = binary.code[instruction_index];
        auto op = instruction.opcode;
        auto width = instruction.data_size();
        switch (op) {
        case OP_PSH:
            if (instruction.argument == OP_ARG_NUM)
                push(constant(instruction.value));
            else if (instruction.argument == OP_ARG_STR)
                push(string(instruction.value));
            else if (instruction.argument == OP_ARG_CLV) {
                std::string type;
                for (const auto &[name, id] : symbols.class_ids)
                    if (id == instruction.value >> 16) {
                        type = name;
                        break;
                    }
                auto id = node(value_kind::class_value, 4, type);
                result.values[id].immediate = instruction.value;
                push(components(id));
            } else if (instruction.argument == OP_ARG_SIG || instruction.argument == OP_ARG_PSIG) {
                signal_parent = instruction.argument == OP_ARG_SIG ? constant(0).front() : pop_word();
                auto id = node(value_kind::signal, 4);
                result.values[id].immediate = instruction.value;
                result.values[id].arguments = {{signal_parent}};
                value_effect(id);
                push(components(id));
            } else
                push(load(address(instruction, width)));
            break;
        case OP_POP:
            if (instruction.argument == OP_ARG_NULL)
                pop(width);
            else {
                auto transfer = facts.array_widths[instruction_index].value_or(width);
                auto storage = address(instruction, transfer);
                storage.index = array_index;
                auto refs = pop(transfer);
                store(std::move(storage), std::move(refs));
                array_index.reset();
            }
            break;
        case OP_DUP:
            if (instruction.argument == OP_ARG_NULL)
                push(read(height - static_cast<int>(width), width));
            else {
                auto storage = address(instruction, width);
                store(std::move(storage), read(height - static_cast<int>(width), width));
            }
            break;
        case OP_SPA: {
            const auto amount = static_cast<int16_t>(instruction.value);
            if (amount > 0)
                for (int offset = 0; offset < amount; offset += 4)
                    write(height + offset, components(node(value_kind::uninitialized, 4)));
            height += amount;
            break;
        }
        case OP_PSF:
            array_index = pop_word();
            break;
        case OP_ASF:
        case OP_COFF: {
            pop(instruction.value);
            auto index = pop_word();
            if (!instruction_index)
                fail("Array access lacks its preceding base load");
            const auto &previous = binary.code[instruction_index - 1];
            auto storage = address(previous, instruction.value);
            if (op == OP_ASF)
                storage.offset += 4;
            storage.index = index;
            push(load(std::move(storage)));
            break;
        }
        case OP_CPY: {
            lifted_address_t storage;
            storage.kind = address_kind::instance;
            storage.width = instruction.value;
            storage.receiver = pop_word();
            auto offset = pop_word();
            storage.index = pop_word();
            const auto &value = result.values[offset.value];
            if (offset.offset || value.kind != value_kind::constant)
                fail("Instance array base offset is not a known number");
            float number;
            std::memcpy(&number, &value.immediate, sizeof(number));
            if (!std::isfinite(number) || number < 0 || number > 65535 || std::floor(number) < number)
                fail("Invalid instance array base offset");
            storage.offset = static_cast<unsigned>(number);
            push(load(std::move(storage)));
            break;
        }
        case OP_BSL:
            native_call(instruction);
            break;
        case OP_BSR: {
            auto callee = target(instruction.value);
            auto arguments = pop(function(callee).parameters_size);
            auto width = analysis.functions[callee].noreturn ? std::optional<unsigned>(0)
                                                             : analysis.functions[callee].result_bytes;
            if (!width)
                fail("Subroutine result width remains unresolved");
            auto id = node(value_kind::script_call, *width);
            result.values[id].immediate = instruction.value;
            result.values[id].arguments.push_back(std::move(arguments));
            value_effect(id);
            push(components(id));
            signal_parent = {node(value_kind::uninitialized, 4), 0};
            break;
        }
        case OP_BST:
        case OP_BTH: {
            std::vector<references_t> arguments;
            if (op == OP_BTH)
                arguments.push_back(pop(4));
            arguments.push_back(pop(function(target(instruction.value)).parameters_size));
            effect(effect_kind::thread, std::move(arguments));
            break;
        }
        case OP_ECB:
        case OP_ESB:
        case OP_ECO:
        case OP_SCO: {
            std::vector<references_t> arguments;
            if (op == OP_ECB || op == OP_ECO)
                arguments.push_back(pop(4));
            arguments.push_back(pop(function(target(instruction.value)).parameters_size));
            arguments.push_back(pop(4));
            arguments.push_back({signal_parent});
            effect(effect_kind::callback, std::move(arguments));
            break;
        }
        case OP_RE:
        case OP_RAE:
            effect(effect_kind::signal,
                   instruction.argument == OP_ARG_PSIG ? std::vector<references_t>{pop(4)}
                                                       : std::vector<references_t>{});
            break;
        case OP_KIL:
        case OP_DEL_THREADS:
        case OP_KILL_THREAD:
        case OP_MS2:
            effect(effect_kind::termination,
                   op == OP_KILL_THREAD || op == OP_MS2 ? std::vector<references_t>{pop(4)}
                                                        : std::vector<references_t>{});
            break;
        case OP_BF:
            effect(effect_kind::branch, {pop(4)});
            break;
        case OP_FEQZB:
            effect(effect_kind::branch, {read(height - 4, 4)});
            break;
        case OP_BRA:
            effect(effect_kind::branch);
            break;
        case OP_RET:
            if (facts.discards_exit_stack)
                effect(effect_kind::return_value);
            else if (facts.result_bytes)
                effect(effect_kind::return_value, {read(0, *facts.result_bytes)});
            else
                fail("Return width remains unresolved");
            break;
        case OP_NOP:
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_SHL:
        case OP_SHR:
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_STR_EQ:
        case OP_STR_NE:
        case OP_NEG:
        case OP_LNT:
        case OP_NOT:
        case OP_INC:
        case OP_DEC:
        case OP_I2S:
        case OP_F2S:
        case OP_PSH_STR:
            operation(instruction);
            break;
        default:
            fail("Instruction has no proven value transfer");
        }
    }

    value_ref_t merge()
    {
        return {node(value_kind::merge, 4), 0};
    }

public:
    value_lifter_t(const image_t &image, const symbols_t &symbols, const stack_analysis_t &analysis,
                   const stack_function_t &facts, const function_t &binary,
                   const std::map<std::pair<unsigned, unsigned>, unsigned> &ids)
        : image(image), symbols(symbols), analysis(analysis), facts(facts), binary(binary), function_ids(ids),
          cfg(facts.control_flow)
    {
        result.object = facts.object;
        result.function = facts.function;
        result.initializer = facts.initializer;
        result.diagnostics = facts.diagnostics;
    }

    lifted_function_t run()
    {
        if (!result.diagnostics.empty())
            return std::move(result);
        if (cfg.blocks.empty())
            throw error_t("Missing analyzed control-flow graph");
        footprint = facts.storage_bytes;
        result.blocks.resize(cfg.blocks.size());
        reachable.resize(cfg.blocks.size());
        std::deque<unsigned> queue{0};
        reachable[0] = true;
        while (!queue.empty()) {
            auto block = queue.front();
            queue.pop_front();
            for (auto next : cfg.blocks[block].successors)
                if (!reachable[next]) {
                    reachable[next] = true;
                    queue.push_back(next);
                }
        }
        auto entry = parameters();
        for (unsigned offset = static_cast<unsigned>(entry.size() * 4); offset < footprint; offset += 4)
            entry.push_back({node(value_kind::uninitialized, 4), 0});
        auto initial_signal = value_ref_t{node(value_kind::uninitialized, 4), 0};
        for (unsigned i = 0; i < cfg.blocks.size(); ++i) {
            auto &block = result.blocks[i];
            block.begin = cfg.blocks[i].begin;
            block.end = cfg.blocks[i].end;
            block.successors = cfg.blocks[i].successors;
            if (!reachable[i])
                continue;
            if (!i && cfg.blocks[i].predecessors.empty()) {
                block.incoming = entry;
                block.incoming_signal_parent = initial_signal;
            } else {
                for (unsigned offset = 0; offset < footprint; offset += 4)
                    block.incoming.push_back(merge());
                block.incoming_signal_parent = merge();
                if (facts.array_widths[block.begin])
                    block.incoming_array_index = merge();
                if (!i) {
                    for (unsigned word = 0; word < block.incoming.size(); ++word)
                        result.values[block.incoming[word].value].arguments.push_back({entry[word]});
                    result.values[block.incoming_signal_parent.value].arguments.push_back({initial_signal});
                }
            }
        }
        for (block_index = 0; block_index < result.blocks.size(); ++block_index) {
            if (!reachable[block_index])
                continue;
            auto &block = result.blocks[block_index];
            memory = block.incoming;
            signal_parent = block.incoming_signal_parent;
            array_index = block.incoming_array_index;
            bool complete = true;
            for (instruction_index = block.begin; instruction_index < block.end; ++instruction_index) {
                if (facts.heights[instruction_index])
                    height = *facts.heights[instruction_index];
                else if (!((facts.discards_exit_stack || facts.initializer) &&
                           binary.code[instruction_index].opcode == OP_RET)) {
                    result.diagnostics.push_back("At " + hex(binary.code[instruction_index].offset) +
                                                 ": Instruction stack height is unresolved");
                    complete = false;
                    break;
                }
                try {
                    instruction();
                } catch (const error_t &error) {
                    result.diagnostics.push_back(error.what());
                    complete = false;
                    break;
                }
            }
            if (!complete)
                continue;
            block.outgoing = memory;
            block.outgoing_array_index = array_index;
            block.outgoing_signal_parent = signal_parent;
        }
        if (!result.diagnostics.empty())
            return std::move(result);
        for (unsigned i = 0; i < result.blocks.size(); ++i) {
            if (!reachable[i])
                continue;
            auto &block = result.blocks[i];
            for (auto predecessor : cfg.blocks[i].predecessors) {
                if (!reachable[predecessor])
                    continue;
                const auto &source = result.blocks[predecessor];
                for (unsigned word = 0; word < block.incoming.size(); ++word)
                    result.values[block.incoming[word].value].arguments.push_back({source.outgoing[word]});
                result.values[block.incoming_signal_parent.value].arguments.push_back({source.outgoing_signal_parent});
                if (block.incoming_array_index) {
                    if (!source.outgoing_array_index) {
                        result.diagnostics.push_back("Array-index state is absent at a control-flow join");
                        continue;
                    }
                    result.values[block.incoming_array_index->value].arguments.push_back(
                        {*source.outgoing_array_index});
                }
            }
        }
        std::map<unsigned, references_t> transient_copies;
        for (unsigned id = 0; id < result.values.size(); ++id) {
            const auto &v = result.values[id];
            if (v.kind != value_kind::operation || v.instruction >= binary.code.size() ||
                binary.code[v.instruction].opcode != OP_PSH)
                continue;
            bool retained = false;
            for (const auto &block : result.blocks)
                if (v.instruction >= block.begin && v.instruction < block.end) {
                    int live = 0;
                    if (!block.successors.empty()) {
                        const auto next = result.blocks[block.successors.front()].begin;
                        if (facts.heights[next])
                            live = *facts.heights[next];
                    } else if (block.end > block.begin) {
                        unsigned end = block.end - 1;
                        while (end > block.begin && binary.code[end - 1].opcode == OP_PSH &&
                               binary.code[end - 1].argument == OP_ARG_SPR)
                            --end;
                        if (end > block.begin && binary.code[end - 1].opcode == OP_SPA &&
                            static_cast<int16_t>(binary.code[end - 1].value) < 0 && facts.heights[end - 1])
                            live = *facts.heights[end - 1];
                        else if (facts.heights[block.end - 1])
                            live = *facts.heights[block.end - 1];
                    }
                    for (unsigned word = 0; word < block.outgoing.size() && word * 4 < static_cast<unsigned>(live);
                         ++word)
                        if (block.outgoing[word].value == id)
                            retained = true;
                }
            if (!retained)
                transient_copies.emplace(id, v.arguments.at(0));
        }
        auto resolve = [&](value_ref_t &ref) {
            std::set<unsigned> visited;
            while (transient_copies.count(ref.value) && visited.insert(ref.value).second) {
                const auto replacement = transient_copies.at(ref.value).at(ref.offset / 4);
                ref = {replacement.value, replacement.offset + ref.offset % 4};
            }
        };
        auto refs = [&](auto &values) {
            for (auto &ref : values)
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
        for (auto &v : result.values) {
            for (auto &args : v.arguments)
                refs(args);
            address(v.address);
        }
        for (auto &block : result.blocks) {
            refs(block.incoming);
            refs(block.outgoing);
            if (block.incoming_array_index)
                resolve(*block.incoming_array_index);
            if (block.outgoing_array_index)
                resolve(*block.outgoing_array_index);
            resolve(block.incoming_signal_parent);
            resolve(block.outgoing_signal_parent);
            for (auto &e : block.effects) {
                for (auto &args : e.arguments)
                    refs(args);
                address(e.address);
            }
            block.effects.erase(std::remove_if(block.effects.begin(),
                                               block.effects.end(),
                                               [&](const auto &e) {
                                                   return e.kind == effect_kind::value &&
                                                          transient_copies.count(e.value);
                                               }),
                                block.effects.end());
        }
        for (const auto &[id, unused] : transient_copies) {
            auto &v = result.values[id];
            v.kind = value_kind::uninitialized;
            v.width = 0;
            v.arguments.clear();
            v.address.reset();
        }
        return std::move(result);
    }
};
}

std::vector<lifted_function_t> lift_values(const image_t &image, const symbols_t &symbols,
                                           const stack_analysis_t &stack)
{
    std::map<std::pair<unsigned, unsigned>, unsigned> ids;
    for (unsigned i = 0; i < stack.functions.size(); ++i)
        if (!stack.functions[i].initializer)
            ids.emplace(std::make_pair(stack.functions[i].object, stack.functions[i].function), i);
    std::vector<lifted_function_t> result;
    result.reserve(stack.functions.size());
    for (const auto &facts : stack.functions) {
        const auto &binary = facts.initializer ? *image.initializers[facts.function].builder
                                               : image.objects[facts.object].functions[facts.function];
        result.push_back(value_lifter_t(image, symbols, stack, facts, binary, ids).run());
    }
    return result;
}

}
