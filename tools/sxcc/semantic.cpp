#include "semantic.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#include <deque>
#include <limits>

namespace sxcc {
namespace {

struct linear_t {
    std::int64_t constant = 0;
    std::map<unsigned, std::int64_t> terms;

    void add(const linear_t &other, int sign = 1)
    {
        constant += sign * other.constant;
        for (auto [variable, coefficient] : other.terms) {
            auto &value = terms[variable];
            value += sign * coefficient;
            if (!value)
                terms.erase(variable);
        }
    }

    void substitute(const std::vector<std::optional<int>> &values)
    {
        for (auto it = terms.begin(); it != terms.end();) {
            if (values[it->first]) {
                constant += it->second * *values[it->first];
                it = terms.erase(it);
            } else
                ++it;
        }
    }
};

struct equation_t {
    linear_t value;
    unsigned function;
    unsigned instruction;
};

struct frame_t {
    linear_t height;
    std::optional<unsigned> array_width;
};

class stack_analyzer_t {
    const image_t &image;
    const symbols_t &symbols;
    stack_analysis_t result;
    std::vector<const function_t *> binaries;
    std::map<std::pair<unsigned, unsigned>, unsigned> ids;
    std::vector<std::vector<std::optional<frame_t>>> frames;
    std::vector<std::vector<std::pair<unsigned, unsigned>>> callers;
    std::vector<equation_t> equations;
    std::vector<equation_t> exit_heights;
    const int capacity = pc_vm_stack_capacity;
    std::vector<std::optional<int>> values;
    std::map<std::pair<unsigned, unsigned>, unsigned> native_variables;

    unsigned script_target(std::uint32_t operand) const
    {
        return ids.at({image.object_order.at(operand >> 16), operand & 0xffff});
    }

    void diagnostic(unsigned id, unsigned instruction, std::string message)
    {
        result.functions[id].diagnostics.push_back("At " + hex(binaries[id]->code[instruction].offset) + ": " +
                                                   message);
    }

