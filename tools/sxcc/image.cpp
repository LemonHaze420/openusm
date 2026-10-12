#include "sxcc.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <limits>

namespace sxcc {

struct data_block_access {
    static constexpr auto size_offset = offsetof(so_data_block, m_size);
    static constexpr auto buffer_offset = offsetof(so_data_block, buffer);
    static constexpr auto flags_offset = offsetof(so_data_block, flags);
};

struct script_var_container_access {
    static constexpr auto block_offset = offsetof(script_var_container, script_var_block);
    static constexpr auto table_offset = offsetof(script_var_container, script_var_to_addr);
    static constexpr auto count_offset = offsetof(script_var_container, field_10);
    static constexpr auto debug_offset = offsetof(script_var_container, debug_info);
    using entry_t = script_var_container::script_var_to_addr_t;
};

static_assert(sizeof(pc_pointer_t) == 4);
static_assert(sizeof(script_executable) == 0x5c);
static_assert(sizeof(script_object) == 0x34);
static_assert(sizeof(vm_executable) == 0x24);
static_assert(sizeof(generic_mash_header) == 0x10);

namespace {

constexpr auto static_size_offset = offsetof(script_object, static_data) + data_block_access::size_offset;

std::uint32_t take_word(bytes_t &record, std::size_t offset)
{
    auto value = read_word(record, offset);
    put_word(record, offset, 0);
    return value;
}

std::size_t count_bytes(std::uint32_t count, std::size_t size, std::size_t available)
{
    if (size != 0 && count > available / size)
        throw error_t("Serialized count exceeds remaining input");
    return static_cast<std::size_t>(count) * size;
}

struct reader_t {
    std::span<const std::uint8_t> bytes;
    std::size_t offset = 0;
    std::vector<bytes_t> &padding;

    bytes_t take(std::size_t size)
    {
        if (offset > bytes.size() || size > bytes.size() - offset)
            throw error_t("Truncated PCSX at " + hex(static_cast<std::uint32_t>(offset)));
        bytes_t result(bytes.begin() + offset, bytes.begin() + offset + size);
        offset += size;
        return result;
    }

    bytes_t array(std::uint32_t count, std::size_t size)
    {
        return take(count_bytes(count, size, bytes.size() - offset));
    }

    void align(unsigned boundary)
    {
        auto size = (boundary - offset % boundary) % boundary;
        padding.push_back(take(size));
    }
};

bytes_t record_bytes(const bytes_t &preserved, std::size_t size)
{
    if (preserved.empty())
        return bytes_t(size, 0);
    if (preserved.size() != size)
        throw error_t("Lossless record has the wrong engine ABI size");
    return preserved;
}

struct writer_t {
    bytes_t bytes;
    const std::vector<bytes_t> &padding;
    std::size_t padding_index = 0;

    void append(std::span<const std::uint8_t> value)
    {
        if (value.size() > UINT32_MAX - bytes.size())
            throw error_t("PCSX exceeds 32-bit size limit");
        bytes.insert(bytes.end(), value.begin(), value.end());
    }

    void align(unsigned boundary)
    {
        auto size = (boundary - bytes.size() % boundary) % boundary;
        if (padding_index < padding.size() && padding[padding_index].size() == size)
            append(padding[padding_index]);
        else
            bytes.insert(bytes.end(), size, 0xe3);
        ++padding_index;
    }

