#include "type_recovery.h"

#include <algorithm>
#include <functional>
#include <tuple>

namespace sxcc {
namespace {

using value_kind = lifted_value_t::kind_t;
using address_kind = lifted_address_t::kind_t;
using effect_kind = lifted_effect_t::kind_t;

bool unknown_type(std::string_view type)
{
    return type.empty() || type.starts_with("opaque<");
}

std::string opaque_type(unsigned width)
{
    return width ? "opaque<" + std::to_string(width) + '>' : "void";
}









struct type_hint_t {
    std::string produced;
    std::string required;
    unsigned width = 4;
    bool fixed = false;
    bool produced_conflict = false;
    bool required_conflict = false;
    bool numeric = false;
    bool textual = false;

    bool operator==(const type_hint_t &) const = default;

    std::string_view resolved() const
    {
        if (produced_conflict) return {};
        if (!produced.empty()) return produced;
        if (required_conflict) return {};
        if (!required.empty()) return required;
        if (width == 4 && numeric != textual) return numeric ? "num" : "str";
        return {};
    }
};

}

recovered_types_t recover_types(const image_t &image, const symbols_t &symbols,
                               const stack_analysis_t &stack, std::vector<lifted_function_t> &functions)
{
    if (functions.size() != stack.functions.size()) throw error_t("Type recovery received inconsistent function facts");
    recovered_types_t result;
    result.functions.resize(functions.size());
    std::map<std::pair<unsigned, unsigned>, unsigned> ids;
    std::map<std::string, unsigned, std::less<>> classes;
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        classes.emplace(image.objects[object].name, object);
        result.parents.push_back(image.objects[object].parent);
    }
    for (unsigned f = 0; f < functions.size(); ++f)
        if (!functions[f].initializer) ids.emplace(std::make_pair(functions[f].object, functions[f].function), f);









    auto constructs = [&](unsigned object) {
        const auto &record = image.objects[object];
        return !record.functions.empty() && record.functions[0].name_hash == record.name_hash;
    };
    std::vector<std::optional<unsigned>> candidates(image.objects.size());
    for (unsigned object = 0; object < image.objects.size(); ++object) {
        const auto &record = image.objects[object];
        if (result.parents[object] || (record.flags & 1u) || !constructs(object)) continue;
        const auto &constructor = functions[ids.at({object, 0})];
        if (!constructor.diagnostics.empty() || constructor.blocks.empty()) continue;
        auto delegation = [&](const lifted_effect_t &effect) -> std::optional<std::pair<unsigned, unsigned>> {
            if (effect.kind != effect_kind::value) return {};
            const auto &call = constructor.values[effect.value];
            if (call.kind != value_kind::script_call || call.arguments.empty() || call.arguments.front().empty()) return {};
            const auto owner = image.object_order.at(call.immediate >> 16);
            const auto member = call.immediate & 0xffff;
            if (owner == object || member >= image.objects[owner].functions.size() ||
                (image.objects[owner].functions[member].flags & 1u)) return {};
            const auto receiver = call.arguments.front().front();
            const auto &value = constructor.values[receiver.value];
            if (receiver.offset || value.kind != value_kind::parameter || value.immediate) return {};
            return std::make_pair(owner, member);
        };
        std::optional<unsigned> base;
        if (!constructor.blocks[0].effects.empty())
            if (auto first = delegation(constructor.blocks[0].effects[0]);
                first && !first->second && constructs(first->first)) base = first->first;
        if (!base) {
            bool ambiguous = false;
            for (const auto &effect : constructor.blocks[0].effects) {
                auto found = delegation(effect);
                if (!found || constructs(found->first)) continue;
                if (base && *base != found->first) { ambiguous = true; break; }
                base = found->first;
            }
            if (ambiguous) continue;
        }
        if (!base) continue;
        const auto &parent = image.objects[*base];
        if ((parent.flags & 1u) || parent.functions.empty() || parent.instance_size > record.instance_size) continue;
        candidates[object] = *base;
    }
    for (unsigned object = 0; object < candidates.size(); ++object) {
        std::set<unsigned> path;
        auto current = object;
        while (candidates[current]) {
            if (!path.insert(current).second) {
                for (auto member : path) candidates[member].reset();
                break;
            }
            current = *candidates[current];
        }
    }
    for (unsigned object = 0; object < candidates.size(); ++object)
        if (candidates[object]) result.parents[object] = image.objects[*candidates[object]].name;
    std::map<std::string_view, std::string_view> hierarchy;
    for (const auto &[name, parent] : symbols.parents) hierarchy.emplace(name, parent);
    for (unsigned object = 0; object < result.parents.size(); ++object)
        if (result.parents[object]) hierarchy[image.objects[object].name] = *result.parents[object];
    auto derives = [&](std::string_view type, std::string_view base) {
        for (unsigned depth = 0; depth <= hierarchy.size(); ++depth) {
            if (type == base) return true;
            auto parent = hierarchy.find(type);
            if (parent == hierarchy.end()) return false;
            type = parent->second;
        }
        return false;
    };


