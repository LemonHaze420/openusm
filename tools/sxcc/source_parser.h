#pragma once

#include "sxcc.h"
#include "script_executable.h"
#include "parse_generic_mash.h"

#include <charconv>

namespace sxcc::detail {

struct token_t {
    std::string text;
    unsigned line = 1;
    bool quoted = false;
};

class lexer_t {
    std::string_view source;
    std::size_t offset = 0;
    unsigned line = 1;

public:
    explicit lexer_t(std::string_view text) : source(text) {}

    token_t next()
    {
        while (offset < source.size()) {
            char c = source[offset];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (c == '\n') ++line;
                ++offset;
            } else if (source.substr(offset, 2) == "//") {
                while (offset < source.size() && source[offset] != '\n') ++offset;
            } else if (source.substr(offset, 2) == "/*") {
                offset += 2;
                while (offset < source.size() && source.substr(offset, 2) != "*/") {
                    if (source[offset++] == '\n') ++line;
                }
                if (offset == source.size()) throw error_t("Unterminated comment");
                offset += 2;
            } else {
                break;
            }
        }
        token_t token{{}, line, false};
        if (offset == source.size()) return token;
        char c = source[offset++];
        if (c == '"') {
            token.quoted = true;
            while (offset < source.size()) {
                c = source[offset++];
                if (c == '"') return token;
                if (c == '\n' || c == '\r') throw error_t("Unescaped newline in string");
                if (c == '\\') {
                    if (offset == source.size()) throw error_t("Unterminated string escape");
                    c = source[offset++];
                    switch (c) {
                    case 'n': c = '\n'; break;
                    case 'r': c = '\r'; break;
                    case 't': c = '\t'; break;
                    case '0': c = '\0'; break;
                    case '\\': case '"': break;
                    case 'x': {
                        if (source.size() - offset < 2) throw error_t("Truncated byte escape");
                        c = static_cast<char>(decode_bytes(source.substr(offset, 2)).front());
                        offset += 2;
                        break;
                    }
                    default: throw error_t("Unknown string escape");
                    }
                }
                token.text += c;
            }
            throw error_t("Unterminated string literal");
        }
        auto identifier = [](char value) {
            return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
                   (value >= '0' && value <= '9') || value == '_';
        };
        const bool numeric = (c >= '0' && c <= '9') ||
                             (c == '.' && offset < source.size() && source[offset] >= '0' && source[offset] <= '9');
        token.text += c;
        if (identifier(c) || numeric) {
            while (offset < source.size()) {
                char next = source[offset];
                bool exponent_sign = numeric && !token.text.starts_with("0x") && !token.text.starts_with("0X") &&
                                     (next == '-' || next == '+') &&
                                     (token.text.back() == 'e' || token.text.back() == 'E');
                if (!identifier(next) && !(numeric && next == '.') && !exponent_sign) break;
                token.text += source[offset++];
            }
        } else if (offset < source.size()) {
            auto pair = source.substr(offset - 1, 2);
            if (c == '.' && source.substr(offset, 2) == "..") {
                token.text = "...";
                offset += 2;
                return token;
            }
            if (pair == "==" || pair == "!=" || pair == "<=" || pair == ">=" || pair == "&&" || pair == "||" ||
                pair == "<<" || pair == ">>" || pair == "++" || pair == "--" || pair == "+=" || pair == "-=" ||
                pair == "*=" || pair == "/=" || pair == "%=" || pair == "&=" || pair == "|=" || pair == "^=" ||
                pair == "::" || pair == "->") {
                token.text += source[offset++];
                if ((pair == "<<" || pair == ">>") && offset < source.size() && source[offset] == '=')
                    token.text += source[offset++];
            }
        }
        return token;
    }
};
class parser_base_t {
protected:
    lexer_t lexer;
    token_t token;
    image_t image;
    [[noreturn]] void fail(const std::string &message) const
    {
        throw error_t("Line " + std::to_string(token.line) + ": " + message);
    }

    void advance() { token = lexer.next(); }

    bool accept(std::string_view text)
    {
        if (!token.quoted && token.text == text) {
            advance();
            return true;
        }
        return false;
    }

    void expect(std::string_view text)
    {
        if (!accept(text)) fail("Expected '" + std::string(text) + "', got '" + token.text + "'");
    }

    std::string string()
    {
        if (!token.quoted) fail("Expected a string literal");
        auto value = std::move(token.text);
        advance();
        return value;
    }

    std::uint32_t integer()
    {
        bool negative = accept("-");
        if (token.quoted || token.text.empty()) fail("Expected integer");
        std::uint64_t value = 0;
        auto text = std::string_view(token.text);
        int base = 10;
        if (text.starts_with("0x")) { text.remove_prefix(2); base = 16; }
        auto parsed = std::from_chars(text.data(), text.data() + text.size(), value, base);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value > UINT32_MAX ||
            (negative && value > 0x80000000ULL))
            fail("Integer exceeds its 32-bit representation");
        advance();
        auto word = static_cast<std::uint32_t>(value);
        return negative ? 0u - word : word;
    }

    unsigned small_integer(unsigned maximum)
    {
        auto value = integer();
        if (value > maximum) fail("Integer exceeds field range");
        return value;
    }

    bytes_t bytes()
    {
        return decode_bytes(string());
    }

    void extra(bytes_t &record, std::size_t size)
    {
        if (record.empty()) record.resize(size);
        expect("{");
        std::set<unsigned> seen;
        while (!accept("}")) {
            auto offset = integer();
            if (!seen.insert(offset).second) fail("Duplicate preserved byte offset");
            expect(":");
            auto value = small_integer(UINT8_MAX);
            if (offset >= record.size()) fail("Preserved byte exceeds engine record");
            record[offset] = static_cast<std::uint8_t>(value);
            expect(";");
        }
    }

    void image_metadata()
    {
        expect("{");
        while (!accept("}")) {
            if (accept("mash")) extra(image.mash_preserved, sizeof(generic_mash_header));
            else if (accept("record")) extra(image.preserved, sizeof(script_executable));
            else if (accept("object_slots")) { image.object_slots = bytes(); expect(";"); }
            else if (accept("string_slots")) { image.string_slots = bytes(); expect(";"); }
            else if (accept("padding")) {
                expect("{");
                while (!accept("}")) {
                    image.padding.push_back(bytes());
                    expect(";");
                }
            } else fail("Unknown image metadata field");
        }
    }

public:
    explicit parser_base_t(std::string_view source) : lexer(source) { advance(); }
};

}
