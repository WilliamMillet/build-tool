#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

#include "errors/error.hpp"
#include "file_utils.hpp"

Lexer::Lexer(const std::string input) { src = FileUtils::read_all(input); }

std::vector<Lexeme> Lexer::lex() try {
    std::vector<Lexeme> lexemes;
    line_no = 0, col_no = 0, file_pos = 0;

    while (!at_end()) {
        auto direct_itm = DIRECT_MAPPINGS.find(peek());
        if (direct_itm != DIRECT_MAPPINGS.end()) {
            std::string lex_val{consume()};
            lexemes.push_back(make_lexeme(direct_itm->second, lex_val));
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
                lexemes.push_back(make_lexeme(LexemeType::NEWLINE));
                consume();
                break;
            }
            case COMMENT: {
                consume_line();
                break;
            }
            case SCOPE_RESOLVER: {
                consume(SCOPE_RESOLVER);
                consume(SCOPE_RESOLVER);
                lexemes.push_back(make_lexeme(LexemeType::SCOPE_RESOLVER));
                break;
            }
            case STRING_QUOTE: {
                lex_string(lexemes);
                break;
            }
            case SINGLE_RULE_NAME_START: {
                lex_rule_id(lexemes);
                break;
            }
            default: {
                std::string unexpected = {peek()};
                throw Error(ErrorType::SYNTAX_ERROR, "Unexpected char '" + unexpected + "'");
            }
        }
    }

    lexemes.push_back(make_lexeme(LexemeType::END_OF_FILE));

    return lexemes;
} catch (Error& e) {
    e.add_ctx("Lexing process");
    e.set_loc({line_no, col_no, file_pos});
    throw;
} catch (std::exception& e) {
    throw Error(ErrorType::UNKNOWN_ERROR, e.what(), {line_no, col_no, file_pos});
}

bool Lexer::valid_identifier_char(char c) { return std::isalnum(c) || c == '_'; };

char Lexer::peek() const { return src.at(file_pos); }

char Lexer::consume() {
    col_no++;

    if (peek() == NEWLINE) {
        line_no++;
        line_starts.push_back(file_pos + 1);
        col_no = 0;
    }

    return src.at(file_pos++);
}

char Lexer::consume(char exp) {
    if (!at_end() && exp == peek()) return consume();

    std::string exp_str{exp};
    std::string actl_str;
    if (at_end()) {
        actl_str += "EOF";
    } else {
        actl_str += peek();
    }
    throw_pinpointed_err("Expected character '" + exp_str + "' but got '" + actl_str + "'");
}

bool Lexer::at_end() const { return file_pos == src.size(); }

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
        std::string cno_str = std::to_string(file_pos);
        throw std::invalid_argument("Error on line " + lno_str + " at char " + cno_str + ": " +
                                    msg);
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
    lexemes.push_back(make_lexeme(LexemeType::STRING, str_val));
}

void Lexer::lex_rule_id(std::vector<Lexeme>& lexemes) {
    size_t opener_idx = line_no;
    consume(SINGLE_RULE_NAME_START);
    std::string id;
    while (!at_end() && peek() != SINGLE_RULE_NAME_END) {
        if (!Lexer::valid_identifier_char(peek())) {
            std::string bad_char = {peek()};
            throw_pinpointed_err("Unexpected character '" + bad_char + "' found in rule");
        }
        id += consume();
    }
    if (at_end()) {
        throw_pinpointed_err("Unclosed string at" + std::to_string(opener_idx));
    }
    consume(SINGLE_RULE_NAME_END);
    lexemes.push_back(make_lexeme(LexemeType::SINGLE_RULE_IDENTIFIER, id));
}

void Lexer::lex_identifier(std::vector<Lexeme>& lexemes) {
    std::string id;
    while (valid_identifier_char(peek())) {
        id += consume();
    }
    lexemes.push_back(make_lexeme(LexemeType::IDENTIFIER, id));
}

Lexeme Lexer::make_lexeme(LexemeType type, std::string val) const {
    return Lexeme{type, val, line_no, col_no, file_pos};
}