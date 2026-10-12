#include "sxcc.h"
#include "language.h"
#include "platform.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <iostream>

namespace {

void usage()
{
    std::cout << "SXCC script toolchain\n"
                 "  sxcc -dis | disassemble [INPUT|-] [-o OUTPUT] [OPTIONS]\n"
                 "  sxcc -dec | decompile   [INPUT|-] [-o OUTPUT] [OPTIONS]\n"
                 "  sxcc -a   | assemble    [INPUT|-] -o OUTPUT [OPTIONS]\n"
                 "  sxcc -c   | compile     [INPUT|-] -o OUTPUT [OPTIONS]\n"
                 "  sxcc --extract-segments [INPUT|-] -o PREFIX\n"
                 "    Extract instructions, bindings and strings for comparison.\n"
                 "  --symbols DICTIONARY\n"
                 "  --platform pc|ps2|xbox (default: pc)\n"
                 "  -v, --verbose: progress/status on stderr\n"
                 "Omitted input or - reads stdin. Text output defaults to stdout.\n"
                 "Binary output requires -o. Errors and warnings go to stderr.\n";
}

sxcc::bytes_t read_input(const std::string &path)
{
    if (path != "-")
        return sxcc::read_file(path);
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1)
        throw sxcc::error_t("Cannot configure standard input");
#endif
    sxcc::bytes_t result;
    std::array<char, 65536> buffer;
    while (std::cin.read(buffer.data(), buffer.size()) || std::cin.gcount()) {
        const auto size = static_cast<std::size_t>(std::cin.gcount());
        if (size > UINT32_MAX - result.size())
            throw sxcc::error_t("Standard input exceeds 32-bit size limit");
        result.insert(result.end(), buffer.data(), buffer.data() + size);
    }
    if (std::cin.bad() || !std::cin.eof())
        throw sxcc::error_t("Cannot read standard input");
    return result;
}

void write_text(const std::string &path, const std::string &text)
{
    if (!path.empty()) {
        sxcc::write_file(path, {reinterpret_cast<const std::uint8_t *>(text.data()), text.size()});
        return;
    }
    std::cout.write(text.data(), static_cast<std::streamsize>(text.size()));
    std::cout.flush();
    if (!std::cout)
        throw sxcc::error_t("Cannot write standard output");
}

void write_check_data(const sxcc::image_t &image, std::span<const std::uint8_t> input, const std::string &prefix)
{
    std::size_t binding_size = 4 + 12 * image.objects.size();
    std::size_t function_count = 0;
    for (const auto &object : image.objects) {
        binding_size += 24 * object.functions.size();
        function_count += object.functions.size();
    }
    for (const auto &initializer : image.initializers)
        function_count += initializer.builder.has_value();
    std::vector<std::uint32_t> code_offsets(function_count + 1);
    auto code_size = [](const sxcc::function_t &function) {
        return function.code.empty() ? 0 : function.code.back().offset + function.code.back().size();
    };
    for (const auto &object : image.objects)
        for (const auto &function : object.functions)
            code_offsets.at(function.code_order + 1) = code_size(function);
    for (const auto &initializer : image.initializers)
        if (initializer.builder)
            code_offsets.at(initializer.builder->code_order + 1) = code_size(*initializer.builder);
    for (std::size_t i = 1; i < code_offsets.size(); ++i)
        code_offsets[i] += code_offsets[i - 1];
    sxcc::bytes_t bindings;
    bindings.reserve(binding_size);
    auto word = [](sxcc::bytes_t &bytes, std::uint32_t value) {
        const auto offset = bytes.size();
        bytes.resize(offset + 4);
        sxcc::put_word(bytes, offset, value);
    };
    word(bindings, static_cast<std::uint32_t>(image.objects.size()));
    for (const auto index : image.object_order) {
        const auto &object = image.objects.at(index);
        word(bindings, object.name_hash);
        word(bindings, static_cast<std::uint32_t>(object.static_data.size()));
        word(bindings, static_cast<std::uint32_t>(object.functions.size()));
        for (const auto &function : object.functions) {
            word(bindings, function.fullname_hash);
            word(bindings, function.name_hash);
            word(bindings, function.parameters_size);
            word(bindings, code_offsets.at(function.code_order));
            word(bindings, code_size(function) / 2);
            word(bindings, function.flags);
        }
    }
    std::size_t string_size = 4 + 4 * image.strings.size();
    for (const auto &string : image.strings)
        string_size += string.size();
    sxcc::bytes_t strings;
    strings.reserve(string_size);
    word(strings, static_cast<std::uint32_t>(image.strings.size()));
    for (const auto &string : image.strings) {
        word(strings, static_cast<std::uint32_t>(string.size()));
        strings.insert(strings.end(), string.begin(), string.end());
    }
    constexpr auto header_size = sizeof(sxcc::generic_mash_header) + sizeof(sxcc::script_executable);
    constexpr auto code_size_offset =
        sizeof(sxcc::generic_mash_header) + offsetof(sxcc::script_executable, sx_exe_image_size);
    sxcc::write_file(prefix + ".instructions", input.subspan(header_size, sxcc::read_word(input, code_size_offset)));
    sxcc::write_file(prefix + ".bindings", bindings);
    sxcc::write_file(prefix + ".strings", strings);
}

}