    void slots(const bytes_t &preserved, std::size_t count)
    {
        auto size = count_bytes(static_cast<std::uint32_t>(count), sizeof(pc_pointer_t), UINT32_MAX);
        auto copy = std::min(size, preserved.size());
        append(std::span(preserved).first(copy));
        bytes.insert(bytes.end(), size - copy, 0);
    }
};

struct code_range_t {
    function_t *function;
    std::uint32_t offset;
    std::uint32_t size;
};

function_t read_function(reader_t &reader, std::vector<code_range_t> &ranges, std::span<const std::uint8_t> code)
{
    function_t function;
    function.preserved = reader.take(sizeof(vm_executable));
    auto &record = function.preserved;
    if (read_word(record, offsetof(vm_executable, debug_info)) != 0)
        throw error_t("Packed debug function records are not a retail PC layout");
    function.fullname_hash = take_word(record, offsetof(vm_executable, fullname));
    function.name_hash = take_word(record, offsetof(vm_executable, name));
    function.parameters_size = take_word(record, offsetof(vm_executable, parms_stacksize));
    function.flags = take_word(record, offsetof(vm_executable, flags));
    auto offset = take_word(record, offsetof(vm_executable, buffer));
    auto words = take_word(record, offsetof(vm_executable, buffer_len));
    if (offset > code.size() || words > (code.size() - offset) / sizeof(std::uint16_t))
        throw error_t("Function code range is outside the executable image");
    function.code = decode_code(code.subspan(offset, words * sizeof(std::uint16_t)));
    function.code_order = offset;
    function.name = "function_" + hex(function.fullname_hash).substr(2);
    ranges.push_back({nullptr, offset, static_cast<std::uint32_t>(words * sizeof(std::uint16_t))});
    return function;
}

}

std::span<const std::uint8_t> assembly_record_defaults(assembly_record_t kind)
{
    static constexpr auto mash = [] {
        std::array<std::uint8_t, sizeof(generic_mash_header)> bytes{};
        bytes[offsetof(generic_mash_header, class_id)] = 0xff;
        bytes[offsetof(generic_mash_header, class_id) + 1] = 0xff;
        return bytes;
    }();
    static constexpr auto image = [] {
        std::array<std::uint8_t, sizeof(script_executable)> bytes{};
        bytes[offsetof(script_executable, flags)] = script_executable::SCRIPT_EXECUTABLE_FLAG_FROM_MASH;
        return bytes;
    }();
    static constexpr auto object = [] {
        std::array<std::uint8_t, sizeof(script_object)> bytes{};
        bytes[offsetof(script_object, static_data) + data_block_access::flags_offset] = SO_DATA_BLOCK_FLAG_FROM_MASH;
        return bytes;
    }();
    static constexpr auto function = [] {
        std::array<std::uint8_t, sizeof(vm_executable)> bytes{};
        for (unsigned i = 0; i < sizeof(vm_executable::field_20); ++i)
            bytes[offsetof(vm_executable, field_20) + i] = 0xcd;
        return bytes;
    }();
    static constexpr std::array<std::uint8_t, sizeof(script_executable::info_t)> initializer{};
    switch (kind) {
    case assembly_record_t::mash:
        return mash;
    case assembly_record_t::image:
        return image;
    case assembly_record_t::object:
        return object;
    case assembly_record_t::function:
        return function;
    case assembly_record_t::initializer:
        return initializer;
    }
    throw error_t("Unknown assembly record kind");
}

image_t decode_image(std::span<const std::uint8_t> bytes)
{
    image_t image;
    reader_t reader{bytes, 0, image.padding};
    image.mash_preserved = reader.take(sizeof(generic_mash_header));
    generic_mash_header header{};
    std::memcpy(&header, image.mash_preserved.data(), sizeof(header));
    if (header.safety_key != static_cast<std::uint32_t>(header.generate_safety_key()) ||
        header.field_8 < static_cast<int>(sizeof(generic_mash_header)) ||
        static_cast<std::size_t>(header.field_8) > bytes.size() || header.class_id != UINT16_MAX)
        throw error_t("Not an unlinked retail PC script mash");
    put_word(image.mash_preserved, offsetof(generic_mash_header, safety_key), 0);


    if (static_cast<std::size_t>(header.field_8) == bytes.size())
        put_word(image.mash_preserved, offsetof(generic_mash_header, field_8), 0);
    image.preserved = reader.take(sizeof(script_executable));
    auto &record = image.preserved;
    constexpr auto name_offset = offsetof(script_executable, field_0);
    constexpr auto name_size = sizeof(script_executable::field_0);
    auto name_begin = reinterpret_cast<const char *>(record.data() + name_offset);
    auto name_end = std::find(name_begin, name_begin + name_size, '\0');
    image.name.assign(name_begin, name_end);
    std::fill(record.begin() + name_offset, record.begin() + name_offset + image.name.size(), 0);
    auto code_size = take_word(record, offsetof(script_executable, sx_exe_image_size));
    auto object_count = take_word(record, offsetof(script_executable, total_script_objects));
    auto string_count = take_word(record, offsetof(script_executable, permanent_string_table_size));
    auto info_count = take_word(record, offsetof(script_executable, field_58));
    reader.align(4);
    auto code = reader.take(code_size);
    reader.align(4);
    image.object_slots = reader.array(object_count, sizeof(pc_pointer_t));
    image.objects.reserve(object_count);
    std::vector<code_range_t> ranges;
    for (unsigned i = 0; i < object_count; ++i) {
        reader.align(8);
        reader.align(4);
        object_t object;
        object.preserved = reader.take(sizeof(script_object));
        auto &data = object.preserved;
        if (read_word(data, offsetof(script_object, debug_info)) != 0)
            throw error_t("Packed debug object records are not a retail PC layout");
        object.name_hash = take_word(data, offsetof(script_object, name));
        object.name = "object_" + hex(object.name_hash).substr(2);
        object.instance_size = take_word(data, offsetof(script_object, data_blocksize));
        object.destructor = static_cast<std::int32_t>(take_word(data, offsetof(script_object, field_28)));
        object.flags = take_word(data, offsetof(script_object, flags));
        auto static_size = take_word(data, static_size_offset);
        auto function_count = take_word(data, offsetof(script_object, total_funcs));
        reader.align(4);
        object.static_data = reader.take(static_size);
        reader.align(4);
        object.function_slots = reader.array(function_count, sizeof(pc_pointer_t));
        object.functions.reserve(function_count);
        for (unsigned j = 0; j < function_count; ++j) {
            reader.align(4);
            object.functions.push_back(read_function(reader, ranges, code));
        }
        image.objects.push_back(std::move(object));
    }
    reader.align(4);
    auto order = reader.array(object_count, sizeof(pc_pointer_t));
    for (unsigned i = 0; i < object_count; ++i)
        image.object_order.push_back(read_word(order, i * sizeof(pc_pointer_t)));
    reader.align(4);
    image.string_slots = reader.array(string_count, sizeof(pc_pointer_t));
    image.strings.reserve(string_count);
    for (unsigned i = 0; i < string_count; ++i) {
        reader.align(4);
        auto size = read_word(reader.take(sizeof(std::uint32_t)), 0);
        image.strings.push_back(reader.take(size));
    }
    reader.align(4);
    reader.align(4);
    auto infos = reader.array(info_count, sizeof(script_executable::info_t));
    reader.align(4);
    image.initializers.reserve(info_count);
    for (unsigned i = 0; i < info_count; ++i) {
        initializer_t initializer;
        auto begin = infos.begin() + i * sizeof(script_executable::info_t);
        initializer.preserved.assign(begin, begin + sizeof(script_executable::info_t));
        auto &data = initializer.preserved;
        initializer.name_hash = take_word(data, offsetof(script_executable::info_t, field_0));
        initializer.offset = take_word(data, offsetof(script_executable::info_t, field_C));
        initializer.kind = static_cast<std::int32_t>(take_word(data, offsetof(script_executable::info_t, field_10)));
        initializer.value = take_word(data, offsetof(script_executable::info_t, field_14));
        initializer.object = take_word(data, offsetof(script_executable::info_t, field_18));
        if (initializer.kind == -1) {
            reader.align(4);
            initializer.builder = read_function(reader, ranges, code);
        }
        image.initializers.push_back(std::move(initializer));
    }
    reader.align(16);
    if (reader.offset != bytes.size())
        throw error_t("Unconsumed bytes after retail PC script metadata");
    std::sort(ranges.begin(), ranges.end(), [](const auto &a, const auto &b) { return a.offset < b.offset; });
    std::uint32_t end = 0;
    std::map<unsigned, unsigned> code_orders;
    for (unsigned i = 0; i < ranges.size(); ++i) {
        const auto &range = ranges[i];
        if (range.offset != end)
            throw error_t("Function ranges overlap or leave unowned bytecode");
        code_orders.emplace(range.offset, i);
        end += range.size;
    }
    if (end != code.size())
        throw error_t("Function records do not cover the executable bytecode");
    for (auto &object : image.objects)
        for (auto &function : object.functions)
            function.code_order = code_orders.at(function.code_order);
    for (auto &initializer : image.initializers)
        if (initializer.builder)
            initializer.builder->code_order = code_orders.at(initializer.builder->code_order);
    validate_image(image);
    return image;
}

bytes_t encode_image(const image_t &image)
{
    validate_image(image);
    std::vector<const function_t *> functions;
    for (const auto &object : image.objects)
        for (const auto &function : object.functions)
            functions.push_back(&function);
    for (const auto &initializer : image.initializers)
        if (initializer.builder)
            functions.push_back(&*initializer.builder);
    std::stable_sort(functions.begin(), functions.end(), [](auto a, auto b) { return a->code_order < b->code_order; });
    bytes_t code;
    std::map<const function_t *, std::pair<std::uint32_t, std::uint32_t>> locations;
    for (auto function : functions) {
        auto bytes = encode_code(function->code);
        if (bytes.size() > UINT32_MAX - code.size())
            throw error_t("Combined bytecode exceeds 32 bits");
        locations.emplace(function,
                          std::pair(static_cast<std::uint32_t>(code.size()),
                                    static_cast<std::uint32_t>(bytes.size() / sizeof(std::uint16_t))));
        code.insert(code.end(), bytes.begin(), bytes.end());
    }
    auto root = record_bytes(image.preserved, sizeof(script_executable));
    if (image.name.size() > sizeof(script_executable::field_0))
        throw error_t("Script name exceeds fixed engine name capacity");
    std::copy(image.name.begin(), image.name.end(), root.begin() + offsetof(script_executable, field_0));
    if (image.name.size() < sizeof(script_executable::field_0))
        root[offsetof(script_executable, field_0) + image.name.size()] = 0;
    put_word(root, offsetof(script_executable, sx_exe_image_size), static_cast<std::uint32_t>(code.size()));
    put_word(root, offsetof(script_executable, total_script_objects), static_cast<std::uint32_t>(image.objects.size()));
    put_word(root,
             offsetof(script_executable, permanent_string_table_size),
             static_cast<std::uint32_t>(image.strings.size()));
    put_word(root, offsetof(script_executable, field_58), static_cast<std::uint32_t>(image.initializers.size()));
    if (image.preserved.empty())
        put_word(root, offsetof(script_executable, flags), script_executable::SCRIPT_EXECUTABLE_FLAG_FROM_MASH);
    writer_t writer{record_bytes(image.mash_preserved, sizeof(generic_mash_header)), image.padding, 0};
    writer.append(root);
    writer.align(4);
    writer.append(code);
    writer.align(4);
    writer.slots(image.object_slots, image.objects.size());
    auto write_function = [&](const function_t &function) {
        auto record = record_bytes(function.preserved, sizeof(vm_executable));
        put_word(record, offsetof(vm_executable, fullname), function.fullname_hash);
        put_word(record, offsetof(vm_executable, name), function.name_hash);
        put_word(record, offsetof(vm_executable, parms_stacksize), function.parameters_size);
        put_word(record, offsetof(vm_executable, flags), function.flags);
        put_word(record, offsetof(vm_executable, buffer), locations.at(&function).first);
        put_word(record, offsetof(vm_executable, buffer_len), locations.at(&function).second);
        writer.append(record);
    };
    for (const auto &object : image.objects) {
        writer.align(8);
        writer.align(4);
        auto record = record_bytes(object.preserved, sizeof(script_object));
        put_word(record, offsetof(script_object, name), object.name_hash);
        put_word(record, offsetof(script_object, data_blocksize), object.instance_size);
        put_word(record, offsetof(script_object, field_28), static_cast<std::uint32_t>(object.destructor));
        put_word(record, offsetof(script_object, flags), object.flags);
        put_word(record, static_size_offset, static_cast<std::uint32_t>(object.static_data.size()));
        put_word(record, offsetof(script_object, total_funcs), static_cast<std::uint32_t>(object.functions.size()));
        if (object.preserved.empty())
            put_word(record,
                     offsetof(script_object, static_data) + data_block_access::flags_offset,
                     SO_DATA_BLOCK_FLAG_FROM_MASH);
        writer.append(record);
        writer.align(4);
        writer.append(object.static_data);
        writer.align(4);
        writer.slots(object.function_slots, object.functions.size());
        for (const auto &function : object.functions) {
            writer.align(4);
            write_function(function);
        }
    }
    writer.align(4);
    bytes_t order(image.object_order.size() * sizeof(pc_pointer_t));
    for (std::size_t i = 0; i < image.object_order.size(); ++i)
        put_word(order, i * sizeof(pc_pointer_t), image.object_order[i]);
    writer.append(order);
    writer.align(4);
    writer.slots(image.string_slots, image.strings.size());
    for (const auto &string : image.strings) {
        writer.align(4);
        bytes_t length(sizeof(std::uint32_t));
        put_word(length, 0, static_cast<std::uint32_t>(string.size()));
        writer.append(length);
        writer.append(string);
    }
    writer.align(4);
    writer.align(4);
    for (const auto &initializer : image.initializers) {
        auto record = record_bytes(initializer.preserved, sizeof(script_executable::info_t));
        put_word(record, offsetof(script_executable::info_t, field_0), initializer.name_hash);
        put_word(record, offsetof(script_executable::info_t, field_C), initializer.offset);
        put_word(record, offsetof(script_executable::info_t, field_10), static_cast<std::uint32_t>(initializer.kind));
        put_word(record, offsetof(script_executable::info_t, field_14), initializer.value);
        put_word(record, offsetof(script_executable::info_t, field_18), initializer.object);
        writer.append(record);
    }
    writer.align(4);
    for (const auto &initializer : image.initializers)
        if (initializer.builder) {
            writer.align(4);
            write_function(*initializer.builder);
        }
    writer.align(16);
    generic_mash_header header{};
    std::memcpy(&header, writer.bytes.data(), sizeof(header));
    if (header.field_8 == 0)
        header.field_8 = static_cast<int>(writer.bytes.size());
    else if (header.field_8 < 0 || static_cast<std::size_t>(header.field_8) > writer.bytes.size())
        throw error_t("Explicit mash shared-data boundary exceeds the rebuilt image");
    if (image.mash_preserved.empty())
        header.class_id = UINT16_MAX;
    header.safety_key = header.generate_safety_key();
    std::memcpy(writer.bytes.data(), &header, sizeof(header));
    return writer.bytes;
}

void validate_image(const image_t &image)
{
    if (image.objects.empty() || image.objects.size() > UINT16_MAX || image.strings.size() > UINT16_MAX)
        throw error_t("PCSX requires 1..65535 objects and at most 65535 strings");
    if (image.object_order.size() != image.objects.size())
        throw error_t("Object lookup order is not a complete permutation");
    auto order = image.object_order;
    std::sort(order.begin(), order.end());
    for (unsigned i = 0; i < order.size(); ++i)
        if (order[i] != i)
            throw error_t("Object lookup order contains duplicate or invalid indices");
    auto validate_function = [&](const function_t &function) {
        if (function.parameters_size % 4 != 0 || function.parameters_size > INT32_MAX)
            throw error_t("Invalid function parameter stack size");
        auto analysis = analyze_function(function);
        if (!analysis.diagnostics.empty())
            throw error_t(function.name + ": " + analysis.diagnostics.front());
        for (const auto &instruction : function.code) {
            auto first = instruction.value >> 16;
            auto second = instruction.value & UINT16_MAX;
            if (instruction.argument == OP_ARG_SFR || instruction.argument == OP_ARG_SDR) {
                if (first >= image.object_order.size())
                    throw error_t("Reference addresses an absent script object");
                const auto &object = image.objects[image.object_order[first]];
                if (instruction.argument == OP_ARG_SFR && second >= object.functions.size())
                    throw error_t("Reference addresses an absent script function");
                if (instruction.argument == OP_ARG_SDR && second >= object.static_data.size())
                    throw error_t("Reference addresses an absent static data field");
            }
            if ((instruction.argument == OP_ARG_STR || instruction.argument == OP_ARG_SIG ||
                 instruction.argument == OP_ARG_PSIG) &&
                first >= image.strings.size())
                throw error_t("Reference addresses an absent permanent string");
            if (instruction.argument == OP_ARG_CLV && second >= image.strings.size())
                throw error_t("Class value references an absent permanent string");
        }
    };
    for (const auto &object : image.objects) {
        if (object.functions.size() > UINT16_MAX || object.instance_size > INT32_MAX)
            throw error_t("Object exceeds engine layout limits");
        if (object.destructor != -1 &&
            (object.destructor < 0 || static_cast<std::size_t>(object.destructor) >= object.functions.size()))
            throw error_t("Invalid destructor function index");
        for (const auto &function : object.functions)
            validate_function(function);
    }
    for (const auto &initializer : image.initializers) {
        if (initializer.object >= image.object_order.size() || initializer.offset % 4 != 0 ||
            initializer.offset > image.objects.front().static_data.size() ||
            image.objects.front().static_data.size() - initializer.offset < 4)
            throw error_t("Initializer addresses invalid object or global storage");
        if ((initializer.kind == -1) != initializer.builder.has_value())
            throw error_t("Initializer builder does not match its initialization kind");
        if (initializer.builder)
            validate_function(*initializer.builder);
    }
}

}
