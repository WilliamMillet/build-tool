#include "lexer.hpp"

#include <cctype>

#include "errors/error.hpp"
#include "io/file_utils.hpp"

Lexer::Lexer(const std::string input) { src = FileUtils::read_all(input); }

std::vector<Lexeme> Lexer::lex() try {
    std::vector<Lexeme> lexemes;
    loc = Location{.line_no = 1, .col_no = 1, .file_idx = 0};

    while (!at_end()) {
        auto direct_itm = DIRECT_MAPPINGS.find(peek());
        if (direct_itm != DIRECT_MAPPINGS.end()) {
            Location lex_loc = loc;
            std::string lex_val{consume()};
            lexemes.push_back({direct_itm->second, lex_val, lex_loc});
            continue;
        }

        if (std::isspace(peek()) && peek() != '\n') {
            consume();
            continue;
        }

        if (valid_identifier_char(peek())) {
            lex_identifier(lexemes);
            continue;
        }

        switch (peek()) {
            case NEWLINE: {
                lexemes.push_back({LexemeType::NEWLINE, "\n", loc});
                consume();
                break;
            }
            case COMMENT: {
                consume_line();
                break;
            }
            case SCOPE_RESOLVER: {
                Location lex_loc = loc;
                consume(SCOPE_RESOLVER);
                consume(SCOPE_RESOLVER);
                lexemes.push_back({LexemeType::SCOPE_RESOLVER, "::", lex_loc});
                break;
            }
            case STRING_QUOTE: {
                lex_string(lexemes);
                break;
            }
            case SINGLE_RULE_NAME_START: {
                lex_rule_qualifier(lexemes);
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
        loc.line_no++;
        loc.col_no = 1;
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

void Lexer::lex_string(std::vector<Lexeme>& lexemes) try {
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
    lexemes.push_back({LexemeType::STRING, str_val, opener_loc});
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Lexing string", loc);
}

void Lexer::lex_rule_qualifier(std::vector<Lexeme>& lexemes) try {
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

    lexemes.push_back({LexemeType::DICT_QUALIFIER, id, opener_loc});
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Lexing rule qualifier ", loc);
}

void Lexer::lex_identifier(std::vector<Lexeme>& lexemes) try {
    Location start_loc = loc;

    std::string id;
    while (valid_identifier_char(peek())) {
        id += consume();
    }
    lexemes.push_back({LexemeType::IDENTIFIER, id, start_loc});
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Lexing identifier ", loc);
}

Lexeme Lexer::make_lexeme(LexemeType type, std::string val) const { return Lexeme{type, val, loc}; }