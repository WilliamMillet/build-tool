#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "../lexer.hpp"
#include "namespace_builder.hpp"

std::vector<ParsedVariable> Parser::parse() {
    std::vector<VarLexemes> var_lex_vec;
    NamespaceBuilder ns;
    while (!at_end()) {
        Lexeme lex = peek();
        if (lex.type == LexemeType::IDENTIFIER) {
            std::string id = consume(LexemeType::IDENTIFIER).value;
            if (match_type({LexemeType::BLOCK_START})) {
                ns.append_identifier(std::move(id));
            } else {
                std::unique_ptr<VarLexemes> unevaluated = parse_assignment(ns.curr_namespace());
                if (unevaluated != nullptr) {
                    var_lex_vec.push_back(std::move(*unevaluated));
                }
            }
        } else if (match_type({LexemeType::BLOCK_END})) {
            if (ns.empty()) {
                throw_pinpointed_err("Closing brace '" + peek().value + "' has no opener");
            }
            ns.pop_identifier();
        } else {
            consume();
        }
    }

    std::vector<ParsedVariable> var_exprs;
    for (VarLexemes v : var_lex_vec) {
        change_lexeme_source(std::move(v.lexemes));
        var_exprs.push_back({v.identifier, parse_expr()});
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

std::unique_ptr<VarLexemes> Parser::parse_assignment(std::string&& assignee_id) {
    expect_type(VALID_IDENTIFIER_SUCCESSORS);

    if (peek().type == LexemeType::EQUALS) {
        consume();
        expect_type(VARIABLE_STARTS);
        while (!at_end() && peek().type != LexemeType::NEWLINE) {
            lexemes.push_back(consume());
        }
        return std::make_unique<VarLexemes>(std::move(assignee_id), std::move(lexemes));
    }

    return nullptr;
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

std::unique_ptr<FnExpr> Parser::parse_fn_args(std::string&& fn_name) {
    std::unique_ptr<FnExpr> fn_expr = std::make_unique<FnExpr>(fn_name);
    consume(LexemeType::MACRO_FN_START);

    while (match_type({LexemeType::DELIMETER})) {
        fn_expr->args.push_back(parse_expr());
    }

    consume(LexemeType::MACRO_FN_END);

    return fn_expr;
}