    linear_t effect(unsigned id, unsigned index, std::optional<unsigned> &array_width)
    {
        const auto &code = binaries[id]->code;
        const auto &instruction = code[index];
        const auto op = instruction.opcode;
        const auto arg = instruction.argument;
        const auto operand = instruction.value;
        const int width = static_cast<int>(instruction.data_size());
        linear_t delta;
        switch (op) {
        case OP_ADD:
        case OP_AND:
        case OP_DIV:
        case OP_EQ:
        case OP_GE:
        case OP_GT:
        case OP_LE:
        case OP_LT:
        case OP_MOD:
        case OP_MUL:
        case OP_NE:
        case OP_OR:
        case OP_SHL:
        case OP_SHR:
        case OP_SUB:
        case OP_XOR:
        case OP_STR_EQ:
        case OP_STR_NE:
            delta.constant = arg == OP_ARG_NULL ? -4 : 0;
            break;
        case OP_BF:
            delta.constant = -4;
            break;
        case OP_BSL: {
            const auto identity = std::make_pair(operand >> 16, operand & 0xffff);
            const auto found = symbols.natives.find(identity);
            if (found == symbols.natives.end())
                throw error_t("Unknown native call identity");
            const auto &native = found->second;
            unsigned consumed = native.parameter_bytes;
            if (native.variadic) {
                if (!index || code[index - 1].opcode != OP_PSH || code[index - 1].argument != OP_ARG_NUM)
                    throw error_t("Variadic call count is not an immediate number");
                float count;
                std::memcpy(&count, &code[index - 1].value, sizeof(count));
                if (!std::isfinite(count) || count < 0 || count > 65535 || std::floor(count) < count)
                    throw error_t("Invalid variadic call count");
                consumed = 4 + static_cast<unsigned>(count) * 4;
                if (consumed < native.parameter_bytes + 4)
                    throw error_t("Variadic call lacks fixed parameters");
            }
            delta.constant = -static_cast<std::int64_t>(consumed);
            if (native.result_bytes)
                delta.constant += *native.result_bytes;
            else {
                auto [it, inserted] = native_variables.emplace(identity, static_cast<unsigned>(values.size()));
                if (inserted)
                    values.emplace_back();
                delta.terms.emplace(it->second, 1);
            }
            break;
        }
        case OP_BSR:
        case OP_BST:
        case OP_BTH:
        case OP_ECB:
        case OP_ESB:
        case OP_ECO:
        case OP_SCO: {
            auto target = script_target(operand);
            delta.constant = -static_cast<std::int64_t>(binaries[target]->parameters_size);
            if (op == OP_BSR) {
                if (!result.functions[target].noreturn)
                    delta.terms.emplace(target, 1);
            } else if (op == OP_BTH || op == OP_ESB || op == OP_SCO)
                delta.constant -= 4;
            else if (op == OP_ECB || op == OP_ECO)
                delta.constant -= 8;
            break;
        }
        case OP_DUP:
            delta.constant = arg == OP_ARG_NULL ? width : arg == OP_ARG_POPO ? -4 : 0;
            break;
        case OP_POP: {
            bool storage = arg == OP_ARG_SPR || arg == OP_ARG_POPO || arg == OP_ARG_SDR || arg == OP_ARG_VAR;
            if (storage || arg == OP_ARG_NULL)
                delta.constant =
                    -(storage && array_width ? static_cast<int>(*array_width) : width) - (arg == OP_ARG_POPO ? 4 : 0);
            if (storage)
                array_width.reset();
            break;
        }
        case OP_PSH:
            if (arg == OP_ARG_NUM || arg == OP_ARG_STR || arg == OP_ARG_CLV || arg == OP_ARG_SIG)
                delta.constant = 4;
            else if (arg == OP_ARG_SPR || arg == OP_ARG_SDR || arg == OP_ARG_VAR)
                delta.constant = width;
            else if (arg == OP_ARG_POPO)
                delta.constant = width - 4;
            else if (arg != OP_ARG_PSIG)
                throw error_t("Unsupported push operand");
            break;
        case OP_SPA:
            delta.constant = static_cast<std::int16_t>(operand);
            break;
        case OP_FEQZB:
        case OP_BRA:
        case OP_DEC:
        case OP_INC:
        case OP_KIL:
        case OP_LNT:
        case OP_NEG:
        case OP_NOP:
        case OP_NOT:
        case OP_RET:
        case OP_I2S:
        case OP_F2S:
        case OP_DEL_THREADS:
            break;
        case OP_RE:
            delta.constant = arg == OP_ARG_PSIG ? -4 : 0;
            break;
        case OP_RAE:
            break;
        case OP_KILL_THREAD:
        case OP_PSH_STR:
        case OP_MS2:
        case OP_ASF:
        case OP_COFF:
            delta.constant = -4;
            break;
        case OP_PSF:
            delta.constant = -4;
            array_width = operand;
            break;
        case OP_CPY:
            delta.constant = static_cast<std::int16_t>(operand) - 12;
            break;
        default:
            throw error_t("No proven stack effect for opcode " + opcode_name(op));
        }
        return delta;
    }

    void constrain(unsigned id, unsigned index, const linear_t &left, const linear_t &right)
    {
        auto value = left;
        value.add(right, -1);
        if (value.constant || !value.terms.empty())
            equations.push_back({std::move(value), id, index});
    }

    void infer_termination()
    {
        std::deque<unsigned> pending;
        std::vector<bool> queued(binaries.size(), true);
        for (unsigned id = 0; id < binaries.size(); ++id) {
            result.functions[id].control_flow = analyze_function(*binaries[id]);
            result.functions[id].noreturn = true;
            pending.push_back(id);
        }
        std::vector<bool> seen;
        std::vector<unsigned> blocks;
        while (!pending.empty()) {
            auto id = pending.front();
            pending.pop_front();
            queued[id] = false;
            auto &facts = result.functions[id];
            const auto &cfg = facts.control_flow;
            if (!facts.noreturn || cfg.blocks.empty())
                continue;
            seen.assign(cfg.blocks.size(), false);
            blocks.clear();
            blocks.push_back(0);
            seen[0] = true;
            for (unsigned cursor = 0; cursor < blocks.size() && facts.noreturn; ++cursor) {
                const auto &block = cfg.blocks[blocks[cursor]];
                bool continues = true;
                for (unsigned i = block.begin; i < block.end; ++i) {
                    const auto &instruction = binaries[id]->code[i];
                    if (instruction.opcode == OP_RET) {
                        facts.noreturn = false;
                        break;
                    }
                    if (instruction.opcode == OP_BSR && result.functions[script_target(instruction.value)].noreturn) {
                        continues = false;
                        break;
                    }
                }
                if (continues)
                    for (auto successor : block.successors)
                        if (!seen[successor]) {
                            seen[successor] = true;
                            blocks.push_back(successor);
                        }
            }
            if (!facts.noreturn)
                for (auto [caller, instruction] : callers[id]) {
                    static_cast<void>(instruction);
                    if (!queued[caller]) {
                        queued[caller] = true;
                        pending.push_back(caller);
                    }
                }
        }
        for (unsigned id = 0; id < binaries.size(); ++id) {
            auto &cfg = result.functions[id].control_flow;
            for (auto &block : cfg.blocks) {
                block.predecessors.clear();
                for (unsigned i = block.begin; i < block.end; ++i) {
                    const auto &instruction = binaries[id]->code[i];
                    if (instruction.opcode == OP_BSR && result.functions[script_target(instruction.value)].noreturn) {
                        block.end = i + 1;
                        block.successors.clear();
                        break;
                    }
                }
            }
            for (unsigned block = 0; block < cfg.blocks.size(); ++block)
                for (auto successor : cfg.blocks[block].successors)
                    cfg.blocks[successor].predecessors.push_back(block);
        }
    }

