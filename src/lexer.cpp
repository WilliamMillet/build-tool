#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "file_utils.hpp"

Lexer::Lexer(const std::string input) { src = FileUtils::read(input); }

std::vector<Lexeme> Lexer::lex() {
    line_no = 0;
    char_no = 0;

    std::vector<Lexeme> lexemes;
    while (!at_end()) {
        auto direct_itm = DIRECT_MAPPINGS.find(peek());
        if (direct_itm != DIRECT_MAPPINGS.end()) {
            std::string lex_val{consume()};
            lexemes.push_back({line_no, direct_itm->second, lex_val});
            continue;
        }

        if (std::isspace(peek())) {
            consume();
            continue;
        }

        if (valid_identifier_char(peek())) {
            lex_identifier(lexemes);
            continue;
        }

        switch (peek()) {
            case NEWLINE: {
                lexemes.push_back(Lexeme{line_no, LexemeType::NEWLINE});
                break;
            }
            case COMMENT: {
                consume_line();
                break;
            }
            case SCOPE_RESOLVER: {
                consume(SCOPE_RESOLVER);
                consume(SCOPE_RESOLVER);
                lexemes.push_back(Lexeme{line_no, LexemeType::SCOPE_RESOLVER});
                break;
            }
            case STRING_QUOTE: {
                lex_string(lexemes);
                break;
            }
            case RULE_NAME_START: {
                lex_rule(lexemes);
                break;
            }
            default: {
                std::string unexpected = {peek()};
                throw_pinpointed_err("Unexpected char '" + unexpected + "'");
                break;
            }
        }
    }

    lexemes.push_back(Lexeme{line_no, LexemeType::END_OF_FILE});

    return lexemes;
}

bool Lexer::valid_identifier_char(char c) { return std::isalnum(c) || c == '_'; };

char Lexer::peek() const { return src.at(char_no); }

char Lexer::consume() { return src.at(char_no++); }

char Lexer::consume(char exp) {
    std::string exp_str{exp};
    std::string actl_str;
    if (at_end()) {
        actl_str += "EOF";
    } else {
        actl_str += peek();
    }
    throw_pinpointed_err("Expected character '" + exp_str + "' but got '" + actl_str + "'");
}

bool Lexer::at_end() const { return char_no == src.size(); }

void Lexer::consume_line() {
    while (peek() != NEWLINE) {
        consume();
    }
}

void Lexer::throw_pinpointed_err(std::string msg) const {
    if (at_end()) {
        throw std::invalid_argument("Unexpected end of file: " + msg);
    } else {
        std::string lno_str = std::to_string(line_no);
        std::string cno_str = std::to_string(char_no);
        throw std::invalid_argument("Error on line " + lno_str + " at char" + cno_str + ": " + msg);
    }
}

void Lexer::lex_string(std::vector<Lexeme>& lexemes) {
    size_t opener_idx = line_no;
    consume(STRING_QUOTE);
    std::string str_val;
    while (!at_end() && peek() != STRING_QUOTE) {
        str_val += consume();
    }
    if (at_end()) {
        throw_pinpointed_err("Unclosed string at" + std::to_string(opener_idx));
    }
    consume(STRING_QUOTE);
    lexemes.push_back(Lexeme{line_no, LexemeType::STRING, str_val});
}

void Lexer::lex_rule(std::vector<Lexeme>& lexemes) {
    size_t opener_idx = line_no;
    consume(RULE_NAME_START);
    std::string id;
    while (!at_end() && peek() != STRING_QUOTE) {
        if (!Lexer::valid_identifier_char(peek())) {
            std::string bad_char = {peek()};
            throw_pinpointed_err("Unexpected character '" + bad_char + "' found in rule");
        }
        id += consume();
    }
    if (at_end()) {
        throw_pinpointed_err("Unclosed string at" + std::to_string(opener_idx));
    }
    consume(RULE_NAME_END);
    lexemes.push_back(Lexeme{line_no, LexemeType::RULE_IDENTIFIER, id});
}

void Lexer::lex_identifier(std::vector<Lexeme>& lexemes) {
    std::string id;
    while (valid_identifier_char(peek())) {
        id += consume();
    }
    lexemes.push_back(Lexeme{line_no, LexemeType::IDENTIFIER, id});
    continue;
}