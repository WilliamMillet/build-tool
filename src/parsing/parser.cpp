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
        const Lexeme lex = peek();
        if (match_type({LexemeType::IDENTIFIER})) {
            const Lexeme id_lex = consume(LexemeType::IDENTIFIER);
            consume(LexemeType::EQUALS);
            var_lexes.push_back(
                {std::move(id_lex.value), consume_var_lexemes(), VarCategory::REGULAR, id_lex.loc});
        } else if (match_type({LexemeType::DICT_QUALIFIER})) {
            const Lexeme rule_lex = consume(LexemeType::DICT_QUALIFIER);
            const VarCategory cat = categorise_dictionary(rule_lex.value);
            const Lexeme id = consume(LexemeType::IDENTIFIER);
            var_lexes.push_back({std::move(id.value), consume_dict_lexemes(), cat, id.loc});
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
    if (match_type({LexemeType::BLOCK_START})) {
        return consume_dict_lexemes();
    }
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
    std::vector<Location> open_paren_locs = {block_start.loc};

    while (!open_paren_locs.empty() && !at_end()) {
        if (match_type({LexemeType::BLOCK_START})) {
            open_paren_locs.push_back(consume(LexemeType::BLOCK_START).loc);
        } else if (match_type({LexemeType::BLOCK_END})) {
            if (open_paren_locs.empty()) {
                throw SyntaxError("Closing parenthesis without opening parenthesis", peek().loc);
            } else {
                open_paren_locs.pop_back();
            }
        }

        assigned_lexemes.push_back(consume());
    }

    if (at_end() && !open_paren_locs.empty()) {
        throw SyntaxError("Unclosed parenthesis", open_paren_locs.back());
    }

    return assigned_lexemes;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Consuming dictionary lexemes", get_loc());
}

std::unique_ptr<Expr> Parser::parse_expr() try {
    std::unique_ptr<Expr> operand1 = parse_term();
    if (match_type(INFIX_OPERATORS)) {
        const Lexeme op = consume();
        switch (op.type) {
            case LexemeType::ADD: {
                return std::make_unique<BinaryOpExpr>(BinaryOpType::ADD, std::move(operand1),
                                                      parse_expr());
            }
            default: {
                throw SyntaxError("Unexpected operand" + peek().value);
            }
        }
    } else {
        return operand1;
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
        case LexemeType::LIST_START: {
            return parse_list();
        }
        case LexemeType::IDENTIFIER: {
            const std::string identifier = consume(LexemeType::IDENTIFIER).value;
            if (at_end()) {
                return std::make_unique<VarRefExpr>(identifier);
            }

            switch (peek().type) {
                case LexemeType::FN_START: {
                    return parse_fn(std::move(identifier));
                }
                case LexemeType::SCOPE_RESOLVER: {
                    consume(LexemeType::SCOPE_RESOLVER);
                    const std::string enum_name = consume(LexemeType::IDENTIFIER).value;
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

std::unique_ptr<FnExpr> Parser::parse_fn(std::string fn_name) try {
    std::unique_ptr<FnExpr> fn_expr = std::make_unique<FnExpr>(fn_name);
    const Lexeme opening_paren = consume(LexemeType::FN_START);

    while (!at_end() && !match_type({LexemeType::FN_END})) {
        fn_expr->add_arg(parse_expr());
        consume(LexemeType::DELIMETER);
    }

    if (at_end()) {
        throw SyntaxError("Unclosed bracket for function '" + fn_name + "'.", opening_paren.loc);
    }

    consume(LexemeType::FN_END);

    return fn_expr;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing function", get_loc());
}

std::unique_ptr<ListExpr> Parser::parse_list() try {
    auto list = std::make_unique<ListExpr>();
    const Lexeme opening_paren = consume(LexemeType::LIST_START);

    while (!at_end() && !match_type({LexemeType::LIST_END})) {
        list->append(parse_expr());
        if (match_type({LexemeType::DELIMETER})) {
            consume(LexemeType::DELIMETER);
        }
    }

    if (at_end()) {
        throw SyntaxError("Unterminated list", opening_paren.loc);
    }

    consume(LexemeType::LIST_END);

    return list;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing list", get_loc());
}

std::unique_ptr<DictionaryExpr> Parser::parse_dictionary() try {
    consume(LexemeType::BLOCK_START);
    consume(LexemeType::NEWLINE);
    std::unique_ptr<DictionaryExpr> dict_expr = std::make_unique<DictionaryExpr>();
    while (!at_end() && !match_type({LexemeType::BLOCK_END})) {
        std::string id = consume(LexemeType::IDENTIFIER).value;
        consume(LexemeType::EQUALS);
        dict_expr->insert_entry(id, parse_expr());
        consume(LexemeType::NEWLINE);
    }
    if (at_end()) {
        throw SyntaxError("Failed to parse dictionary");
    }

    consume(LexemeType::BLOCK_END);

    return dict_expr;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Parsing dictionary", get_loc());
}

VarCategory Parser::categorise_dictionary(const std::string& id) {
    if (id == "Rule") {
        return VarCategory::SINGLE_RULE;
    } else if (id == "MultiRule") {
        return VarCategory::MULTI_RULE;
    } else if (id == "Clean") {
        return VarCategory::CLEAN;
    } else if (id == "Config") {
        return VarCategory::CONFIG;
    } else {
        throw SyntaxError("Invalid rule type '" + id + "'");
    }
}