    void visit(unsigned id)
    {
        const auto &binary = *binaries[id];
        auto &facts = result.functions[id];
        const auto &cfg = facts.control_flow;
        facts.diagnostics = cfg.diagnostics;
        if (!facts.diagnostics.empty())
            return;
        auto &states = frames[id];
        states.resize(binary.code.size());
        states[0] = frame_t{{facts.initializer ? 0 : static_cast<std::int64_t>(binary.parameters_size), {}}, {}};
        std::deque<unsigned> work{0};
        while (!work.empty()) {
            unsigned block_id = work.front();
            work.pop_front();
            const auto &block = cfg.blocks[block_id];
            auto state = *states[block.begin];
            bool complete = true;
            for (unsigned i = block.begin; i < block.end; ++i) {
                states[i] = state;
                const auto &instruction = binary.code[i];
                if (instruction.opcode == OP_RET && !facts.discards_exit_stack && !facts.initializer)
                    constrain(id, i, state.height, linear_t{0, {{id, 1}}});
                try {
                    state.height.add(effect(id, i, state.array_width));
                } catch (const error_t &error) {
                    diagnostic(id, i, error.what());
                    complete = false;
                    break;
                }
            }
            if (!complete)
                continue;
            exit_heights.push_back({state.height, id, block.end - 1});
            for (auto successor : block.successors) {
                auto begin = cfg.blocks[successor].begin;
                if (!states[begin]) {
                    states[begin] = state;
                    work.push_back(successor);
                } else if (!((facts.discards_exit_stack || facts.initializer) && binary.code[begin].opcode == OP_RET)) {
                    constrain(id, begin, state.height, states[begin]->height);
                    if (state.array_width != states[begin]->array_width)
                        diagnostic(id, begin, "Control-flow join has incompatible array-store prefixes");
                }
            }
        }
    }

public:
    stack_analyzer_t(const image_t &image, const symbols_t &symbols) : image(image), symbols(symbols)
    {
        for (unsigned o = 0; o < image.objects.size(); ++o)
            for (unsigned f = 0; f < image.objects[o].functions.size(); ++f) {
                ids.emplace(std::make_pair(o, f), static_cast<unsigned>(binaries.size()));
                binaries.push_back(&image.objects[o].functions[f]);
                result.functions.push_back({o, f, false, {}, {}, {}});
            }
        for (unsigned i = 0; i < image.initializers.size(); ++i)
            if (image.initializers[i].builder) {
                binaries.push_back(&*image.initializers[i].builder);
                result.functions.push_back({image.object_order.at(image.initializers[i].object), i, true, {}, {}, {}});
            }
        values.resize(binaries.size());
        frames.resize(binaries.size());
        callers.resize(binaries.size());
    }

