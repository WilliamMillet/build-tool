#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "../lexer.hpp"

std::vector<ParsedVariable> Parser::parse() {
    std::vector<VarLexemes> var_lexes;
    while (!at_end()) {
        Lexeme lex = peek();
        if (match_type({LexemeType::IDENTIFIER})) {
            std::string id = consume(LexemeType::IDENTIFIER).value;
            expect_type({LexemeType::EQUALS});
            var_lexes.push_back({std::move(id), consume_var_lexemes(), VarCategory::REGULAR});
        } else if (match_type({LexemeType::RULE_IDENTIFIER})) {
            std::string rule_str = consume(LexemeType::RULE_IDENTIFIER).value;
            VarCategory cat = categorise_Dictionary(rule_str);
            std::string id = consume(LexemeType::IDENTIFIER).value;
            var_lexes.push_back({std::move(id), consume_Dictionary_lexemes(), cat});
        } else {
            consume();
        }
    }

    std::vector<ParsedVariable> var_exprs;
    for (VarLexemes v : var_lexes) {
        change_lexeme_source(std::move(v.lexemes));
        var_exprs.push_back({v.identifier, parse_expr(), v.category});
    }
    return var_exprs;
}

Lexeme Parser::peek() const { return lexemes.at(parse_pos); }

bool Parser::at_end() const { return parse_pos >= lexemes.size(); }

Lexeme Parser::consume() {
    parse_pos++;
    return lexemes.at(parse_pos - 1);
}

Lexeme Parser::consume(LexemeType lex) {
    expect_type({lex});
    return consume();
}

void Parser::change_lexeme_source(std::vector<Lexeme>&& new_source) {
    this->lexemes = std::move(new_source);
    parse_pos = 0;
}

void Parser::throw_pinpointed_err(std::string msg) const {
    if (at_end()) {
        throw std::invalid_argument("Unexpected end of file: " + msg);
    } else {
        std::string lno_str = std::to_string(lexemes.at(parse_pos).line_no);
        throw std::invalid_argument("Error on line " + lno_str + ": " + msg);
    }
}

void Parser::expect_type(std::vector<LexemeType> type_pool) const {
    if (!match_type(type_pool)) {
        throw_pinpointed_err("Unexpected token '" + peek().value);
    }
}

bool Parser::match_type(std::vector<LexemeType> type_pool) const {
    return !at_end() && std::ranges::contains(type_pool, peek().type);
}

std::vector<Lexeme> Parser::consume_var_lexemes() {
    consume(LexemeType::EQUALS);
    expect_type(VARIABLE_STARTS);
    std::vector<Lexeme> assigned_lexemes;
    while (!at_end() && peek().type != LexemeType::NEWLINE) {
        assigned_lexemes.push_back(consume());
    }
    return assigned_lexemes;
}

std::vector<Lexeme> Parser::consume_Dictionary_lexemes() {
    const Lexeme block_start = consume(LexemeType::BLOCK_START);

    std::vector<Lexeme> assigned_lexemes = {block_start};
    std::vector<size_t> open_paren_line_nos = {block_start.line_no};

    while (!open_paren_line_nos.empty() && !at_end()) {
        if (match_type({LexemeType::BLOCK_START})) {
            open_paren_line_nos.push_back(consume(LexemeType::BLOCK_START).line_no);
        } else if (match_type({LexemeType::BLOCK_END})) {
            if (open_paren_line_nos.empty()) {
                throw_pinpointed_err("Closing parenthesis '" + peek().value +
                                     " ' does not have a corresponding opening parenthesis");
            } else {
                open_paren_line_nos.pop_back();
            }
        }

        assigned_lexemes.push_back(consume());
    }

    if (at_end() && !open_paren_line_nos.empty()) {
        const std::string bad_line = std::to_string(open_paren_line_nos.back());
        throw std::invalid_argument("Unclosed parenthesis on line " + bad_line);
    }

    return assigned_lexemes;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    std::unique_ptr<Expr> operand1 = parse_term();
    if (!match_type(INFIX_OPERATORS)) {
        return operand1;
    } else {
        Lexeme op = consume();
        switch (op.type) {
            case LexemeType::ADD: {
                return std::make_unique<BinaryOpExpr>(BinaryOpType::ADD, std::move(operand1),
                                                      parse_expr());
            }
            default: {
                throw_pinpointed_err("Unexpected token or EOF '" + peek().value);
            }
        }
    }
}

std::unique_ptr<Expr> Parser::parse_term() {
    switch (peek().type) {
        case LexemeType::STRING: {
            return std::make_unique<StringExpr>(consume(LexemeType::STRING).value);
        }
        case LexemeType::BLOCK_START: {
            return parse_config_obj();
        }
        case LexemeType::IDENTIFIER: {
            std::string identifier = consume(LexemeType::IDENTIFIER).value;
            switch (peek().type) {
                case LexemeType::MACRO_FN_START: {
                    return parse_fn_args(std::move(identifier));
                }
                case LexemeType::SCOPE_RESOLVER: {
                    consume(LexemeType::SCOPE_RESOLVER);
                    std::string enum_name = consume(LexemeType::IDENTIFIER).value;
                    return std::make_unique<EnumExpr>(identifier, enum_name);
                }
                default: {
                    return std::make_unique<VarRefExpr>(identifier);
                }
            }
        }
        default: {
            throw_pinpointed_err("Unexpected token '" + peek().value);
        }
    }
}

std::unique_ptr<FnExpr> Parser::parse_fn_args(std::string fn_name) {
    std::unique_ptr<FnExpr> fn_expr = std::make_unique<FnExpr>(fn_name);
    consume(LexemeType::MACRO_FN_START);
    fn_expr->args.push_back(parse_expr());

    while (match_type({LexemeType::DELIMETER})) {
        fn_expr->args.push_back(parse_expr());
    }

    consume(LexemeType::MACRO_FN_END);

    return fn_expr;
}

std::unique_ptr<DictionaryExpr> Parser::parse_config_obj() {
    std::unique_ptr<DictionaryExpr> cfg_expr = std::make_unique<DictionaryExpr>();
    while (!at_end() && !match_type({LexemeType::BLOCK_END})) {
        std::string id = consume(LexemeType::IDENTIFIER).value;
        consume(LexemeType::EQUALS);
        cfg_expr->fields_map[id] = parse_expr();
        consume(LexemeType::DELIMETER);
    }
    if (at_end()) {
        throw_pinpointed_err("Failed to parse Dictionary");
    }

    consume(LexemeType::BLOCK_END);

    return cfg_expr;
}

VarCategory Parser::categorise_Dictionary(std::string& id) {
    if (id == "Rule") {
        return VarCategory::RULE;
    }
    if (id == "MultiRule") {
        return VarCategory::MULTI_RULE;
    }
    if (id == "Clean") {
        return VarCategory::CLEAN;
    }
    if (id == "Config") {
        return VarCategory::CONFIG;
    }

    throw_pinpointed_err("Invalid rule type");
}
