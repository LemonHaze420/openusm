#pragma once

#include "sxcc.h"
#include "parse_generic_mash.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace sxcc {


enum class platform_t { pc, ps2, xbox };

inline platform_t parse_platform(std::string_view name)
{
    if (name == "pc") return platform_t::pc;
    if (name == "ps2") return platform_t::ps2;
    if (name == "xbox") return platform_t::xbox;
    throw error_t("--platform must be pc, ps2, or xbox");
}

inline const char *platform_name(platform_t platform)
{
    switch (platform) {
    case platform_t::pc: return "pc";
    case platform_t::ps2: return "ps2";
    case platform_t::xbox: return "xbox";
    }
    throw error_t("Invalid platform");
}

inline void inspect_console(platform_t platform, const std::string &path,
                            std::span<const std::uint8_t> input, const std::string *output,
                            std::ostream *status)
{
    std::span<const std::uint8_t> bytes = input;
    bytes_t companion;
    std::string container;
    std::uint32_t code_offset = 0;
    const std::string_view text(reinterpret_cast<const char *>(input.data()), input.size());
    if (platform == platform_t::xbox && text.starts_with("scrobjs")) {
        if (path == "-") throw error_t("Xbox text metadata requires a named input file to locate its .xbsxl companion");
        auto code_path = std::filesystem::path(path);
        code_path.replace_extension(".xbsxl");
        companion = read_file(code_path.string());
        bytes = companion;
        container = "Xbox text metadata + .xbsxl bytecode";

        std::istringstream metadata{std::string(text)};
        std::string token;
        while (metadata >> token) {
            if (token != "codesiz") continue;
            std::uint64_t size = 0, offset = 0;
            if (!(metadata >> size >> token >> offset) || token != "code" ||
                offset > bytes.size() || size > bytes.size() - offset)
                throw error_t("Xbox metadata code range exceeds its .xbsxl companion");
        }
    } else if (platform == platform_t::ps2 ||
               (input.size() >= 16 && read_word(input, 12, 2) == UINT16_MAX)) {
        if (input.size() < 0x70) throw error_t("Truncated console script header (requires 0x70 bytes)");
        generic_mash_header header{};
        std::memcpy(&header, input.data(), sizeof(header));
        if (header.safety_key != static_cast<std::uint32_t>(header.generate_safety_key()) ||
            header.class_id != UINT16_MAX || header.field_8 < 0x70 ||
            static_cast<std::size_t>(header.field_8) > input.size())
            throw error_t("Invalid unlinked console script mash header");
        code_offset = 0x70;
        const auto size = read_word(input, 0x34);
        if (size > input.size() - code_offset) throw error_t("Console bytecode exceeds the input file");
        bytes = input.subspan(code_offset, size);
        container = "Console mash (0x70-byte header; metadata not imported)";
    } else {
        container = "Raw Xbox bytecode";
    }

    std::vector<instruction_t> code;
    try {
        code = decode_code(bytes);
    } catch (const error_t &error) {
        throw error_t(std::string(platform_name(platform)) +
            " bytecode is outside the currently supported instruction subset: " + error.what());
    }

    const auto encoded = encode_code(code);
    if (encoded.size() != bytes.size() || !std::equal(encoded.begin(), encoded.end(), bytes.begin()))
        throw error_t("Console bytecode contains unsupported encoding bits");

    if (output) {
        std::ostringstream listing;
        listing << "// target: " << (platform == platform_t::xbox ? "xb" : "ps2") << '\n'
                << "// type: " << (code_offset ? "mash" : "raw") << '\n';
        for (const auto &instruction : code) {
            listing << hex(instruction.offset) << ": " << opcode_name(instruction.opcode);
            if (instruction.argument != OP_ARG_NULL) {
                listing << ' ' << argument_name(instruction.argument) << ' ';
                if (instruction.argument == OP_ARG_NUM || instruction.argument == OP_ARG_NUMR)
                    listing << "bits(" << hex(instruction.value) << ')';
                else listing << hex(instruction.value);
            }
            if (instruction.size_override) listing << " @size(" << *instruction.size_override << ')';
            listing << ";\n";
        }
        const auto result = listing.str();
        if (output->empty()) {
            std::cout.write(result.data(), static_cast<std::streamsize>(result.size()));
            std::cout.flush();
            if (!std::cout) throw error_t("Cannot write standard output");
        } else write_file(*output, {reinterpret_cast<const std::uint8_t *>(result.data()), result.size()});
    }
    if (status) *status << "Platform: " << platform_name(platform) << "\nContainer: " << container
           << "\nCode offset: " << hex(code_offset) << "\nCode bytes: " << bytes.size()
           << "\nInstructions: " << code.size() << '\n';
}

}
