#ifndef LEXER_H
#define LEXER_H

#include <array>
#include <string>
#include <vector>

// TO DO - Probably organise this in a class

constexpr static char BLOCK_START = '{';
constexpr static char BLOCK_END = '}';
constexpr static char LIST_START = '[';
constexpr static char LIST_END = ']';
constexpr static char MACRO_FN_START = '(';
constexpr static char MACRO_FN_END = ')';
constexpr static char STRING_QUOTE = '"';
constexpr static char SCOPE_RESOLVER = ':';
constexpr static char DELIMETER = ',';
constexpr static char EQUALS_CHAR = '=';
constexpr static char ADD_CHAR = '+';
constexpr static char COMMENT = '#';
constexpr static char NEWLINE = '\n';

enum class LexemeType {
    IDENTIFIER,
    ADD,
    EQUALS,
    LINE_START,
    BLOCK_START,
    BLOCK_END,
    LIST_START,
    LIST_END,
    MACRO_FN_START,
    MACRO_FN_END,
    STRING,
    DELIMETER,
    SCOPE_RESOLVER,
    COMMENT,
    NEWLINE,
    // The lexeme has not yet been categorised
    UNIDENTIFIED,
    END_OF_FILE,
};

struct LexEntry {
    char key;
    LexemeType type;
};

// Characters that can only be part of a one sized token
constexpr static std::array<char, 12> LONE_TOKS = {BLOCK_START, BLOCK_END,      LIST_START,
                                                   LIST_END,    MACRO_FN_START, MACRO_FN_END,
                                                   DELIMETER,   EQUALS_CHAR,    ADD_CHAR};

// Characters that can only be part of a one sized token and map directly to a one sized type
constexpr static std::array<LexEntry, 12> DIRECT_LONE_MAPPINGS = {
    {{BLOCK_START, LexemeType::BLOCK_START},
     {BLOCK_END, LexemeType::BLOCK_END},
     {LIST_START, LexemeType::LIST_START},
     {LIST_END, LexemeType::LIST_END},
     {MACRO_FN_START, LexemeType::MACRO_FN_START},
     {MACRO_FN_END, LexemeType::MACRO_FN_END},
     {DELIMETER, LexemeType::DELIMETER},
     {EQUALS_CHAR, LexemeType::EQUALS},
     {NEWLINE, LexemeType::NEWLINE},
     {ADD_CHAR, LexemeType::ADD}}};

struct Lexeme {
    LexemeType type;
    std::string value;
    size_t line_no;

    Lexeme(size_t line_no_, LexemeType type_, std::string value_ = "")
        : type(type_), value(value_), line_no(line_no_) {};

    Lexeme(size_t line_no_) : type(LexemeType::UNIDENTIFIED), line_no(line_no_) {};
};

/** Generate a vector of lexemes from a build file */
std::vector<Lexeme> lex_file(const std::string input_file = "Buildfile");

/** Parse a token that is known to be one character long */
void parse_lone(char c, std::vector<Lexeme>& lexeme_list, Lexeme& curr_lex, size_t& line_no);

/** Determine if a character can be a valid component of an identifier */
bool valid_identifier_char(char c);

#endif