    auto ancestor = [&](std::string_view left, std::string_view right) -> std::string_view {
        for (unsigned depth = 0; depth <= hierarchy.size(); ++depth) {
            if (derives(right, left)) return left;
            auto parent = hierarchy.find(left);
            if (parent == hierarchy.end()) return {};
            left = parent->second;
        }
        return {};
    };



    auto weaker = [](std::string_view type, unsigned width) { return type == "nullptr" || (width == 4 && type == "num"); };
    auto join = [&](std::string_view left, std::string_view right, unsigned width) -> std::string_view {
        if (left == right || weaker(right, width)) return left;
        if (weaker(left, width)) return right;
        return ancestor(left, right);
    };
    auto meet = [&](std::string_view left, std::string_view right, unsigned width) -> std::string_view {
        if (left == right || weaker(right, width)) return left;
        if (weaker(left, width)) return right;
        if (derives(left, right)) return left;
        if (derives(right, left)) return right;
        return {};
    };
    auto produce = [&](type_hint_t &hint, std::string_view type) {
        if (hint.fixed || hint.produced_conflict || unknown_type(type)) return false;
        if (hint.produced.empty()) { hint.produced.assign(type); return true; }
        if (hint.produced == type) return false;
        auto merged = join(hint.produced, type, hint.width);
        if (merged.empty()) { hint.produced.clear(); hint.produced_conflict = true; return true; }
        if (merged == hint.produced) return false;
        hint.produced = std::string(merged);
        return true;
    };
    auto require = [&](type_hint_t &hint, std::string_view type) {
        if (hint.fixed || hint.required_conflict || unknown_type(type)) return false;
        if (hint.required.empty()) { hint.required.assign(type); return true; }
        if (hint.required == type) return false;
        auto merged = meet(hint.required, type, hint.width);
        if (merged.empty()) { hint.required.clear(); hint.required_conflict = true; return true; }
        if (merged == hint.required) return false;
        hint.required = std::string(merged);
        return true;
    };
    auto note_operand = [](type_hint_t &hint, bool numeric) {
        if (hint.fixed) return false;
        bool &flag = numeric ? hint.numeric : hint.textual;
        if (flag) return false;
        flag = true;
        return true;
    };
    auto mirror = [](type_hint_t &into, const type_hint_t &from) {
        auto next = from;
        next.width = into.width;
        if (into == next) return false;
        into = std::move(next);
        return true;
    };
    std::vector<std::vector<type_hint_t>> hints(functions.size());
    std::vector<type_hint_t> returns(functions.size());
    std::vector<std::vector<unsigned>> parameters(functions.size());
    std::map<std::uint32_t, type_hint_t> native_returns;
    std::map<storage_key_t, type_hint_t> storage;
    auto storage_hint = [&](const storage_key_t &key) -> type_hint_t & {
        auto [entry, inserted] = storage.try_emplace(key);
        if (inserted) entry->second.width = std::get<3>(key);
        return entry->second;
    };
    for (const auto &initializer : image.initializers) {
        auto &hint = storage_hint({static_cast<unsigned>(address_kind::shared), 0, initializer.offset, 4});
        hint.produced = initializer.kind >= 0 ? "str"
            : initializer.kind != -4 ? image.objects.at(image.object_order.at(initializer.object)).name
            : initializer.value ? "num" : "nullptr";
        hint.fixed = initializer.kind != -4 || initializer.value != 0;
    }
    for (unsigned f = 0; f < functions.size(); ++f) {
        const auto &function = functions[f];
        hints[f].resize(function.values.size());
        for (unsigned v = 0; v < function.values.size(); ++v) {
            const auto &value = function.values[v];
            auto &hint = hints[f][v];
            hint.width = value.width;
            if (value.kind == value_kind::parameter) parameters[f].push_back(v);
            if (!unknown_type(value.type)) { hint.produced = value.type; hint.fixed = true; }
            if (value.kind == value_kind::signal) { hint.produced = "signal"; hint.fixed = true; }
            if (value.kind == value_kind::native_call) {
                auto &native = native_returns[value.immediate];
                native.width = value.width;
                if (!unknown_type(value.type)) { native.produced = value.type; native.fixed = true; }
            }
        }
        returns[f].width = stack.functions[f].result_bytes.value_or(0);
        if ((stack.functions[f].noreturn && !stack.functions[f].initializer) || stack.functions[f].result_bytes == 0) {
            returns[f].produced = "void";
            returns[f].fixed = true;
        }
    }
    std::function<std::string_view(unsigned, std::span<const value_ref_t>, bool, std::set<unsigned> &)> group_hint;
    group_hint = [&](unsigned f, std::span<const value_ref_t> refs, bool produced_only, std::set<unsigned> &active) -> std::string_view {
        if (refs.empty()) return "void";
        auto first = refs.front();
        const auto &value = functions[f].values[first.value];
        if (refs.size() == 1 && value.kind == value_kind::constant && !value.immediate) return "nullptr";
        bool whole = first.offset == 0 && value.width == refs.size() * 4;
        for (unsigned i = 0; whole && i < refs.size(); ++i)
            whole = refs[i].value == first.value && refs[i].offset == i * 4;
        if (whole) return produced_only ? std::string_view(hints[f][first.value].produced) : hints[f][first.value].resolved();
        if (refs.size() == 1) {
            const auto &component = hints[f][first.value].produced;
            if (component == "vector3d" || component == "position3d") return "num";
        }
        if (refs.size() > 1 && value.kind == value_kind::merge && value.width == 4 &&
            active.insert(first.value).second) {
            std::string_view result;
            for (unsigned edge = 0; edge < value.arguments.size(); ++edge) {
                std::vector<value_ref_t> incoming;
                for (auto ref : refs) {
                    const auto &part = functions[f].values[ref.value];
                    if (ref.offset || part.kind != value_kind::merge || part.arguments.size() != value.arguments.size() ||
                        part.arguments[edge].size() != 1) break;
                    incoming.push_back(part.arguments[edge][0]);
                }
                if (incoming.size() != refs.size()) { result = {}; break; }
                const auto type = group_hint(f, incoming, produced_only, active);
                if (type.empty()) continue;
                if (result.empty()) result = type;
                else {
                    result = join(result, type, static_cast<unsigned>(refs.size() * 4));
                    if (result.empty()) break;
                }
            }
            active.erase(first.value);
            return result;
        }
        return {};
    };
    auto group_type = [&](unsigned f, std::span<const value_ref_t> refs, bool produced_only = true) {
        std::set<unsigned> active;
        return group_hint(f, refs, produced_only, active);
    };


