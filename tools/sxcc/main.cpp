#include "sxcc.h"
#include "language.h"
#include "platform.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {

void usage()
{
    std::cout << "SXCC script toolchain\n"
                 "  sxcc -dis | disassemble [INPUT|-] [-o OUTPUT] [OPTIONS]\n"
                 "  sxcc -dec | decompile   [INPUT|-] [-o OUTPUT] [OPTIONS]\n"
                 "  sxcc -a   | assemble    [INPUT|-] -o OUTPUT [OPTIONS]\n"
                 "  sxcc -c   | compile     [INPUT|-] -o OUTPUT [OPTIONS]\n"
                 "  --symbols DICTIONARY\n"
                 "  --platform pc|ps2|xbox (default: pc)\n"
                 "  -v, --verbose: progress/status on stderr\n"
                 "Omitted input or - reads stdin. Text output defaults to stdout.\n"
                 "Binary output requires -o. Errors and warnings go to stderr.\n";
}

sxcc::bytes_t read_input(const std::string &path)
{
    if (path != "-") return sxcc::read_file(path);
    if (_setmode(_fileno(stdin), _O_BINARY) == -1) throw sxcc::error_t("Cannot configure standard input");
    sxcc::bytes_t result;
    std::array<char, 65536> buffer;
    while (std::cin.read(buffer.data(), buffer.size()) || std::cin.gcount()) {
        const auto size = static_cast<std::size_t>(std::cin.gcount());
        if (size > UINT32_MAX - result.size()) throw sxcc::error_t("Standard input exceeds 32-bit size limit");
        result.insert(result.end(), buffer.data(), buffer.data() + size);
    }
    if (std::cin.bad() || !std::cin.eof()) throw sxcc::error_t("Cannot read standard input");
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
    if (!std::cout) throw sxcc::error_t("Cannot write standard output");
}

}

int main(int argc, char **argv)
{
    try {
        if (_setmode(_fileno(stdout), _O_BINARY) == -1) throw sxcc::error_t("Cannot configure standard output");
        if (argc == 1 || (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h"))) {
            usage();
            return 0;
        }
        std::string_view command = argv[1];
        if (command == "-dis") command = "disassemble";
        else if (command == "-dec") command = "decompile";
        else if (command == "-c") command = "compile";
        else if (command == "-a") command = "assemble";
        bool supported = command == "disassemble" || command == "decompile" || command == "compile" || command == "assemble";
        if (!supported) throw sxcc::error_t("Unknown command " + std::string(command));
        std::string input_path = "-", output_path, dictionary;
        auto platform = sxcc::platform_t::pc;
        bool platform_set = false, input_set = false, output_set = false, verbose = false;
        for (int i = 2; i < argc; ++i) {
            const std::string_view option = argv[i];
            if (option == "--symbols") {
                if (++i == argc || !dictionary.empty() || !*argv[i]) throw sxcc::error_t("--symbols requires one dictionary");
                dictionary = argv[i];
            } else if (option == "--platform") {
                if (++i == argc || platform_set) throw sxcc::error_t("--platform requires exactly one platform");
                platform = sxcc::parse_platform(argv[i]);
                platform_set = true;
            } else if (option == "-o") {
                if (++i == argc || output_set || !*argv[i]) throw sxcc::error_t("-o requires exactly one output path");
                output_path = argv[i];
                output_set = true;
            } else if (option == "-v" || option == "--verbose") {
                verbose = true;
            } else if (option != "-" && option.starts_with("-")) {
                throw sxcc::error_t("Unknown option " + std::string(option));
            } else {
                if (input_set) throw sxcc::error_t("Only one input is accepted; specify output with -o");
                input_path = argv[i];
                input_set = true;
            }
        }
        bool binary_output = command == "assemble" || command == "compile";
        bool accepts_symbols = command != "assemble";
        if (binary_output && !output_set) throw sxcc::error_t("Binary output requires -o OUTPUT");
        if (!accepts_symbols && !dictionary.empty()) throw sxcc::error_t("This command does not consume a symbol dictionary");
        if (platform != sxcc::platform_t::pc) {
            bool supported_console = command == "disassemble";
            if (!supported_console) throw sxcc::error_t("Console support is limited to bytecode-only disassembly; full metadata/native profiles are not available");
            if (!dictionary.empty()) throw sxcc::error_t("Console disassembly does not apply a PC symbol dictionary; omit --symbols");
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
            const auto image = command == "assemble" ? sxcc::assemble_source(source) :
                sxcc::language::lower(sxcc::language::parse(source), sxcc::load_symbols(dictionary));
            const auto output = sxcc::encode_image(image);
            sxcc::write_file(output_path, output);
            if (verbose) std::cerr << (command == "assemble" ? "Assembled " : "Compiled ") << output.size() << " bytes\n";
            return 0;
        }
        auto image = sxcc::decode_image(input);
        auto symbols = sxcc::load_symbols(dictionary);
        sxcc::resolve_symbols(image, symbols);
        const auto source = command == "decompile" ? sxcc::emit_decompiled_source(image, symbols) : sxcc::emit_assembly(image);
        write_text(output_path, source);
        if (verbose) std::cerr << (command == "decompile" ? "Decompiled " : "Disassembled ") << image.objects.size() << " objects\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "sxcc: " << error.what() << '\n';
        return 1;
    }
}
