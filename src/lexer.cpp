#include "lexer.hpp"

#include <cctype>

#include "errors/error.hpp"
#include "file_utils.hpp"

Lexer::Lexer(const std::string input) { src = FileUtils::read_all(input); }

std::vector<Lexeme> Lexer::lex() try {
    std::vector<Lexeme> lexemes;
    loc = Location{};

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
                throw SyntaxError("Unexpected char '" + unexpected + "'");
            }
        }
    }

    lexemes.push_back(make_lexeme(LexemeType::END_OF_FILE));

    return lexemes;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Lexing", loc);
}

bool Lexer::valid_identifier_char(char c) { return std::isalnum(c) || c == '_'; };

char Lexer::peek() const { return src.at(loc.file_idx); }

char Lexer::consume() {
    loc.col_no++;

    if (peek() == NEWLINE) {
        loc.file_idx++;
        line_starts.push_back(loc.file_idx + 1);
        loc.col_no = 0;
    }

    return src.at(loc.file_idx++);
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
    throw SyntaxError("Expected character '" + exp_str + "' but got '" + actl_str + "'");
}

bool Lexer::at_end() const { return loc.file_idx == src.size(); }

void Lexer::consume_line() {
    while (peek() != NEWLINE) {
        consume();
    }
}

void Lexer::lex_string(std::vector<Lexeme>& lexemes) {
    Location opener_loc = loc;
    consume(STRING_QUOTE);
    std::string str_val;
    while (!at_end() && peek() != STRING_QUOTE) {
        str_val += consume();
    }
    if (at_end()) {
        throw SyntaxError("Unclosed string detected", opener_loc);
    }
    consume(STRING_QUOTE);
    lexemes.push_back(make_lexeme(LexemeType::STRING, str_val));
}

void Lexer::lex_rule_id(std::vector<Lexeme>& lexemes) {
    Location opener_loc = loc;
    consume(SINGLE_RULE_NAME_START);
    std::string id;
    while (!at_end() && peek() != SINGLE_RULE_NAME_END) {
        if (!Lexer::valid_identifier_char(peek())) {
            std::string bad_char = {peek()};
            throw SyntaxError("Unexpected character '" + bad_char + "' found in rule");
        }
        id += consume();
    }
    if (at_end()) {
        throw SyntaxError("Unclosed rule identifier found", opener_loc);
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

Lexeme Lexer::make_lexeme(LexemeType type, std::string val) const { return Lexeme{type, val, loc}; }