    stack_analysis_t run()
    {
        for (unsigned id = 0; id < binaries.size(); ++id)
            for (unsigned i = 0; i < binaries[id]->code.size(); ++i)
                if (binaries[id]->code[i].opcode == OP_BSR)
                    callers[script_target(binaries[id]->code[i].value)].emplace_back(id, i);
        infer_termination();
        for (unsigned id = 0; id < binaries.size(); ++id) {
            auto &facts = result.functions[id];
            if (facts.initializer) {
                const auto &functions = image.objects[facts.object].functions;
                if (functions.empty())
                    throw error_t("Initializer owner has no constructor");
                const auto &constructor = functions.front();
                const unsigned receiver = (constructor.flags & 1u) ? 0 : 4;
                if (constructor.parameters_size < receiver)
                    throw error_t("Constructor parameter storage lacks its receiver");
                auto width = constructor.parameters_size - receiver;
                if (width > static_cast<unsigned>(capacity))
                    throw error_t("Initializer output exceeds VM stack storage");


                values[id] = static_cast<int>(width);
                facts.storage_bytes = width;
            }


            facts.discards_exit_stack = !facts.initializer && facts.function == 0 && callers[id].empty();
            if (facts.discards_exit_stack && !facts.noreturn)
                values[id] = 0;
        }
        for (unsigned id = 0; id < binaries.size(); ++id)
            visit(id);
        bool changed;
        do {
            changed = false;
            for (auto &equation : equations) {
                auto &value = equation.value;
                value.substitute(values);
                if (value.terms.size() != 1)
                    continue;
                auto [variable, coefficient] = *value.terms.begin();
                auto width = -value.constant / coefficient;
                if (value.constant % coefficient || width < 0 || width > UINT16_MAX || width % 4)
                    continue;
                values[variable] = static_cast<int>(width);
                changed = true;
            }
        } while (changed);
        for (auto &equation : equations) {
            equation.value.substitute(values);
            if (equation.value.terms.empty() && equation.value.constant)
                diagnostic(equation.function,
                           equation.instruction,
                           "Conflicting stack-height constraints (difference " +
                               std::to_string(equation.value.constant) + ")");
        }
        for (auto &exit : exit_heights) {
            exit.value.substitute(values);
            if (exit.value.terms.empty() && (exit.value.constant < 0 || exit.value.constant > capacity))
                diagnostic(exit.function, exit.instruction, "Control-flow exit exceeds VM stack storage");
            else if (exit.value.terms.empty())
                result.functions[exit.function].storage_bytes =
                    std::max(result.functions[exit.function].storage_bytes, static_cast<unsigned>(exit.value.constant));
        }
        for (unsigned id = 0; id < binaries.size(); ++id) {
            auto &facts = result.functions[id];
            if (values[id])
                facts.result_bytes = static_cast<unsigned>(*values[id]);
            facts.heights.resize(binaries[id]->code.size());
            facts.array_widths.resize(binaries[id]->code.size());
            for (unsigned i = 0; i < frames[id].size(); ++i) {
                if (!frames[id][i])
                    continue;
                facts.array_widths[i] = frames[id][i]->array_width;
                auto height = frames[id][i]->height;
                height.substitute(values);
                if (!height.terms.empty())
                    continue;
                if (height.constant < 0 || height.constant > capacity || height.constant % 4)
                    diagnostic(id, i, "Invalid stack height " + std::to_string(height.constant));
                else {
                    facts.storage_bytes = std::max(facts.storage_bytes, static_cast<unsigned>(height.constant));
                    if (!((facts.discards_exit_stack || facts.initializer) && binaries[id]->code[i].opcode == OP_RET))
                        facts.heights[i] = static_cast<int>(height.constant);
                }
            }
        }
        std::deque<unsigned> invalid;
        for (unsigned id = 0; id < binaries.size(); ++id)
            if (!result.functions[id].diagnostics.empty())
                invalid.push_back(id);
        while (!invalid.empty()) {
            auto id = invalid.front();
            invalid.pop_front();
            auto &facts = result.functions[id];
            facts.result_bytes.reset();
            std::fill(facts.heights.begin(), facts.heights.end(), std::nullopt);
            for (auto [caller, instruction] : callers[id]) {
                if (!result.functions[caller].diagnostics.empty())
                    continue;
                diagnostic(caller, instruction, "Call target has inconsistent stack analysis");
                invalid.push_back(caller);
            }
        }
        if (std::none_of(result.functions.begin(), result.functions.end(), [](const auto &function) {
                return !function.diagnostics.empty();
            })) {
            for (auto [identity, variable] : native_variables)
                if (values[variable])
                    result.native_results.emplace(identity, static_cast<unsigned>(*values[variable]));
        }
        return std::move(result);
    }
};
}

stack_analysis_t analyze_stack(const image_t &image, const symbols_t &symbols)
{
    return stack_analyzer_t(image, symbols).run();
}

}
