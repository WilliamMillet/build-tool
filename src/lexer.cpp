#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

std::vector<Lexeme> lex_file(const std::string input_file) {
    std::filesystem::path bf_path(input_file);
    if (!std::filesystem::exists(bf_path)) {
        throw std::invalid_argument("File '" + input_file + "' not found");
    }
    std::ifstream bf_file(bf_path);
    if (!bf_file.is_open()) {
        throw std::system_error(errno, std::generic_category(),
                                "Failed to open '" + input_file + "'.");
    }

    std::vector<Lexeme> lexeme_list;
    Lexeme curr_lex{0};

    size_t line_no = 0;
    char c;
    while (bf_file.get(c)) {
        if (std::isspace(c)) {
            continue;
        } else if (curr_lex.type == LexemeType::COMMENT) {
            while (c && bf_file.peek() != NEWLINE) {
                bf_file.get(c);
            }
        } else if (c == SCOPE_RESOLVER) {
            // Scope resolver has contain two column characters
            bf_file.get();
            if (c != SCOPE_RESOLVER) {
                throw new std::invalid_argument("Error on line " + std::to_string(line_no) +
                                                ": Unexpected ':' character");
            }
            lexeme_list.push_back(SCOPE_RESOLVER);
        } else if (curr_lex.type == LexemeType::STRING && c != STRING_QUOTE) {
            // Any non STRING_QUOTE value can go inside a string
            curr_lex.value += c;
        } else if (valid_identifier_char(c)) {
            curr_lex.value += c;
            curr_lex.type = LexemeType::IDENTIFIER;
        } else if (std::ranges::contains(LONE_TOKS, c)) {
            parse_lone(c, lexeme_list, curr_lex, line_no);
        }

        if (c == NEWLINE) line_no++;
    }

    lexeme_list.push_back(Lexeme{line_no, LexemeType::END_OF_FILE});

    return lexeme_list;
}

void parse_lone(char c, std::vector<Lexeme>& lexeme_list, Lexeme& curr_lex, size_t& line_no) {
    if (c == STRING_QUOTE) {
        if (curr_lex.type == LexemeType::STRING) {
            lexeme_list.push_back(curr_lex);
            curr_lex = Lexeme{line_no};
        } else {
            lexeme_list.push_back(curr_lex);
            curr_lex = Lexeme{line_no, LexemeType::STRING};
        }
    } else {
        // All non string lone tokens can be added as is
        lexeme_list.push_back(curr_lex);
        lexeme_list.emplace_back(line_no, LexemeType::BLOCK_START, std::string{BLOCK_START});
    }
}

bool valid_identifier_char(char c) { return std::isalnum(c) || c == '_'; };