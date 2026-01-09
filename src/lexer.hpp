#ifndef LEXER_H
#define LEXER_H

#include <array>
#include <filesystem>
#include <string>
#include <vector>

constexpr static std::string_view DEFAULT_BUILD_FILE_NAME = "Buildfile";

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
    SCOPE_RESOLVER,
    DELIMETER,
    COMMENT,
    NEWLINE,
    UNIDENTIFIED
};

struct LexEntry {
    char key;
    LexemeType type;
};

// Characters that can only be part of a one sized token
constexpr static std::array<LexEntry, 12> DIRECT_LONE_MAPPINGS = {
    {{BLOCK_START, LexemeType::BLOCK_START},
     {BLOCK_END, LexemeType::BLOCK_END},
     {LIST_START, LexemeType::LIST_START},
     {LIST_END, LexemeType::LIST_END},
     {MACRO_FN_START, LexemeType::MACRO_FN_START},
     {MACRO_FN_END, LexemeType::MACRO_FN_END},
     {SCOPE_RESOLVER, LexemeType::SCOPE_RESOLVER},
     {DELIMETER, LexemeType::DELIMETER},
     {EQUALS_CHAR, LexemeType::EQUALS},
     {ADD_CHAR, LexemeType::ADD},
     {NEWLINE, LexemeType::NEWLINE}}};

struct Lexeme {
    LexemeType type;
    std::string value;

    Lexeme(LexemeType type_, std::string value_) : type(type_), value(value_) {};

    Lexeme() : type(LexemeType::UNIDENTIFIED) {};
};

std::vector<Lexeme> lex_file(const std::string_view input_file = DEFAULT_BUILD_FILE_NAME);

/** Determine if a character can be a valid component of an identifier */
bool valid_identifier_char(char c);

#endif