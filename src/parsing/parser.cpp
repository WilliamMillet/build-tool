#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "../errors/error.hpp"
#include "../lexer.hpp"

Parser::Parser(std::vector<Lexeme> _lexemes) : lexemes(_lexemes) {};

std::vector<ParsedVariable> Parser::parse() try {
    std::vector<VarLexemes> var_lexes;
    while (!at_end()) {
        Lexeme lex = peek();
        if (match_type({LexemeType::IDENTIFIER})) {
            Lexeme id_lex = consume(LexemeType::IDENTIFIER);
            consume(LexemeType::EQUALS);
            var_lexes.push_back(
                {std::move(id_lex.value), consume_var_lexemes(), VarCategory::REGULAR, id_lex.loc});
        } else if (match_type({LexemeType::SINGLE_RULE_IDENTIFIER})) {
            Lexeme rule_lex = consume(LexemeType::SINGLE_RULE_IDENTIFIER);
            VarCategory cat = categorise_dictionary(rule_lex.value);
            std::string id = consume(LexemeType::IDENTIFIER).value;
            var_lexes.push_back({std::move(id), consume_dict_lexemes(), cat, rule_lex.loc});
        } else {
            consume();
        }
    }

    std::vector<ParsedVariable> var_exprs;
    for (VarLexemes v : var_lexes) {
        change_lexeme_source(std::move(v.lexemes));
        var_exprs.push_back({v.identifier, parse_expr(), v.category, v.start_loc});
    }
    return var_exprs;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing", get_loc());
}

Lexeme Parser::peek() const { return lexemes.at(parse_pos); }

bool Parser::at_end() const { return parse_pos >= lexemes.size(); }

Location Parser::get_loc() const { return at_end() ? Location::eof_loc() : peek().loc; }

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

void Parser::expect_type(std::vector<LexemeType> type_pool) const {
    if (!match_type(type_pool)) {
        throw SyntaxError("Unexpected token '" + peek().value);
    }
}

bool Parser::match_type(std::vector<LexemeType> type_pool) const {
    return !at_end() && std::ranges::contains(type_pool, peek().type);
}

std::vector<Lexeme> Parser::consume_var_lexemes() try {
    expect_type(VARIABLE_STARTS);
    std::vector<Lexeme> assigned_lexemes;
    while (!at_end() && peek().type != LexemeType::NEWLINE) {
        assigned_lexemes.push_back(consume());
    }
    return assigned_lexemes;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Consuming variable lexemes", get_loc());
}

std::vector<Lexeme> Parser::consume_dict_lexemes() try {
    const Lexeme block_start = consume(LexemeType::BLOCK_START);

    std::vector<Lexeme> assigned_lexemes = {block_start};
    std::vector<Location> open_paren_line_nos = {block_start.loc};

    while (!open_paren_line_nos.empty() && !at_end()) {
        if (match_type({LexemeType::BLOCK_START})) {
            open_paren_line_nos.push_back(consume(LexemeType::BLOCK_START).loc);
        } else if (match_type({LexemeType::BLOCK_END})) {
            if (open_paren_line_nos.empty()) {
                throw SyntaxError("Closing parenthesis without opening parenthesis", peek().loc);
            } else {
                open_paren_line_nos.pop_back();
            }
        }

        assigned_lexemes.push_back(consume());
    }

    if (at_end() && !open_paren_line_nos.empty()) {
        throw SyntaxError("Unclosed parenthesis", open_paren_line_nos.back());
    }

    return assigned_lexemes;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Consuming variable lexemes", get_loc());
}

std::unique_ptr<Expr> Parser::parse_expr() try {
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
                throw SyntaxError("Unexpected operand" + peek().value);
            }
        }
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing expression", get_loc());
}

std::unique_ptr<Expr> Parser::parse_term() try {
    switch (peek().type) {
        case LexemeType::STRING: {
            return std::make_unique<StringExpr>(consume(LexemeType::STRING).value);
        }
        case LexemeType::BLOCK_START: {
            return parse_dictionary();
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
            throw SyntaxError("Unexpected token '" + peek().value);
        }
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing term", get_loc());
}

std::unique_ptr<FnExpr> Parser::parse_fn_args(std::string fn_name) try {
    std::unique_ptr<FnExpr> fn_expr = std::make_unique<FnExpr>(fn_name);
    consume(LexemeType::MACRO_FN_START);
    fn_expr->args.push_back(parse_expr());

    while (match_type({LexemeType::DELIMETER})) {
        fn_expr->args.push_back(parse_expr());
    }

    consume(LexemeType::MACRO_FN_END);

    return fn_expr;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing function arguments", get_loc());
}

std::unique_ptr<DictionaryExpr> Parser::parse_dictionary() try {
    std::unique_ptr<DictionaryExpr> cfg_expr = std::make_unique<DictionaryExpr>();
    while (!at_end() && !match_type({LexemeType::BLOCK_END})) {
        std::string id = consume(LexemeType::IDENTIFIER).value;
        consume(LexemeType::EQUALS);
        cfg_expr->fields_map[id] = parse_expr();
        consume(LexemeType::DELIMETER);
    }
    if (at_end()) {
        throw SyntaxError("Failed to parse dictionary");
    }

    consume(LexemeType::BLOCK_END);

    return cfg_expr;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing dictionary", get_loc());
}

VarCategory Parser::categorise_dictionary(std::string& id) {
    if (id == "Rule") {
        return VarCategory::SINGLE_RULE;
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

    throw SyntaxError("Invalid rule type '" + id + "'");
}
