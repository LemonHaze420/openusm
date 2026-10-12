#include "language.h"
#include "source_parser.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace sxcc::language {
namespace {

using expression_ptr = std::unique_ptr<expression_t>;
using statement_ptr = std::unique_ptr<statement_t>;
using expression_kind = expression_t::kind_t;
using statement_kind = statement_t::kind_t;

class parser_t : private detail::parser_base_t {
    program_t program;
    unsigned next_order = 0;

    detail::token_t peek() const
    {
        auto lookahead = lexer;
        return lookahead.next();
    }

    bool identifier_token(const detail::token_t &value) const
    {
        if (value.quoted || value.text.empty())
            return false;
        auto first = value.text.front();
        return (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') || first == '_';
    }

    std::string identifier()
    {
        if (!identifier_token(token))
            fail("Expected an identifier");
        auto result = std::move(token.text);
        advance();
        return result;
    }

    std::string member_name()
    {
        if (accept("~"))
            return '~' + identifier();
        auto name = identifier();
        if (name == "operator") {
            if (token.text != "==" && token.text != "!=" && token.text != "++" && token.text != "--" &&
                token.text != "+" && token.text != "-" && token.text != "*" && token.text != "/" && token.text != "=")
                fail("Unsupported native operator name");
            name += token.text;
            advance();
        }
        return name;
    }

    void close_template()
    {
        if (token.text == ">>")
            token.text = ">";
        else
            expect(">");
    }

    std::string type_name()
    {
        return identifier();
    }

    expression_ptr node(expression_kind kind, std::string text = {}) const
    {
        auto result = std::make_unique<expression_t>();
        result->kind = kind;
        result->line = token.line;
        result->text = std::move(text);
        return result;
    }

    static int precedence(std::string_view operation)
    {
        if (operation == "=" || operation == "+=" || operation == "-=" || operation == "*=" || operation == "/=" ||
            operation == "%=" || operation == "&=" || operation == "|=" || operation == "^=" || operation == "<<=" ||
            operation == ">>=")
            return 1;
        if (operation == "?")
            return 2;
        if (operation == "||")
            return 3;
        if (operation == "&&")
            return 4;
        if (operation == "|")
            return 5;
        if (operation == "^")
            return 6;
        if (operation == "&")
            return 7;
        if (operation == "==" || operation == "!=")
            return 8;
        if (operation == "<" || operation == ">" || operation == "<=" || operation == ">=")
            return 9;
        if (operation == "<<" || operation == ">>")
            return 10;
        if (operation == "+" || operation == "-")
            return 11;
        if (operation == "*" || operation == "/" || operation == "%")
            return 12;
        return 0;
    }

    expression_ptr prefix()
    {
        expression_ptr result;
        if (accept("{")) {
            result = node(expression_kind::aggregate);
            if (!accept("}")) {
                do
                    result->operands.push_back(expression());
                while (accept(",") && token.text != "}");
                expect("}");
            }
        } else if (token.quoted) {
            result = node(expression_kind::string, string());

        } else if (token.text == "static_cast" && peek().text == "<") {
            advance();
            expect("<");
            std::string signature = type_name() + " (";
            expect("(");
            if (accept("*"))
                signature += "*";
            else {
                signature += identifier();
                expect("::");
                expect("*");
                signature += "::*";
            }
            expect(")");
            expect("(");
            signature += ")(";
            if (!accept(")")) {
                bool comma = false;
                do {
                    if (comma)
                        signature += ",";
                    signature += type_name();
                    comma = true;
                } while (accept(","));
                expect(")");
            }
            signature += ")";
            close_template();
            expect("(");
            expect("&");
            auto scope = node(expression_kind::name, identifier());
            expect("::");
            auto reference = node(expression_kind::member, member_name());
            reference->bits = 1;
            reference->operands.push_back(std::move(scope));
            expect(")");
            result = node(expression_kind::bit_cast, std::move(signature));
            result->operands.push_back(std::move(reference));
        } else if (token.text == "bit_cast" && peek().text == "<") {
            advance();
            expect("<");
            result = node(expression_kind::bit_cast, type_name());
            close_template();
            expect("(");
            result->operands.push_back(expression());
            expect(")");
        } else if (accept("(")) {
            result = expression();
            expect(")");
        } else if (token.text == "+" || token.text == "-" || token.text == "!" || token.text == "~" ||
                   token.text == "++" || token.text == "--" || token.text == "*") {
            auto operation = token.text;
            auto line = token.line;
            advance();
            result = node(expression_kind::unary, std::move(operation));
            result->line = line;
            result->operands.push_back(prefix());
        } else if (!token.text.empty() && !identifier_token(token)) {
            result = node(expression_kind::number, token.text);
            char *end = nullptr;
            errno = 0;
            float number = std::strtof(token.text.c_str(), &end);
            if (end != token.text.data() + token.text.size() || !std::isfinite(number) ||
                (errno == ERANGE && std::fpclassify(number) == FP_ZERO))
                fail("Invalid num literal");
            std::memcpy(&result->bits, &number, sizeof(number));
            advance();
        } else {
            result = node(expression_kind::name, identifier());
        }
        for (;;) {
            if (accept("(")) {
                auto call = node(expression_kind::call);
                call->operands.push_back(std::move(result));
                if (!accept(")")) {
                    do {
                        call->operands.push_back(expression());
                    } while (accept(","));
                    expect(")");
                }
                result = std::move(call);
            } else if (accept("{")) {
                if (result->kind != expression_kind::name)
                    fail("An aggregate initializer requires a type name");
                auto aggregate = node(expression_kind::aggregate, std::move(result->text));
                if (!accept("}")) {
                    for (;;) {
                        aggregate->operands.push_back(expression());
                        if (accept("}"))
                            break;
                        expect(",");
                        if (accept("}"))
                            break;
                    }
                }
                result = std::move(aggregate);
            } else if (token.text == "." || token.text == "::") {
                const bool qualified = token.text == "::";
                advance();
                auto member = node(expression_kind::member, member_name());
                member->bits = qualified;
                member->operands.push_back(std::move(result));
                result = std::move(member);
            } else if (accept("[")) {
                auto index = node(expression_kind::index);
                index->operands.push_back(std::move(result));
                index->operands.push_back(expression());
                expect("]");
                result = std::move(index);
            } else if (token.text == "++" || token.text == "--") {
                auto postfix = node(expression_kind::postfix, token.text);
                advance();
                postfix->operands.push_back(std::move(result));
                result = std::move(postfix);
            } else
                break;
        }
        return result;
    }

    expression_ptr expression(int minimum = 1)
    {
        auto left = prefix();
        for (;;) {
            auto priority = precedence(token.text);
            if (priority < minimum)
                return left;
            auto operation = token.text;
            advance();
            if (operation == "?") {
                auto conditional = node(expression_kind::conditional);
                conditional->operands.push_back(std::move(left));
                conditional->operands.push_back(expression());
                expect(":");
                conditional->operands.push_back(expression());
                left = std::move(conditional);
            } else {
                auto binary = node(priority == 1 ? expression_kind::assignment : expression_kind::binary, operation);
                binary->operands.push_back(std::move(left));
                binary->operands.push_back(expression(priority == 1 ? priority : priority + 1));
                left = std::move(binary);
            }
        }
    }

    declaration_t declaration(bool parameter = false)
    {
        declaration_t result;
        result.type = type_name();
        if (parameter && (token.text == "," || token.text == ")"))
            return result;
        result.name = identifier();
        if (accept("[")) {
            result.count = integer();
            result.is_array = true;
            if (!result.count)
                fail("Array length must be positive");
            expect("]");
        }
        return result;
    }

    statement_ptr statement()
    {
        auto result = std::make_unique<statement_t>();
        result->line = token.line;
        if (accept("{")) {
            while (!accept("}")) {
                if (token.text.empty())
                    fail("Unterminated statement block");
                result->children.push_back(statement());
            }
        } else if (accept("if")) {
            result->kind = statement_kind::conditional;
            expect("(");
            result->expressions.push_back(expression());
            expect(")");
            result->children.push_back(statement());
            if (accept("else"))
                result->children.push_back(statement());
        } else if (accept("switch")) {
            result->kind = statement_kind::switch_statement;
            expect("(");
            result->expressions.push_back(expression());
            expect(")");
            expect("{");
            while (!accept("}")) {
                auto arm = std::make_unique<statement_t>();
                arm->kind = statement_kind::switch_case;
                arm->line = token.line;
                if (accept("case")) {
                    arm->expressions.push_back(expression());
                    expect(":");
                } else {
                    expect("default");
                    expect(":");
                }
                while (token.text != "case" && token.text != "default" && token.text != "}")
                    arm->children.push_back(statement());
                result->children.push_back(std::move(arm));
            }
        } else if (accept("while")) {
            result->kind = statement_kind::while_loop;
            expect("(");
            result->expressions.push_back(expression());
            expect(")");
            result->children.push_back(statement());
        } else if (accept("do")) {
            result->kind = statement_kind::do_loop;
            result->children.push_back(statement());
            expect("while");
            expect("(");
            result->expressions.push_back(expression());
            expect(")");
            expect(";");
        } else if (accept("for")) {
            result->kind = statement_kind::for_loop;
            expect("(");
            result->children.push_back(statement());
            if (!accept(";")) {
                result->expressions.push_back(expression());
                expect(";");
            } else
                result->expressions.push_back(nullptr);
            if (!accept(")")) {
                result->expressions.push_back(expression());
                expect(")");
            } else
                result->expressions.push_back(nullptr);
            result->children.push_back(statement());
        } else if (accept("return")) {
            result->kind = statement_kind::return_value;
            if (!accept(";")) {
                result->expressions.push_back(expression());
                expect(";");
            }
        } else if (accept("break")) {
            result->kind = statement_kind::break_loop;
            expect(";");
        } else if (accept("continue")) {
            result->kind = statement_kind::continue_loop;
            expect(";");
        } else if (accept("goto")) {
            result->kind = statement_kind::jump;
            result->name = identifier();
            expect(";");
        } else if (identifier_token(token) && peek().text == ":") {
            result->kind = statement_kind::label;
            result->name = identifier();
            expect(":");
        } else if (identifier_token(token) && identifier_token(peek())) {
            result->kind = statement_kind::declaration;
            result->declaration = declaration();
            if (accept("="))
                result->expressions.push_back(expression());
            expect(";");
        } else if (!accept(";")) {
            result->kind = statement_kind::expression;
            result->expressions.push_back(expression());
            expect(";");
        }
        return result;
    }

    void parameter_list(language::function_t &function, sxcc::function_t &binary)
    {
        binary.name = function.name + '(';
        expect("(");
        if (!accept(")")) {
            do {
                auto parameter = declaration(true);
                if (parameter.name.empty())
                    parameter.name = "parameter_" + std::to_string(function.parameters.size());
                if (!function.parameters.empty())
                    binary.name += ',';
                binary.name += parameter.type;
                function.parameters.push_back(std::move(parameter));
            } while (accept(","));
            expect(")");
        }
        binary.name += ')';
        binary.name_hash = name_hash(function.name);
        binary.fullname_hash = name_hash(binary.name);
    }


    void function(class_t &owner, std::string name, std::string result_type, bool is_static, bool constructor,
                  bool destructor, access_t access)
    {
        language::function_t function;
        function.index = static_cast<unsigned>(image.objects[owner.index].functions.size());
        function.name = std::move(name);
        function.result = std::move(result_type);
        function.constructor = constructor;
        function.access = access;
        function.destructor = destructor;
        sxcc::function_t binary;
        binary.flags = 4u | (is_static ? 1u : 0u);
        binary.code_order = UINT32_MAX;
        parameter_list(function, binary);
        if (accept("->")) {
            if (!constructor)
                fail("Only constructors need an explicit trailing VM result type");
            function.result = type_name();
        }
        if (accept(":")) {
            const auto &parent = image.objects[owner.index].parent;
            if (!constructor || !parent)
                fail("A base initializer requires a derived constructor");
            if (identifier() != *parent)
                fail("Initializer does not name the direct base class");
            expect("(");
            if (!accept(")")) {
                do {
                    function.base_arguments.push_back(expression());
                } while (accept(","));
                expect(")");
            }
        }
        if (!accept(";")) {
            if (token.text != "{")
                fail("A script function requires a body");
            function.body = statement();
            binary.code_order = next_order++;
        }
        auto &object = image.objects[owner.index];
        if (destructor) {
            if (object.destructor != -1)
                fail("Duplicate destructor");
            object.destructor = static_cast<std::int32_t>(function.index);
        }
        object.functions.push_back(std::move(binary));
        owner.functions.push_back(std::move(function));
    }


    void object()
    {
        class_t owner;
        owner.index = static_cast<unsigned>(image.objects.size());
        sxcc::object_t object;
        object.name = identifier();
        object.name_hash = name_hash(object.name);
        if (owner.index == 0)
            object.flags |= 1u;
        if (accept(":")) {
            accept("public");
            object.parent = identifier();
            object.parent_evidence = "source declaration";
        }
        const auto class_name = object.name;
        const bool global = (object.flags & 1) != 0;
        auto access = access_t::private_member;
        image.objects.push_back(std::move(object));
        expect("{");
        while (!accept("}")) {
            if (token.text.empty())
                fail("Unterminated class declaration");
            if (token.text == "public" || token.text == "private" || token.text == "protected") {
                access = token.text == "public"      ? access_t::public_member
                         : token.text == "protected" ? access_t::protected_member
                                                     : access_t::private_member;
                advance();
                expect(":");
                continue;
            }
            const bool is_static = accept("static") || global;
            if (accept("~")) {
                auto name = identifier();
                if (name != class_name)
                    fail("Destructor name does not match its class");
                function(owner, '~' + name, "void", is_static, false, true, access);
                continue;
            }
            if (token.text == class_name && peek().text == "(") {
                auto name = identifier();
                function(owner, std::move(name), "void", is_static, true, false, access);
                continue;
            }
            auto field = declaration();
            field.is_static = is_static;
            field.access = access;
            if (token.text == "(") {
                function(owner, std::move(field.name), std::move(field.type), is_static, false, false, access);
            } else {
                if (accept("=")) {
                    if (accept("{")) {
                        auto value = std::make_unique<expression_t>();
                        value->kind = expression_kind::aggregate;
                        value->text = field.type;
                        if (!accept("}")) {
                            do
                                value->operands.push_back(expression());
                            while (accept(",") && token.text != "}");
                            expect("}");
                        }
                        owner.field_initializers.emplace_back(field.name, std::move(value));
                    } else
                        owner.field_initializers.emplace_back(field.name, expression());
                }
                expect(";");
                owner.fields.push_back(std::move(field));
            }
        }
        expect(";");
        program.classes.push_back(std::move(owner));
    }


    void definition()
    {
        auto first = identifier();
        std::string class_name, result_type = "void";
        if (token.text == "::")
            class_name = std::move(first);
        else {
            result_type = std::move(first);
            class_name = identifier();
        }
        expect("::");
        std::string name;
        if (accept("~"))
            name = "~" + identifier();
        else
            name = member_name();
        auto object = std::find_if(
            image.objects.begin(), image.objects.end(), [&](const auto &o) { return o.name == class_name; });
        if (object == image.objects.end())
            fail("Definition names an undeclared class");
        auto &owner = program.classes.at(static_cast<unsigned>(object - image.objects.begin()));

        if (token.text == "=") {
            auto field =
                std::find_if(owner.fields.begin(), owner.fields.end(), [&](const auto &f) { return f.name == name; });
            if (field == owner.fields.end() || !field->is_static || field->type != result_type || field->is_array)
                fail("Static definition has no matching scalar member declaration");
            for (const auto &[existing, value] : owner.field_initializers)
                if (existing == name)
                    fail("Duplicate static definition");
            advance();
            auto value = expression();
            expect(";");
            owner.field_initializers.emplace_back(name, std::move(value));
            return;
        }
        language::function_t parsed;
        parsed.name = name;
        parsed.result = result_type;
        sxcc::function_t signature;
        parameter_list(parsed, signature);
        if (accept("->"))
            parsed.result = type_name();
        auto found = std::find_if(owner.functions.begin(), owner.functions.end(), [&](const auto &f) {
            return object->functions[f.index].fullname_hash == signature.fullname_hash;
        });
        if (found == owner.functions.end())
            fail("Definition has no matching member declaration");
        if (found->body)
            fail("Duplicate function definition");
        if (found->result != parsed.result)
            fail("Definition return type differs from declaration");
        if (accept(":")) {
            if (!found->constructor || !object->parent || identifier() != *object->parent)
                fail("Invalid base initializer");
            expect("(");
            if (!accept(")")) {
                do {
                    found->base_arguments.push_back(expression());
                } while (accept(","));
                expect(")");
            }
        }
        if (token.text != "{")
            fail("Function definition requires a body");
        found->parameters = std::move(parsed.parameters);
        found->body = statement();
        object->functions[found->index].code_order = next_order++;
    }

    void native()
    {
        native_t declaration;
        declaration.result = type_name();
        auto name = member_name();
        if (accept("::")) {
            declaration.class_name = std::move(name);
            name = member_name();
        } else
            declaration.class_name = "_global_slc";
        if (token.text == ";" || token.text == "[") {
            declaration_t variable;
            variable.name = name;
            variable.type = *declaration.result;
            if (accept("[")) {
                variable.count = token.text == "]" ? 0 : integer();
                variable.is_array = true;
                expect("]");
            }
            expect(";");
            if (!program.native_variables.emplace(name, std::move(variable)).second)
                fail("Duplicate native variable");
            return;
        }
        declaration.signature = name + '(';
        expect("(");
        if (!accept(")")) {
            do {
                if (accept("...")) {
                    if (!declaration.parameters.empty())
                        declaration.signature += ',';
                    declaration.signature += "...";
                    declaration.variadic = true;
                    break;
                }
                auto parameter = this->declaration(true);
                if (!declaration.parameters.empty())
                    declaration.signature += ',';
                declaration.signature += parameter.type;
                declaration.parameters.push_back(std::move(parameter.type));
            } while (accept(","));
            expect(")");
        }
        declaration.signature += ')';
        expect(";");
        program.natives.push_back(std::move(declaration));
    }


public:
    explicit parser_t(std::string_view source) : parser_base_t(source) {}

    program_t parse()
    {
        expect("namespace");
        image.name = identifier();
        expect("{");
        while (!accept("}")) {
            if (accept("class"))
                object();
            else if (accept("extern"))
                native();
            else if (accept("static")) {
                expect("const");
                expect("char");
                auto name = identifier();
                expect("[");
                expect("]");
                expect("=");
                auto text = string();
                expect(";");
                if (!program.string_objects.emplace(std::move(name), std::move(text)).second)
                    fail("Duplicate constant string object");
            } else
                definition();
        }
        if (!token.text.empty())
            fail("Unexpected text after script");
        if (program.classes.empty())
            fail("A script requires an application class");
        for (const auto &owner : program.classes)
            for (const auto &function : owner.functions)
                if (!function.body)
                    fail("Declared function has no definition");
        program.image = std::move(image);
        return std::move(program);
    }
};

}

program_t parse(std::string_view source)
{
    return parser_t(source).parse();
}

}
