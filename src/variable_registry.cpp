#include "variable_registry.hpp"

#include <algorithm>
#include <ranges>
#include <string>
#include <unordered_map>

VariableRegistry::VariableRegistry(std::vector<Lexeme> lexemes) {
    this->lexemes = lexemes;
    std::vector<UnevaluatedVariable> unevaluated_vec;
    while (!at_end()) {
        Lexeme lex = peek();
        if (lex.type == LexemeType::IDENTIFIER) {
            std::unique_ptr<UnevaluatedVariable> unevaluated = parse_potential_assignment();
            if (unevaluated != nullptr) {
                unevaluated_vec.push_back(std::move(*unevaluated));
            }
        } else {
            consume();
        }
    }
};

Lexeme VariableRegistry::peek() const { return lexemes.at(parse_pos); }

bool VariableRegistry::at_end() const { return parse_pos >= lexemes.size(); }

Lexeme VariableRegistry::consume() {
    parse_pos++;
    return lexemes.at(parse_pos - 1);
}

std::unique_ptr<UnevaluatedVariable> VariableRegistry::parse_potential_assignment() {
    std::string identifier = peek().value;
    consume();

    if (at_end() || !std::ranges::contains(valid_identifier_successors, peek().type)) {
        throw_pinpointed_err("Invalid identifier expression");
    }

    if (peek().type == LexemeType::EQUALS) {
        consume();
        return std::make_unique<UnevaluatedVariable>(consume_expr(identifier));
    }

    return nullptr;
}

UnevaluatedVariable VariableRegistry::consume_expr(std::string assigning_to) {
    if (at_end() || std::ranges::contains(variable_starts, peek().type)) {
        throw_pinpointed_err("Invalid assignment expression");
    }

    std::vector<std::string> deps;
    std::vector<Lexeme> lexemes;

    while (!at_end() && peek().type != LexemeType::NEWLINE) {
        Lexeme lex = consume();
        if (lex.type == LexemeType::IDENTIFIER) {
            deps.push_back(lex.value);
        }
        lexemes.push_back(lex);
    }

    return UnevaluatedVariable{assigning_to, std::move(deps), std::move(lexemes)};
}

// Value VariableRegistry::parse_value() {
//     if (at_end() || std::ranges::contains(variable_starts, peek().type)) {
//         throw_pinpointed_err("Invalid assignment expression");
//     }

//     Lexeme var_start = peek();
//     switch (var_start.type) {
//         case LexemeType::STRING:
//             consume();
//             return Value(var_start.value);
//         case LexemeType::LIST_START:
//             ValueList vl;
//             while (peek().type != LexemeType::LIST_END) {
//                 vl.push_back(std::make_unique<Value>(parse_value()));
//                 if (at_end()) {
//                     throw_pinpointed_err("Invalid list");
//                 }
//             }
//             return Value(vl);
//         case LexemeType::ENUM_TYPE:
//             // ScopedEnumValue sev = {}
//         default:
//             throw_pinpointed_err("Unexpected variable type")
//     }
// }

void VariableRegistry::throw_pinpointed_err(std::string msg) const {
    if (at_end()) {
        throw std::invalid_argument("Unexpected end of file: " + msg);
    } else {
        std::string lno_str = std::to_string(lexemes.at(parse_pos).line_no);
        throw std::invalid_argument("Error on line " + lno_str + ": " + msg);
    }
}