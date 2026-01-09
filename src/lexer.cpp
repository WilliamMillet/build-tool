#include "lexer.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ranges>

std::vector<Lexeme> lex_file(const std::string& input_file) {
    std::filesystem::path bf_path(input_file);
    if (!std::filesystem::exists(bf_path)) {
        throw new std::invalid_argument("File '" + input_file + "' not found");
    }
    std::ifstream bf_file(bf_path);
    if (!bf_file.is_open()) {
        throw new std::system_error(errno, std::generic_category(),
                                    "Failed to open '" + input_file + "'.");
    }

    std::vector<Lexeme> lexeme_list = {{LexemeType::LINE_START, "0"}};
    Lexeme curr_lex;

    size_t line_no = 0;
    char c;
    while (bf_file.get(c)) {
        if (curr_lex.type == LexemeType::COMMENT) {
            while (c && bf_file.peek() != NEWLINE) {
                bf_file.get(c);
            }
        } else if (curr_lex.type == LexemeType::STRING && c != STRING_QUOTE) {
            // Any non STRING_QUOTE value can go inside a string
            curr_lex.value += c;
        } else if (valid_identifier_char(c)) {
            curr_lex.value += c;
            curr_lex.type = LexemeType::IDENTIFIER;
        } else if (std::ranges::any_of(DIRECT_LONE_MAPPINGS,
                                       [c](LexEntry e) { return e.key == c; })) {
            if (c == STRING_QUOTE) {
                if (curr_lex.type == LexemeType::STRING) {
                    lexeme_list.push_back(curr_lex);
                    curr_lex = Lexeme{};
                } else {
                    lexeme_list.push_back(curr_lex);
                    curr_lex = Lexeme{LexemeType::STRING, ""};
                }
            } else {
                // All non string lone tokens can be added as is
                lexeme_list.push_back(curr_lex);
                if (c == NEWLINE) {
                    line_no++;
                    lexeme_list.emplace_back(LexemeType::LINE_START, std::to_string(line_no));
                } else {
                    lexeme_list.emplace_back(LexemeType::BLOCK_START, std::string{BLOCK_START});
                }
            }
        }
    }

    return lexeme_list;
}

bool valid_identifier_char(char c) { return std::isalnum(c) || c == '_'; };