    auto owner_hint = [&](unsigned f, std::span<const value_ref_t> refs) -> type_hint_t * {
        if (refs.empty()) return nullptr;
        auto first = refs.front();
        const auto &value = functions[f].values[first.value];
        if (first.offset || value.width != refs.size() * 4) return nullptr;
        for (unsigned i = 0; i < refs.size(); ++i)
            if (refs[i].value != first.value || refs[i].offset != i * 4) return nullptr;
        if (value.kind == value_kind::native_call) return &native_returns.at(value.immediate);
        if (value.kind == value_kind::script_call)
            return &returns[ids.at({image.object_order.at(value.immediate >> 16), value.immediate & 0xffff})];
        return &hints[f][first.value];
    };
    auto constrain = [&](unsigned f, std::span<const value_ref_t> refs, std::string_view type) {
        auto *hint = owner_hint(f, refs);
        return hint && require(*hint, type);
    };
    auto operand = [&](unsigned f, std::span<const value_ref_t> refs, bool numeric) {
        auto *hint = owner_hint(f, refs);
        return hint && note_operand(*hint, numeric);
    };
    auto index_operand = [&](unsigned f, const lifted_address_t &address) {
        if (!address.index) return false;
        return operand(f, std::span<const value_ref_t>(&*address.index, 1), true);
    };
    auto storage_key = [&](unsigned f, const lifted_address_t &address) -> std::optional<storage_key_t> {
        unsigned owner = address.owner;
        if (address.kind == address_kind::local) return {};
        if (address.kind == address_kind::instance) {
            if (!address.receiver || address.receiver->offset) return {};
            auto found = classes.find(hints[f][address.receiver->value].resolved());
            if (found == classes.end()) return {};
            owner = found->second;



            if (address.offset + address.width > image.objects[owner].instance_size) return {};
            for (unsigned depth = 0; depth < image.objects.size() && result.parents[owner]; ++depth) {
                auto parent = classes.find(*result.parents[owner]);
                if (parent == classes.end() || address.offset + address.width > image.objects[parent->second].instance_size) break;
                owner = parent->second;
            }
        }
        return storage_key_t{static_cast<unsigned>(address.kind), owner, address.offset, address.width};
    };
    auto callee_arguments = [&](unsigned f, unsigned callee, const std::vector<value_ref_t> &arguments) {
        bool changed = false;
        for (auto p : parameters[callee]) {
            const auto &parameter = functions[callee].values[p];
            const auto begin = parameter.immediate / 4;
            const auto words = parameter.width / 4;
            if (begin + words > arguments.size()) throw error_t("Recovered call arguments exceed their parameter storage");
            const auto refs = std::span<const value_ref_t>(arguments).subspan(begin, words);
            changed |= constrain(f, refs, hints[callee][p].resolved());
            changed |= produce(hints[callee][p], group_type(f, refs));
            changed |= require(hints[callee][p], group_type(f, refs, false));
        }
        return changed;
    };
    auto parameter_words = [&](unsigned callee) {
        const auto &facts = functions[callee];
        return image.objects[facts.object].functions[facts.function].parameters_size / 4;
    };
    auto signal_owner = [&](std::uint32_t identity) -> std::string_view {
        const auto &bytes = image.strings.at(identity >> 16);
        const std::string_view name(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        const auto separator = name.find("::");
        if (separator == std::string_view::npos) return {};
        const auto owner = name.substr(0, separator);
        if (classes.count(owner) || symbols.type_sizes.count(std::string(owner))) return owner;
        return {};
    };
    bool changed;
    do {
        changed = false;
        for (unsigned f = 0; f < functions.size(); ++f) {
            auto &function = functions[f];
            if (!function.diagnostics.empty()) continue;
            const auto &binary = function.initializer ? *image.initializers[function.function].builder
                                                     : image.objects[function.object].functions[function.function];
            for (unsigned v = 0; v < function.values.size(); ++v) {
                const auto &value = function.values[v];
                if (value.kind == value_kind::merge) {
                    for (const auto &argument : value.arguments) changed |= produce(hints[f][v], group_type(f, argument));
                    for (const auto &argument : value.arguments) changed |= require(hints[f][v], group_type(f, argument, false));
                    for (const auto &argument : value.arguments) changed |= constrain(f, argument, hints[f][v].resolved());
                } else if (value.kind == value_kind::signal) {
                    for (const auto &argument : value.arguments)
                        changed |= constrain(f, argument, signal_owner(value.immediate));
                } else if (value.kind == value_kind::operation) {
                    auto op = binary.code.at(value.instruction).opcode;


                    const bool textual = op == OP_STR_EQ || op == OP_STR_NE || op == OP_PSH_STR;
                    for (const auto &argument : value.arguments) changed |= operand(f, argument, !textual);
                } else if (value.kind == value_kind::native_call) {
                    changed |= mirror(hints[f][v], native_returns.at(value.immediate));
                    const auto &native = symbols.natives.at({value.immediate >> 16, value.immediate & 0xffff});
                    for (unsigned argument = 0; argument < native.parameters.size(); ++argument)
                        changed |= constrain(f, value.arguments.at(argument), native.parameters[argument]);
                } else if (value.kind == value_kind::script_call) {
                    auto callee = ids.at({image.object_order.at(value.immediate >> 16), value.immediate & 0xffff});
                    changed |= mirror(hints[f][v], returns[callee]);
                    changed |= callee_arguments(f, callee, value.arguments.front());
                } else if (value.kind == value_kind::indexed_store && value.address) {
                    changed |= index_operand(f, *value.address);
                } else if (value.kind == value_kind::load && value.address) {
                    changed |= index_operand(f, *value.address);
                    if (value.address->kind == address_kind::local && value.address->index) {
                        std::set<unsigned> visited;
                        std::function<void(value_ref_t)> trace = [&](value_ref_t ref) {
                            if (!visited.insert(ref.value).second) return;
                            const auto &producer = function.values[ref.value];
                            if (producer.kind == value_kind::indexed_store && producer.address &&
                                producer.address->offset == value.address->offset && producer.address->width == value.address->width) {
                                changed |= produce(hints[f][v], group_type(f, producer.arguments[1]));
                                changed |= require(hints[f][v], group_type(f, producer.arguments[1], false));
                            }
                            if (producer.kind == value_kind::indexed_store) {
                                for (auto input : producer.arguments[0]) trace(input);
                            } else if (producer.kind == value_kind::merge) {
                                for (const auto &argument : producer.arguments) for (auto input : argument) trace(input);
                            }
                        };
                        for (const auto &argument : value.arguments) for (auto ref : argument) trace(ref);
                    }
                    if (auto key = storage_key(f, *value.address)) {
                        auto &hint = storage_hint(*key);
                        changed |= produce(hints[f][v], hint.produced);
                        changed |= require(hints[f][v], hint.required);
                        changed |= require(hint, hints[f][v].required);
                        if (hint.numeric) changed |= note_operand(hints[f][v], true);
                        if (hint.textual) changed |= note_operand(hints[f][v], false);
                        if (hints[f][v].numeric) changed |= note_operand(hint, true);
                        if (hints[f][v].textual) changed |= note_operand(hint, false);
                    }
                }
            }
            for (const auto &block : function.blocks) for (const auto &effect : block.effects) {
                if (effect.kind == effect_kind::branch)
                    for (const auto &argument : effect.arguments) changed |= operand(f, argument, true);
                if (effect.kind == effect_kind::signal)
                    for (const auto &argument : effect.arguments)
                        changed |= constrain(f, argument, signal_owner(binary.code[effect.instruction].value));
                if (effect.kind == effect_kind::store && effect.address && !effect.arguments.empty() && !effect.arguments[0].empty()) {
                    changed |= index_operand(f, *effect.address);
                    if (auto key = storage_key(f, *effect.address)) {
                        auto &hint = storage_hint(*key);
                        changed |= produce(hint, group_type(f, effect.arguments[0]));
                        changed |= require(hint, group_type(f, effect.arguments[0], false));
                        changed |= constrain(f, effect.arguments[0], hint.resolved());
                    }
                }




                if (effect.kind == effect_kind::thread || effect.kind == effect_kind::callback) {
                    const auto &instruction = binary.code.at(effect.instruction);
                    const auto op = instruction.opcode;
                    const unsigned block = op == OP_BTH || op == OP_ECB || op == OP_ECO ? 1 : 0;
                    if (effect.arguments.size() <= block) continue;
                    const auto callee = ids.at({image.object_order.at(instruction.value >> 16), instruction.value & 0xffff});
                    if (effect.arguments[block].size() != parameter_words(callee)) continue;
                    changed |= callee_arguments(f, callee, effect.arguments[block]);
                    if (effect.kind == effect_kind::callback && effect.arguments.size() > block + 1u)
                        changed |= constrain(f, effect.arguments[block + 1], "signal");
                    continue;
                }
                if (effect.kind != effect_kind::return_value || effect.arguments.empty()) continue;
                changed |= produce(returns[f], group_type(f, effect.arguments.front()));
                changed |= require(returns[f], group_type(f, effect.arguments.front(), false));
                changed |= constrain(f, effect.arguments.front(), returns[f].resolved());
                if (!function.initializer) continue;
                const auto &initializer = image.initializers[function.function];
                const auto object = image.object_order.at(initializer.object);
                const auto callee = ids.at({object, 0});
                const unsigned receiver = (image.objects[object].functions[0].flags & 1u) ? 0 : 4;
                for (auto p : parameters[callee]) {
                    const auto &parameter = functions[callee].values[p];
                    if (parameter.immediate < receiver) continue;
                    const auto begin = (parameter.immediate - receiver) / 4;
                    const auto words = parameter.width / 4;
                    if (begin + words > effect.arguments[0].size()) throw error_t("Constructor builder argument storage is incomplete");
                    const auto refs = std::span<const value_ref_t>(effect.arguments[0]).subspan(begin, words);
                    changed |= constrain(f, refs, hints[callee][p].resolved());
                    changed |= produce(hints[callee][p], group_type(f, refs));
                    changed |= require(hints[callee][p], group_type(f, refs, false));
                }
            }
        }
    } while (changed);
    auto recovered = [](const type_hint_t &hint) {
        auto type = hint.resolved();
        if (type == "nullptr") return std::string("num");
        return type.empty() ? opaque_type(hint.width) : std::string(type);
    };
    for (unsigned f = 0; f < functions.size(); ++f) {
        auto &output = result.functions[f];
        if (!functions[f].diagnostics.empty()) continue;
        output.result = recovered(returns[f]);
        for (unsigned v = 0; v < functions[f].values.size(); ++v) {
            auto &value = functions[f].values[v];
            value.type = recovered(hints[f][v]);
            if (value.kind == value_kind::parameter) output.parameters.push_back({value.immediate, value.width, value.type});
        }
        std::sort(output.parameters.begin(), output.parameters.end(), [](const auto &left, const auto &right) { return left.offset < right.offset; });
    }
    for (const auto &[key, hint] : storage) result.storage.emplace(key, recovered(hint));
    return result;
}

}