int main(int argc, char **argv)
{
    try {
#ifdef _WIN32
        if (_setmode(_fileno(stdout), _O_BINARY) == -1)
            throw sxcc::error_t("Cannot configure standard output");
#endif
        if (argc == 1 || (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h"))) {
            usage();
            return 0;
        }
        std::string_view command = argv[1];
        if (command == "-dis")
            command = "disassemble";
        else if (command == "-dec")
            command = "decompile";
        else if (command == "-c")
            command = "compile";
        else if (command == "-a")
            command = "assemble";
        bool supported = command == "disassemble" || command == "decompile" || command == "compile" ||
                         command == "assemble" || command == "--extract-segments";
        if (!supported)
            throw sxcc::error_t("Unknown command " + std::string(command));
        std::string input_path = "-", output_path, dictionary;
        auto platform = sxcc::platform_t::pc;
        bool platform_set = false, input_set = false, output_set = false, verbose = false;
        for (int i = 2; i < argc; ++i) {
            const std::string_view option = argv[i];
            if (option == "--symbols") {
                if (++i == argc || !dictionary.empty() || !*argv[i])
                    throw sxcc::error_t("--symbols requires one dictionary");
                dictionary = argv[i];
            } else if (option == "--platform") {
                if (++i == argc || platform_set)
                    throw sxcc::error_t("--platform requires exactly one platform");
                platform = sxcc::parse_platform(argv[i]);
                platform_set = true;
            } else if (option == "-o") {
                if (++i == argc || output_set || !*argv[i])
                    throw sxcc::error_t("-o requires exactly one output path");
                output_path = argv[i];
                output_set = true;
            } else if (option == "-v" || option == "--verbose") {
                verbose = true;
            } else if (option != "-" && option.starts_with("-")) {
                throw sxcc::error_t("Unknown option " + std::string(option));
            } else {
                if (input_set)
                    throw sxcc::error_t("Only one input is accepted; specify output with -o");
                input_path = argv[i];
                input_set = true;
            }
        }
        bool binary_output = command == "assemble" || command == "compile" || command == "--extract-segments";
        bool accepts_symbols = command != "assemble" && command != "--extract-segments";
        if (binary_output && !output_set)
            throw sxcc::error_t("Binary output requires -o OUTPUT");
        if (!accepts_symbols && !dictionary.empty())
            throw sxcc::error_t("This command does not consume a symbol dictionary");
        if (platform != sxcc::platform_t::pc) {
            bool supported_console = command == "disassemble";
            if (!supported_console)
                throw sxcc::error_t("Console support is limited to bytecode-only disassembly; full metadata/native "
                                    "profiles are not available");
            if (!dictionary.empty())
                throw sxcc::error_t("Console disassembly does not apply a PC symbol dictionary; omit --symbols");
        }
        auto input = read_input(input_path);
        if (platform != sxcc::platform_t::pc) {
            const auto *destination = &output_path;
            auto *status = verbose ? &std::cerr : nullptr;
            sxcc::inspect_console(platform, input_path, input, destination, status);
            return 0;
        }
        if (command == "assemble" || command == "compile") {
            const std::string_view source(reinterpret_cast<const char *>(input.data()), input.size());
            const auto image = command == "assemble" ? sxcc::assemble_source(source)
                                                     : sxcc::language::lower(sxcc::language::parse(source),
                                                                             sxcc::load_symbols(dictionary));
            const auto output = sxcc::encode_image(image);
            sxcc::write_file(output_path, output);
            if (verbose)
                std::cerr << (command == "assemble" ? "Assembled " : "Compiled ") << output.size() << " bytes\n";
            return 0;
        }
        auto image = sxcc::decode_image(input);
        if (command == "--extract-segments") {
            write_check_data(image, input, output_path);
            return 0;
        }
        auto symbols = sxcc::load_symbols(dictionary);
        sxcc::resolve_symbols(image, symbols);
        const auto source =
            command == "decompile" ? sxcc::emit_decompiled_source(image, symbols) : sxcc::emit_assembly(image);
        write_text(output_path, source);
        if (verbose)
            std::cerr << (command == "decompile" ? "Decompiled " : "Disassembled ") << image.objects.size()
                      << " objects\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "sxcc: " << error.what() << '\n';
        return 1;
    }
}
